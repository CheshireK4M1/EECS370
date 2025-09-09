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
bool checkInt(char *, char *);
bool checkRegRange(char *);

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

  // this bool value is for checking if we have .fill opcode in the line
  // if so, there should be only .fill opcode from then on
  bool fillEncountered = false;

  // by here the file should be ok to read and process
  while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {

    // store the present assembly line into struct
    strcpy(cmdList[lineCount].opcode, opcode);
    strcpy(cmdList[lineCount].arg0, arg0);
    strcpy(cmdList[lineCount].arg1, arg1);
    strcpy(cmdList[lineCount].arg2, arg2);

    // switch to true if .fill is encountered
    (strcmp(cmdList[lineCount].opcode, ".fill") == 0) ? (fillEncountered = true)
                                                      : (void)0;
    // if .fill has been encountered, all the following lines must be .fill
    if (fillEncountered) {
      (strcmp(cmdList[lineCount].opcode, ".fill") == 0) ? (void)0 : exit(1);
      //(checkInt(arg0, arg0)) ? (void)0 : exit(1);
    } else {
      // check if the register arguments are valid numbers
      (checkInt(arg0, arg1)) ? (void)0 : exit(1);

      // check if the first two reg values are within the range [0,7]
      // arg2 could be a label, so we do not check it here
      (checkRegRange(arg0) && checkRegRange(arg1)) ? (void)0 : exit(1);
    }

    // check if there is a label. If so, take down its name and address
    if (label[0] != '\0') {
      for (int labelCheck = 0; labelCheck < labelCount; labelCheck++) {
        if (!strcmp(label, labelList[labelCheck].label)) {
          // deal with duplicate label
          exit(1);
        }
      }
      strcpy(cmdList[lineCount].label, label);
      strcpy(labelList[labelCount].label, label);
      labelList[labelCount].address = lineCount;
      labelCount += 1;
    } else {
      cmdList[lineCount].label[0] = '\0';
    }
    lineCount += 1;
  }
  // note here float and double are still recorded identically as strings

  // reading is done, now start processing
  for (int lineNum = 0; lineNum < lineCount; lineNum++) {
    int opcodeIndex = -1;
    long long decimal = 0;
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
      // deal with unrecognized opcode
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
          // destReg value for add and nor must be within range [0,7]
          (checkRegRange(cmdList[lineNum].arg2)) ? (void)0 : exit(1);
          toBinary(strtol(cmdList[lineNum].arg2, NULL, 10), buffer, 3);
          strcat(machineCodeOut, buffer);
        } else {
          exit(1);
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
          // see if the offset is out of 16bit range
          (offset > 32767 || offset < -32768) ? exit(1) : (void)0;
          toBinary(offset & 0xFFFF, offsetField, 16);
          strcat(machineCodeOut, offsetField);
        } else {
          // find the address of the label
          // no need to check the range of address here because we always
          // have less than 1000 lines
          bool labelFound = false;
          for (int j = 0; j < labelCount; j++) {
            if (!strcmp(cmdList[lineNum].arg2, labelList[j].label)) {
              if (opcodeIndex == 4) { // special offset calculation for beq
                int offset = labelList[j].address - (lineNum + 1);
                toBinary(offset & 0xFFFF, offsetField, 16);
                strcat(machineCodeOut, offsetField);
                labelFound = true;
                break;
              } else { // normal offset calculation for lw and sw
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
        decimal = strtoll(cmdList[lineNum].arg0, NULL, 10);
        // check if the decimal is out of 32bit range
        if (decimal > 2147483647 || decimal < -2147483648) {
          // printf("%s\n", "exceeding 32 bit range");
          exit(1);
        }
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
  // return value will be 1 if the string is a number
  // and 0 if it have 0 or more than 1 successfully matched items
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

// check if a string is a valid int
bool checkInt(char *arg0_check, char *arg1_check) {
  char *endptr_0, *endptr_1;
  strtol(arg0_check, &endptr_0, 10);
  strtol(arg1_check, &endptr_1, 10);
  if (*endptr_0 != '\0' || *endptr_1 != '\0') {
    // having the endptr pointing to char besides '\0' mean that
    // there are invalid char in the string
    return false;
  }
  return true;
}

// check if the value of the reg arg is within range [0,7]
bool checkRegRange(char *reg_check) {
  int reg_value = strtol(reg_check, NULL, 10);
  if (reg_value < 0 || reg_value > 7) {
    return false;
  }
  return true;
}
