PUBLIC laplace

INCLUDELIB kernel32.lib

extern CreateThread: PROC
extern WaitForMultipleObjects: PROC
extern CloseHandle: PROC

; Constants
MAX_THREADS EQU 64
NUM_CHANNELS EQU 3
LANE_WIDTH EQU 8

.DATA
    ; procedure arguments
	img_width  dd 0
	img_height dd 0
	src_array  dq 0
	dst_array  dq 0
	num_threads dd 0
	amplification dd 0

    ; thread handles
    thread_handles dq MAX_THREADS dup(0)
.CODE

laplace PROC
    ; Prologue
    push rbp
    mov rbp, rsp

    ; Save arguments to global variables
    mov dword ptr [img_width], ecx
    mov dword ptr [img_height], edx
    mov qword ptr [src_array], r8
    mov qword ptr [dst_array], r9
    mov eax, dword ptr [rsp + 30h]
    mov dword ptr [num_threads], eax
    mov eax, dword ptr [rsp + 38h]
    mov dword ptr [amplification], eax

    ; Save registers
    push rbx
    push rsi

    ; if num_threads > MAX_THREADS, set num_threads to MAX_THREADS
    cmp qword ptr [num_threads], MAX_THREADS
    jle num_threads_ok
    mov qword ptr [num_threads], MAX_THREADS
num_threads_ok: 

    ; Initialize rbx to 0 for the thread loop
    mov rbx, 0
thread_loop_start:
    ; Loop condition: compare rbx (thread index) with num_threads
    cmp rbx, qword ptr [num_threads]
    jge thread_loop_end ; Exit loop if rbx >= num_threads


    ; Create a new thread
    push 0 ; lpThreadId
    push 0 ; dwCreationFlags - 0 = start thread immediately
    sub rsp, 20h ; Reserve space for arguments
    mov r9, rbx ; lpParameter - Pass thread index as argument
    mov r8, offset thread_func ; lpStartAddress - Address of thread function
    mov rdx, 0 ; dwStackSize - Default stack size
    mov rcx, 0 ; lpThreadAttributes - Default thread attributes

    ; Create the thread
    call CreateThread

    ; Save the thread handle
    lea rsi, thread_handles
    mov qword ptr [rsi + rbx * 8], rax

    ; Increment thread index
    inc rbx
    jmp thread_loop_start ; Repeat the loop
thread_loop_end:

    ; Wait for all threads to finish
    mov rcx, qword ptr [num_threads]
    lea rdx, thread_handles
    mov r8, 1  ; bWaitAll = TRUE
    mov r9, -1 ; dwMilliseconds = INFINITE
    call WaitForMultipleObjects

    mov rcx, 0
close_handles_loop:
    ; Loop condition: compare rcx with num_threads
    cmp rcx, qword ptr [num_threads]
    jge close_handles_end ; Exit loop if rcx >= num_threads

    ; Close the thread handle
    lea rsi, thread_handles
    mov rcx, qword ptr [rsi + rcx * 8]
    call CloseHandle

    ; Increment thread index
    inc rcx
    jmp close_handles_loop ; Repeat the loop
close_handles_end:

    ; Restore registers
    pop rsi
    pop rbx

    ; Epilogue
    mov rsp, rbp
    pop rbp

    ret
laplace ENDP




.DATA
    y_loop_term             dq 0 ; Termination condition for y loop
    row_loop_term           dq 0 ; Termination condition for row loop
    first_subpixel_proc_idx dq 0 ; Index of the first subpixel to process per-subpixel instead of per-lane

.CODE

thread_func PROC
    ; Procedure prolog
    push rbp
    mov rbp, rsp

    ; Save registers
    push r8      ; y iterator
    push r9      ; row iterator
    push r11     ; array index (3D to 1D)
    push r12     ; shifted index (index of neighbor)
    push r13     ; laplace sum
    push rsi     ; source array
    push rdi     ; target array

    ; Set termination condition for y loop
    mov rax, qword ptr [img_height]
    dec rax
    mov qword ptr [y_loop_term], rax ; height - 1

    ; Set termination condition for row loop
    mov rax, qword ptr [img_width]
    dec rax
    imul rax, NUM_CHANNELS
    mov qword ptr [row_loop_term], rax ; (width - 1) * NUM_CHANNELS

    ; Calculate first per-subpixel processing index
    mov rax, qword ptr [img_width]
    imul rax, NUM_CHANNELS
    mov rsi, LANE_WIDTH
    imul rsi, 4
    sub rax, rsi
    sub rax, NUM_CHANNELS
    inc rax
    mov qword ptr [first_subpixel_proc_idx], rax

    ; Save source and target array pointers
    mov rsi, qword ptr [src_array]
    mov rdi, qword ptr [dst_array]

    ; Zero out ymm7 (for zero-extension during unpacking bytes to words)
    vpxor ymm7, ymm7, ymm7

    ; Initialize y iterator (r8) to thread ID (rcx), skipping the 0-th row
    mov r8, rcx
    inc r8
