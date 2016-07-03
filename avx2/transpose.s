.code64
.global transposew16x16
.global transposew16x16_64

# Transposes a 16x16 matrix, where the input is at a given memory location,
# given in the rsi register. The output is given in the ymm0-15
# registers, where each register indicates a row.
# The input has 32 byte gaps between each row.
transposew16x16:
    # Transpose the two square matrices of the upper half of the input
    vmovdqa       ymm0, [rsi + 32*0]
    vmovdqa       ymm1, [rsi + 32*2]
    vmovdqa       ymm2, [rsi + 32*4]
    vmovdqa       ymm3, [rsi + 32*6]
    vmovdqa       ymm4, [rsi + 32*8]
    vmovdqa       ymm5, [rsi + 32*10]
    vmovdqa       ymm6, [rsi + 32*12]
    vmovdqa       ymm7, [rsi + 32*14]

    vpunpcklwd    ymm8, ymm0, ymm1
    vpunpckhwd    ymm9, ymm0, ymm1
    vpunpcklwd    ymm10, ymm2, ymm3
    vpunpckhwd    ymm11, ymm2, ymm3
    vpunpcklwd    ymm12, ymm4, ymm5
    vpunpckhwd    ymm13, ymm4, ymm5
    vpunpcklwd    ymm14, ymm6, ymm7
    vpunpckhwd    ymm15, ymm6, ymm7

    vpunpckldq    ymm0, ymm8, ymm10
    vpunpckhdq    ymm1, ymm8, ymm10
    vpunpckldq    ymm2, ymm9, ymm11
    vpunpckhdq    ymm3, ymm9, ymm11
    vpunpckldq    ymm4, ymm12, ymm14
    vpunpckhdq    ymm5, ymm12, ymm14
    vpunpckldq    ymm6, ymm13, ymm15
    vpunpckhdq    ymm7, ymm13, ymm15

    vpunpcklqdq   ymm8, ymm0, ymm4
    vpunpckhqdq   ymm9, ymm0, ymm4
    vpunpcklqdq   ymm10, ymm1, ymm5
    vpunpckhqdq   ymm11, ymm1, ymm5
    vpunpcklqdq   ymm12, ymm2, ymm6
    vpunpckhqdq   ymm13, ymm2, ymm6
    vpunpcklqdq   ymm14, ymm3, ymm7
    vpunpckhqdq   ymm15, ymm3, ymm7

    vmovdqa       buffer1, ymm8
    vmovdqa       buffer2, ymm10
    vmovdqa       buffer3, ymm12
    vmovdqa       buffer4, ymm14

    # Note that ymm8-15 contain the transposed part.
    # Do the same with the lower half, but be a bit more careful about
    # register use to avoid having to swap out to memory all results.
    vmovdqa       ymm0, [rsi + 32*16]
    vmovdqa       ymm1, [rsi + 32*18]
    vmovdqa       ymm2, [rsi + 32*20]
    vmovdqa       ymm3, [rsi + 32*22]
    vmovdqa       ymm4, [rsi + 32*24]
    vmovdqa       ymm5, [rsi + 32*26]
    vmovdqa       ymm6, [rsi + 32*28]
    vmovdqa       ymm7, [rsi + 32*30]

    vpunpcklwd    ymm8, ymm0, ymm1
    vpunpckhwd    ymm1, ymm0, ymm1
    vpunpcklwd    ymm10, ymm2, ymm3
    vpunpckhwd    ymm3, ymm2, ymm3
    vpunpcklwd    ymm12, ymm4, ymm5
    vpunpckhwd    ymm5, ymm4, ymm5
    vpunpcklwd    ymm14, ymm6, ymm7
    vpunpckhwd    ymm7, ymm6, ymm7

    vpunpckldq    ymm0, ymm8, ymm10
    vpunpckldq    ymm2, ymm1, ymm3
    vpunpckhdq    ymm3, ymm1, ymm3
    vpunpckhdq    ymm1, ymm8, ymm10
    vpunpckldq    ymm4, ymm12, ymm14
    vpunpckldq    ymm6, ymm5, ymm7
    vpunpckhdq    ymm7, ymm5, ymm7
    vpunpckhdq    ymm5, ymm12, ymm14

    vpunpckhqdq   ymm8, ymm0, ymm4
    vpunpcklqdq   ymm0, ymm0, ymm4
    vpunpckhqdq   ymm12, ymm2, ymm6
    vpunpcklqdq   ymm4, ymm2, ymm6
    vpunpckhqdq   ymm10, ymm1, ymm5
    vpunpcklqdq   ymm2, ymm1, ymm5
    vpunpckhqdq   ymm14, ymm3, ymm7
    vpunpcklqdq   ymm6, ymm3, ymm7

    # Now we have the 1st, 3rd, 5th and 7th rows (counting from 0) stored
    # in ymm{9, 11, 13, 15} respectively, and the rows 8-15 in
    # ymm{0, 8, 2, 10, 4, 12, 6, 14} respectively.
    # Now perform the final step, where we fix the fact we have two blocks
    # transposed separately.
    vperm2i128    ymm1, ymm8, ymm9, 0x02
    vmovdqa       buffer5, ymm1
    vperm2i128    ymm9, ymm8, ymm9, 0x13
    vperm2i128    ymm3, ymm10, ymm11, 0x02
    vperm2i128    ymm11, ymm10, ymm11, 0x13
    vperm2i128    ymm5, ymm12, ymm13, 0x02
    vperm2i128    ymm13, ymm12, ymm13, 0x13
    vperm2i128    ymm7, ymm14, ymm15, 0x02
    vperm2i128    ymm15, ymm14, ymm15, 0x13

    vmovdqa       ymm10, buffer1
    vperm2i128    ymm8, ymm0, ymm10, 0x13
    vperm2i128    ymm0, ymm0, ymm10, 0x02
    vmovdqa       ymm12, buffer2
    vperm2i128    ymm10, ymm2, ymm12, 0x13
    vperm2i128    ymm2, ymm2, ymm12, 0x02
    vmovdqa       ymm14, buffer3
    vperm2i128    ymm12, ymm4, ymm14, 0x13
    vperm2i128    ymm4, ymm4, ymm14, 0x02
    vmovdqa       ymm1, buffer4
    vperm2i128    ymm14, ymm6, ymm1, 0x13
    vperm2i128    ymm6, ymm6, ymm1, 0x02
    vmovdqa       ymm1, buffer5
    ret



