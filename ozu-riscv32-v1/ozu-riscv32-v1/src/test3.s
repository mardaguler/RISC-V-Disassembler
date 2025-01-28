        # Basic branch test
	.text

main:
        #;;  Set a base address
        lui    x3, 0x10000
l_0:    
        addi x5, zero, 1
        j l_1
        addi x10, x10, 0xf0
        ori x0, x0, 0
        addi x5, zero, 100
        ecall        
l_1:
        bne zero, zero, l_3
        ori x0, x0, 0
        addi x6, zero, 0xFF
l_2:
        beq zero, zero, l_4
        ori x0, x0, 0
        # Should not reach here
        addi x30, zero, 0xFF
l_3:
        # Should not reach here
        addi x31, zero, 0xFF
l_4:
        addi a7, zero, 0x5d
        ecall
        
