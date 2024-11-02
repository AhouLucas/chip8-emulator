#include "../headers/common.h"
#include "../headers/chip8.h"
#include "../headers/stack.h"
#include "raylib.h"


// Fonts sprite data
const uint8_t font_sprite[] = 
{
0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


/////////////////////
/* Initialization */
///////////////////
chip8* chip8Init() {
    chip8* cpu = malloc(sizeof(chip8));

    cpu->ram = malloc(RAM_SIZE);
    cpu->display = malloc(DISPLAY_SIZE*sizeof(bool));
    cpu->s=init_stack(STACK_SIZE);
    cpu->registers=malloc(NUM_REGISTERS);
    cpu->PC=0x200;
    cpu->I=0;
    cpu->delay_timer=UINT8_MAX;
    cpu->sound_timer=UINT8_MAX;
    cpu->keypad=calloc(16, sizeof(bool));

    return cpu;
}

void chip8Free(chip8* cpu) {
    free(cpu->ram);
    free(cpu->display);
    free_stack(cpu->s);
    free(cpu->registers);
    free(cpu->keypad);
    free(cpu);
    return;
}


void chip8LoadRom(chip8* cpu, const char* path) {
    // Read instructions from file
    FILE *fileptr;
    long filelen;

    fileptr = fopen(path, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    fread(cpu->ram + 0x200, filelen, 1, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file

    return;
}


void chip8SetFont(chip8* cpu) {
    for (size_t i = 0; i < 80; i++) {
        cpu->ram[i] = font_sprite[i];
    }
}



 ///////////////////////////////////
/* Instruction process functions */
//////////////////////////////////

uint16_t chip8Fetch(chip8* cpu) {
    uint16_t instruction = 0;
    // Combine two successive bytes from ram that form one instruction
    // And increments program counter (PC) by 2;
    instruction = ((cpu->ram)[cpu->PC] << 8) + (cpu->ram)[cpu->PC + 1];
    cpu->PC += 2;
    return instruction;
}

// Decode and execute instruction
void chip8Decode(chip8* cpu, uint16_t instruction) {
    uint8_t type = (instruction & FIRST_NIBBLE_MASK) >> 12; // First nibble
    uint8_t x = (instruction & SECOND_NIBBLE_MASK) >> 8; // Second nibble
    uint8_t y = (instruction & THIRD_NIBBLE_MASK) >> 4; // Third nibble
    uint8_t n = (instruction & FOURTH_NIBBLE_MASK); // Fourth nibble
    uint8_t nn = (y<<4) + n; // Second byte (third and fourth nibble)
    uint16_t nnn = (x << 8) + nn; // Second, third and fourth nibble


    switch (type) {
    case 0:
        
        // 00E0 (Clear screen)
        if (instruction == 0x00E0) {
            memset(cpu->display, 0, DISPLAY_SIZE);
        }
        // 00EE (Returning from a subroutine)
        else if (instruction == 0x00EE) {
            cpu->PC = pop(cpu->s);
        }

        break;
    
    // 1NNN (Jump)
    case 1:
        cpu->PC = nnn;
        break;

    // 2NNN (Subroutine call)
    case 2:
        push(cpu->s, cpu->PC);
        cpu->PC = nnn;
        break;

    // 3XNN (Skips if VX = NN)
    case 3:
        if ((cpu->registers)[x] == nn) cpu->PC += 2;
        break;
    
    // 4XNN (Skips if VX != NN)
    case 4:
        if ((cpu->registers)[x] != nn) cpu->PC += 2;
        break;

    // 5XY0 (Skips if VX = VY)
    case 5:
        if ((cpu->registers)[x] == (cpu->registers)[y]) cpu->PC += 2;
        break;

    // 9XY0 (Skips if VX != VY)
    case 9:
        if ((cpu->registers)[x] != (cpu->registers)[y]) cpu->PC += 2;
        break;

    // 6XNN (Set VX to NN)
    case 6:
        cpu->registers[x] = nn;
        break;
    
    // 7XNN (Add NN to VX)
    case 7:
        (cpu->registers)[x] += nn;
        break;

    // Logical/Arithmetic operations (determined by the last nibble)
    case 8:
        switch (n) {
        // 8XY0 (Set: VX to the value of VY)
        case 0:
            (cpu->registers)[x] = (cpu->registers)[y];
            break;
        
        // 8XY1 (Binary OR)
        case 1:
            (cpu->registers)[x] = (cpu->registers)[x] | (cpu->registers)[y];
            break;

        // 8XY2 (Binary AND)
        case 2:
            (cpu->registers)[x] = (cpu->registers)[x] & (cpu->registers)[y];
            break;

        // 8XY3 (Binary XOR)
        case 3:
            (cpu->registers)[x] = (cpu->registers)[x] ^ (cpu->registers)[y];
            break;

        // 8XY4 (Add: Set VX to VX + VY)
        case 4:
            uint8_t result = (cpu->registers)[x] + (cpu->registers)[y];
            if (result > 255) (cpu->registers)[0xF] = 1; // Set VF flag to 1 if there is an overflow;
            (cpu->registers)[x] = result;
            break;
        
        // 8XY5 (Substract: Set VX to VX - VY)
        case 5:
            if ((cpu->registers)[x] >= (cpu->registers)[y]) (cpu->registers)[0xF] = 1;
            else (cpu->registers)[0xF] = 0;

            (cpu->registers)[x] = (cpu->registers)[x] - (cpu->registers)[y];
            break;
        
        // 8XY7 (Substract: Set VX to VY - VX)
        case 7:
            if ((cpu->registers)[y] >= (cpu->registers)[x]) (cpu->registers)[0xF] = 1;
            else (cpu->registers)[0xF] = 0;

            (cpu->registers)[x] = (cpu->registers)[y] - (cpu->registers)[x];
            break;

        // 8XY6 (Shift VX 1bit to the right)
        case 6:
            if (SHIFT_INSTRUCTION_COSMAC_VIP) (cpu->registers)[x] = (cpu->registers)[y];
            (cpu->registers)[0xF] = (cpu->registers)[x] & 0x1; // Sets VF to 1 if the bit that was shifted out was 1, 0 otherwise;
            (cpu->registers)[x] = ((cpu->registers)[x] >> 1) & 0xFF;
            break;

        // 8XYE (Shift VX 1bit to the left)
        case 0xE:
            if (SHIFT_INSTRUCTION_COSMAC_VIP) (cpu->registers)[x] = (cpu->registers)[y];
            (cpu->registers)[0xF] = ((cpu->registers)[x]>>7) & 0x1; // Sets VF to 1 if the bit that was shifted out was 1, 0 otherwise;
            (cpu->registers)[x] = ((cpu->registers)[x] << 1) & 0xFF;
            break;

        default:
            break;
        }

        break;

    // ANNN (Set index)
    case 0xA:
        cpu->I = nnn;
        break;

    // BNNN (Jump with offset: jump to NNN + V0) (!!! Different implementations possible !!!)
    case 0xB:
        cpu->PC = nnn + (cpu->registers)[0];
        break;
    
    // CXNN (Random: Generate a random number, binary-ANDs it with NN and put the result in VX)
    case 0xC:
        (cpu->registers)[x] = rand() & nn;
        break;

    // DXYN (Display)
    case 0xD:
         uint8_t x_coord_init = cpu->registers[x] % DISPLAY_WIDTH;

            uint8_t x_coord = x_coord_init;
            uint8_t y_coord = cpu->registers[y] % DISPLAY_HEIGHT;

            cpu->registers[0xF] = 0;

            for (int i = 0; i < n; i++) {
                uint8_t sprite_byte = cpu->ram[cpu->I + i];
                for (int j = 7; j >= 0; j--) {
                    uint8_t pixel = (sprite_byte >> j) & 1;
                    int screen_pixel_index = y_coord * DISPLAY_WIDTH + x_coord;

                    if ((pixel == 1) && (cpu->display[screen_pixel_index] == 0)) {
                        cpu->display[screen_pixel_index] = 1;
                        cpu->registers[0xF] = 1;
                    } else if ((pixel == 1) && (cpu->display[screen_pixel_index] == 1)) {
                        cpu->display[screen_pixel_index] = 0;
                    }

                    x_coord++;
                    if (x_coord >= DISPLAY_WIDTH) break;
                }

                x_coord = x_coord_init;
                y_coord++;

                if (y_coord >= DISPLAY_HEIGHT) break;
            };
        break;

    // Keyboard inputs
    case 0xE:
        switch (nn) {
        // EX9E (Skip if key corresponding to VX is pressed)
        case 0x9E:
            if (cpu->keypad[cpu->registers[x]]) cpu->PC += 2;
            break;

        // EXA1 (Skip if key corresponding to VX is not pressed)
        case 0xA1:
            if (!cpu->keypad[cpu->registers[x]]) cpu->PC += 2;
            break;

        default:
            break;
        }

        break;
    
    case 0xF:
        switch (nn) {
            // FX07 (Sets VX to the value of the current value of the delay timer)
            case 0x07:
                (cpu->registers)[x] = cpu->delay_timer;
                break;

            // FX15 (Sets the delay timer to the value of VX)
            case 0x15:
                cpu->delay_timer = (cpu->registers)[x];
                break;

            // FX18 (Sets the sound timer to the value of VX)
            case 0x18:
                cpu->sound_timer = (cpu->registers)[x];
                break;

            // FX1E (Add VX to index)
            // Note: we update VF if index I goes from 0x0FFF=4095 to above 0x1000=4096
            case 0x1E:
                uint8_t result = cpu->I + (cpu->registers)[x];
                if (cpu->I <= 4095 && result > 4095) (cpu->registers)[0xF] = 1;
                cpu->I = result;
                break;

            // FX0A (Blocks execution while waiting for a key press)
            // Note: this is done by decrementing the program counter (PC) unless a key is pressed
            case 0x0A: {
                bool found = false;
                for (int i = 0; i < 16; i++) {
                    if (cpu->keypad[i] == 1) {
                        cpu->registers[x] = i;
                        found = true;
                        break;
                    }
                }
                if (!found) cpu->PC -= 2;
                break;
            }
            
            // FX29 (Font character)
            // Note: a character (8bit = 1byte) is represented by a sprite of 5 bytes located in the first 80 bytes in the ram (80/5 = 16 characters)
            case 0x29:
                cpu->I = (cpu->registers)[x]*5;
                break;

            // FX33 (Binary-coded decimal conversion)
            // It takes the byte in VX (any number from 0 to 255) and places each decimal digits in I, I+1 and I+2
            // Eg: if VX = 156 = 0x9C, this instruction would place 1 in I, 5 in I+1 and 6 in I+2
            case 0x33:
                uint8_t num = (cpu->registers)[x];
                uint8_t first_digit = num / 100;
                uint8_t second_digit = (num - (100*first_digit)) / 10;
                uint8_t third_digit = num - (100*first_digit) - (10*second_digit);

                (cpu->ram)[cpu->I] = first_digit;
                (cpu->ram)[cpu->I + 1] = second_digit;
                (cpu->ram)[cpu->I + 2] = third_digit;
                break;

            // FX55 (Store in memory)
            // The value of each register V0 to VX (included) will be stored in memory starting from I to I+X
            case 0x55:
                for (uint8_t i = 0; i <= x; i++) {
                    (cpu->ram)[cpu->I + i] = (cpu->registers)[i];
                }
                break;
            
            // FX65 (Load from memory)
            case 0x65:
                for (uint8_t i = 0; i <= x; i++) {
                   (cpu->registers)[i] = (cpu->ram)[cpu->I + i];
                }
                break;

            default:
                break;
        }
    

    default:
        break;
    }

}


void chip8DecrementTimers(chip8* cpu) {
    cpu->delay_timer--;
    cpu->sound_timer--;
}


void chip8KeypadInput(chip8* cpu) {
    if (IsKeyDown(88)) {
        cpu->keypad[0x0] = 1;
    } else if (IsKeyUp(KEY_X)) {
        cpu->keypad[0x0] = 0;
    }
    if (IsKeyDown(49)) {
        cpu->keypad[0x1] = 1;
    } else if (IsKeyUp(KEY_ONE)) {
        cpu->keypad[0x1] = 0;
    }
    if (IsKeyDown(50)) {
        cpu->keypad[0x2] = 1;
    } else if (IsKeyUp(KEY_TWO)) {
        cpu->keypad[0x2] = 0;
    }
    if (IsKeyDown(51)) {
        cpu->keypad[0x3] = 1;
    } else if (IsKeyUp(KEY_THREE)) {
        cpu->keypad[0x3] = 0;
    }
    if (IsKeyDown(81)) {
        cpu->keypad[0x4] = 1;
    } else if (IsKeyUp(KEY_Q)) {
        cpu->keypad[0x4] = 0;
    }
    if (IsKeyDown(87)) {
        cpu->keypad[0x5] = 1;
    } else if (IsKeyUp(KEY_W)) {
        cpu->keypad[0x5] = 0;
    }
    if (IsKeyDown(69)) {
        cpu->keypad[0x6] = 1;
    } else if (IsKeyUp(KEY_E)) {
        cpu->keypad[0x6] = 0;
    }
    if (IsKeyDown(65)) {
        cpu->keypad[0x7] = 1;
    } else if (IsKeyUp(KEY_A)) {
        cpu->keypad[0x7] = 0;
    }
    if (IsKeyDown(83)) {
        cpu->keypad[0x8] = 1;
    } else if (IsKeyUp(KEY_S)) {
        cpu->keypad[0x8] = 0;
    }
    if (IsKeyDown(68)) {
        cpu->keypad[0x9] = 1;
    } else if (IsKeyUp(KEY_D)) {
        cpu->keypad[0x9] = 0;
    }
    if (IsKeyDown(90)) {
        cpu->keypad[0xA] = 1;
    } else if (IsKeyUp(KEY_Z)) {
        cpu->keypad[0xA] = 0;
    }
    if (IsKeyDown(67)) {
        cpu->keypad[0xB] = 1;
    } else if (IsKeyUp(KEY_C)) {
        cpu->keypad[0xB] = 0;
    }
    if (IsKeyDown(52)) {
        cpu->keypad[0xC] = 1;
    } else if (IsKeyUp(KEY_FOUR)) {
        cpu->keypad[0xC] = 0;
    }
    if (IsKeyDown(82)) {
        cpu->keypad[0xD] = 1;
    } else if (IsKeyUp(KEY_R)) {
        cpu->keypad[0xD] = 0;
    }
    if (IsKeyDown(70)) {
        cpu->keypad[0xE] = 1;
    } else if (IsKeyUp(KEY_F)) {
        cpu->keypad[0xE] = 0;
    }
    if (IsKeyDown(86)) {
        cpu->keypad[0xF] = 1;
    } else if (IsKeyUp(KEY_V)) {
        cpu->keypad[0xF] = 0;
    }
}
 


void chip8Process(chip8* cpu) {

    // Get Keyboard key pressed
    chip8KeypadInput(cpu);

    // Decrement timers
    chip8DecrementTimers(cpu);

    /* process instruction */
    for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
        uint16_t instruction = chip8Fetch(cpu);
        chip8Decode(cpu, instruction);
    }

}


////////////////////////
/* Graphics function */
///////////////////////

void chip8DrawDisplay(chip8* cpu) {
    for (size_t i = 0; i < DISPLAY_WIDTH; i++) {
        for (size_t j = 0; j < DISPLAY_HEIGHT; j++) {
            if ((cpu->display)[i + DISPLAY_WIDTH*j]) DrawRectangle(i*CASE_SIZE, j*CASE_SIZE, CASE_SIZE, CASE_SIZE, WHITE);
        }
    }
    return;
}