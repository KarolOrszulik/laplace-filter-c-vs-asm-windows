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