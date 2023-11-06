#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "utils.h"

/** BEGIN Machine State Definition **/
static byteType *memory = NULL;
static int memorySizeInBytes = 0;

#define REGISTER_COUNT 15
static wordType registers[REGISTER_COUNT];

static bool signFlag = FALSE;
static bool zeroFlag = FALSE;
static bool overflowFlag = FALSE;

static status stat = STAT_AOK;

static wordType pc = 0x0;
static int cycleCounter = 0;
/** END Machine State Definition **/

static wordType getMemorySizeInBytes();
static void isGoodAddress(int address);
static void printMemory();

static void printRegisters();
static void printFlags();
static void printStatus();
static void printPC();

bool intToBool(int i) {
  return i != 0;
}

bool wordToBool(wordType i) {
  return i != 0;
}

/** BEGIN <file>.yo input to memory **/
#define MAX_LINE_LENGTH 1024
#define SEPARATORS " :x"
#define BAD_NUMBER_ARGS 1
static int hexCharacter2HexDigit(char c);
static int getAddress(char* addressAsString);
static unsigned char getByte(char *nibbles);
static void getAndSetBytesInMemory(int address, char *nibbles);
static void parseLine(char *line);
/** END <file.>yo input to memory **/

/** BEGIN Machine State Definition **/
void initializeMemory(wordType memorySize) {
  memorySizeInBytes = memorySize;
  memory = (byteType*)calloc(memorySize, sizeof(byteType));
  if (memory == NULL) {
    fprintf(stderr, "ERROR: failed to initialize memory\n");
    exit(CALLOC_ERROR);
  }
}

static wordType getMemorySizeInBytes() {
  return memorySizeInBytes;
}

static void isGoodAddress(int address) {
  if (address < 0 || address >= memorySizeInBytes) {
    fprintf(stderr, "ERROR: bad address 0x%0x\n", address);
    exit(BAD_ADDRESS);
  } 
}

byteType getByteFromMemory(wordType address) {
  isGoodAddress(address);
  return memory[address];
}

void setByteInMemory(wordType address, byteType value) {
  isGoodAddress(address);
  memory[address] = value;
}

wordType getWordFromMemory(wordType address) {
  isGoodAddress(address + sizeof(wordType) - 1);
  return *((wordType *)(memory + address));
}

void setWordInMemory(wordType address, wordType value) {
  isGoodAddress(address + sizeof(wordType) - 1);
  *((wordType *)(memory + address)) = value;
}

static void printBytesInMemoryOrder(wordType value) {
  byteType *ptr = (byteType *)&value;
  byteType *end = ptr + sizeof(wordType);
  while(ptr < end) {
    printf("%02x", *ptr++);
  }
}

static void printMemory() {
  wordType memorySizeInBytes = getMemorySizeInBytes();
  wordType wordSizeInBytes = sizeof(wordType);
  wordType value = 0;
  for (wordType i = 0 ; i < memorySizeInBytes ; i += wordSizeInBytes) {
    isGoodAddress(i);
    value = getWordFromMemory(i); 
    if (value == 0) {
      continue;
    }
    printf("0x%04lx: ", i);
    printBytesInMemoryOrder(value);
    printf("\n");
  }
}

void initializeRegisters() {
  memset(registers, 0, sizeof(wordType) * REGISTER_COUNT);
}

static void isGoodRegisterIndex(int registerIndex) {
  if (registerIndex < 0 || registerIndex >= REGISTER_COUNT) {
    fprintf(stderr, "ERROR: bad register 0x%0x\n", registerIndex);
    exit(BAD_REGISTER);
  } 
}

wordType getRegister(int registerIndex) {
  isGoodRegisterIndex(registerIndex);
  return registers[registerIndex];
}

void setRegister(int registerIndex, wordType value) {
  isGoodRegisterIndex(registerIndex);
  registers[registerIndex] = value;
}

static const char* const registerNames[] = {"%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi", "%r08", "%r09", "%r10", "%r11", "%r12", "%r13", "%r14"}; 

static void printRegisters() {
  wordType value = 0;
  for (int i = 0 ; i < REGISTER_COUNT ; ++i) {
    isGoodRegisterIndex(i);
    value = getRegister(i);
    if (value != 0) {
      printf("%s: 0x%016lx\n", registerNames[i], getRegister(i));
    }
  }
}

static bool isE() {
  return zeroFlag;
}

static bool isNE() {
  return !isE();
}

static bool isLE() {
  return signFlag ^ overflowFlag || zeroFlag;
}

static bool isL() {
  return signFlag ^ overflowFlag;
}

static bool isGE() {
  return !(signFlag ^ overflowFlag);
}

static bool isG() {
  return !(signFlag ^ overflowFlag) && !zeroFlag;
}

bool Cond(int ifun) {
  if (ifun == UCND) {
    return TRUE;
  } else if (ifun == LE) {
    return isLE();
  } else if (ifun == L) {
    return isL();
  } else if (ifun == E) {
    return isE();
  } else if (ifun == NE) {
    return isNE();
  } else if (ifun == GE) {
    return isGE();
  } else if (ifun == G) {
    return isG();
  } else {
    fprintf(stderr, "ERROR: unknown ifun (0x%01x) in doOPQ\n", ifun);
    exit(BAD_IFUN);
  }
}

