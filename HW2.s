# ==========================================
# Himadri Saha
# Md Nezad
# EECE 4821 - Computer Architecture
# HW 2
# ==========================================

.data
# Allocate memory for array[10], initialize all to 0
array:
    .word 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

.text
main:
    # Setup loop vars
    li t1, 0              # t1 = i = 0 (loop index)
    li t2, 10             # t2 = loop limit
    la t0, array          # t0 = base address of array

main_loop:
    # -- Call fibonacci(i) --
    mv a0, t1             # a0 = i (pass as argument)
    jal fibonacci         # call fibonacci, result returned in a0

    # -- Store result into array[i] --
    slli t3, t1, 2        # t3 = i * 4 (byte offset)
    add t4, t0, t3        # t4 = address of array[i]
    sw a0, 0(t4)          # array[i] = result

    # -- Next loop iteration --
    addi t1, t1, 1        # i++
    blt t1, t2, main_loop # loop while i < 10

end:
    # Done - stay here forever
    j end


# ========================================
# Function: fibonacci(int n)
# Input: a0 = n
# Output: a0 = fibonacci(n)
# ========================================
fibonacci:
    addi sp, sp, -16      
    sw ra, 12(sp)         
    sw s0, 8(sp)          
    sw s1, 4(sp)
    sw s2, 0(sp)

    mv s0, a0             # save argument n
    li s1, 0              # first = 0
    li s2, 1              # second = 1

    mv t0, a0             # save argument n in t0
    li s1, 0              # first = 0
    li s2, 1              # second = 1

    # --- Base case (if n <= 1) return n ---
    li t1, 1
    ble t0, t1, fib_base_case

    # --- Loop from i = 2 to n ---
    li t3, 2              # i = 2

fib_loop:
    bgt t3, t0, fib_loop_end    # if i > n, exit loop
    add s0, s1, s2              # result = first + second
    mv s1, s2                   # first = second
    mv s2, s0                   # second = result
    addi t3, t3, 1              # i++
    j fib_loop

fib_loop_end:             
    mv a0, s2             # return result
    j fib_done
    
fib_base_case:
    mv a0, s0             # return n (original code)

fib_done:
    lw ra, 12(sp)         
    lw s0, 8(sp)
    lw s1, 4(sp)
    lw s2, 0(sp)
    addi sp, sp, 16
    ret