/*
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 *
 * Make sure to NOT modify printState or any of the associated functions
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DO NOT CHANGE THE FOLLOWING DEFINITIONS

// Machine Definitions
#define MEMORYSIZE                                                             \
  65536 /* maximum number of words in memory (maximum number of lines in a     \
           given file)*/
#define NUMREGS 8 /*total number of machine registers [0,7]*/

// File Definitions
#define MAXLINELENGTH                                                          \
  1000 /* MAXLINELENGTH is the max number of characters we read */
#define MAXLINENUM 100 /* MAXLINENUM is the max number of lines we read */
// below are the masking number applicable to aal 8 instructions
#define OPCODEMASK 0x01C00000
#define REGAMASK 0x00380000
#define REGBMASK 0x00070000

typedef struct stateStruct {
  int pc;
  int mem[MEMORYSIZE];
  int reg[NUMREGS];
  int numMemory; // counts the number of memory locations with data, and serves
                 // as the index for memory lines
  int numInstructionsExecuted; // counts the number of instructions executed
} stateType;

typedef struct assemblyStruct {
  int opcode;
  int regA;
  int regB;
  int destReg;
  int offset;
} assemblyLine;

void printState(stateType *);

void printStats(stateType *);

static inline int convertNum(int32_t);

int main(int argc, char **argv) {
  char line[MAXLINELENGTH] = {0};
  stateType state = {0};
  assemblyLine assembly = {0};
  FILE *filePtr;

  if (argc != 2) {
    printf("error: usage: %s <machine-code file>\n", argv[0]);
    exit(1);
  }

  filePtr = fopen(argv[1], "r");
  if (filePtr == NULL) {
    printf("error: can't open file %s , please ensure you are providing the "
           "correct path",
           argv[1]);
    perror("fopen");
    exit(2);
  }

  /* read the entire machine-code file into memory */
  for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
       state.numMemory++) {
    if (state.numMemory >= MEMORYSIZE) {
      fprintf(stderr, "exceeded memory size\n");
      exit(2);
    }
    if (sscanf(line, "%x", state.mem + state.numMemory) != 1) {
      fprintf(stderr, "error in reading address %d\n", state.numMemory);
      exit(2);
    }
    printf("mem[ %d ] 0x%08X\n", state.numMemory, state.mem[state.numMemory]);
  }

  // execute the instructions line by line
  for (int memLine = 0; memLine < state.numMemory; memLine++) {
    // reading opcode, regA, regB
    assembly.opcode = (state.mem[memLine] & OPCODEMASK) >> 22;
    assembly.regA = (state.mem[memLine] & REGAMASK) >> 19;
    assembly.regB = (state.mem[memLine] & REGBMASK) >> 16;
    if (assembly.opcode == 0 || assembly.opcode == 1) {
      // R-type instruction (add, nor)
      assembly.destReg = state.mem[memLine] & 0x00000007;
    } else if (assembly.opcode >= 2 && assembly.opcode <= 4) {
      // I-type instruction (lw, sw, beq)
      assembly.offset = convertNum(state.mem[memLine] & 0x0000FFFF);
    } // no special value to extract for jalr, halt and noop
  }

  // Your code ends here!
  printf("%s", "printing done\n");
  return (0);
}

/*
 * DO NOT MODIFY ANY OF THE CODE BELOW.
 */

void printState(stateType *statePtr) {
  int i;
  printf("\n@@@\nstate:\n");
  printf("\tpc %d\n", statePtr->pc);
  printf("\tmemory:\n");
  for (i = 0; i < statePtr->numMemory; i++) {
    printf("\t\tmem[ %d ] 0x%08X\n", i, statePtr->mem[i]);
  }
  printf("\tregisters:\n");
  for (i = 0; i < NUMREGS; i++) {
    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
  }
  printf("end state\n");
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) {
  return num - ((num & (1 << 15)) ? 1 << 16 : 0);
}

/*
 * print end of run statistics like in the spec. **This is not required**,
 * but is helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print "@@@" or "end state" in this function
 */
void printStats(stateType *statePtr) {
  printf("machine halted\n");
  printf("total of %d instructions executed\n",
         statePtr->numInstructionsExecuted);
  printf("final state of machine:\n");
}

/*
 * Write any helper functions that you wish down here.
 */
