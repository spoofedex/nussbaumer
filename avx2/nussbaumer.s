.code64
.global nussbaumer1024_forward
.global nussbaumer1024_inverse
.extern transposew16x16
.extern transposew16x16_64
.extern reduce_mask

# Rotate the polynomial a certain number of places. The number of places must be at most 15.
# ymm15 will be subtracted from (so this must be equal to 0, modulo the choosen q).
.macro rotate reg0, reg1, places
  .if \places == 0
  .elseif \places == 8
    vpsubw        ymm10, ymm15, \reg1                  # ymm10 = -\reg1
    vperm2i128    \reg1, \reg0, \reg1, 0x21
    vperm2i128    \reg0, ymm10, \reg0, 0x21
  .elseif \places <= 7
    vpsubw        ymm10, ymm15, \reg1                  # ymm10 = -\reg1
    vperm2i128    ymm10, ymm10, \reg0, 0x21            # ymm10 = -p[24] .. -[p31] p[0] .. p[7]
    vperm2i128    ymm11, \reg0, \reg1, 0x21            # ymm11 = p[8] .. p[15] p[16] .. p[23]
    vpalignr      \reg0, \reg0, ymm10, 16 - 2*\places
    vpalignr      \reg1, \reg1, ymm11, 16 - 2*\places
  .else
    vpsubw        ymm10, ymm15, \reg1                  # ymm10 = -\reg1
    vperm2i128    ymm11, \reg0, \reg1, 0x21            # ymm11 = p[8] .. p[15] p[16] .. p[23]
    vpalignr      \reg1, ymm11, \reg0, 32 - 2*\places
    vperm2i128    ymm11, ymm10, \reg0, 0x21            # ymm11 = -p[24] .. -p[31] p[0] .. p[7]
    vpalignr      \reg0, ymm11, ymm10, 32 - 2*\places
  .endif
.endm


# Perform a FFT butterfly of polynomials represented in registers pol0_0, pol0_1 and pol1_0, pol1_1, rotating the
# second by "places" places.
.macro butterfly pol0_0, pol0_1, pol1_0, pol1_1, places
  .if (\places % 32) < 16
    # Rotate the second polynomial by the amount of places, modulo 16.
    rotate        \pol1_0, \pol1_1, (\places % 32)

    # Perform the butterfly
    .if \places < 32
      vpsubw        ymm10, \pol0_0, \pol1_0
      vpsubw        ymm11, \pol0_1, \pol1_1
      vpaddw        \pol0_0, \pol0_0, \pol1_0
      vpaddw        \pol0_1, \pol0_1, \pol1_1
    .else
      vpaddw        ymm10, \pol0_0, \pol1_0
      vpaddw        ymm11, \pol0_1, \pol1_1
      vpsubw        \pol0_0, \pol0_0, \pol1_0
      vpsubw        \pol0_1, \pol0_1, \pol1_1
    .endif
    vmovdqa       \pol1_0, ymm10
    vmovdqa       \pol1_1, ymm11
  .else
    # Rotate the second polynomial by the amount of places, modulo 16.
    rotate        \pol1_0, \pol1_1, ((\places % 32) - 16)

    # Perform the butterfly, keeping in mind we didn't shift all the way yet, which is simply swapping registers,
    # sign inversing the second.
    .if \places < 32
      vpaddw        ymm10, \pol0_0, \pol1_1
      vpsubw        ymm11, \pol0_1, \pol1_0
      vpsubw        \pol0_0, \pol0_0, \pol1_1
      vpaddw        \pol0_1, \pol0_1, \pol1_0
    .else
      vpsubw        ymm10, \pol0_0, \pol1_1
      vpaddw        ymm11, \pol0_1, \pol1_0
      vpaddw        \pol0_0, \pol0_0, \pol1_1
      vpsubw        \pol0_1, \pol0_1, \pol1_0
    .endif
    vmovdqa       \pol1_0, ymm10
    vmovdqa       \pol1_1, ymm11
  .endif
.endm

# Load 4 polynomials into (ymm0, ymm1), ..., (ymm6, ymm7). These polynomials are located at \base, and \dist represents
# the distance between two polynomials to load.
.macro load_pols base, dist
    vmovdqa       ymm0, [\base + 2*32*0*\dist]
    vmovdqa       ymm1, [\base + 2*32*0*\dist + 2*16]
    vmovdqa       ymm2, [\base + 2*32*1*\dist]
    vmovdqa       ymm3, [\base + 2*32*1*\dist + 2*16]
    vmovdqa       ymm4, [\base + 2*32*2*\dist]
    vmovdqa       ymm5, [\base + 2*32*2*\dist + 2*16]
    vmovdqa       ymm6, [\base + 2*32*3*\dist]
    vmovdqa       ymm7, [\base + 2*32*3*\dist + 2*16]
