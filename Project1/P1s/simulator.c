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
  bool running = true; // to determine when to halt the simulator
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

  // initialize pc before execution
  state.pc = 0;

  // print initial state
  printState(&state);

  // execute the instructions line by line
  while (running) {
    state.reg[0] = 0; // ensure reg[0] is always 0 when executing a new line
    int thisLine = state.pc; // current line in memory being executed
    // reading opcode, regA, regB
    assembly.opcode = (state.mem[thisLine] & OPCODEMASK) >> 22;
    assembly.regA = (state.mem[thisLine] & REGAMASK) >> 19;
    assembly.regB = (state.mem[thisLine] & REGBMASK) >> 16;
    if (assembly.opcode == 0 || assembly.opcode == 1) {
      // R-type instruction (add, nor)
      assembly.destReg = state.mem[thisLine] & 0x00000007;
      if (assembly.opcode == 0) {
        // add
        state.reg[assembly.destReg] =
            state.reg[assembly.regA] + state.reg[assembly.regB];
      } else if (assembly.opcode == 1) {
        // nor
        state.reg[assembly.destReg] =
            ~(state.reg[assembly.regA] | state.reg[assembly.regB]);
      }
      // increment pc by 1 after executing R-type instruction
      state.pc += 1;
    } else if (assembly.opcode >= 2 && assembly.opcode <= 4) {
      // I-type instruction (lw, sw, beq)
      assembly.offset = convertNum(state.mem[thisLine] & 0x0000FFFF);
      if (assembly.opcode == 2) {
        // lw loads regB with the value at memory address (regA + offset)
        state.reg[assembly.regB] =
            state.mem[state.reg[assembly.regA] + assembly.offset];

        state.pc += 1;
      } else if (assembly.opcode == 3) {
        // sw stores the value in regB to memory address (regA + offset)
        state.mem[state.reg[assembly.regA] + assembly.offset] =
            state.reg[assembly.regB];

        state.pc += 1;
      } else if (assembly.opcode == 4) {
        // beq branches to (pc+1+offset) if regA and regB have the same value
        (state.reg[assembly.regA] == state.reg[assembly.regB])
            // print state before changing pc
            ? (state.pc = state.pc + 1 + assembly.offset)
            : (state.pc += 1);
      }
    } else if (assembly.opcode == 5) {
      // jalr
      state.reg[assembly.regB] = state.pc + 1;
      state.pc = state.reg[assembly.regA];
    } else if (assembly.opcode == 6) {
      // halt, which ends the simulation
      // printStats(&state);
      running = false;
      state.pc += 1;
      // breaking from the while loop, so increment pc here as special case
      state.numInstructionsExecuted += 1;
      break;
    } else if (assembly.opcode == 7) {
      // noop
      state.pc += 1;
    }
    // execution is done, do some regular tasks
    state.numInstructionsExecuted += 1;
    printState(&state);
  }

  // print the final state for end
  if (running == false) {
    printStats(&state);
    printState(&state);
  }

  // Your code ends here!
  return (0);
}

/*
 * DO NOT MODIFY ANY OF THE CODE BELOW.
 */

// \t for horizontal tab
// \n for new line
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
  // subtract 2^16 from the present value to represent a negative number
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
