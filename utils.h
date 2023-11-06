#ifndef UTILS
#define UTILS

/** BEGIN Exit codes **/
#define CALLOC_ERROR 2
#define BAD_ADDRESS 3
#define BAD_REGISTER 4
#define BAD_FILENAME 5
#define BAD_IFUN 6
/** END Exit codes **/

/** BEGIN Instruction IDs **/
#define HALT 0x0
#define NOP  0x1
#define RRMOVQ  0x2
#define CMOVXX  0x2
#define IRMOVQ  0x3
#define RMMOVQ  0x4
#define MRMOVQ  0x5
#define OPQ  0x6
#define JXX  0x7
#define CALL  0x8
#define RET  0x9
#define PUSHQ  0xa
#define POPQ  0xb
/** END Instruction IDs **/

/** BEGIN Function IDs **/
#define ADD  0x0
#define SUB  0x1
#define AND  0x2
#define XOR  0x3

#define UCND  0x0
#define LE  0x1
#define L  0x2
#define E  0x3
#define NE  0x4
#define GE  0x5
#define G  0x6
/** END Function IDs **/

/** BEGIN Register IDs **/
#define RSP  0x4
/** END Register IDs **/

/** BEGIN STEP MODE **/
#define NO_STEP  0
#define MACHINE_STEP  1
#define STAGE_STEP 2
/** END STEP MODE **/


FILE *parseCommandLine(int argc, char **argv, int *stepMode);

/** BEGIN types **/
typedef unsigned char byteType;
typedef long int wordType;

typedef enum {FALSE = 0,TRUE} bool;
/* 
 * Use the below functions to safely convert types to bool.
 * A direct cast may result in a subtle error.
 */
bool intToBool(int i); /* safely convert int to bool */
bool wordToBool(wordType i); /* safely convert wordType to bool */

typedef enum {STAT_AOK, STAT_HLT} status;
/** END types **/

/** BEGIN Machine State Interface Definitions **/
void initializeMemory(wordType memorySize);
void loadMemory(FILE *input);
byteType getByteFromMemory(wordType address);
void setByteInMemory(wordType address, byteType value);
wordType getWordFromMemory(wordType address);
void setWordInMemory(wordType address, wordType value);

void initializeRegisters();
wordType getRegister(int registerID);
void setRegister(int registerID, wordType value);

bool Cond(int ifun);
void setFlags(bool sf, bool zf, bool of);

status getStatus();
void setStatus(status newStatus);

wordType getPC();
void setPC(wordType value);

void incrementCycleCounter();
int getCycleCount();

void printMachineState();
/** END Machine State Interface Definitions **/

void applyStageStepMode(int stepMode, char *stageID, int icode, int ifun, int rA, int rB, wordType valC, wordType valP, 
          wordType valA, wordType valB, wordType valE, bool Cnd, wordType valM);
void applyStepMode(int stepMode);
#endif