# Transposes a 16x16 matrix, where the input is at a given memory location,
# given in the rsi register. The output is given in the ymm0-15
# registers, where each register indicates a row.
# The input has 32*3 byte gaps between each row (used for inverse transform)
transposew16x16_64:
    # Transpose the two square matrices of the upper half of the input
    vmovdqa       ymm0, [rsi + 64*0]
    vmovdqa       ymm1, [rsi + 64*2]
    vmovdqa       ymm2, [rsi + 64*4]
    vmovdqa       ymm3, [rsi + 64*6]
    vmovdqa       ymm4, [rsi + 64*8]
    vmovdqa       ymm5, [rsi + 64*10]
    vmovdqa       ymm6, [rsi + 64*12]
    vmovdqa       ymm7, [rsi + 64*14]

    vpunpcklwd    ymm8, ymm0, ymm1
    vpunpckhwd    ymm9, ymm0, ymm1
    vpunpcklwd    ymm10, ymm2, ymm3
    vpunpckhwd    ymm11, ymm2, ymm3
    vpunpcklwd    ymm12, ymm4, ymm5
    vpunpckhwd    ymm13, ymm4, ymm5
    vpunpcklwd    ymm14, ymm6, ymm7
    vpunpckhwd    ymm15, ymm6, ymm7

    vpunpckldq    ymm0, ymm8, ymm10
    vpunpckhdq    ymm1, ymm8, ymm10
    vpunpckldq    ymm2, ymm9, ymm11
    vpunpckhdq    ymm3, ymm9, ymm11
    vpunpckldq    ymm4, ymm12, ymm14
    vpunpckhdq    ymm5, ymm12, ymm14
    vpunpckldq    ymm6, ymm13, ymm15
    vpunpckhdq    ymm7, ymm13, ymm15

    vpunpcklqdq   ymm8, ymm0, ymm4
    vpunpckhqdq   ymm9, ymm0, ymm4
    vpunpcklqdq   ymm10, ymm1, ymm5
    vpunpckhqdq   ymm11, ymm1, ymm5
    vpunpcklqdq   ymm12, ymm2, ymm6
    vpunpckhqdq   ymm13, ymm2, ymm6
    vpunpcklqdq   ymm14, ymm3, ymm7
    vpunpckhqdq   ymm15, ymm3, ymm7

    vmovdqa       buffer1, ymm8
    vmovdqa       buffer2, ymm10
    vmovdqa       buffer3, ymm12
    vmovdqa       buffer4, ymm14

    # Note that ymm8-15 contain the transposed part.
    # Do the same with the lower half, but be a bit more careful about
    # register use to avoid having to swap out to memory all results.
    vmovdqa       ymm0, [rsi + 64*16]
    vmovdqa       ymm1, [rsi + 64*18]
    vmovdqa       ymm2, [rsi + 64*20]
    vmovdqa       ymm3, [rsi + 64*22]
    vmovdqa       ymm4, [rsi + 64*24]
    vmovdqa       ymm5, [rsi + 64*26]
    vmovdqa       ymm6, [rsi + 64*28]
    vmovdqa       ymm7, [rsi + 64*30]

    vpunpcklwd    ymm8, ymm0, ymm1
    vpunpckhwd    ymm1, ymm0, ymm1
    vpunpcklwd    ymm10, ymm2, ymm3
    vpunpckhwd    ymm3, ymm2, ymm3
    vpunpcklwd    ymm12, ymm4, ymm5
    vpunpckhwd    ymm5, ymm4, ymm5
    vpunpcklwd    ymm14, ymm6, ymm7
    vpunpckhwd    ymm7, ymm6, ymm7

    vpunpckldq    ymm0, ymm8, ymm10
    vpunpckldq    ymm2, ymm1, ymm3
    vpunpckhdq    ymm3, ymm1, ymm3
    vpunpckhdq    ymm1, ymm8, ymm10
    vpunpckldq    ymm4, ymm12, ymm14
    vpunpckldq    ymm6, ymm5, ymm7
    vpunpckhdq    ymm7, ymm5, ymm7
    vpunpckhdq    ymm5, ymm12, ymm14

    vpunpckhqdq   ymm8, ymm0, ymm4
    vpunpcklqdq   ymm0, ymm0, ymm4
    vpunpckhqdq   ymm12, ymm2, ymm6
    vpunpcklqdq   ymm4, ymm2, ymm6
    vpunpckhqdq   ymm10, ymm1, ymm5
    vpunpcklqdq   ymm2, ymm1, ymm5
    vpunpckhqdq   ymm14, ymm3, ymm7
    vpunpcklqdq   ymm6, ymm3, ymm7

    # Now we have the 1st, 3rd, 5th and 7th rows (counting from 0) stored
    # in ymm{9, 11, 13, 15} respectively, and the rows 8-15 in
    # ymm{0, 8, 2, 10, 4, 12, 6, 14} respectively.
    # Now perform the final step, where we fix the fact we have two blocks
    # transposed separately.
    vperm2i128    ymm1, ymm8, ymm9, 0x02
    vmovdqa       buffer5, ymm1
    vperm2i128    ymm9, ymm8, ymm9, 0x13
    vperm2i128    ymm3, ymm10, ymm11, 0x02
    vperm2i128    ymm11, ymm10, ymm11, 0x13
    vperm2i128    ymm5, ymm12, ymm13, 0x02
    vperm2i128    ymm13, ymm12, ymm13, 0x13
    vperm2i128    ymm7, ymm14, ymm15, 0x02
    vperm2i128    ymm15, ymm14, ymm15, 0x13

    vmovdqa       ymm10, buffer1
    vperm2i128    ymm8, ymm0, ymm10, 0x13
    vperm2i128    ymm0, ymm0, ymm10, 0x02
    vmovdqa       ymm12, buffer2
    vperm2i128    ymm10, ymm2, ymm12, 0x13
    vperm2i128    ymm2, ymm2, ymm12, 0x02
    vmovdqa       ymm14, buffer3
    vperm2i128    ymm12, ymm4, ymm14, 0x13
    vperm2i128    ymm4, ymm4, ymm14, 0x02
    vmovdqa       ymm1, buffer4
    vperm2i128    ymm14, ymm6, ymm1, 0x13
    vperm2i128    ymm6, ymm6, ymm1, 0x02
    vmovdqa       ymm1, buffer5
    ret


.section .bss
.align 32
buffer1: .space 32
buffer2: .space 32
buffer3: .space 32
buffer4: .space 32
buffer5: .space 32
