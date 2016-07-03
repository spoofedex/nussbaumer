.code64
.global componentwise32_64_prepare
.global componentwise32_64_run
.extern reduce_mask
.extern schoolbook4

.macro reduce reg, temp_reg, and_reg
    vpsrlw        \temp_reg, \reg, 11
    vpand         \reg, \reg, \and_reg
    vpaddw        \reg, \reg, \temp_reg
.endm

# Prepare for componentwise multiplication through the Karatusba method, of 64 polynomials
# with 32 coefficients each. The input consists of 32*64 16-bit integers, where the first
# 64 integers represent the first coefficients, the next the second coefficients, etc. The
# output, which is also the input, requires room for 6912 16-bit integers.
# The inputs must have at most 14 bits, and be non-negative.
# The outputs have at most 15 bits.
componentwise32_64_prepare:
    vmovdqa       ymm15, [reduce_mask]
    mov           rcx, 16

.loop:
    vmovdqa       ymm0, [rdi]
    vmovdqa       ymm1, [rdi + 64*2*4]
    vmovdqa       ymm2, [rdi + 64*2*8]
    vmovdqa       ymm3, [rdi + 64*2*12]
    vmovdqa       ymm4, [rdi + 64*2*16]
    vmovdqa       ymm5, [rdi + 64*2*20]
    vmovdqa       ymm6, [rdi + 64*2*24]
    vmovdqa       ymm7, [rdi + 64*2*28]

    vpaddw        ymm8, ymm0, ymm4
    vmovdqa       [rdi + 64*2*32], ymm8
    vpaddw        ymm9, ymm1, ymm5
    vmovdqa       [rdi + 64*2*36], ymm9
    vpaddw        ymm10, ymm2, ymm6
    vmovdqa       [rdi + 64*2*40], ymm10
    vpaddw        ymm11, ymm3, ymm7
    vmovdqa       [rdi + 64*2*44], ymm11

    # This may become 16 bits; reduce
    vpaddw        ymm12, ymm8, ymm10
    reduce        ymm12, ymm14, ymm15
    vmovdqa       [rdi + (8 + 4 + 4)*64*2*4], ymm12
    vpaddw        ymm13, ymm9, ymm11
    reduce        ymm13, ymm14, ymm15
    vmovdqa       [rdi + (8 + 4 + 4 + 1)*64*2*4], ymm13

    vpaddw        ymm12, ymm12, ymm13
    vmovdqa       [rdi + 26*64*2*4], ymm12

    vpaddw        ymm12, ymm8, ymm9
    reduce        ymm12, ymm14, ymm15
    vmovdqa       [rdi + (8 + 4 + 6 + 4)*64*2*4], ymm12
    vpaddw        ymm13, ymm10, ymm11
    reduce        ymm13, ymm14, ymm15
    vmovdqa       [rdi + (8 + 4 + 6 + 5)*64*2*4], ymm13

    # Second block of 8 and down
    vpaddw        ymm8, ymm0, ymm2
    reduce        ymm8, ymm14, ymm15
    vmovdqa       [rdi + (8 + 4)*64*2*4], ymm8
    vpaddw        ymm9, ymm1, ymm3
    reduce        ymm9, ymm14, ymm15
    vmovdqa       [rdi + (8 + 4 + 1)*64*2*4], ymm9

    vpaddw        ymm8, ymm8, ymm9
    vmovdqa       [rdi + (8 + 4 + 6 + 6)*64*2*4], ymm8


    # Third block of 8 and down
    vpaddw        ymm8, ymm4, ymm6
    reduce        ymm8, ymm14, ymm15
    vmovdqa       [rdi + (8 + 6)*64*2*4], ymm8
    vpaddw        ymm9, ymm5, ymm7
    reduce        ymm9, ymm14, ymm15
    vmovdqa       [rdi + (8 + 6 + 1)*64*2*4], ymm9

    vpaddw        ymm8, ymm8, ymm9
    vmovdqa       [rdi + (8 + 4 + 6 + 7)*64*2*4], ymm8


    # Extra blocks of 4, directly from the 32
    vpaddw        ymm0, ymm0, ymm1
    vmovdqa       [rdi + (8 + 4 + 6)*64*2*4], ymm0
    vpaddw        ymm2, ymm2, ymm3
    vmovdqa       [rdi + (8 + 4 + 6 + 1)*64*2*4], ymm2
    vpaddw        ymm4, ymm4, ymm5
    vmovdqa       [rdi + (8 + 4 + 6 + 2)*64*2*4], ymm4
    vpaddw        ymm6, ymm6, ymm7
    vmovdqa       [rdi + (8 + 4 + 6 + 3)*64*2*4], ymm6

    add           rdi, 32
    dec           rcx
    jnz           .loop

    ret



