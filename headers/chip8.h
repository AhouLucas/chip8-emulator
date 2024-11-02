#ifndef CHIP8_H
#define CHIP8_H

#include "common.h"
#include "stack.h"

#define RAM_SIZE 4000
#define STACK_SIZE 512
#define NUM_REGISTERS 16
#define DISPLAY_SIZE 2048
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define TARGET_FPS 60
#define INSTRUCTIONS_PER_FRAME 10
#define CASE_SIZE 20

/*
SHIFT_INSTRUCTION_COSMAC_VIP :

If set to true, 8XY6 and 8XYE instructions will use shift instructions from the
original COSMAC VIP. They will thus put the value of VY into VX before shifting.
If set to false, these will not put the value of VY into VX and will directly shift.
*/
#define SHIFT_INSTRUCTION_COSMAC_VIP true


#define FIRST_NIBBLE_MASK 0xF000
#define SECOND_NIBBLE_MASK 0xF00
#define THIRD_NIBBLE_MASK 0xF0
#define FOURTH_NIBBLE_MASK 0xF


typedef struct {
    uint8_t* ram;
    bool* display;
    stack* s;
    uint8_t* registers;
    size_t PC;
    size_t I;
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool* keypad;
} chip8;


extern const uint8_t font_sprite[];


// Initialization
chip8* chip8Init(void);
void chip8Free(chip8*);
void chip8LoadRom(chip8*, const char*);
void chip8SetFont(chip8*);

// Instruction process
uint16_t chip8Fetch(chip8*);
void chip8Decode(chip8*, uint16_t);
void chip8DecrementTimers(chip8*);
void chip8KeypadInput(chip8*);

void chip8Process(chip8*);



// Graphics
void chip8DrawDisplay(chip8* cpu);

#endif // (ClassName_H)