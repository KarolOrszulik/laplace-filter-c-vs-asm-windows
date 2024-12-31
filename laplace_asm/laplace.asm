PUBLIC laplace


.data
    NUM_CHANNELS dq 3
    LANE_WIDTH dq 8

	img_width  dq 0
	img_height dq 0

	src_array dq 0
	dst_array dq 0

	num_threads dq 0

	amplification dq 0


.code

laplace PROC
    ; Prologue
    push rbp
    mov rbp, rsp

    ; Save arguments to global variables
    mov dword ptr img_width, ecx
    mov dword ptr img_height, edx
    mov qword ptr src_array, r8
    mov qword ptr dst_array, r9
    mov eax, dword ptr [rsp + 30h]
    mov qword ptr num_threads, rax
    mov eax, dword ptr [rsp + 38h]
    mov qword ptr amplification, rax

    push rcx ; Save registers

    ; Initialize rcx to 0 for the thread loop
    xor rcx, rcx
thread_loop_start:
    ; Loop condition: compare rcx (thread index) with num_threads
    cmp rcx, qword ptr [num_threads]
    jge thread_loop_end ; Exit loop if rcx >= num_threads

    ; Call thread_func, passing thread ID (rcx) as argument in rdx
    mov rdx, rcx
    call thread_func

    ; Increment thread index
    inc rcx
    jmp thread_loop_start ; Repeat the loop
thread_loop_end:

    pop rcx ; Restore register

    ; Epilogue
    mov rsp, rbp
    pop rbp

    ret
laplace ENDP




.DATA
y_loop_term                 dq 0 ; Termination condition for y loop
xc_loop_term                dq 0 ; Termination condition for row loop
first_subpixel_proc_idx     dq 0 ; Indices per row

.CODE

thread_func PROC
    ; Procedure prolog
    push rbp
    mov rbp, rsp

    ; Save registers
    push r8      ; r8 = y iterator
    push r9      ; r9 = x iterator
    push r11     ; r11 = array index (3D to 1D)
    push r12     ; r12 = shifted index (index of neighbor)
    push r13     ; r13 = laplace sum
    push rsi     ; rsi = source array
    push rdi     ; rdi = target array

    ; Set termination condition for y loop
    mov rax, qword ptr [img_height]
    dec rax
    mov qword ptr [y_loop_term], rax ; height - 1

    ; Set termination condition for row loop
    mov rax, qword ptr [img_width]
    dec rax
    imul rax, NUM_CHANNELS
    mov qword ptr [xc_loop_term], rax ; (width - 1) * NUM_CHANNELS

    ; Calculate first per-subpixel processing index
    mov rax, qword ptr [img_width]
    imul rax, NUM_CHANNELS
    push rbx
    mov rbx, LANE_WIDTH
    imul rbx, 4
    sub rax, rbx
    pop rbx
    sub rax, NUM_CHANNELS
    inc rax
    mov qword ptr [first_subpixel_proc_idx], rax

    ; Save source and target array pointers
    mov rsi, qword ptr [src_array]
    mov rdi, qword ptr [dst_array]

    ; Zero out ymm7 (for zero-extension during unpacking bytes to words)
    vpxor ymm7, ymm7, ymm7

    ; Initialize y iterator (r8) to thread ID (rdx), skipping the 0-th row
    mov r8, rdx
    inc r8
y_loop_start:
    ; Y loop condition
    cmp r8, qword ptr [y_loop_term]
    jge y_loop_end

    ; Y loop body
        ; Initialize x iterator (r9) to skip the first pixel
        mov r9, NUM_CHANNELS
    row_loop_start:
        ; Row loop condition
        cmp r9, qword ptr [xc_loop_term]
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
        jmp after_processing

    only_process_subpixel:
        call process_subpixel
        add r9, 1 ; Advance by 1 subpixel

    after_processing:
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