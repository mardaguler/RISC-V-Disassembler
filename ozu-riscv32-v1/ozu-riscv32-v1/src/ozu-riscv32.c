#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "ozu-riscv32.h"

/***************************************************************/
/* Print out a list of commands available                      */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********OZU-RV32 Disassembler and Simulator Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                              */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                               */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                           */
/***************************************************************/
void cycle() {                                                
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
}

/***************************************************************/
/* Simulate RISC-V for n cycles                                */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                      */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/**************************************************************************************/ 
/* Dump region of memory to the terminal (make sure provided address is word aligned) */
/**************************************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal             */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < RISCV_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	
}

/***************************************************************/
/* Read a command from standard input.                         */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;

	printf("OZU-RISCV SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			runAll(); 
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting OZU-RISCV! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                   */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < RISCV_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                             */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                   */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */
	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* decode and execute instruction                           */ 
/************************************************************/

int32_t sext_32(uint32_t value, int bit_count) { // works for both signed and unsigned
    if (value & (1 << (bit_count - 1))) { // to see sign bit (MSB)
        // If sign bit is set, perform sign extension by setting all higher bits to 1
        return (value | (0xFFFFFFFF << bit_count)); // unsigned 0
    } else {
        return value;
	}
}


	//PC memorydeki adressi gosterir
void handle_instruction()
{
	/*YOU NEED TO IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	int current_ins = mem_read_32(CURRENT_STATE.PC);
	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	

	/* AND --> Masking, OR ---> Merging*/
	uint32_t opcode = current_ins & 127;
	uint32_t rd = ( current_ins >> 7) & 31; // var = (ins >> start_index_of_the_bit -1) & (2^length_of_field)
	uint32_t rs1 = ( current_ins >> 15) & 31; // bastaki sayısı shiftleyerek buluyoruz, bit sayıs o operandın bit sayıs (2^5 -1, 0 dan da baslıo cnku)
	uint32_t rs2 = ( current_ins >> 20) & 31;
	uint32_t funct3 = ( current_ins >> 12) & 7;
	uint32_t funct7 = ( current_ins >> 25) & 127;

	uint32_t immI = (current_ins >> 20) & 0b111111111111; //2^(12) - 1
	uint32_t immS = (funct7 << 5) | rd;
	uint32_t immU = (current_ins >> 12) & 0b11111111111111111111;

	uint32_t immB = (((((funct7 >> 6) << 1) | (rd & 1)) << 10) // imm[12]
	| ((funct7 & 0b111111 /*6 bits*/) << 4 )  // imm[10:5]
	| ((rd >> 1) & 0b1111 /*4 bits*/)) << 1;  // imm[4:1]
	
	uint32_t immJ = ((((((immU >> 19) << 8) // imm i başa getiriyosun
	| (immU & 0b11111111)) << 1) //imm[19:12]
	| ((immU >> 8) & 1) << 10)  
	| ((immU >> 9) & 0b1111111111)) << 1; //imm[10:1]

	uint32_t shamt = (immI & 0b11111); //  imm[5:0], 2^5 - 1 // hem unsigned hem de signed de işe yarıo (arastır bi)
	uint32_t s_shamt = (int32_t) (immI & 0b11111); // only signed


	// >> sağ shift, << sol shift

	/* 2^12 - 1 */ // 00000101010010101 // shifleyip and le masklıyoruz => 0000000000000000000001100011 misal

	switch(opcode) { // ADD opcode in binary den hex hali ,hex seklinde tanımlıoz 
		case 0x33:
		if (funct7 == 0) { // func3 ü 0 olan, ADD dan baska opcode lar de var o yuzden bi if daha acıoz içine
			if (funct3 == 0) {
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] + CURRENT_STATE.REGS[rs2]; // rs2 is alwyas in the same place
			}

			if (funct3 == 1) { // sll
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] << CURRENT_STATE.REGS[rs2];
			}

			if (funct3 == 2) { // slt
				if(sext_32(CURRENT_STATE.REGS[rs1],5) < sext_32(CURRENT_STATE.REGS[rs2],5)) {
				NEXT_STATE.REGS[rd] = 1;
				}
				else {
				NEXT_STATE.REGS[rd] = 0;
				}
			}

			if (funct3 == 3) { // sltu
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] < CURRENT_STATE.REGS[rs2];
			}

			if (funct3 == 4){ // xor
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] ^ CURRENT_STATE.REGS[rs2];
			}

			if (funct3 == 5) { // SRL
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] >> CURRENT_STATE.REGS[rs2];
			}

			if (funct3 == 6) { //or
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] | CURRENT_STATE.REGS[rs2];
			}
		}

		if (funct7 == 1) {
			if (funct3==0) { // mul
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] * CURRENT_STATE.REGS[rs2];
			}
			if (funct3==5) { // divu
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] / (uint32_t)CURRENT_STATE.REGS[rs2];
			}

			if (funct3==4) { // div
				NEXT_STATE.REGS[rd] = sext_32(CURRENT_STATE.REGS[rs1], 5) / sext_32(CURRENT_STATE.REGS[rs2],5);
			}
		}

		if(funct7 == 32) { 
			if (funct3 == 0) { //sub
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] - CURRENT_STATE.REGS[rs2];
			}
			// u unsigned - << u x[rs2]
			if (funct3 == 5 ) { // SRA (signed srl)
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] >> sext_32(CURRENT_STATE.REGS[rs2], 5);
			}
		}
		break;

		case 0b0010011: // I - types
		if (funct3 == 0) {
			printf("addi %d %d\n", sext_32(immI,12), immI);
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] + sext_32(immI,12); // length is 12 in ı type immediate
		}
		if (funct3 ==2) { //slti
			if(sext_32(CURRENT_STATE.REGS[rs1],5) < sext_32(immI,12)) {
				NEXT_STATE.REGS[rd] = 1;
			}
			else {
				NEXT_STATE.REGS[rd] = 0;
			}
		}
		if (funct3 ==3) {
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] < sext_32(immI,12);
		}
		if (funct3 ==4) {
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] ^ sext_32(immI,12);
		}
		if (funct3 ==6) {
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] | sext_32(immI,12);
		}
		if (funct3 ==7) {
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] & sext_32(immI,12); // andi
		}
		if (funct3 ==1) {
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] << shamt;
		}
		if (funct3 ==5) {
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] >> shamt;

			if (funct7 ==1) {
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] >> (int32_t)shamt;
			}
		}
		break;

		/*	U-Type	*/

		case 0b0110111:		// LUI
		NEXT_STATE.REGS[rd] = sext_32(immU << 12, 20); //DEĞİŞTİRİLDİ ??? MANTIK YANLIŞ OLABİLİR
		break;

		case 0b0010111:   	//AUPIC
		NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + sext_32(immU << 12,20);  //DEĞİŞTİRİLDİ ??? MANTIK YANLIŞ OLABİLİR
		break;

		/*	Load/Store İnstructions*/

		case 0b0000011:
		if (funct3 == 0) // LB
		{
			NEXT_STATE.REGS[rd] = sext_32(mem_read_32(CURRENT_STATE.REGS[rs1] + sext_32(immI,12)) & 0b11111111, 8); // [7:0] length(8) 2^8 - 1 // masking to get 8 bit ( mem_write_32 is 32 bit)
		}

		if (funct3 == 1) // LH
		{
			NEXT_STATE.REGS[rd] = sext_32(mem_read_32(CURRENT_STATE.REGS[rs1] + sext_32(immI,12)) & 0b111111111111111, 16); // [15:0] length(16), 2^16-1, masking to get 8 bit
		}

		if (funct3 == 2) // LW
		{
			NEXT_STATE.REGS[rd] = sext_32(mem_read_32(CURRENT_STATE.REGS[rs1] + sext_32(immI,12)) & 0b11111111111111111111111111111111, 32); // [31:0] length(32) 2^32-1 
		}

		if (funct3 == 4) // LBU
		{
			NEXT_STATE.REGS[rd] = mem_read_32((CURRENT_STATE.REGS[rs1] + sext_32(immI,12))) & 0b11111111;// [7:0] //x[rd] = M[x[rs1] + sext(offset)][7:0]
		}

		if (funct3 == 5) // LHU
		{
			NEXT_STATE.REGS[rd] = mem_read_32((CURRENT_STATE.REGS[rs1] +sext_32(immI,12))) & 0b111111111111111;// [7:0] //x[rd] = M[x[rs1] + sext(offset)][7:0]
		}
		break;

		case 0b0100011:
		if(funct3==0) { //sb
			mem_write_32(CURRENT_STATE.REGS[rs1] + sext_32(immS,12), CURRENT_STATE.REGS[rs2] & 0b11111111); //byte sext ==> [11:5] 7 bit
		}

		if(funct3==1) { //sh
			mem_write_32(CURRENT_STATE.REGS[rs1] + sext_32(immS,12), CURRENT_STATE.REGS[rs2] & 0b1111111111111111);//half
		}

		if(funct3==2) { //sw
			mem_write_32(CURRENT_STATE.REGS[rs1] + sext_32(immS,12), CURRENT_STATE.REGS[rs2] & 0b11111111111111111111111111111111);//word
		}
		break;


		/*	B-Type Format	*/
		case 0b1100011:
		if (funct3==0) { //beq
			if(CURRENT_STATE.REGS[rs1] == CURRENT_STATE.REGS[rs2])
			{
				NEXT_STATE.PC = CURRENT_STATE.PC + sext_32(immB,12);
			}
		}

		if (funct3==1) { //bne
			if(CURRENT_STATE.REGS[rs1] != CURRENT_STATE.REGS[rs2])
			{
				NEXT_STATE.PC = CURRENT_STATE.PC + sext_32(immB,12);
			}
		}

		if (funct3==4) { // blt ==========> HATA OLABİLİR
			if (sext_32(CURRENT_STATE.REGS[rs1],5) < sext_32(CURRENT_STATE.REGS[rs2], 5))
			{
				NEXT_STATE.PC = CURRENT_STATE.PC + sext_32(immB,12);
			}
		}

		if (funct3==5) { // bge
			if (sext_32(CURRENT_STATE.REGS[rs1],5) >= sext_32(CURRENT_STATE.REGS[rs2], 5))
			{
				NEXT_STATE.PC = CURRENT_STATE.PC + sext_32(immB,12);
			}
		}

		if (funct3==6) { // bltu
			if(CURRENT_STATE.REGS[rs1] < CURRENT_STATE.REGS[rs2])
			{
				NEXT_STATE.PC = CURRENT_STATE.PC + sext_32(immB,12);
			}
		}

		if (funct3==7) { // bgeu
			if(CURRENT_STATE.REGS[rs1] >= (int32_t) CURRENT_STATE.REGS[rs2])
			{
				NEXT_STATE.PC = CURRENT_STATE.PC + sext_32(immB,12);
			}
		}
		break;
		/*	J-Type format	*/
		case 0b1101111:
		NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4; //x[rd] = pc+4;
		NEXT_STATE.PC = CURRENT_STATE.PC + sext_32(immJ, 20); //pc += sext(offset)

		//CURRENT_STATE.REGS[17] // this is x17 (17.th register)

		/* ECALL */
		case 0b1110011: 
		RUN_FLAG = FALSE;
		break;

		
		case 0b1100111:
		NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4;		//t =pc+4;
		NEXT_STATE.PC = CURRENT_STATE.REGS[rd] +sext_32(immI,12);	//NEXT_STATE.PC = (CURRENT_STATE.REGS[rs1] + sext_32(immI, 12)) &  ~1; // ;  ~1 complement the value "1"
		break;

	}
}


	


