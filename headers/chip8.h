#ifndef CHIP8_H
#define CHIP8_H

#include "common.h"
#include "stack.h"

#define RAM_SIZE 4000
#define STACK_SIZE 512
#define NUM_REGISTERS 16
#define DISPLAY_SIZE 2048
#define DISPLAY_WIDTH 32
#define DISPLAY_HEIGHT 64
#define TARGET_FPS 60
#define CASE_SIZE 10

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
    int8_t keypressed; // -1 if no key is pressed
} chip8;


extern const uint8_t font_sprite[];


// Instruction process
uint16_t fetch(chip8*);
void decode(chip8*, uint16_t);


// Graphics
void display_set_pixel(chip8*, uint8_t, uint8_t, bool);
void draw_display(chip8* cpu);

#endif // (ClassName_H)