y_loop_start:
    ; y loop condition
    cmp r8, qword ptr [y_loop_term]
    jge y_loop_end

    ; y loop body
        ; Initialize x iterator (r9) to skip the first pixel
        mov r9, NUM_CHANNELS
    row_loop_start:
        ; Row loop condition
        cmp r9, qword ptr [row_loop_term]
        jge row_loop_end

        ; Calculate array index
        mov r11, r8
        imul r11, qword ptr [img_width]
        imul r11, NUM_CHANNELS
        add r11, r9

        ; Determine whether to process a subpixel or a lane
        cmp r9, qword ptr [first_subpixel_proc_idx]
        jge only_process_subpixel

        call process_lane
        add r9, LANE_WIDTH ; Advance by LANE_WIDTH subpixels
        jmp row_loop_start

    only_process_subpixel:
        call process_subpixel
        add r9, 1 ; Advance by 1 subpixel
        jmp row_loop_start

    row_loop_end:
    ; Advance y iterator for N threads (process every N-th row)
    add r8, qword ptr [num_threads]
    jmp y_loop_start

y_loop_end:
    ; Restore registers
    pop rdi
    pop rsi
    pop r13
    pop r12
    pop r11
    pop r9
    pop r8

    ; Procedure epilog
    mov rsp, rbp
    pop rbp

    ret
thread_func ENDP


; Sets the index for the center element
set_center_element_idx MACRO
    mov r12, r11
ENDM

; Sets the index for the left neighbor
set_left_neighbour_idx MACRO
    mov r12, r11
    sub r12, NUM_CHANNELS
ENDM

; Sets the index for the right neighbor
set_right_neighbour_idx MACRO
    mov r12, r11
    add r12, NUM_CHANNELS
ENDM

; Sets the index for the top neighbor
set_top_neighbour_idx MACRO
    mov r12, r11
    mov rax, qword ptr [img_width]
    imul rax, NUM_CHANNELS
    sub r12, rax
ENDM

; Sets the index for the bottom neighbor
set_bottom_neighbour_idx MACRO
    mov r12, r11
    mov rax, qword ptr [img_width]
    imul rax, NUM_CHANNELS
    add r12, rax
ENDM


; Adds zero-extended neighbor to r13
; rsi - source array
; r12 - array index of neighbor
load_add_neighbour MACRO
    movzx rax, byte ptr [rsi + r12]
    add r13, rax
ENDM

; Clamps rax to the range 0-255
clamp_rax MACRO
    LOCAL check_upper_bound, clamp_end
    cmp rax, 0
    jge check_upper_bound
    mov rax, 0
    jmp clamp_end
check_upper_bound:
    cmp rax, 255
    jle clamp_end
    mov rax, 255
clamp_end:
ENDM


process_subpixel PROC
    ; r13 - laplace sum

    ; Zero out laplace sum
    mov r13, 0

    ; Center element
    set_center_element_idx
    load_add_neighbour
    imul r13, -4

    ; Left neighbor
    set_left_neighbour_idx
    load_add_neighbour

    ; Right neighbor
    set_right_neighbour_idx
    load_add_neighbour

    ; Top neighbor
    set_top_neighbour_idx
    load_add_neighbour

    ; Bottom neighbor
    set_bottom_neighbour_idx
    load_add_neighbour

    ; Apply amplification
    imul r13, qword ptr [amplification]

    ; Clamp to 0-255
    mov rax, r13
    clamp_rax

    ; Write back to target array
    mov byte ptr [rdi + r11], al
    ret
process_subpixel ENDP


.DATA
    negative_four WORD -4

.CODE

; Load `[rsi + r12]` into ymm1 and unpack its bytes to words
; (Assumes ymm7 is zeroed out for zero-extension)
load_neighbour_simd MACRO
    vmovdqu ymm1, ymmword ptr [rsi + r12]
    vpunpcklbw ymm1, ymm1, ymm7 ; Unpack low bytes to words, zero-extend using ymm7 (set to 0)
ENDM

; Perform word-wise addition of ymm1 to ymm0
add_neighbour_simd MACRO
    vpaddw ymm0, ymm0, ymm1
ENDM


process_lane PROC
    ; ymm0 - laplace sum
    ; ymm1 - currently processed neighbor
    ; ymm2 - multiplication factor for center element
    ; r11  - array index of start of lane
    ; rdi  - target array

    ; Zero out laplace sum (ymm0)
    vpxor ymm0, ymm0, ymm0

    ; Left neighbor
    set_left_neighbour_idx
    load_neighbour_simd
    add_neighbour_simd

    ; Right neighbor
    set_right_neighbour_idx
    load_neighbour_simd
    add_neighbour_simd

    ; Top neighbor
    set_top_neighbour_idx
    load_neighbour_simd
    add_neighbour_simd

    ; Bottom neighbor
    set_bottom_neighbour_idx
    load_neighbour_simd
    add_neighbour_simd

    ; Center element (multiplied by -4)
    set_center_element_idx
    load_neighbour_simd
    ; Broadcast -4 to all elements of ymm2
    vpbroadcastw ymm2, word ptr [negative_four]
    ; Multiply center element by -4
    vpmullw ymm1, ymm1, ymm2
    add_neighbour_simd

    ; Apply amplification
    vpbroadcastw ymm1, word ptr [amplification]
    vpmullw ymm0, ymm0, ymm1

    ; Pack words back to bytes
    vpackuswb ymm0, ymm0, ymm0

    ; Write back to target array
    vmovdqu ymmword ptr [rdi + r11], ymm0

    ret
process_lane ENDP



END