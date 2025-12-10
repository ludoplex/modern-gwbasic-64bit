    .text
    .globl  asm_add_int
    .globl  asm_sub_int
    .globl  asm_mul_int
    .globl  asm_div_int
    .globl  asm_add_double
    .globl  asm_sub_double
    .globl  asm_mul_double
    .globl  asm_div_double

/* 64-bit integer addition
 * Parameters: rdi = a, rsi = b
 * Returns: rax = a + b
 */
asm_add_int:
    movq    %rdi, %rax      # Move first parameter to return register
    addq    %rsi, %rax      # Add second parameter
    ret

/* 64-bit integer subtraction
 * Parameters: rdi = a, rsi = b
 * Returns: rax = a - b
 */
asm_sub_int:
    movq    %rdi, %rax      # Move first parameter to return register
    subq    %rsi, %rax      # Subtract second parameter
    ret

/* 64-bit integer multiplication
 * Parameters: rdi = a, rsi = b
 * Returns: rax = a * b
 */
asm_mul_int:
    movq    %rdi, %rax      # Move first parameter to rax
    imulq   %rsi            # Multiply with second parameter (result in rax)
    ret

/* 64-bit integer division
 * Parameters: rdi = a, rsi = b
 * Returns: rax = a / b
 */
asm_div_int:
    movq    %rdi, %rax      # Move dividend to rax
    cqo                     # Sign extend rax to rdx:rax
    idivq   %rsi            # Divide by second parameter (quotient in rax)
    ret

/* Double-precision floating point addition
 * Parameters: xmm0 = a, xmm1 = b
 * Returns: xmm0 = a + b
 */
asm_add_double:
    addsd   %xmm1, %xmm0    # Add xmm1 to xmm0
    ret

/* Double-precision floating point subtraction
 * Parameters: xmm0 = a, xmm1 = b
 * Returns: xmm0 = a - b
 */
asm_sub_double:
    subsd   %xmm1, %xmm0    # Subtract xmm1 from xmm0
    ret

/* Double-precision floating point multiplication
 * Parameters: xmm0 = a, xmm1 = b
 * Returns: xmm0 = a * b
 */
asm_mul_double:
    mulsd   %xmm1, %xmm0    # Multiply xmm0 by xmm1
    ret

/* Double-precision floating point division
 * Parameters: xmm0 = a, xmm1 = b
 * Returns: xmm0 = a / b
 */
asm_div_double:
    divsd   %xmm1, %xmm0    # Divide xmm0 by xmm1
    ret

    .section .note.GNU-stack,"",@progbits
