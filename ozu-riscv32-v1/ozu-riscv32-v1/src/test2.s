        # Basic arithmetic instructions
        # No overflow exceptions should occur
	.text
main:   
        addi    x2, zero, 1024
        add	x3, x2, x2
        or      x4, x3, x2
        ori     x5, x4, 321
        addi    x5, zero, 1234
        sll     x6, x5, 3
        slt     x6, x5, x4
        addi	x7, x6, 999
        sub	x8, x7, x2
        xor     x9, x4, x3
        xori    x10, x2, 255
        srl     x11, x6, 5
        sra     x12, x6, 4
        and     x13, x11, x5
        andi    x14, x4, 100
        auipc   x15, 12345
        lui     x17, 100
        mul     x18, x2, x3
        divu    x19, x18, x3
        
        #;;  Quit out 
        addi a7, zero, 0x5d
        ecall
        
        
                        
