#ifndef main_h
#define main_h

// Global variable

extern char* registers_name[2][8];
extern uint16_t registers[8];
extern uint8_t memory[0xFFFF];
extern uint8_t * text;
extern uint8_t * data;
extern uint32_t PC;

// Bit mask functions:

#define BM4(x) (x & 0b11110000)
#define BM5(x) (x & 0b11111000)
#define BM6(x) (x & 0b11111100)
#define BM7(x) (x & 0b11111110)
#define BMP(x) (x & 0b00111000)
#define FLAG(x) (x & 0b00111000)
#define LASTBIT1(x) (x & 0b00000001)
#define LASTBIT2(x) ((x & 0b00000010) >> 1)
#define MOD(x) ((x & 0b11000000) >> 6)
#define REG(x) ((x & 0b00111000) >> 3)
#define RM(x) (x & 0b00000111)


// Special values:
// 0b1111111 (push, inc, dec, call, call, jmp, jmp)
// 0b100000 (add, addc, cmp, sub, ssb, or, xor, and)
// 0b1111011 (neg, mul, imul, div, idiv, not, test)
// 0b110100 (shl, shr, sar, rol, ror, rcl, rcr)

#define SPECIAL1 0b11111110
#define SPECIAL2 0b10000000
#define SPECIAL3 0b11110110
#define SPECIAL4 0b11010000

// MOV

#define MOV1 0b10001000
#define MOV2 0b11000110
#define MOV3 0b10110000
#define MOV4 0b10100000
#define MOV5 0b10100010
#define MOV6 0b10001110
#define MOV7 0b10001100

// PUSH

#define PUSH1 0b00110000
#define PUSH2 0b01010000

// POP

#define POP1 0b00000000
#define POP2 0b01011000

// XCHG

#define XCHG1 0b10000110
#define XCHG2 0b10010000

// IN

#define IN1 0b11100100
#define IN2 0b11101100

// OUT

#define OUT1 0b11100110
#define OUT2 0b11101110

#define XLAT 0b11010111

#define LEA 0b10001101

#define LDS 0b11000101

#define LES 0b11000100

#define LAHF 0b10011111

#define SAHF 0b10011110

#define PUSHF 0b10011100

#define POPF 0b10011101

// ADD

#define ADD1 0b00000000
#define ADD2 0b00000000
#define ADD3 0b00000100

// ADC

#define ADC1 0b00010000
#define ADC2 0b00010000
#define ADC3 0b00010100

// INC

#define INC1 0b00000000
#define INC2 0b01000000

// AAA
// BAA
// SUB

#define SUB1 0b00101000
#define SUB2 0b00101000
#define SUB3 0b00101100

// SSB

#define SSB1 0b00011000
#define SSB2 0b00011000
// #define SBB3 0b0001110

// DEC
#define DEC1 0b00001000
#define DEC2 0b01001000
// NEG

#define NEG 0b00011000

// CMP

#define CMP1 0b00111000
#define CMP2 0b00111000
#define CMP3 0b00111100

// AAS
// DAS
// MUL
#define MUL 0b00100000
// IMUL
#define IMUL 0b00101000
// AAM
// DIV
#define DIV 0b00110000
// IDIV
#define IDIV 0b00111000
// AAD
#define CBW 0b10011000
#define CWD 0b10011001
// NOT
// SHL/SAL

#define SHL 0b00100000
#define SHR 0b00101000
#define SAR 0b00111000
#define ROL 0b00000000
#define ROR 0b00001000
#define RCL 0b00010000
#define RCR 0b00011000

// AND

#define AND1 0b00100000
#define AND2 0b00100000
#define AND3 0b00100100

// TEST

#define TEST1 0b10000100
#define TEST2 0b00000000
#define TEST3 0b10101000

// OR

#define OR1 0b00001000
#define OR2 0b00001000
#define OR3 0b00001100

// XOR

#define XOR1 0b00110000
#define XOR2 0b00001000
#define XOR3 0b00110100

#define REP 0b11110010
#define MOVS 0b10100100
#define CMPS 0b10100110
#define SCAS 0b10101110
#define LODS 0b10101100
#define STOS 0b10101010

// CALL

#define CALL1 0b11101000
#define CALL2 0b00010000
#define CALL3 0b10011010
#define CALL4 0b00011000

// JMP

#define JMP1 0b11101001
#define JMP2 0b11101011
#define JMP3 0b00100000
#define JMP4 0b11101010
#define JMP5 0b00101000

// RET

#define RET1 0b11000011
#define RET2 0b11000010
#define RET3 0b11001011
#define RET4 0b11001010

// JUMPS
#define JE 0b01110100
#define JL 0b01111100
#define JLE 0b01111110
#define JB 0b01110010
#define JBE 0b01110110
#define JP 0b01111010
#define JO 0b01110000
#define JS 0b01111000
#define JNE 0b01110101
#define JNL 0b01111101
#define JNLE 0b01111111
#define JNB 0b01110011
#define JNBE 0b01110111
#define JNO 0b01110001
#define JNS 0b01111001
#define LOOP 0b11100010
// LOOPZ/LOOPE
// LOPPNZ/LOOPNE
// JCXZ
// INT

#define INT1 0b11001101
#define INT2 0b11001100

// INTO
// IRET
// CLC
// CMC
// STC
#define CLD 0b11111100
#define STD 0b11111101
// CLI
// STI
// HLT

#define HLT 0b11110100

// WAIT
// ESC
// LOCK

#endif