.endm

# Store 4 polynomials from (ymm0, ymm1), ..., (ymm6, ymm7). These polynomials are stored at \base, and \dist represents
# the distance between two polynomials to store.
.macro store_pols base, dist
    vmovdqa       [\base + 2*32*0*\dist], ymm0
    vmovdqa       [\base + 2*32*0*\dist + 2*16], ymm1
    vmovdqa       [\base + 2*32*1*\dist], ymm2
    vmovdqa       [\base + 2*32*1*\dist + 2*16], ymm3
    vmovdqa       [\base + 2*32*2*\dist], ymm4
    vmovdqa       [\base + 2*32*2*\dist + 2*16], ymm5
    vmovdqa       [\base + 2*32*3*\dist], ymm6
    vmovdqa       [\base + 2*32*3*\dist + 2*16], ymm7
.endm

# Reduce a vector after 4 rounds of the FFT. The result will be at most 12 bits.
.macro reduce reg
    vpaddw        \reg, \reg, ymm4
    vpsrlw        ymm14, \reg, 11
    vpand         \reg, \reg, ymm5
    vpaddw        \reg, \reg, ymm14
.endm


# Perform the Nussbaumer forward transform. The coefficients in the output will have at most 14 bits and will be
# non-negative.
nussbaumer1024_forward:
    push          rbx

    # Perform the transform of the matrix, duplicated twice.
    mov           rdx, rdi
    lea           rdi, [buffer]
    call          transpose_block_double

    add           rsi, 32
    lea           rdi, [buffer + 2*512]
    call          transpose_block_double

    add           rsi, 2*512 - 32
    lea           rdi, [buffer + 32]
    call          transpose_block_double

    add           rsi, 32
    add           rdi, 2*512
    call          transpose_block_double

    # We set ymm15 to all zeroes, for sign inversion
    vpxor         ymm15, ymm15, ymm15


    #########################################################################################
    # FFT levels 2 and 3 (1 being trivially done above)                                     #
    # m = r = 32                                                                            #
    # j = 4, 3                                                                              #
    #########################################################################################
    # Repeat 8 blocks of 4, 2 levels of depth
    lea           rbx, [buffer]
    mov           rcx, 8
.lower_j4_3:
    load_pols     rbx, 8

    butterfly     ymm0, ymm1, ymm4, ymm5, 0
    butterfly     ymm2, ymm3, ymm6, ymm7, 0
    butterfly     ymm0, ymm1, ymm2, ymm3, 0
    butterfly     ymm4, ymm5, ymm6, ymm7, 16

    store_pols    rbx, 8

    # Move to the next polynomial
    add           rbx, 64
    dec           rcx
    jnz           .lower_j4_3

    # Do the other 8 blocks
    # Skip 24 more polynomials to arrive at the bottom half
    add           rbx, 64*24
    mov           rcx, 8
.upper_j4_3:
    load_pols     rbx, 8

    butterfly     ymm0, ymm1, ymm4, ymm5, 16
    butterfly     ymm2, ymm3, ymm6, ymm7, 16
    butterfly     ymm0, ymm1, ymm2, ymm3, 8
    butterfly     ymm4, ymm5, ymm6, ymm7, 24

    store_pols    rbx, 8

    # Move to the next polynomial
    add           rbx, 64
    dec           rcx
    jnz           .upper_j4_3


    #########################################################################################
    # FFT levels 4 and 5                                                                    #
    # j = 2, 1                                                                              #
    # Distances 4 and 2                                                                     #
    #########################################################################################
    lea           rbx, [buffer]
    mov           rcx, 8                              # 8 blocks
    xor           r11, r11
    lea           rsi, forward_2_1_bf
.j2_1:
    # Load the three exponents for the butterflies (r8 for j = 2, r9 and r10 for j = 1)
    lodsq
    mov           r8, rax
    lodsq
    mov           r9, rax
    lodsq
    mov           r10, rax

