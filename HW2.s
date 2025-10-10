# ==========================================
# Himadri Saha
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
    # -- Setup stack frame --
    addi sp, sp, -16      # allocate space on stack
    sw ra, 12(sp)         # save return address
    sw s0, 8(sp)          # save s0 (used for input)
    sw s1, 4(sp)          # save s1 (first)
    sw s2, 0(sp)          # save s2 (second)

    # -- Initialize fibonacci vars --
    li s1, 0              # first = 0
    li s2, 1              # second = 1
    mv s0, a0             # s0 = n (copy input)

    # -- Handle base case --
    li t0, 1
    ble s0, t0, fib_base_case  # if n <= 1, return n

    # -- Start loop from i = 2 to n --
    li t1, 2              # t1 = i = 2
fib_loop:
    add t2, s1, s2        # result = first + second
    mv s1, s2             # first = second
    mv s2, t2             # second = result
    addi t1, t1, 1        # i++
    ble t1, s0, fib_loop  # while i <= n

    mv a0, t2             # return value = result
    j fib_done

fib_base_case:
    mv a0, s0             # return n if n <= 1

fib_done:
    # -- Restore stack and return --
    lw ra, 12(sp)         # restore return address
    lw s0, 8(sp)
    lw s1, 4(sp)
    lw s2, 0(sp)
    addi sp, sp, 16       # pop stack
    ret
