#include "../headers/common.h"
#include "../headers/chip8.h"
#include "../headers/stack.h"
#include "raylib.h"



int main(int argc, char* argv[]) {

    chip8 cpu = {
        .ram=malloc(RAM_SIZE),
        .display=malloc(DISPLAY_SIZE*sizeof(bool)),
        .s=init_stack(STACK_SIZE),
        .registers=malloc(NUM_REGISTERS),
        .PC=80, // First instruction after font sprites
        .I=0,
        .delay_timer=UINT8_MAX,
        .sound_timer=UINT8_MAX,
        .keypressed=-1,
    };

    // Place fonts from address 0x050 to 0x09F
    for (size_t i = 0; i < 80; i++) {
        cpu.ram[i] = font_sprite[i];
    }


    // Parse filename in arguments
    char filename[50];
	while ((argc > 1) && (argv[1][0] == '-'))
	{
		switch (argv[1][1])
		{
			case 'f':
                sprintf(filename, &argv[1][2]);
				break;

			default:
				printf("Wrong Argument: %s\n", argv[1]);
		}

		++argv;
		--argc;
	}


    // Read instructions from file
    FILE *fileptr;
    long filelen;

    fileptr = fopen(filename, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    fread(cpu.ram + 80, filelen, 1, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file



    // MAIN LOOP
    InitWindow(CASE_SIZE*32, CASE_SIZE*64, "Chip8 emulator");
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose()) {

        // Instructions process :
        // A standard speed of 700 chip8 instructions fits well.
        // So we execute 10 instructions per frame, that would make 600 instructions per second.
        /* process instruction */
        if (cpu.PC < 80 + filelen) {
            uint16_t instruction = fetch(&cpu);
            decode(&cpu, instruction);
            // for (int i = 0; i < 2048; i++) {
            //     if (cpu.display[i] != 0) printf("%d ", i);
            // }
            // printf("\n");
        }


        // Decrement timers
        cpu.delay_timer--;
        cpu.sound_timer--;
        

        
        // Get Keyboard key pressed
        
        char key = GetCharPressed();
        switch (key) {
            // 1
            case '&':
                cpu.keypressed = 1;
                break;
            // 2
            case 4294967273:
                cpu.keypressed = 2;
                break;
            // 3
            case '"':
                cpu.keypressed = 3;
                break;
            // C
            case 39:
                cpu.keypressed = 12;
                break;
            // 4
            case 'a':
                cpu.keypressed = 4;
                break;
            // 5
            case 'z':
                cpu.keypressed = 5;
                break;
            // 6
            case 'e':
                cpu.keypressed = 6;
                break;
            // D
            case 'r':
                cpu.keypressed = 13;
                break;
            // 7
            case 'q':
                cpu.keypressed = 7;
                break;
            // 8
            case 's':
                cpu.keypressed = 8;
                break;
            // 9
            case 'd':
                cpu.keypressed = 9;
                break;
            // E
            case 'f':
                cpu.keypressed = 14;
                break;
            // A
            case 'w':
                cpu.keypressed = 10;
                break;
            // 0
            case 'x':
                cpu.keypressed = 0;
                break;
            // B
            case 'c':
                cpu.keypressed = 11;
                break;
            // F
            case 'v':
                cpu.keypressed = 15;
                break;
            default:
                cpu.keypressed = -1;
                break;
        }


        BeginDrawing();
        ClearBackground(BLACK);

        draw_display(&cpu);

        EndDrawing();

    }


    // Cleaning
    free(cpu.ram);
    free(cpu.display);
    free(cpu.registers);
    free_stack(cpu.s);

    return 0;
}