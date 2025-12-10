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
 * Parameters: x0 = a, x1 = b
 * Returns: x0 = a + b
 */
asm_add_int:
    add     x0, x0, x1      // Add x1 to x0
    ret

/* 64-bit integer subtraction
 * Parameters: x0 = a, x1 = b
 * Returns: x0 = a - b
 */
asm_sub_int:
    sub     x0, x0, x1      // Subtract x1 from x0
    ret

/* 64-bit integer multiplication
 * Parameters: x0 = a, x1 = b
 * Returns: x0 = a * b
 */
asm_mul_int:
    mul     x0, x0, x1      // Multiply x0 by x1
    ret

/* 64-bit integer division
 * Parameters: x0 = a, x1 = b
 * Returns: x0 = a / b
 * Note: Division by zero returns 0
 */
asm_div_int:
    cbz     x1, .div_by_zero // Check if divisor is zero
    sdiv    x0, x0, x1      // Signed divide x0 by x1
    ret
.div_by_zero:
    mov     x0, #0          // Return 0 on division by zero
    ret

/* Double-precision floating point addition
 * Parameters: d0 = a, d1 = b
 * Returns: d0 = a + b
 */
asm_add_double:
    fadd    d0, d0, d1      // Add d1 to d0
    ret

/* Double-precision floating point subtraction
 * Parameters: d0 = a, d1 = b
 * Returns: d0 = a - b
 */
asm_sub_double:
    fsub    d0, d0, d1      // Subtract d1 from d0
    ret

/* Double-precision floating point multiplication
 * Parameters: d0 = a, d1 = b
 * Returns: d0 = a * b
 */
asm_mul_double:
    fmul    d0, d0, d1      // Multiply d0 by d1
    ret

/* Double-precision floating point division
 * Parameters: d0 = a, d1 = b
 * Returns: d0 = a / b
 * Note: Division by zero returns +/-Infinity (IEEE 754 compliant)
 */
asm_div_double:
    fdiv    d0, d0, d1      // Divide d0 by d1
    ret

    .section .note.GNU-stack,"",@progbits
