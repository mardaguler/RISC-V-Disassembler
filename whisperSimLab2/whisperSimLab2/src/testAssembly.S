.globl main
.text
main:

        lui    x3, 0x10000

        addi  x5, zero, 255
        add    x6, x5, x5
        add    x7, x6, x5
        addi  x28, x7, -254
        
        #;; Place a test pattern in memory
        sw     x5, 0(x3)
        sw     x6, 4(x3)
        sw     x7, 8(x3)
        sw     x28, 12(x3)

        lw     x29,  0(x3)
        lw     x30, 4(x3)
        lw     x31, 8(x3)
        lw     x4, 12(x3)

        addi  x3, x3, 16
        sw     x5, 0(x3)
        sb     x6, 4(x3)
        sh     x7, 8(x3)
        sw     x28, 12(x3)

        addi  x3, x3, 16
        lb     x12,  -4(x3)
        lh     x13,  -8(x3)
        lbu    x14,  -12(x3)
        lhu   x15,  -16(x3)
               
        #;;  Quit out 
        addi a7, zero, 0x5d
        ecall

# KEEP THE ABOVE TWO LINES and
# Put your disassembled program HERE...
