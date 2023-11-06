#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

const int MAX_MEM_SIZE  = (1 << 13);

void fetchStage(int *icode, int *ifun, int *rA, int *rB, wordType *valC, wordType *valP) {
  wordType pc = getPC();

  // icode:ifun
  *icode = (getByteFromMemory(pc) & 0xf0) >> 4;
  *ifun = getByteFromMemory(pc) & 0x0f;

  // rA:rB
  // valC
  switch (*icode) {
    case HALT:
      pc += 1;
      break;

    case NOP:
      pc += 1;
      break;

    case RRMOVQ: // CMOVXX
      *rA = (getByteFromMemory(pc + 1) & 0xf0) >> 4;
      *rB = getByteFromMemory(pc + 1) & 0x0f;
      pc += 2;
      break;

    case IRMOVQ:
      *rA = (getByteFromMemory(pc + 1) & 0xf0) >> 4;
      *rB = getByteFromMemory(pc + 1) & 0x0f;
      *valC = getWordFromMemory(pc + 2);
      pc += 10;
      break;
    
    case RMMOVQ:
      *rA = (getByteFromMemory(pc + 1) & 0xf0) >> 4;
      *rB = getByteFromMemory(pc + 1) & 0x0f;
      *valC = getWordFromMemory(pc + 2);
      pc += 10;
      break;
    
    case MRMOVQ:
      *rA = (getByteFromMemory(pc + 1) & 0xf0) >> 4;
      *rB = getByteFromMemory(pc + 1) & 0x0f;
      *valC = getWordFromMemory(pc + 2);
      pc += 10;
      break;
    
    case OPQ:
      *rA = (getByteFromMemory(pc + 1) & 0xf0) >> 4;
      *rB = getByteFromMemory(pc + 1) & 0x0f;
      pc += 2;
      break;
    
    case JXX: // JMP
      *valC = getWordFromMemory(pc + 1);
      pc += 9;
      break;
    
    case CALL:
      *valC = getWordFromMemory(pc + 1);
      pc += 9;
      break;
    
    case RET:
      pc += 1;
      break;
    
    case PUSHQ:
      *rA = (getByteFromMemory(pc + 1) & 0xf0) >> 4;
      *rB = getByteFromMemory(pc + 1) & 0x0f;
      pc += 2;
      break;
    
    case POPQ:
      *rA = (getByteFromMemory(pc + 1) & 0xf0) >> 4;
      *rB = getByteFromMemory(pc + 1) & 0x0f;
      pc += 2;
      break;
    
    default:
      break;
  }

  // valP
  *valP = pc;
}

void decodeStage(int icode, int rA, int rB, wordType *valA, wordType *valB) {
  switch (icode) {
    case HALT:
      break;

    case NOP:
      break;

    case RRMOVQ: // CMOVXX
      *valA = getRegister(rA);
      break;

    case IRMOVQ:
      break;
    
    case RMMOVQ:
      *valA = getRegister(rA);
      *valB = getRegister(rB);
      break;
    
    case MRMOVQ:
      *valB = getRegister(rB);
      break;
    
    case OPQ:
      *valA = getRegister(rA);
      *valB = getRegister(rB);
      break;
    
    case JXX: // JMP
      break;
    
    case CALL:
      *valB = getRegister(RSP);
      break;
    
    case RET:
      *valA = getRegister(RSP);
      *valB = getRegister(RSP);
      break;
    
    case PUSHQ:
      *valA = getRegister(rA);
      *valB = getRegister(RSP);
      break;
    
    case POPQ:
      *valA = getRegister(RSP);
      *valB = getRegister(RSP);
      break;
    
    default:
      break;
  }
}

void executeStage(int icode, int ifun, wordType valA, wordType valB, wordType valC, wordType *valE, bool *Cnd) {
  bool sf = FALSE;
  bool zf = FALSE;
  bool of = FALSE;
  switch (icode) {
    case HALT:
      break;

    case NOP:
      break;

    case RRMOVQ: // CMOVXX
      *valE = 0 + valA;
      break;

    case IRMOVQ:
      *valE = 0 + valC;
      break;
    
    case RMMOVQ:
      *valE = valB + valC;
      break;
    
    case MRMOVQ:
      *valE = valB + valC;
      break;
    
    case OPQ:
      switch (ifun) {
        case ADD:
          *valE = valB + valA;
          if (((valA < 0) == (valB < 0)) && ((*valE < 0) != (valA < 0))) of = TRUE;
          break;
        
        case SUB:
          *valE = valB - valA;
          if (((-valA < 0) == (valB < 0)) && ((*valE < 0) != (-valA < 0))) of = TRUE;
          break;
        
        case AND:
          *valE = valB & valA;
          break;
        
        case XOR:
          *valE = valB ^ valA;
          break;
        
        default:
          break;
      }
      if (*valE < 0) sf = TRUE;
      if (*valE == 0) zf = TRUE;
      setFlags(sf, zf, of);
      break;
    
    case JXX: // JMP
      *Cnd = Cond(ifun);
      break;
    
    case CALL:
      *valE = valB - 8;
      break;
    
    case RET:
      *valE = valB + 8;
      break;
    
    case PUSHQ:
      *valE = valB - 8;
      break;
    
    case POPQ:
      *valE = valB + 8;
      break;
    
    default:
      break;
  }
}

