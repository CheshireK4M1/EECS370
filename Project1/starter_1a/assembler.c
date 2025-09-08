/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);
void toBinary(unsigned int, char *, int);

struct cmdInfo {
  char label[100];
  char opcode[100];
  char arg0[100];
  char arg1[100];
  char arg2[100];
};

struct labelInfo {
  char label[100];
  int address;
};

int main(int argc, char **argv) {
  char *inFileString, *outFileString;
  FILE *inFilePtr, *outFilePtr;
  char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
      arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
  struct cmdInfo cmdList[MAXLINELENGTH];
  struct labelInfo labelList[MAXLINELENGTH];
  char presentline[MAXLINELENGTH];
  int lineCount = 0, labelCount = 0;
  char *opcodes[] = {"add",  "nor",  "lw",   "sw",   "beq",
                     "jalr", "halt", "noop", ".fill"};

  if (argc != 3) {
    printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
           argv[0]);
    exit(1);
  }

  inFileString = argv[1];
  outFileString = argv[2];
  label[0] = '\0';

  inFilePtr = fopen(inFileString, "r");
  if (inFilePtr == NULL) {
    printf("error in opening %s\n", inFileString);
    exit(1);
  }

  // Check for blank lines in the middle of the code.
  checkForBlankLinesInCode(inFilePtr);

  outFilePtr = fopen(outFileString, "w");
  if (outFilePtr == NULL) {
    printf("error in opening %s\n", outFileString);
    exit(1);
  }

  // by here the file should be ok to read and process
  while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
    // store the present assembly line into struct
    strcpy(cmdList[lineCount].opcode, opcode);
    strcpy(cmdList[lineCount].arg0, arg0);
    strcpy(cmdList[lineCount].arg1, arg1);
    strcpy(cmdList[lineCount].arg2, arg2);

    // check if there is a label. If so, take down its name and address
    if (label[0] != '\0') {
      strcpy(cmdList[lineCount].label, label);
      strcpy(labelList[labelCount].label, label);
      labelList[labelCount].address = lineCount;
      labelCount += 1;
    } else {
      cmdList[lineCount].label[0] = '\0';
    }
    lineCount += 1;
  }

  // reading is done, now start processing
  for (int lineNum = 0; lineNum < lineCount; lineNum++) {
    int opcodeIndex = -1;
    long decimal = 0;
    // determine the opcode for translation
    bool found = false;
    for (int j = 0; j < 9; j++) {
      if (!strcmp(cmdList[lineNum].opcode, opcodes[j])) {
        opcodeIndex = j;
        found = true;
        break;
      }
    }
    if (!found) {
      exit(1);
    }

    // start translating to machine code
    char machineCodeOut[32] = "0000000";
    unsigned int machineCodeMask = 0b111;
    char buffer[4];
    int finalOutput = 0;
    if (opcodeIndex != 8) {
      if (opcodeIndex == 0 || opcodeIndex == 1) { // add and nor
        // opcode
        toBinary(opcodeIndex, buffer, 3);
        strcat(machineCodeOut, buffer);
        // reg1
        toBinary(strtol(cmdList[lineNum].arg0, NULL, 10), buffer, 3);
        strcat(machineCodeOut, buffer);
        // reg2
        toBinary(strtol(cmdList[lineNum].arg1, NULL, 10), buffer, 3);
        strcat(machineCodeOut, buffer);
        // unused bits
        strcat(machineCodeOut, "0000000000000");
        // destReg
        if (isNumber(cmdList[lineNum].arg2)) {
          toBinary(strtol(cmdList[lineNum].arg2, NULL, 10), buffer, 3);
          strcat(machineCodeOut, buffer);
        } else {
          // find the address of the label
          bool labelFound = false;
          for (int j = 0; j < labelCount; j++) {
            if (!strcmp(cmdList[lineNum].arg2, labelList[j].label)) {
              toBinary(labelList[j].address, buffer, 3);
              strcat(machineCodeOut, buffer);
              labelFound = true;
              break;
            }
          }
          if (!labelFound) {
            exit(1);
          }
        }
      } else if (opcodeIndex >= 2 && opcodeIndex <= 4) { // lw, sw, beq
        char offsetField[17];
        // opcode
        toBinary(opcodeIndex, buffer, 3);
        strcat(machineCodeOut, buffer);
        // reg1
        toBinary(strtol(cmdList[lineNum].arg0, NULL, 10), buffer, 3);
        strcat(machineCodeOut, buffer);
        // reg2
        toBinary(strtol(cmdList[lineNum].arg1, NULL, 10), buffer, 3);
        strcat(machineCodeOut, buffer);
        // offsetField
        if (isNumber(cmdList[lineNum].arg2)) {
          int offset = strtol(cmdList[lineNum].arg2, NULL, 10);
          toBinary(offset & 0xFFFF, offsetField, 16);
          strcat(machineCodeOut, offsetField);
        } else {
          // find the address of the label
          bool labelFound = false;
          for (int j = 0; j < labelCount; j++) {
            if (!strcmp(cmdList[lineNum].arg2, labelList[j].label)) {
              if (opcodeIndex == 4) { // special offset calculation for beq
                int offset = labelList[j].address - (lineNum + 1);
                toBinary(offset & 0xFFFF, offsetField, 16);
                strcat(machineCodeOut, offsetField);
                labelFound = true;
                break;
              } else {
                toBinary(labelList[j].address, offsetField, 16);
                strcat(machineCodeOut, offsetField);
                labelFound = true;
                break;
              }
            }
          }
          if (!labelFound) {
            exit(1);
          }
        }
      } else if (opcodeIndex == 5) { // jalr
        // opcode
        toBinary(opcodeIndex, buffer, 3);
        strcat(machineCodeOut, buffer);
        // reg1
        toBinary(strtol(cmdList[lineNum].arg0, NULL, 10), buffer, 3);
        strcat(machineCodeOut, buffer);
        // reg2
        toBinary(strtol(cmdList[lineNum].arg1, NULL, 10), buffer, 3);
        strcat(machineCodeOut, buffer);
        // unused bits
        strcat(machineCodeOut, "0000000000000000000");
      } else if (opcodeIndex == 6 || opcodeIndex == 7) { // halt and noop
        // opcode
        toBinary(opcodeIndex, buffer, 3);
        strcat(machineCodeOut, buffer);
        // unused bits
        strcat(machineCodeOut, "0000000000000000000000");
      } else {
        exit(1);
      }
      // Actually it is not correct to convert variable type here
      // Because there could be an overflow for int if the long value is too
      // large
      finalOutput = strtol(machineCodeOut, NULL, 2);
      printHexToFile(outFilePtr, finalOutput);
    } else { //.fill as a special case
      if (isNumber(cmdList[lineNum].arg0)) {
        decimal = strtol(cmdList[lineNum].arg0, NULL, 10);
        unsigned long mask = 0xFFFFFFFF;
        finalOutput = (int)(decimal & mask);
        printHexToFile(outFilePtr, finalOutput);
      } else {
        for (int j = 0; j < labelCount; j++) {
          if (!strcmp(cmdList[lineNum].arg0, labelList[j].label)) {
            finalOutput = (int)(labelList[j].address & 0xFFFFFFFF);
            printHexToFile(outFilePtr, finalOutput);
            break;
          }
        }
      }
    }
  }
  return (0);
}

// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line) {
  char whitespace[4] = {'\t', '\n', '\r', ' '};
  int nonempty_line = 0;
  for (int line_idx = 0; line_idx < strlen(line); ++line_idx) {
    int line_char_is_whitespace = 0;
    for (int whitespace_idx = 0; whitespace_idx < 4; ++whitespace_idx) {
      if (line[line_idx] == whitespace[whitespace_idx]) {
        line_char_is_whitespace = 1;
        break;
      }
    }
    if (!line_char_is_whitespace) {
      nonempty_line = 1;
      break;
    }
  }
  return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of
// the file. Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
  char line[MAXLINELENGTH];
  int blank_line_encountered = 0;
  int address_of_blank_line = 0;
  rewind(inFilePtr);

  for (int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL;
       ++address) {
    // Check for line too long
    if (strlen(line) >= MAXLINELENGTH - 1) {
      printf("error: line too long\n");
      exit(1);
    }

    // Check for blank line.
    if (lineIsBlank(line)) {
      if (!blank_line_encountered) {
        blank_line_encountered = 1;
        address_of_blank_line = address;
      }
    } else {
      if (blank_line_encountered) {
        printf("Invalid Assembly: Empty line at address %d\n",
               address_of_blank_line);
        exit(2);
      }
    }
  }
  rewind(inFilePtr);
}

/*
 * NOTE: The code defined below is not to be modifed as it is implimented
 * correctly.
 */

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory
 * already allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2) {
  char line[MAXLINELENGTH];
  char *ptr = line;

  /* delete prior values */
  label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

  /* read the line from the assembly-language file */
  if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
    /* reached end of file */
    return (0);
  }

  /* check for line too long */
  if (strlen(line) == MAXLINELENGTH - 1) {
    printf("error: line too long\n");
    exit(1);
  }

  // Ignore blank lines at the end of the file.
  if (lineIsBlank(line)) {
    return 0;
  }

  /* is there a label? */
  ptr = line;
  if (sscanf(ptr, "%[^\t\n ]", label)) {
    /* successfully read label; advance pointer over the label */
    ptr += strlen(label);
  }

  /*
   * Parse the rest of the line.  Would be nice to have real regular
   * expressions, but scanf will suffice.
   */
  sscanf(ptr,
         "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r "
         "]%*[\t\n\r ]%[^\t\n\r ]",
         opcode, arg0, arg1, arg2);

  return (1);
}

static inline int isNumber(char *string) {
  int num;
  char c;
  /*
  sscanf returns the number of successfully matched items
  before its firstfailure
  */
  return ((sscanf(string, "%d%c", &num, &c)) == 1);
}

// Prints a machine code word in the proper hex format to the file
static inline void printHexToFile(FILE *outFilePtr, int word) {
  fprintf(outFilePtr, "0x%08X\n", word);
}

// below are self-defined functions
// convert an int into a bit string
void toBinary(unsigned int value, char *buffer, int width) {
  for (int i = width - 1; i >= 0; i--) {
    buffer[width - 1 - i] = (value & (1u << i)) ? '1' : '0';
  }
  buffer[width] = '\0'; // null terminator
}
