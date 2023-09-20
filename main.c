#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include <io.h>

// Define the dimensions of screen
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320

typedef struct Chip_8 {
    uint8_t memory[4096]; // 4KB memory
    unsigned short opcode;
    // registers
    uint8_t V[16]; // general purpose 8-bit registers
    uint16_t I; // 16-bit register for memory addresses

    // special registers
    uint8_t delay_register;
    uint8_t sound_register;

    // pointer & addresses
    uint16_t PC; // program counter
    uint8_t SP; // stack pointer
    uint16_t stack[16]; // stack used to store addresses for subroutines

    // keyboard
    uint8_t key[16];

    // display - top-left (0,0) to bottom-right (63,31)
    bool display[64 * 32]; // pixels are either on or off
    bool draw_flag;
} Chip_8;

void initialise_key_states(Chip_8 *chip) {
    for (int i = 0; i < 16; i++) {
        chip->key[i] = 0;
    }
}

Chip_8* init_chip() {
    // allocate and initialise variables
    Chip_8 *chip = (Chip_8 *) calloc(1, sizeof(Chip_8));
    chip->opcode = 0;
    chip->I = 0;
    chip->SP = 0;
    chip->PC = 0x200; // 0x200 is where the program is loaded to
    chip->draw_flag = false;
    memset(chip->memory, 0, sizeof(chip->memory));

    initialise_key_states(chip);

    // load font-set
    unsigned char font_set[80] =
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
    for(int i = 0; i < 80; i++)
        chip->memory[i] = font_set[i];

    return chip;
}

int init_graphics(SDL_Window **window, SDL_Renderer **renderer) {
    // initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // initialise window
    *window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                               SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // initialise renderer
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    return 0;
}

void close_graphics(SDL_Window *window, SDL_Renderer *renderer) {
    // Destroy window and renderer & quit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void update_key_states(Chip_8 *chip, SDL_Event event) {
    if (event.type == SDL_KEYDOWN) {
        SDL_Keycode keyPressed = event.key.keysym.sym;
        initialise_key_states(chip);

        switch (keyPressed) {
            case SDLK_1:
                chip->V[0] = 1;
                break;
            case SDLK_2:
                chip->V[1] = 1;
                break;
            case SDLK_3:
                chip->V[2] = 1;
                break;
            case SDLK_4:
                chip->V[3] = 1;
                break;
            case SDLK_q:
                chip->V[4] = 1;
                break;
            case SDLK_w:
                chip->V[5] = 1;
                break;
            case SDLK_e:
                chip->V[6] = 1;
                break;
            case SDLK_r:
                chip->V[7] = 1;
                break;
            case SDLK_a:
                chip->V[8] = 1;
                break;
            case SDLK_s:
                chip->V[9] = 1;
                break;
            case SDLK_d:
                chip->V[10] = 1;
                break;
            case SDLK_f:
                chip->V[11] = 1;
                break;
            case SDLK_y:
                chip->V[12] = 1;
                break;
            case SDLK_x:
                chip->V[13] = 1;
                break;
            case SDLK_c:
                chip->V[14] = 1;
                break;
            case SDLK_v:
                chip->V[15] = 1;
                break;
            default:
                break;
        }
    }
}

// Function to draw the boolean array
void drawDisplay(SDL_Renderer *renderer, Chip_8 *chip) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int pixelWidth = SCREEN_WIDTH / 64;
    int pixelHeight = SCREEN_HEIGHT / 32;

    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            if (chip->display[y * 64 + x]) {
                // Scale the pixel coordinates to fit the window dimensions
                int scaledX = x * pixelWidth;
                int scaledY = y * pixelHeight;

                // Draw a filled rectangle (pixel) at the scaled coordinates
                SDL_Rect pixelRect = {scaledX, scaledY, pixelWidth, pixelHeight};
                SDL_RenderFillRect(renderer, &pixelRect);
            }
        }
    }
}

// open rom file and load into memory array (at index 512)
int load_program_to_memory(Chip_8 *chip, char *path) {
    FILE *file = fopen(path, "rb"); // it took me quite a while to figure out you need to use 'rb'
    if (!file) return -1;
    // memcpy(&chip->memory[512], file, 3584);
    fread(&chip->memory[512], 1, 3584, file);
    fclose(file);
}

void draw(SDL_Renderer **renderer, Chip_8 *chip) {
    // Clear the screen
    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
    SDL_RenderClear(*renderer);

    // Draw the boolean array
    drawDisplay(*renderer, chip);
    // Update the screen
    SDL_RenderPresent(*renderer);
}

void clear_display(Chip_8 *chip) {
    for (int i = 0; i < 32 * 64; i++) {
        chip->display[i] = false;
    }
}

int wait_and_get_key_value(Chip_8 *chip) {
    bool keyPressDetected = false;
    SDL_Event event;

    while (!keyPressDetected) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                keyPressDetected = true; // Quit the loop if the window is closed
            } else if (event.type == SDL_KEYDOWN) {
                // A key was pressed; store the key value
                SDL_Keycode keyPressed = event.key.keysym.sym;

                // Output the key value (for example, print it)
                printf("Key Pressed: %s\n", SDL_GetKeyName(keyPressed));

                // Store the key value and exit the loop
                keyPressDetected = true;
            }
        }
    }

    for (int i = 0; i < 16; i++)
        if (chip->key[i] == 1) return i;

    return -1;
}

void decode_and_execute(Chip_8 *chip) {
    unsigned short opcode = chip->opcode;
    switch (opcode >> 12) {
        case 0x0:
            switch (opcode & 0x000F) {
                case 0x0000: // 0x00E0: Clears the screen
                    clear_display(chip);
                    chip->PC += 2;
                    break;
                case 0x000E: // 0x00EE: Returns from subroutine
                    chip->PC = chip->stack[chip->SP];
                    chip->SP--;
                    break;

                default:
                    printf("No such opcode: 0x%X\n", opcode);
                    break;
            }
            break;
        case 0x1: // 1nnn: jump to nnn (address)
            chip->PC = opcode & 0x0FFF;
            break;
        case 0x2: // 2nnn: call subroutine at nnn
            chip->stack[chip->SP] = chip->PC;
            chip->SP++;
            chip->PC = opcode & 0x0FFF;
            break;
        case 0x3: // skip next instruction if V[x] == kk
            if (chip->V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) chip->PC += 4;
            else chip->PC += 2;
            break;
        case 0x4: // skip next instruction if V[x] != kk
            if (chip->V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) chip->PC += 4;
            else chip->PC += 2;
            break;
        case 0x5: // skip next instruction if V[x] == V[y]
            if (chip->V[(opcode & 0x0F00) >> 8] == chip->V[(opcode & 0x00F0) >> 4]) chip->PC += 4;
            else chip->PC += 2;
            break;
        case 0x6: // set V[x] to kk (byte)
            chip->V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            chip->PC += 2;
            break;
        case 0x7: // add kk to V[x]
            chip->V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            chip->PC += 2;
            break;
        case 0x8:
            switch (opcode & 0x000F) {
                case 0x0000: // set Vx = Vy
                    chip->V[(opcode & 0x0F00) >> 8] = chip->V[(opcode & 0x0F00) >> 4];
                    chip->PC += 2;
                    break;
                case 0x0001: // set Vx = Vx OR Vy
                    chip->V[(opcode & 0x0F00) >> 8] |= chip->V[(opcode & 0x0F00) >> 4];
                    chip->PC += 2;
                    break;
                case 0x0002: // set Vx = Vx AND Vy
                    chip->V[(opcode & 0x0F00) >> 8] &= chip->V[(opcode & 0x0F00) >> 4];
                    chip->PC += 2;
                    break;
                case 0x0003: // set Vx = Vx XOR Vy
                    chip->V[(opcode & 0x0F00) >> 8] ^= chip->V[(opcode & 0x0F00) >> 4];
                    chip->PC += 2;
                    break;
                case 0x0004: // set Vx = Vx + Vy, set VF = 1 if addition exceeds 8bits (carry bit)
                    chip->V[0xF] = chip->V[(opcode & 0x00F0) >> 4] > (0xFF - chip->V[(opcode & 0x0F00) >> 8]) ? 1 : 0;
                    chip->V[(opcode & 0x0F00) >> 8] += chip->V[(opcode & 0x00F0) >> 4];
                    chip->PC += 2;
                    break;
                case 0x0005: // set Vx = Vx - Vy, set VF = 1 if not borrowed
                    chip->V[0xF] = chip->V[(opcode & 0x0F00) >> 8] > (0xFF - chip->V[(opcode & 0x00F0) >> 4]) ? 1 : 0;
                    chip->V[(opcode & 0x0F00) >> 8] -= chip->V[(opcode & 0x00F0) >> 4];
                    chip->PC += 2;
                    break;
                case 0x0006: // set Vx = Vx >> 1, set VF = LSB
                    chip->V[0xF] = chip->V[(opcode & 0x0F00) >> 8] & 0x1;
                    chip->V[(opcode & 0x0F00) >> 8] >>= 1;
                    chip->PC += 2;
                    break;
                case 0x0007: // set Vx = Vy - Vx, set VF = 1 if not borrowed
                    chip->V[0xF] = chip->V[(opcode & 0x00F0) >> 4] > (0xFF - chip->V[(opcode & 0x0F00) >> 8]) ? 1 : 0;
                    chip->V[(opcode & 0x0F00) >> 8] = chip->V[(opcode & 0x00F0) >> 4] - chip->V[(opcode & 0x0F00) >> 8];
                    chip->PC += 2;
                    break;
                case 0x000E: // set Vx = Vx << 1, set VF = MSB
                    chip->V[0xF] = (chip->V[(opcode & 0x0F00) >> 8] >> 7) & 0x1;
                    chip->V[(opcode & 0x0F00) >> 8] <<= 1;
                    chip->PC += 2;
                    break;
                default:
                    printf("No such opcode: 0x%X\n", opcode);
                    break;
            }
            break;
        case 0x9: // skip next instruction if Vx != Vy
            if (chip->V[(opcode & 0x0F00) >> 8] != chip->V[(opcode & 0x00F0) >> 4]) chip->PC += 4;
            else chip->PC += 2;
            break;
        case 0xA: // Annn: set I to nnn (address)
            chip->I = opcode % 0x0FFF;
            chip->PC += 2;
            break;
        case 0xB: // jump to nnn + V0
            chip->PC = (opcode & 0x0FFF) + chip->V[0];
            chip->PC += 2;
            break;
        case 0xC: // set Vx = random byte & kk
            chip->V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & rand() % 256;
            chip->PC += 2;
            break;
        case 0xD: // display n-byte sprite, starting at I
        {
            unsigned short x = chip->V[(opcode & 0x0F00) >> 8];
            unsigned short y = chip->V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            chip->V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = chip->memory[chip->I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    // check if current pixel is 1
                    if ((pixel & (0x80 >> xline)) != 0) {
                        // check if pixel on display is set to 1
                        if (chip->display[(x + xline + ((y + yline) * 64))] == 1)
                            chip->V[0xF] = 1; // register collision, set VF to 1
                        chip->display[x + xline + ((y + yline) * 64)] ^= 1; // XOR pixel
                    }
                }
            }

            chip->draw_flag = true;
            chip->PC += 2;
        }
            break;
        case 0xE:
            switch (opcode & 0x00FF) {
                case 0x009E: // skip next instruction if key with value of Vx is pressed
                    if (chip->key[chip->V[(opcode & 0x0F00) >> 8]] != 0) chip->PC += 4;
                    else chip->PC += 2;
                    break;
                case 0x00A1: // skip next instruction if key with value of Vx is not pressed
                    if (chip->key[chip->V[(opcode & 0x0F00) >> 8]] == 0) chip->PC += 4;
                    else chip->PC += 2;
                    break;
                default:
                    printf("No such opcode: 0x%X\n", opcode);
                    break;
            }
            break;
        case 0xF:
            switch (opcode & 0x00FF) {
                case 0x0007: // set Vx = delay_register
                    chip->V[(opcode & 0x0F00) >> 8] = chip->delay_register;
                    chip->PC += 2;
                    break;
                case 0x000A: // wait for key press, store key value in Vx
                    chip->V[(opcode & 0x0F00) >> 8] = wait_and_get_key_value(chip);
                    chip->PC += 2;
                    break;
                case 0x0015: // set delay_register = Vx
                    chip->delay_register = chip->V[(opcode & 0x0F00) >> 8];
                    chip->PC += 2;
                    break;
                case 0x0018: // set sound_register = Vx
                    chip->sound_register = chip->V[(opcode & 0x0F00) >> 8];
                    chip->PC += 2;
                    break;
                case 0x001E: // set I = I + Vx
                    chip->I += chip->V[(opcode & 0x0F00) >> 8];
                    chip->PC += 2;
                    break;
                case 0x0029: // TODO to be implemented
                    chip->I = chip->V[(opcode & 0x0F00) >> 8] * 5;
                    chip->PC += 2;
                    break;
                case 0x0033: // BCD-presentation of Vx at memory[I, I+1, I+2]
                    chip->memory[chip->I] = chip->V[(opcode & 0x0F00) >> 8] / 100;
                    chip->memory[chip->I + 1] = (chip->V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    chip->memory[chip->I + 2] = (chip->V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    chip->PC += 2;
                    break;
                case 0x0055: // store V0...Vx in memory at I
                    for (int i = 0; i < (opcode & 0x0F00) >> 2; i++) {
                        memcpy(&chip->memory[chip->I + i], &chip->V[i], sizeof(uint8_t));
                    }
                    chip->PC += 2;
                    break;
                case 0x0065: // store memory to V0...Vx starting at I
                    for (int i = 0; i < (opcode & 0x0F00) >> 2; i++) {
                        memcpy(&chip->V[i], &chip->memory[chip->I + i], sizeof(uint8_t));
                    }
                    chip->PC += 2;
                    break;
                default:
                    printf("No such opcode: 0x%X\n", opcode);
                    break;
            }
            break;
        default:
            printf("No such opcode: 0x%X\n", opcode);
    }
}

void emulate(Chip_8 *chip) {
    // ---fetch opcode---
    // the opcode is two bytes long, the memory-array contains one byte-addresses each. We concatenate both bytes:
    chip->opcode = chip->memory[chip->PC] << 8 | chip->memory[chip->PC + 1];
    // ---decode & execute opcode---
    decode_and_execute(chip);
    // ---update timers---
    if (chip->delay_register > 0) chip->delay_register--;
    if (chip->sound_register > 0) {
        if (chip->sound_register == 1) printf("sound start\n"); // TODO: beep-sound to be implemented
        chip->sound_register--;
        if (chip->sound_register == 0) printf("sound stop\n");
    }
}

int main(int argc, char *argv[]) {
    printf("Test\n");
    fflush(stdout);
    if (argc != 2) return -1;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (init_graphics(&window, &renderer) == 1) return 1;

    // initialise chip-8
    Chip_8* chip = init_chip();
    load_program_to_memory(chip, argv[1]);

    // Main loop
    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            // ---update key states---
            update_key_states(chip, e);
        }

        emulate(chip);

        if (chip->draw_flag) {
            draw(&renderer, chip);
            chip->draw_flag = false;
        }

    }

    close_graphics(window, renderer);

    return 0;
}
