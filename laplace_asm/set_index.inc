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