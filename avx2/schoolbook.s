.code64
.global schoolbook4
.global schoolbook4_test
.global reduce_mask

# Perform the reduction for the low part of a multiplication.
# This requires a temp_reg, which is overwritten with another value, and
# a mask reg which represents the bits present in the ring.
.macro reduce_low reg, temp_reg, mask_reg
    vpsrlw        \temp_reg, \reg, 11
    vpand         \reg, \reg, \mask_reg
    vpaddw        \reg, \reg, \temp_reg
.endm

# Perform the reduction for a reduced low variable, with the high half of
# the multiplication.
.macro reduce_high low_reg, high_reg, temp_reg, mask_reg
    vpsrlw        \temp_reg, \high_reg, 6
    vpsllw        \high_reg, \high_reg, 5
    vpand         \high_reg, \high_reg, \mask_reg
    vpaddw        \high_reg, \high_reg, \temp_reg
    vpaddw        \low_reg, \low_reg, \high_reg
.endm


# Multiplies sixteen polynomial pairs with 4 coefficients each according to the
# schoolbook method. The input must be at most 15 bits, and must be non-negative.
# Parameters:
#   ymm8-11:  Registers with 8 words, containing the set of four coefficients of
#             the first polynomial.
#   ymm12-15: Same as above, but for the second polynomials.
# Returns:
#   ymm0-ymm6: Set of 8 words in each register, indicating the coefficients of
#              the result. They need not be in reduced form: the maximum number
#              of bits set are 13 for ymm0/ymm6, 14 for all others
schoolbook4:
    vmovdqa       ymm7, [reduce_mask]

    # c1 = a1*b1
    vpmullw       ymm0, ymm8, ymm12           # ymm0 = low(a1*b1)
    reduce_low    ymm0, ymm6, ymm7

    vpmulhuw      ymm1, ymm8, ymm12           # ymm1 = high(a1*b1)
    reduce_high   ymm0, ymm1, ymm6, ymm7

    # c2 = a1*b2 + a2*b1
    vpmullw       ymm1, ymm8, ymm13           # ymm1 = low(a1*b2)
    reduce_low    ymm1, ymm6, ymm7
    vpmullw       ymm2, ymm9, ymm12           # ymm2 = low(a2*b1)
    reduce_low    ymm2, ymm6, ymm7
    vpaddw        ymm1, ymm1, ymm2

    vpmulhuw      ymm2, ymm8, ymm13           # ymm2 = high(a1*b2)
    vpmulhuw      ymm3, ymm9, ymm12           # ymm3 = high(a2*b1)
    vpaddw        ymm2, ymm2, ymm3
    reduce_high   ymm1, ymm2, ymm6, ymm7

    # c3 = a1*b3 + a2*b2 + a3*b1
    vpmullw       ymm2, ymm8, ymm14           # ymm2 = low(a1*b3)
    reduce_low    ymm2, ymm6, ymm7
    vpmullw       ymm3, ymm9, ymm13           # ymm3 = low(a2*b2)
    reduce_low    ymm3, ymm6, ymm7
    vpaddw        ymm2, ymm2, ymm3
    vpmullw       ymm3, ymm10, ymm12          # ymm3 = low(a3*b1)
    reduce_low    ymm3, ymm6, ymm7
    vpaddw        ymm2, ymm2, ymm3

    vpmulhuw      ymm3, ymm8, ymm14           # ymm3 = high(a1*b3)
    vpmulhuw      ymm4, ymm9, ymm13           # ymm4 = high(a2*b2)
    vpaddw        ymm3, ymm3, ymm4
    vpmulhuw      ymm4, ymm10, ymm12          # ymm4 = high(a3*b1)
    vpaddw        ymm3, ymm3, ymm4
    reduce_high   ymm2, ymm3, ymm6, ymm7

    # c4 = a1*b4 + a2*b3 + a3*b2 + a4*b1
    vpmullw       ymm3, ymm8, ymm15           # ymm3 = low(a1*b4)
    reduce_low    ymm3, ymm6, ymm7
    vpmullw       ymm4, ymm9, ymm14           # ymm4 = low(a2*b3)
    reduce_low    ymm4, ymm6, ymm7
    vpaddw        ymm3, ymm3, ymm4
    vpmullw       ymm4, ymm10, ymm13          # ymm4 = low(a3*b2)
    reduce_low    ymm4, ymm6, ymm7
    vpaddw        ymm3, ymm3, ymm4
    vpmullw       ymm4, ymm11, ymm12          # ymm4 = low(a4*b1)
    reduce_low    ymm4, ymm6, ymm7
    vpaddw        ymm3, ymm3, ymm4

    vpmulhuw      ymm4, ymm8, ymm15           # ymm4 = high(a1*b4)
    vpmulhuw      ymm5, ymm9, ymm14           # ymm5 = high(a2*b3)
    vpaddw        ymm4, ymm4, ymm5
    vpmulhuw      ymm5, ymm10, ymm13          # ymm5 = high(a3*b2)
    vpaddw        ymm4, ymm4, ymm5
    vpmulhuw      ymm5, ymm11, ymm12          # ymm5 = high(a4*b1)
    vpaddw        ymm4, ymm4, ymm5
    reduce_high   ymm3, ymm4, ymm6, ymm7

    # c5 = a2*b4 + a3*b3 + a4*b2
    vpmullw       ymm4, ymm9, ymm15           # ymm4 = low(a2*b4)
    reduce_low    ymm4, ymm6, ymm7
    vpmullw       ymm5, ymm10, ymm14          # ymm5 = low(a3*b3)
    reduce_low    ymm5, ymm6, ymm7
    vpaddw        ymm4, ymm4, ymm5
    vpmullw       ymm5, ymm11, ymm13          # ymm5 = low(a4*b2)
    reduce_low    ymm5, ymm6, ymm7
    vpaddw        ymm4, ymm4, ymm5

    vpmulhuw      ymm5, ymm9, ymm15           # ymm5 = high(a2*b4)
    vpmulhuw      ymm6, ymm10, ymm14          # ymm6 = high(a3*b3)
    vpaddw        ymm5, ymm5, ymm6
    vpmulhuw      ymm6, ymm11, ymm13          # ymm6 = high(a4*b2)
    vpaddw        ymm5, ymm5, ymm6
    reduce_high   ymm4, ymm5, ymm6, ymm7

    # c6 = a3*b4 + a4*b3
    vpmullw       ymm5, ymm10, ymm15          # ymm5 = low(a3*b4)
    reduce_low    ymm5, ymm8, ymm7
    vpmullw       ymm6, ymm11, ymm14          # ymm6 = low(a4*b3)
    reduce_low    ymm6, ymm8, ymm7
    vpaddw        ymm5, ymm5, ymm6

    vpmulhuw      ymm6, ymm10, ymm15          # ymm6 = high(a3*b4)
    vpmulhuw      ymm8, ymm11, ymm14          # ymm8 = high(a4*b3)
    vpaddw        ymm6, ymm6, ymm8
    reduce_high   ymm5, ymm6, ymm8, ymm7

    # c7 = a4*b4
    vpmullw       ymm6, ymm11, ymm15          # ymm6 = low(a4*b4)
    reduce_low    ymm6, ymm10, ymm7

    vpmulhuw      ymm8, ymm11, ymm15          # ymm8 = high(a4*b4)
    reduce_high   ymm6, ymm8, ymm9, ymm7

    ret


# Test the schoolbook method by reading/writing the polynomials from the first argument.
schoolbook4_test:
    vmovdqa       ymm8, [rdi]
    vmovdqa       ymm9, [rdi + 2*16*1]
    vmovdqa       ymm10, [rdi + 2*16*2]
    vmovdqa       ymm11, [rdi + 2*16*3]
    vmovdqa       ymm12, [rdi + 2*16*4]
    vmovdqa       ymm13, [rdi + 2*16*5]
    vmovdqa       ymm14, [rdi + 2*16*6]
    vmovdqa       ymm15, [rdi + 2*16*7]

    call          schoolbook4

    vmovdqa       [edi], ymm0
    vmovdqa       [edi + 2*16*1], ymm1
    vmovdqa       [edi + 2*16*2], ymm2
    vmovdqa       [edi + 2*16*3], ymm3
    vmovdqa       [edi + 2*16*4], ymm4
    vmovdqa       [edi + 2*16*5], ymm5
    vmovdqa       [edi + 2*16*6], ymm6
    vmovdqa       [edi + 2*16*7], ymm7
    ret


.section .rodata

.align 32
reduce_mask: .word 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF
             .word 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF
