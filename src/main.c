#include "../headers/common.h"
#include "../headers/chip8.h"
#include "../headers/stack.h"
#include "raylib.h"



int main(int argc, char* argv[]) {

    // Parse filename in arguments
    char filename[50];
	while ((argc > 1) && (argv[1][0] == '-'))
	{
		switch (argv[1][1])
		{
			case 'f':
                int shift = 1;
                while (argv[1][2+shift] == ' ') shift++;
                sprintf(filename, &argv[1][2+shift]);
				break;

			default:
				printf("Wrong Argument: %s\n", argv[1]);
		}

		++argv;
		--argc;
	}



    // CPU Initialization
    chip8* cpu = chip8Init();
    chip8SetFont(cpu);
    chip8LoadRom(cpu, filename);


    // MAIN LOOP
    InitWindow(CASE_SIZE*DISPLAY_WIDTH, CASE_SIZE*DISPLAY_HEIGHT, "Chip8 emulator");
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose()) {

        // Process instructions
        chip8Process(cpu);

        BeginDrawing();
        ClearBackground(BLACK);

        chip8DrawDisplay(cpu);

        EndDrawing();

    }


    // Cleaning
    chip8Free(cpu);

    return 0;
}