# Actually execute the componentwise multiplication prepared above.
# The output will have at most 12 bits.
componentwise32_64_run:
    # First, multiply all the 4-degree polynomials by the schoolbook method
    # We multiply 27 groups of 4-polynomials with 64 coefficients
    mov           rcx, 27
    lea           r9, [buffer]
.run_line:
    # Loop through 4 blocks of 16 coefficients
    xor           rax, rax
    mov           r11, 4
.run_line_part:
    vmovdqa       ymm8, [rdx + rax]
    vmovdqa       ymm9, [rdx + rax + 128]
    vmovdqa       ymm10, [rdx + rax + 2*128]
    vmovdqa       ymm11, [rdx + rax + 3*128]
    vmovdqa       ymm12, [rsi + rax]
    vmovdqa       ymm13, [rsi + rax + 128]
    vmovdqa       ymm14, [rsi + rax + 2*128]
    vmovdqa       ymm15, [rsi + rax + 3*128]

    call          schoolbook4

    vmovdqa       [r9 + rax], ymm0
    vmovdqa       [r9 + rax + 128], ymm1
    vmovdqa       [r9 + rax + 2*128], ymm2
    vmovdqa       [r9 + rax + 3*128], ymm3
    vmovdqa       [r9 + rax + 4*128], ymm4
    vmovdqa       [r9 + rax + 5*128], ymm5
    vmovdqa       [r9 + rax + 6*128], ymm6

    # Loop to the next parts
    add           rax, 32
    dec           r11
    jnz           .run_line_part

    add           r9, 8*64*2
    add           rdx, 4*64*2
    add           rsi, 4*64*2
    dec           rcx
    jnz           .run_line

    ####################################################################
    # Perform the Karatsuba steps for 8-bit polynomials                #
    ####################################################################
    mov           rax, 9
    lea           r8, [buffer]
    lea           r10, [buffer]

    # Perform a reduction, so the result will be at most 12 bits.
    vmovdqa       ymm15, [reduce_mask]
    vmovdqa       ymm14, [positive_add]
.karatsuba8_loop:
    mov           rcx, 128*2 + 32*3
.next_coef8:
    vmovdqa       ymm0, [r8 + 0*128 + rcx]
    vmovdqa       ymm1, [r8 + 4*128 + rcx]
    vmovdqa       ymm2, [r8 + 8*128 + rcx]
    vmovdqa       ymm3, [r8 + 12*128 + rcx]
    vmovdqa       ymm4, [r10 + 18*8*128 + rcx]
    vmovdqa       ymm5, [r10 + (18*8 + 4)*128 + rcx]

    vpsubw        ymm8, ymm1, ymm2
    vpsubw        ymm9, ymm5, ymm8
    vpsubw        ymm9, ymm9, ymm3
    vpaddw        ymm8, ymm8, ymm4
    vpsubw        ymm8, ymm8, ymm0

    vpaddw        ymm8, ymm8, ymm14
    vpaddw        ymm9, ymm9, ymm14
    reduce        ymm8, ymm13, ymm15
    reduce        ymm9, ymm13, ymm15

    vmovdqa       [r8 + 4*128 + rcx], ymm8
    vmovdqa       [r8 + 8*128 + rcx], ymm9

    sub           rcx, 32
    jns           .next_coef8

    mov           rcx, 32*3
.next_coef8_4th:
    # Calculate the 4th coefficient
    vmovdqa       ymm0, [r8 + 3*128 + rcx]
    vmovdqa       ymm2, [r8 + (8 + 3)*128 + rcx]
    vmovdqa       ymm4, [r10 + (18*8 + 3)*128 + rcx]

    vpsubw        ymm0, ymm4, ymm0
    vpsubw        ymm0, ymm0, ymm2
    vpaddw        ymm0, ymm0, ymm14
    reduce        ymm0, ymm13, ymm15

    vmovdqa       [r8 + 7*128 + rcx], ymm0

    sub           rcx, 32
    jns           .next_coef8_4th

    # Reduce the other coefficients
    mov           rcx, 128*3 + 32*3
.reduce_loop1:
    vmovdqa       ymm0, [r8 + rcx]
    reduce        ymm0, ymm13, ymm15
    vmovdqa       [r8 + rcx], ymm0
    sub           rcx, 32
    jns           .reduce_loop1

    mov           rcx, 128*2 + 32*3
.reduce_loop2:
    vmovdqa       ymm0, [r8 + 128*12 + rcx]
    reduce        ymm0, ymm13, ymm15
    vmovdqa       [r8 + 128*12 + rcx], ymm0
    sub           rcx, 32
    jns           .reduce_loop2

    add           r8, 128*16
    add           r10, 128*8
    dec           rax
    jnz           .karatsuba8_loop

    ####################################################################
    # Perform the Karatsuba steps for 16-bit polynomials               #
    ####################################################################
    mov           rax, 3
    lea           r8, [buffer]
    lea           r10, [buffer]
