#include <stdint.h>

#define FALSE 0
#define TRUE  1

/******************************************************************************/
/* RISCV memory layout                                                        */
/******************************************************************************/
#define MEM_TEXT_BEGIN  0x00010000
#define MEM_TEXT_END      0x10000000
/*Memory address 0x10000000 to 0xBFFFFFFF access by $gp*/
#define MEM_DATA_BEGIN  0x10000000
#define MEM_DATA_END   0xBFFFFFFF

/*stack and data segments occupy the same memory space. Stack grows backward (from higher address to lower address) */
#define MEM_STACK_BEGIN 0xBFFFFFFF
#define MEM_STACK_END  0x10000000

typedef struct {
	uint32_t begin, end;
	uint8_t *mem;
} mem_region_t;

/* memory will be dynamically allocated at initialization */
mem_region_t MEM_REGIONS[] = {
	{ MEM_TEXT_BEGIN, MEM_TEXT_END, NULL },
	{ MEM_DATA_BEGIN, MEM_DATA_END, NULL }
};

#define NUM_MEM_REGION 2
#define RISCV_REGS 32

typedef struct CPU_State_Struct {

  uint32_t PC;		    /* program counter */
  uint32_t REGS[RISCV_REGS]; /* register file. */
} CPU_State;



/***************************************************************/
/* CPU State info.                                             */
/***************************************************************/

CPU_State CURRENT_STATE, NEXT_STATE;
int RUN_FLAG;	/* run flag*/
uint32_t INSTRUCTION_COUNT;
uint32_t PROGRAM_SIZE; /*in words*/

char prog_file[32]; /*name of input file*/


/***************************************************************/
/* Function Declerations.                                                                                                */
/***************************************************************/
void help();
uint32_t mem_read_32(uint32_t address);
void mem_write_32(uint32_t address, uint32_t value);
void cycle();
void run(int num_cycles);
void runAll();
void mdump(uint32_t start, uint32_t stop) ;
void rdump();
void handle_command();
void reset();
void init_memory();
void load_program();
void handle_instruction(); /*YOU SHOULD IMPLEMENT THIS*/
void initialize();
void print_program(); /*YOU SHOULD IMPLEMENT THIS*/
void print_instruction(uint32_t);