.j2_1_second_part:
    # First butterfly
    vmovdqa       ymm8, [rbx + 2*32*0*2]
    vmovdqa       ymm9, [rbx + 2*32*0*2 + 2*16]
    vmovdqa       ymm12, [rbx + 2*32*2*2]
    vmovdqa       ymm13, [rbx + 2*32*2*2 + 2*16]
    call          [bfly_table + 8*r8]
    vmovdqa       ymm0, ymm8
    vmovdqa       ymm1, ymm9
    vmovdqa       ymm4, ymm12
    vmovdqa       ymm5, ymm13

    # Second butterfly
    vmovdqa       ymm8, [rbx + 2*32*1*2]
    vmovdqa       ymm9, [rbx + 2*32*1*2 + 2*16]
    vmovdqa       ymm12, [rbx + 2*32*3*2]
    vmovdqa       ymm13, [rbx + 2*32*3*2 + 2*16]
    call          [bfly_table + 8*r8]
    vmovdqa       ymm2, ymm8
    vmovdqa       ymm3, ymm9

    # Fourth butterfly (that is, the lower of j = 1)
    vmovdqa       ymm8, ymm4
    vmovdqa       ymm9, ymm5
    call          [bfly_table + 8*r10]
    vmovdqa       [rbx + 2*32*2*2], ymm8
    vmovdqa       [rbx + 2*32*2*2 + 2*16], ymm9
    vmovdqa       [rbx + 2*32*3*2], ymm12
    vmovdqa       [rbx + 2*32*3*2 + 2*16], ymm13

    # Third butterfly (the upper of j = 0)
    vmovdqa       ymm8, ymm0
    vmovdqa       ymm9, ymm1
    vmovdqa       ymm12, ymm2
    vmovdqa       ymm13, ymm3
    call          [bfly_table + 8*r9]
    vmovdqa       [rbx + 2*32*0*2], ymm8
    vmovdqa       [rbx + 2*32*0*2 + 2*16], ymm9
    vmovdqa       [rbx + 2*32*1*2], ymm12
    vmovdqa       [rbx + 2*32*1*2 + 2*16], ymm13

    # Move to the next set
    xor           r11, 1
    jz            .j2_1_next_block

    # Move to the next in the block
    add           rbx, 2*32
    jmp           .j2_1_second_part

.j2_1_next_block:
    dec           rcx
    jz            .j2_1_done

    # Set rbx to the base of the next polynomial
    add           rbx, 2*32*7
    jmp           .j2_1
.j2_1_done:
    #########################################################################################
    # FFT levels 6                                                                          #
    # j = 0                                                                                 #
    # Distance 1; first element is not calculated, as it is not used in the algorithm.      #
    # Each input will first be reduced once, as it won't fit otherwise.                     #
    #########################################################################################
    lea           rbx, [buffer]

    # This part won't use ymm4/ymm5 anymore. Use ymm4 to store the addition required to make all
    # numbers positive, and in ymm5 store the mask used for the reduction.
    vmovdqa       ymm4, [positive_add]
    vmovdqa       ymm0, [positive_add2]
    vmovdqa       ymm5, [reduce_mask]

    lea           rsi, [forward_0_bf]
    mov           rcx, 32                             # 32 blocks
.j3:
    lodsq
    vmovdqa       ymm8, [rbx + 2*32*0]
    vmovdqa       ymm9, [rbx + 2*32*0 + 2*16]
    vmovdqa       ymm12, [rbx + 2*32*1]
    vmovdqa       ymm13, [rbx + 2*32*1 + 2*16]

    reduce        ymm8
    reduce        ymm9
    reduce        ymm12
    reduce        ymm13

    call          [bfly_table + 8*rax]

    vpaddw        ymm8, ymm8, ymm0
    vpaddw        ymm9, ymm9, ymm0
    vpaddw        ymm12, ymm12, ymm0
    vpaddw        ymm13, ymm13, ymm0

    vmovdqa       [rbx + 2*32*0], ymm8
    vmovdqa       [rbx + 2*32*0 + 2*16], ymm9
    vmovdqa       [rbx + 2*32*1], ymm12
    vmovdqa       [rbx + 2*32*1 + 2*16], ymm13

    add           rbx, 2*32*2
    dec           rcx
    jnz           .j3

    # Transpose the data
    lea           rsi, [buffer]
    mov           rdi, rdx
    call          transpose_block

    lea           rsi, [buffer + 1*2*32*16]
    add           rdi, 2*16
    call          transpose_block

    lea           rsi, [buffer + 2*2*32*16]
    add           rdi, 2*16
    call          transpose_block

    lea           rsi, [buffer + 3*2*32*16]
    add           rdi, 2*16
    call          transpose_block

    lea           rsi, [buffer + 32]
    add           rdi, 64*2*16 - 3*2*16
    call          transpose_block

    lea           rsi, [buffer + 32 + 1*2*32*16]
    add           rdi, 2*16
    call          transpose_block

    lea           rsi, [buffer + 32 + 2*2*32*16]
    add           rdi, 2*16
    call          transpose_block

    lea           rsi, [buffer + 32 + 3*2*32*16]
    add           rdi, 2*16
    call          transpose_block

    pop           rbx
    ret