void memoryStage(int icode, wordType valA, wordType valP, wordType valE, wordType *valM) {
  switch (icode) {
    case HALT:
      break;

    case NOP:
      break;

    case RRMOVQ: // CMOVXX
      break;

    case IRMOVQ:
      break;
    
    case RMMOVQ:
      setWordInMemory(valE, valA);
      break;
    
    case MRMOVQ:
      *valM = getWordFromMemory(valE);
      break;
    
    case OPQ:
      break;
    
    case JXX: // JMP
      break;
    
    case CALL:
      setWordInMemory(valE, valP);
      break;
    
    case RET:
      *valM = getWordFromMemory(valA);
      break;
    
    case PUSHQ:
      setWordInMemory(valE, valA);
      break;
    
    case POPQ:
      *valM = getWordFromMemory(valA);
      break;
    
    default:
      break;
  }
}

void writebackStage(int icode, int rA, int rB, wordType valE, wordType valM) {
  switch (icode) {
    case HALT:
      break;

    case NOP:
      break;

    case RRMOVQ: // CMOVXX
      setRegister(rB, valE);
      break;

    case IRMOVQ:
      setRegister(rB, valE);
      break;
    
    case RMMOVQ:
      break;
    
    case MRMOVQ:
      setRegister(rA, valM);
      break;
    
    case OPQ:
      setRegister(rB, valE);
      break;
    
    case JXX: // JMP
      break;
    
    case CALL:
      setRegister(RSP, valE);
      break;
    
    case RET:
      setRegister(RSP, valE);
      break;
    
    case PUSHQ:
      setRegister(RSP, valE);
      break;
    
    case POPQ:
      setRegister(RSP, valE);
      setRegister(rA, valM);
      break;
    
    default:
      break;
  }
}

void pcUpdateStage(int icode, wordType valC, wordType valP, bool Cnd, wordType valM) {
  switch (icode) {
    case HALT:
      setPC(valP);
      setStatus(STAT_HLT);
      break;

    case NOP:
      setPC(valP);
      break;

    case RRMOVQ: // CMOVXX
      setPC(valP);
      break;

    case IRMOVQ:
      setPC(valP);
      break;
    
    case RMMOVQ:
      setPC(valP);
      break;
    
    case MRMOVQ:
      setPC(valP);
      break;
    
    case OPQ:
      setPC(valP);
      break;
    
    case JXX: // JMP
      Cnd ? setPC(valC) : setPC(valP);
      break;
    
    case CALL:
      setPC(valC);
      break;
    
    case RET:
      setPC(valM);
      break;
    
    case PUSHQ:
      setPC(valP);
      break;
    
    case POPQ:
      setPC(valP);
      break;
    
    default:
      break;
  }
}

void stepMachine(int stepMode) {
  /* FETCH STAGE */
  int icode = 0, ifun = 0;
  int rA = 0, rB = 0;
  wordType valC = 0;
  wordType valP = 0;
 
  /* DECODE STAGE */
  wordType valA = 0;
  wordType valB = 0;

  /* EXECUTE STAGE */
  wordType valE = 0;
  bool Cnd = 0;

  /* MEMORY STAGE */
  wordType valM = 0;

  fetchStage(&icode, &ifun, &rA, &rB, &valC, &valP);
  applyStageStepMode(stepMode, "Fetch", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  decodeStage(icode, rA, rB, &valA, &valB);
  applyStageStepMode(stepMode, "Decode", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  executeStage(icode, ifun, valA, valB, valC, &valE, &Cnd);
  applyStageStepMode(stepMode, "Execute", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  memoryStage(icode, valA, valP, valE, &valM);
  applyStageStepMode(stepMode, "Memory", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  writebackStage(icode, rA, rB, valE, valM);
  applyStageStepMode(stepMode, "Writeback", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  pcUpdateStage(icode, valC, valP, Cnd, valM);
  applyStageStepMode(stepMode, "PC update", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  incrementCycleCounter();
}

/** 
 * main
 * */
int main(int argc, char **argv) {
  int stepMode = 0;
  FILE *input = parseCommandLine(argc, argv, &stepMode);

  initializeMemory(MAX_MEM_SIZE);
  initializeRegisters();
  loadMemory(input);

  applyStepMode(stepMode);
  while (getStatus() != STAT_HLT) {
    stepMachine(stepMode);
    applyStepMode(stepMode);
#ifdef DEBUG
    printMachineState();
    printf("\n");
#endif
  }
  printMachineState();
  return 0;
}