.karatsuba16_loop:
    mov           rcx, 128*6 + 32*3
.next_coef16:
    vmovdqa       ymm0, [r8 + 0*128 + rcx]
    vmovdqa       ymm1, [r8 + 8*128 + rcx]
    vmovdqa       ymm2, [r8 + 16*128 + rcx]
    vmovdqa       ymm3, [r8 + 24*128 + rcx]
    vmovdqa       ymm4, [r10 + 6*16*128 + rcx]
    vmovdqa       ymm5, [r10 + (6*16 + 8)*128 + rcx]

    vpsubw        ymm8, ymm1, ymm2
    vpsubw        ymm9, ymm5, ymm8
    vpsubw        ymm9, ymm9, ymm3
    vpaddw        ymm8, ymm8, ymm4
    vpsubw        ymm8, ymm8, ymm0

    vmovdqa       [r8 + 8*128 + rcx], ymm8
    vmovdqa       [r8 + 16*128 + rcx], ymm9

    sub           rcx, 32
    jns           .next_coef16

    mov           rcx, 32*3
.next_coef16_final:
    # Calculate the final coefficient
    vmovdqa       ymm0, [r8 + 7*128 + rcx]
    vmovdqa       ymm2, [r8 + (16 + 7)*128 + rcx]
    vmovdqa       ymm4, [r10 + (16*6 + 7)*128 + rcx]

    vpsubw        ymm0, ymm4, ymm0
    vpsubw        ymm0, ymm0, ymm2

    vmovdqa       [r8 + 15*128 + rcx], ymm0

    sub           rcx, 32
    jns           .next_coef16_final

    add           r8, 128*32
    add           r10, 128*16
    dec           rax
    jnz           .karatsuba16_loop

    ####################################################################
    # Perform the Karatsuba steps for 32-bit polynomials               #
    # Optimized by calculating the result modulo X^32 + 1              #
    ####################################################################
    mov           rcx, 128*14 + 32*3
.karatsuba32_next_coef:
    vmovdqa       ymm0, [buffer + 0*128 + rcx]
    vmovdqa       ymm1, [buffer + 16*128 + rcx]
    vmovdqa       ymm2, [buffer + 32*128 + rcx]
    vmovdqa       ymm3, [buffer + 48*128 + rcx]
    vmovdqa       ymm4, [buffer + 64*128 + rcx]
    vmovdqa       ymm5, [buffer + 80*128 + rcx]

    vpsubw        ymm6, ymm1, ymm2
    vpaddw        ymm7, ymm0, ymm3
    vpaddw        ymm8, ymm6, ymm7
    vpsubw        ymm9, ymm6, ymm7

    # Reduce to 12 bits again
    vpaddw        ymm8, ymm8, ymm14
    reduce        ymm8, ymm13, ymm15
    vpaddw        ymm9, ymm9, ymm14
    reduce        ymm9, ymm13, ymm15

    vpsubw        ymm8, ymm8, ymm5
    vpaddw        ymm9, ymm9, ymm4

    # Reduce again to prevent negative values
    vpaddw        ymm8, ymm8, ymm14
    reduce        ymm8, ymm13, ymm15
    vpaddw        ymm9, ymm9, ymm14
    reduce        ymm9, ymm13, ymm15

    vmovdqa       [rdi + 0*128 + rcx], ymm8
    vmovdqa       [rdi + 16*128 + rcx], ymm9

    sub           rcx, 32
    jns           .karatsuba32_next_coef

    mov           rcx, 32*3
.next_coef32_final:
    # Calculate the final coefficient
    vmovdqa       ymm0, [buffer + 15*128 + rcx]
    vmovdqa       ymm2, [buffer + (32 + 15)*128 + rcx]
    vmovdqa       ymm4, [buffer + (2*32 + 15)*128 + rcx]

    vpsubw        ymm5, ymm0, ymm2
    vpsubw        ymm6, ymm4, ymm0
    vpsubw        ymm6, ymm6, ymm2

    vpaddw        ymm5, ymm5, ymm14
    reduce        ymm5, ymm13, ymm15
    vpaddw        ymm6, ymm6, ymm14
    reduce        ymm6, ymm13, ymm15

    vmovdqa       [rdi + 15*128 + rcx], ymm5
    vmovdqa       [rdi + 31*128 + rcx], ymm6
    sub           rcx, 32
    jns           .next_coef32_final

    ret


.section .bss
.align 32
buffer: .space 2*64*8*27

.section .rodata
.align 32
# The register to add during the reduction in order to make all numbers positive.
positive_add:
  .short 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752, 32752