# Inverse Nussbaumer transform. The source and destination may be the same.
# The input coefficients must have at most 12 bits; the output will be properly reduced.
nussbaumer1024_inverse:
    push          rbx
    mov           r11, rdi
    mov           r9, rsi

    # Transpose the data
    lea           rdi, [buffer]
    call          transpose_block_inverse

    lea           rsi, [r9 + 2*16]
    lea           rdi, [buffer + 32*2*16]
    call          transpose_block_inverse

    lea           rsi, [r9 + 2*16*2]
    lea           rdi, [buffer + 32*2*16*2]
    call          transpose_block_inverse

    lea           rsi, [r9 + 2*16*3]
    lea           rdi, [buffer + 32*2*16*3]
    call          transpose_block_inverse

    lea           rsi, [r9 + 64*2*16]
    lea           rdi, [buffer + 16*2]
    call          transpose_block_inverse

    lea           rsi, [r9 + 64*2*16 + 2*16]
    lea           rdi, [buffer + 32*2*16 + 16*2]
    call          transpose_block_inverse

    lea           rsi, [r9 + 64*2*16 + 2*16*2]
    lea           rdi, [buffer + 32*2*16*2 + 16*2]
    call          transpose_block_inverse

    lea           rsi, [r9 + 64*2*16 + 2*16*3]
    lea           rdi, [buffer + 32*2*16*3 + 16*2]
    call          transpose_block_inverse

    #########################################################################################
    # FFT levels 1 and 2                                                                    #
    # j = 0, 1                                                                              #
    #########################################################################################
    # Repeat 8 blocks of 4, 2 levels of depth
    lea           rbx, [buffer]
    vpxor         ymm15, ymm15, ymm15
    mov           rcx, 16
.invblock_j0_1:
    load_pols     rbx, 1

    butterfly     ymm0, ymm1, ymm2, ymm3, 0
    butterfly     ymm4, ymm5, ymm6, ymm7, 0
    butterfly     ymm0, ymm1, ymm4, ymm5, 0
    butterfly     ymm2, ymm3, ymm6, ymm7, 48

    store_pols    rbx, 1

    # Move to the next polynomial
    add           rbx, 64*4
    dec           rcx
    jnz           .invblock_j0_1


    #########################################################################################
    # FFT levels 3 and 4                                                                    #
    # j = 2, 3                                                                              #
    # Distances 4 and 8                                                                     #
    #########################################################################################
    lea           rbx, [buffer]
    lea           rsi, inverse_2_3_bf
    mov           rdx, 4                              # 4 sets of polynomials
    vmovdqa       ymm4, [positive_add]
    vmovdqa       ymm5, [reduce_mask]

.j2_3:
    # Load the three exponents for the butterflies (r8 for j = 2, r9 and r10 for j = 3)
    lodsq
    mov           r8, rax
    lodsq
    mov           r9, rax
    lodsq
    mov           r10, rax

    mov           rcx, 4                              # 4 blocks

.j2_3_set:
    # First butterfly
    vmovdqa       ymm8, [rbx + 2*32*0*4]
    vmovdqa       ymm9, [rbx + 2*32*0*4 + 2*16]
    vmovdqa       ymm12, [rbx + 2*32*1*4]
    vmovdqa       ymm13, [rbx + 2*32*1*4 + 2*16]
    call          [bfly_table + 8*r8]
    reduce        ymm8
    reduce        ymm9
    reduce        ymm12
    reduce        ymm13
    vmovdqa       ymm0, ymm8
    vmovdqa       ymm1, ymm9
    vmovdqa       ymm2, ymm12
    vmovdqa       ymm3, ymm13

    # Second butterfly
    vmovdqa       ymm8, [rbx + 2*32*2*4]
    vmovdqa       ymm9, [rbx + 2*32*2*4 + 2*16]
    vmovdqa       ymm12, [rbx + 2*32*3*4]
    vmovdqa       ymm13, [rbx + 2*32*3*4 + 2*16]
    call          [bfly_table + 8*r8]
    reduce        ymm8
    reduce        ymm9
    reduce        ymm12
    reduce        ymm13
    vmovdqa       ymm6, ymm8
    vmovdqa       ymm7, ymm9

    # Fourth butterfly (that is, the lower of j = 3)
    vmovdqa       ymm8, ymm2
    vmovdqa       ymm9, ymm3
    call          [bfly_table + 8*r10]
    vmovdqa       [rbx + 2*32*1*4], ymm8
    vmovdqa       [rbx + 2*32*1*4 + 2*16], ymm9
    vmovdqa       [rbx + 2*32*3*4], ymm12
    vmovdqa       [rbx + 2*32*3*4 + 2*16], ymm13

    # Third butterfly (the upper of j = 3)
    vmovdqa       ymm8, ymm0
    vmovdqa       ymm9, ymm1
    vmovdqa       ymm12, ymm6
    vmovdqa       ymm13, ymm7
    call          [bfly_table + 8*r9]
    vmovdqa       [rbx + 2*32*0*4], ymm8
    vmovdqa       [rbx + 2*32*0*4 + 2*16], ymm9
    vmovdqa       [rbx + 2*32*2*4], ymm12
    vmovdqa       [rbx + 2*32*2*4 + 2*16], ymm13

    # Move to the next in the set
    add           rbx, 2*32*16
    dec           rcx
    jnz           .j2_3_set

    # Move to the next block
    sub           rbx, 2*32*64 - 2*32
    dec           rdx
    jnz           .j2_3


    #########################################################################################
    # FFT levels 5 and 6                                                                    #
    # j = 4, 5                                                                              #
    # Distances 16 and 32                                                                   #
    #########################################################################################
    lea           rbx, [buffer]
    lea           rsi, inverse_4_5_bf
    mov           rcx, 16                              # 16 sets of polynomials