void setFlags(bool sf, bool zf, bool of) {
  signFlag = sf;
  zeroFlag = zf;
  overflowFlag = of;
}

static void printFlags() {
  printf("SF: %d \tZF: %d\t OF: %d\n", signFlag, zeroFlag, overflowFlag);
}

void setStatus(status newStatus) {
  stat = newStatus;
}

status getStatus() {
  return stat;
}

static void printStatus() {
  printf("STAT: ");
  if (stat == STAT_AOK) {
    printf("AOK\n");
  } else {
    printf("HLT\n");
  }
}

void setPC(wordType value) {
  isGoodAddress(value);
  pc = value;
}

wordType getPC() {
  return pc;
}

static void printPC() {
  printf("PC: 0x%04x\n", (int)pc);
}

void incrementCycleCounter() {
  ++cycleCounter;
}

int getCycleCount() {
  return cycleCounter;
}

void printMachineState() {
  printf("\nCycle Count: %d\n\n", getCycleCount());
  printMemory();
  printf("\n");
  printRegisters();
  printf("\n");
  printFlags();
  printStatus();
  printPC();
}
/** END Machine State Definition **/


/** BEGIN <file>.yo input to memory **/
FILE *parseCommandLine(int argc, char **argv, int *stepMode) {
  FILE *inputFile = NULL;

  if (argc != 2 && argc != 3) {
    printf("Usage: %s <yo-file> [-m|-s]\n", argv[0]);
    exit(BAD_NUMBER_ARGS);
  }

  inputFile = fopen(argv[1], "r");
  if (inputFile == NULL) {
    fprintf(stderr, "ERROR: unable to open %s for reading\n", argv[1]);
    exit(BAD_FILENAME);
  }

  *stepMode = NO_STEP;
  if (argc == 3 && (strcmp(argv[2], "-m") == 0 )) {
    *stepMode = MACHINE_STEP;
  } else if (argc == 3 && (strcmp(argv[2], "-s") == 0 )) {
    *stepMode = STAGE_STEP;
  }

  return inputFile;
}

int hexCharacter2HexDigit(char c) {
  if (isdigit((int)c)) {
    return c - '0';
  }
    
  if (isupper((int)c)) {
    return c - 'A' + 10;
  }
	
	return c - 'a' + 10;
}

int getAddress(char* addressAsString) {
  int address = 0;

  while (*addressAsString != '\0') {
    address = (address << 4) + hexCharacter2HexDigit(*addressAsString);
    addressAsString += 1;
  }
  
  return address;
}

unsigned char getByte(char *nibbles) {
  byteType byteType = hexCharacter2HexDigit(*nibbles);
  return (byteType << 4) + hexCharacter2HexDigit(*(nibbles + 1));
}

void getAndSetBytesInMemory(int address, char *nibbles) {
  byteType byteType = 0;
#ifdef DEBUG
  int i = address;
#endif

  while (*nibbles != '\0') {
    byteType = getByte(nibbles);
    isGoodAddress(address);
    setByteInMemory(address, byteType);
    nibbles += 2;
    address += 1;
  }

  #ifdef DEBUG
    printf("0x%04x: ", i);
    while (i < address) {    
      printf("%02x", getByteFromMemory(i));
      ++i;
    }
    printf("\n");
  #endif
}

void parseLine(char *line) {
  char *token = strtok(line, SEPARATORS);
  int address = 0;

  // Expecting a '0' character as in '0x0000' for the address
  if (token == NULL || !isxdigit(*token)) {
    return;
  }

  token = strtok(NULL, SEPARATORS);
  // expecting hex-digit for address
  if (token != NULL && isxdigit(*token)) {
    address = getAddress(token);
  }

  token = strtok(NULL, SEPARATORS);
  // expecting hex-digit for bytes
  if (token != NULL && isxdigit(*token)) { 
    getAndSetBytesInMemory(address, token);
  }
}

void loadMemory(FILE *input) {
  char readBuffer[MAX_LINE_LENGTH];
  
  while (fgets(readBuffer, MAX_LINE_LENGTH, input)) {
    parseLine(readBuffer);
  }
}

void applyStageStepMode(int stepMode, char *stageID, int icode, int ifun, int rA, int rB, wordType valC, wordType valP, 
          wordType valA, wordType valB, wordType valE, bool Cnd, wordType valM) {
  if (stepMode != STAGE_STEP) {
    return;
  }
  
  printf("%s:\n", stageID);
  printf("icode:ifun = %01x:%01x  rA:rB = %01x:%01x  ", icode, ifun, rA, rB);
  printf("valC = 0x%016lx  valP = 0x%016lx\n", valC, valP);
  printf("valA = 0x%016lx  valB = 0x%016lx\n", valA, valB);
  printf("valE = 0x%016lx  Cnd = %d\n", valE, Cnd);
  printf("valM = 0x%016lx\n", valM);

  getchar();
}

void applyStepMode(int stepMode) {
  if (stepMode == NO_STEP) {
    return;
  }

  printMachineState();
  getchar();
}
/** END <file.>yo input to memory **/

