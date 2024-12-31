.CODE


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


END