.j4_5:
    # Load the three exponents for the butterflies (r8 for j = 4, r9 and r10 for j = 5)
    lodsq
    mov           r8, rax
    lodsq
    mov           r9, rax
    lodsq
    mov           r10, rax

    # First butterfly
    vmovdqa       ymm8, [rbx + 2*32*0*16]
    vmovdqa       ymm9, [rbx + 2*32*0*16 + 2*16]
    vmovdqa       ymm12, [rbx + 2*32*1*16]
    vmovdqa       ymm13, [rbx + 2*32*1*16 + 2*16]
    call          [bfly_table + 8*r8]
    reduce        ymm8
    reduce        ymm9
    reduce        ymm12
    reduce        ymm13
    vmovdqa       ymm0, ymm8
    vmovdqa       ymm1, ymm9
    vmovdqa       ymm2, ymm12
    vmovdqa       ymm3, ymm13

    # Second butterfly
    vmovdqa       ymm8, [rbx + 2*32*2*16]
    vmovdqa       ymm9, [rbx + 2*32*2*16 + 2*16]
    vmovdqa       ymm12, [rbx + 2*32*3*16]
    vmovdqa       ymm13, [rbx + 2*32*3*16 + 2*16]
    call          [bfly_table + 8*r8]
    reduce        ymm8
    reduce        ymm9
    reduce        ymm12
    reduce        ymm13
    vmovdqa       ymm6, ymm8
    vmovdqa       ymm7, ymm9

    # Fourth butterfly (that is, the lower of j = 5)
    vmovdqa       ymm8, ymm2
    vmovdqa       ymm9, ymm3
    call          [bfly_table + 8*r10]

    # Perform the next step of the algorithm immediately as well
    rotate        ymm12, ymm13, 1
    vpaddw        ymm8, ymm8, ymm12
    vpaddw        ymm9, ymm9, ymm13

    reduce        ymm8
    reduce        ymm9

    # Perform the final reduction and the multiplication by 1/64 = 32.
    # The input is at most 12 bits.
    vpsrlw        ymm14, ymm8, 11
    vpaddw        ymm8, ymm8, ymm14
    vpsrlw        ymm14, ymm9, 11
    vpaddw        ymm9, ymm9, ymm14

    # Replace 2047 by 0
    vpcmpeqw      ymm14, ymm8, ymm5
    vpsubw        ymm8, ymm8, ymm14
    vpcmpeqw      ymm14, ymm9, ymm5
    vpsubw        ymm9, ymm9, ymm14
    vpandw        ymm8, ymm8, ymm5
    vpandw        ymm9, ymm9, ymm5

    vpsrlw        ymm14, ymm8, 11
    vpaddw        ymm8, ymm8, ymm14
    vpsrlw        ymm14, ymm9, 11
    vpaddw        ymm9, ymm9, ymm14

    vpsllw        ymm14, ymm8, 5
    vpsrlw        ymm8, ymm8, 6
    vporw         ymm8, ymm8, ymm14
    vpsllw        ymm14, ymm9, 5
    vpsrlw        ymm9, ymm9, 6
    vporw         ymm9, ymm9, ymm14
    vpandw        ymm8, ymm8, ymm5
    vpandw        ymm9, ymm9, ymm5

    vmovdqa       [rbx + 2*32*1*16], ymm8
    vmovdqa       [rbx + 2*32*1*16 + 2*16], ymm9

    # Third butterfly (the upper of j = 5)
    vmovdqa       ymm8, ymm0
    vmovdqa       ymm9, ymm1
    vmovdqa       ymm12, ymm6
    vmovdqa       ymm13, ymm7
    call          [bfly_table + 8*r9]

    # Perform the next step of the algorithm immediately as well
    rotate        ymm12, ymm13, 1
    vpaddw        ymm8, ymm8, ymm12
    vpaddw        ymm9, ymm9, ymm13

    reduce        ymm8
    reduce        ymm9

    # Perform the final reduction and the multiplication by 1/64 = 32.
    # The input is at most 12 bits.
    vpsrlw        ymm14, ymm8, 11
    vpaddw        ymm8, ymm8, ymm14
    vpsrlw        ymm14, ymm9, 11
    vpaddw        ymm9, ymm9, ymm14

    # Replace 2047 by 0
    vpcmpeqw      ymm14, ymm8, ymm5
    vpsubw        ymm8, ymm8, ymm14
    vpcmpeqw      ymm14, ymm9, ymm5
    vpsubw        ymm9, ymm9, ymm14
    vpandw        ymm8, ymm8, ymm5
    vpandw        ymm9, ymm9, ymm5

    vpsrlw        ymm14, ymm8, 11
    vpaddw        ymm8, ymm8, ymm14
    vpsrlw        ymm14, ymm9, 11
    vpaddw        ymm9, ymm9, ymm14

    vpsllw        ymm14, ymm8, 5
    vpsrlw        ymm8, ymm8, 6
    vporw         ymm8, ymm8, ymm14
    vpsllw        ymm14, ymm9, 5
    vpsrlw        ymm9, ymm9, 6
    vporw         ymm9, ymm9, ymm14
    vpandw        ymm8, ymm8, ymm5
    vpandw        ymm9, ymm9, ymm5

    vmovdqa       [rbx + 2*32*0*16], ymm8
    vmovdqa       [rbx + 2*32*0*16 + 2*16], ymm9

    # Move to the next in the set
    add           rbx, 2*32
    dec           rcx
    jnz           .j4_5

    # Finally, transpose the buffer into the destination
    # Perform the transform of the matrix, duplicated twice.
    lea           rsi, [buffer]
    lea           rdi, [r11]
    call          transpose_block_inverse_final

    lea           rsi, [buffer + 2*16]
    lea           rdi, [r11 + 2*32*16]
    call          transpose_block_inverse_final

    lea           rsi, [buffer + 2*32*16]
    lea           rdi, [r11 + 2*16]
    call          transpose_block_inverse_final

    lea           rsi, [buffer + 2*32*16 + 2*16]
    lea           rdi, [r11 + 2*32*16 + 2*16]
    call          transpose_block_inverse_final

    pop           rbx
    ret