/************************************************************/
/* Initialize Memory                                        */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/**********************************************************************/
/* Print the program loaded into memory (in RISC-V assembly format)   */ 
/**********************************************************************/
void print_program(){
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}


/******************************************************************************/
/* Print the instruction at given memory address (in RISC-V assembly format)  */
/******************************************************************************/
void print_instruction(uint32_t addr){
	/*YOU NEED TO IMPLEMENT THIS FUNCTION*/

	int current_ins = mem_read_32(addr); // addr değişkenini verion hep adresi yenilesin
	NEXT_STATE.PC = CURRENT_STATE.PC + 4;

	


	/* AND --> Masking, OR ---> Merging*/
	uint32_t opcode = current_ins & 127;
	uint32_t rd = ( current_ins >> 7) & 31; // var = (ins >> start_index_of_the_bit -1) & (2^length_of_field)
	uint32_t rs1 = ( current_ins >> 15) & 31; // bastaki sayısı shiftleyerek buluyoruz, bit sayıs o operandın bit sayıs (2^5 -1, 0 dan da baslıo cnku)
	uint32_t rs2 = ( current_ins >> 20) & 31;
	uint32_t funct3 = ( current_ins >> 12) & 7;
	uint32_t funct7 = ( current_ins >> 25) & 127;

	uint32_t immI = (current_ins >> 20) & 0b111111111111; //2^(12) - 1
	uint32_t immS = (funct7 << 5) | rd;
	uint32_t immU = (current_ins >> 12) & 0b11111111111111111111;

	uint32_t immB = (((((funct7 >> 6) << 1) | (rd & 1)) << 10) // imm[12]
	| ((funct7 & 0b111111 /*6 bits*/) << 4 )  // imm[10:5]
	| ((rd >> 1) & 0b1111 /*4 bits*/)) << 1;  // imm[4:1]
	
	uint32_t immJ = ((((((immU >> 19) << 8) // imm i başa getiriyosun
	| (immU & 0b11111111)) << 1) //imm[19:12]
	| ((immU >> 8) & 1) << 10)  
	| ((immU >> 9) & 0b1111111111)) << 1; //imm[10:1]

	uint32_t shamt = (immI & 0b11111); //  imm[5:0], 2^5 - 1 // hem unsigned hem de signed de işe yarıo (arastır bi)
	uint32_t s_shamt = (int32_t) (immI & 0b11111); // only signed


	switch(opcode) { // ADD opcode in binary den hex hali ,hex seklinde tanımlıoz 
		case 0b0110011:
		if (funct7 == 0) { // func3 ü 0 olan, ADD dan baska opcode lar de var o yuzden bi if daha acıoz içine
			if (funct3 == 0) {
				printf("add x%d, x%d, x%d\n",rd,rs1,rs2);
			}

			if (funct3 == 1) { // sll
				printf("sll x%d, x%d, x%d\n",rd,rs1,rs2);
			}

			if (funct3 == 2) { // slt
				printf("slt x%d, x%d, x%d\n",rd,rs1,rs2);
			}

			if (funct3 == 3) { // sltu
				printf("sltu x%d, x%d, x%d\n",rd,rs1,rs2);
			}

			if (funct3 == 4){ // xor
				printf("xor x%d, x%d, x%d\n",rd, rs1,rs2);
			}

			if (funct3 == 5) { // SRL
				printf("srl x%d, x%d, x%d\n",rd,rs1,rs2);
			}

			if (funct3 == 6) { //or
				printf("or x%d, x%d, x%d\n",rd,rs1,rs2);
			}

			if (funct3 == 7) { //and
				printf("and x%d, x%d, x%d\n",rd,rs1,rs2);
			}
		}

		if (funct7 == 1) {
			if (funct3==0) { // mul
				printf("mul x%d, x%d, x%d\n",rd,rs1,sext_32(rs2,5));
			}
			if (funct3==1) { // divu
				printf("divu x%d, x%d,x %d\n",rd,rs1,sext_32(rs2,5));
			}

			if (funct3==4) { // div
				printf("div x%d, x%d,x %d\n",rd,rs1,sext_32(rs2,5));
			}
		}

		if(funct7 == 32) { 
			if (funct3 == 7) { //sub
				printf("sub x%d, x%d, x%d\n",rd,rs1,rs2);
			}
			// SLT signed o haftaya https://msyksphinz-self.github.io/riscv-isadoc/html/rvi.html da s olarak o var. signed oluomus
			// u unsigned - << u x[rs2]
			if (funct3 == 5 ) { // SRA (signed srl)
				printf("sra x%d, x%d, x%d\n",rd, rs1, sext_32(rs2,5));
			}
		}
		break;

	case 0b0010011: // I - types
	if (funct3 == 0) { //ADDI
		 printf("addi\tx%d, x%d, 0x%d\n", rd,rs1,sext_32(immI,12));
	}
	if (funct3 ==2) {
		 printf("slti x%d, x%d, 0x%x\n", rd,rs1,sext_32(immI,12));
	}
	if (funct3 ==3) {
		 printf("sltiu x%d, x%d, 0x%x\n",rd,rs1,sext_32(immI,12));

	}
	if (funct3 ==4) {
		 printf("xori x%d, x%d, 0x%x\n", rd,rs1,sext_32(immI,12));
	}
	if (funct3 ==6) {
		 printf("ori x%d, x%d, 0x%x\n", rd,rs1,sext_32(immI,12));
	}
	if (funct3 ==7) {
		 printf("andi x%d, x%d, 0x%x\n", rd,rs1,sext_32(immI,12));
	}
	if (funct3 ==1) {
		 printf("slli x%d, x%d, 0x%x\n", rd,rs1,sext_32(immI,12));
	}
	if (funct3 ==5) {
		 printf("srli x%d, x%d, 0x%x\n", rd,rs1,sext_32(immI,12));

		if (funct7 ==1) {
		 	printf("srai x%d, x%d, 0x%x\n", rd,rs1,sext_32(immI,12));
		}
	}
	break;

	/*	U-Type	*/

	case 0b0110111:		// LUI
	printf("lui x%d, %d\n", rd,sext_32(immU,12));
	break;

	case 0b0010111:   	//AUPIC
	printf("auipc x%d, %d\n", rd,sext_32(immU,12));
	break;

	/*	Load/Store İnstructions*/

	case 0b0000011:
	if (funct3 == 0) // LB
	{
		printf("lb x%d, %d(0x%x)\n", rd,sext_32(immI,12),rs1); // immI == offseet
	}

	if (funct3 == 1) // LH
	{
		printf("lh x%d, %d(0x%x)\n", rd,sext_32(immI,12),rs1); // immI == offseet
	}

	if (funct3 == 2) // LW
	{
		printf("lw  x%d, %d(0x%x)\n", rd,sext_32(immI,12),rs1); // immI == offseet
	}

	if (funct3 == 4) // LBU
	{
		printf("lbu  x%d, %d(0x%x)\n", rd,sext_32(immI,12),rs1); // immI == offseet
	}

	if (funct3 == 5) // LHU
	{
		printf("lhu  x%d,  %d(0x%x)\n", rd,sext_32(immI,12),rs1); // immI == offseet
	}
	break;

	case 0b0100011:
	if(funct3==0) { //sb
		printf("sb x%d, %d(0x%x)\n", rs2,sext_32(immS,12),rs1); // 
	}

	if(funct3==1) { //sh
		printf("sh x%d, %d(0x%x)\n", rs2,sext_32(immS,12),rs1); // 
	}

	if(funct3==2) { //sw
		printf("sw x%d, %d(0x%x)\n", rs2,sext_32(immS,12),rs1); // 
	}
	break;


	/*	B-Type Format	*/
	case 0b1100011:
	if (funct3==0) { //beq
		printf("beq x%d, x%d, %d\n", rs1,rs2,sext_32(immB,12)); // 
	}

	if (funct3==1) { //bne
		printf("bne x%d, x%d , %d\n", rs1,rs2,sext_32(immB,12)); // 
	}

	if (funct3==4) { // blt ==========> HATA OLABİLİR
		printf("blt x%d, x%d , %d\n", rs1,rs2,sext_32(immB,12)); // 
	}

	if (funct3==5) { // bge
		printf("bge x%d, x%d , %d\n", rs1,rs2,sext_32(immB,12)); // 
	}

	if (funct3==6) { // bltu
		printf("bltu x%d, x%d , %d\n", rs1,rs2,sext_32(immB,12)); // 
	}

	if (funct3==7) { // bgeu
		printf("bne x%d, x%d , %d\n", rs1,rs2,sext_32(immB,12)); // 
	}
	break;

	/*	J-Type format	*/
	case 0b1101111:
		printf("jal x%d, %d\n",rd,sext_32(immJ,20));  
	break;

	case 0b1100111:
		printf("jalr x%d, %d\n",rd,sext_32(immI,12));  
	break;

		//CURRENT_STATE.REGS[17] // this is x17 (17.th register)
		/* ECALL */

	case 0b1110011:
		printf("ecall\n");
	break;

	}
}

/***************************************************************/
/* main()                                                      */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n********************************\n");
	printf("Welcome to OZU-RISCV SIMULATOR...\n");
	printf("*********************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