# Build the functions for all possible butterflies
.macro make_butterfly_func num
butterfly\num:
    butterfly      ymm8, ymm9, ymm12, ymm13, \num
    ret
.endm
make_butterfly_func 0
make_butterfly_func 1
make_butterfly_func 2
make_butterfly_func 3
make_butterfly_func 4
make_butterfly_func 5
make_butterfly_func 6
make_butterfly_func 7
make_butterfly_func 8
make_butterfly_func 9
make_butterfly_func 10
make_butterfly_func 11
make_butterfly_func 12
make_butterfly_func 13
make_butterfly_func 14
make_butterfly_func 15
make_butterfly_func 16
make_butterfly_func 17
make_butterfly_func 18
make_butterfly_func 19
make_butterfly_func 20
make_butterfly_func 21
make_butterfly_func 22
make_butterfly_func 23
make_butterfly_func 24
make_butterfly_func 25
make_butterfly_func 26
make_butterfly_func 27
make_butterfly_func 28
make_butterfly_func 29
make_butterfly_func 30
make_butterfly_func 31
make_butterfly_func 32
make_butterfly_func 33
make_butterfly_func 34
make_butterfly_func 35
make_butterfly_func 36
make_butterfly_func 37
make_butterfly_func 38
make_butterfly_func 39
make_butterfly_func 40
make_butterfly_func 41
make_butterfly_func 42
make_butterfly_func 43
make_butterfly_func 44
make_butterfly_func 45
make_butterfly_func 46
make_butterfly_func 47
make_butterfly_func 48
make_butterfly_func 49
make_butterfly_func 50
make_butterfly_func 51
make_butterfly_func 52
make_butterfly_func 53
make_butterfly_func 54
make_butterfly_func 55
make_butterfly_func 56
make_butterfly_func 57
make_butterfly_func 58
make_butterfly_func 59
make_butterfly_func 60
make_butterfly_func 61
make_butterfly_func 62
make_butterfly_func 63
make_butterfly_func 64

transpose_block_double:
    call          transposew16x16

    vmovdqa       [rdi + 0*2*32], ymm0
    vmovdqa       [rdi + 0*2*32 + 2*0x400], ymm0
    vmovdqa       [rdi + 1*2*32], ymm1
    vmovdqa       [rdi + 1*2*32 + 2*0x400], ymm1
    vmovdqa       [rdi + 2*2*32], ymm2
    vmovdqa       [rdi + 2*2*32 + 2*0x400], ymm2
    vmovdqa       [rdi + 3*2*32], ymm3
    vmovdqa       [rdi + 3*2*32 + 2*0x400], ymm3
    vmovdqa       [rdi + 4*2*32], ymm4
    vmovdqa       [rdi + 4*2*32 + 2*0x400], ymm4
    vmovdqa       [rdi + 5*2*32], ymm5
    vmovdqa       [rdi + 5*2*32 + 2*0x400], ymm5
    vmovdqa       [rdi + 6*2*32], ymm6
    vmovdqa       [rdi + 6*2*32 + 2*0x400], ymm6
    vmovdqa       [rdi + 7*2*32], ymm7
    vmovdqa       [rdi + 7*2*32 + 2*0x400], ymm7
    vmovdqa       [rdi + 8*2*32], ymm8
    vmovdqa       [rdi + 8*2*32 + 2*0x400], ymm8
    vmovdqa       [rdi + 9*2*32], ymm9
    vmovdqa       [rdi + 9*2*32 + 2*0x400], ymm9
    vmovdqa       [rdi + 10*2*32], ymm10
    vmovdqa       [rdi + 10*2*32 + 2*0x400], ymm10
    vmovdqa       [rdi + 11*2*32], ymm11
    vmovdqa       [rdi + 11*2*32 + 2*0x400], ymm11
    vmovdqa       [rdi + 12*2*32], ymm12
    vmovdqa       [rdi + 12*2*32 + 2*0x400], ymm12
    vmovdqa       [rdi + 13*2*32], ymm13
    vmovdqa       [rdi + 13*2*32 + 2*0x400], ymm13
    vmovdqa       [rdi + 14*2*32], ymm14
    vmovdqa       [rdi + 14*2*32 + 2*0x400], ymm14
    vmovdqa       [rdi + 15*2*32], ymm15
    vmovdqa       [rdi + 15*2*32 + 2*0x400], ymm15
    ret

transpose_block:
    call          transposew16x16

    vmovdqa       [rdi + 0*2*64], ymm0
    vmovdqa       [rdi + 1*2*64], ymm1
    vmovdqa       [rdi + 2*2*64], ymm2
    vmovdqa       [rdi + 3*2*64], ymm3
    vmovdqa       [rdi + 4*2*64], ymm4
    vmovdqa       [rdi + 5*2*64], ymm5
    vmovdqa       [rdi + 6*2*64], ymm6
    vmovdqa       [rdi + 7*2*64], ymm7
    vmovdqa       [rdi + 8*2*64], ymm8
    vmovdqa       [rdi + 9*2*64], ymm9
    vmovdqa       [rdi + 10*2*64], ymm10
    vmovdqa       [rdi + 11*2*64], ymm11
    vmovdqa       [rdi + 12*2*64], ymm12
    vmovdqa       [rdi + 13*2*64], ymm13
    vmovdqa       [rdi + 14*2*64], ymm14
    vmovdqa       [rdi + 15*2*64], ymm15
    ret

transpose_block_inverse:
    call          transposew16x16_64

    vmovdqa       [rdi + 0*2*32], ymm0
    vmovdqa       [rdi + 1*2*32], ymm1
    vmovdqa       [rdi + 2*2*32], ymm2
    vmovdqa       [rdi + 3*2*32], ymm3
    vmovdqa       [rdi + 4*2*32], ymm4
    vmovdqa       [rdi + 5*2*32], ymm5
    vmovdqa       [rdi + 6*2*32], ymm6
    vmovdqa       [rdi + 7*2*32], ymm7
    vmovdqa       [rdi + 8*2*32], ymm8
    vmovdqa       [rdi + 9*2*32], ymm9
    vmovdqa       [rdi + 10*2*32], ymm10
    vmovdqa       [rdi + 11*2*32], ymm11
    vmovdqa       [rdi + 12*2*32], ymm12
    vmovdqa       [rdi + 13*2*32], ymm13
    vmovdqa       [rdi + 14*2*32], ymm14
    vmovdqa       [rdi + 15*2*32], ymm15
    ret


transpose_block_inverse_final:
    call          transposew16x16

    vmovdqa       [rdi + 0*2*32], ymm0
    vmovdqa       [rdi + 1*2*32], ymm1
    vmovdqa       [rdi + 2*2*32], ymm2
    vmovdqa       [rdi + 3*2*32], ymm3
    vmovdqa       [rdi + 4*2*32], ymm4
    vmovdqa       [rdi + 5*2*32], ymm5
    vmovdqa       [rdi + 6*2*32], ymm6
    vmovdqa       [rdi + 7*2*32], ymm7
    vmovdqa       [rdi + 8*2*32], ymm8
    vmovdqa       [rdi + 9*2*32], ymm9
    vmovdqa       [rdi + 10*2*32], ymm10
    vmovdqa       [rdi + 11*2*32], ymm11
    vmovdqa       [rdi + 12*2*32], ymm12
    vmovdqa       [rdi + 13*2*32], ymm13
    vmovdqa       [rdi + 14*2*32], ymm14
    vmovdqa       [rdi + 15*2*32], ymm15
    ret

.section .rodata
.align 32

# The register to add during the reduction in the middle of the Fourier transform, in order to make all numbers
# positive.
positive_add:
  .short 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752

# Same as above, but when the number is only 13 bits
positive_add2:
  .short 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188, 8188

# The butterfly exponents for j = 2, 1, for each block. The values are sequences of 3, the first one indicating
# the exponents for j = 2, the other two for j = 1 (the upper and the lower, respectively)
forward_2_1_bf:
  .quad 0, 0, 16
  .quad 16, 8, 24
  .quad 8, 4, 20
  .quad 24, 12, 28
  .quad 4, 2, 18
  .quad 20, 10, 26
  .quad 12, 6, 22
  .quad 28, 14, 30

# the exponents for j = 0. The first butterfly is skipped, as the first result is ignored.
forward_0_bf:
  .quad 0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30, 1, 17, 9, 25, 5
  .quad 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31

# Inverse butterfly exponents for j = 2, 3
inverse_2_3_bf:
  .quad 0, 0, 64 - 16
  .quad 64 - 8, 64 - 4, 64 - 20
  .quad 64 - 16, 64 - 8, 64 - 24
  .quad 64 - 24, 64 - 12, 64 - 28

# Inverse butterfly exponents for j = 4, 5
inverse_4_5_bf:
  .quad 0, 0, 64 - 16
  .quad 64 - 2, 64 - 1, 64 - 17
  .quad 64 - 4, 64 - 2, 64 - 18
  .quad 64 - 6, 64 - 3, 64 - 19
  .quad 64 - 8, 64 - 4, 64 - 20
  .quad 64 - 10, 64 - 5, 64 - 21
  .quad 64 - 12, 64 - 6, 64 - 22
  .quad 64 - 14, 64 - 7, 64 - 23
  .quad 64 - 16, 64 - 8, 64 - 24
  .quad 64 - 18, 64 - 9, 64 - 25
  .quad 64 - 20, 64 - 10, 64 - 26
  .quad 64 - 22, 64 - 11, 64 - 27
  .quad 64 - 24, 64 - 12, 64 - 28
  .quad 64 - 26, 64 - 13, 64 - 29
  .quad 64 - 28, 64 - 14, 64 - 30
  .quad 64 - 30, 64 - 15, 64 - 31

# Jump table for different butterflies
bfly_table:
  .quad butterfly0, butterfly1, butterfly2, butterfly3
  .quad butterfly4, butterfly5, butterfly6, butterfly7
  .quad butterfly8, butterfly9, butterfly10, butterfly11
  .quad butterfly12, butterfly13, butterfly14, butterfly15
  .quad butterfly16, butterfly17, butterfly18, butterfly19
  .quad butterfly20, butterfly21, butterfly22, butterfly23
  .quad butterfly24, butterfly25, butterfly26, butterfly27
  .quad butterfly28, butterfly29, butterfly30, butterfly31
  .quad butterfly32, butterfly33, butterfly34, butterfly35
  .quad butterfly36, butterfly37, butterfly38, butterfly39
  .quad butterfly40, butterfly41, butterfly42, butterfly43
  .quad butterfly44, butterfly45, butterfly46, butterfly47
  .quad butterfly48, butterfly49, butterfly50, butterfly51
  .quad butterfly52, butterfly53, butterfly54, butterfly55
  .quad butterfly56, butterfly57, butterfly58, butterfly59
  .quad butterfly60, butterfly61, butterfly62, butterfly63


.section .bss
.align 32
buffer: .space 4096
