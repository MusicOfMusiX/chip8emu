//Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//For RNG
#include <time.h>
//Video (and audio, not included yet) manager
#include "SDL.h"

static uint8_t DISPLAY[64][32]; //Setup 2D display array
static int upscale = 10; //Upscale number.

static uint8_t FONTSET[80] =
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

static int KEYMAP[0x10]; //down: 1, up: 0

//Create static variables
static FILE* GAME; //The game ROM
static uint8_t RAM[4096]; //Main memory, 4K

//Setup registers
static uint8_t REGISTERS[16]; 
static uint16_t I;
static uint16_t PC;
static uint8_t SP;
static uint16_t STACK[16];
static uint8_t DT;
static uint8_t ST;

//For SDL, represents all pixels on the window (640x320)
uint32_t pixels[640 * 320];

//Initialise fuctions (Prevents implicit use warnings)
void emuinit(void); //Load game ROM and initialise registers
void execute(void); //The infinite loop where the fetch-decode-execute cycle kicks in
void timer(void); //Manages the two timer registers
void draw(void); //For SDL, draws the pixels - Upscaled.
	
int main(int argc, char *argv[])
	{
	//Initialise SDL components
	int quit = 0;
    SDL_Event event;
 
    SDL_Init(SDL_INIT_VIDEO);
 
    SDL_Window * window = SDL_CreateWindow("chip8emu by H.J", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, 0);
 	
 	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 640, 320);
 	
    memset(pixels, 200, 640 * 320 * sizeof(Uint32)); //Fill pixel array with 0s
    
	//Emulator init
    emuinit();
	
    while (!quit) //Key input - Execute - Draw loop
    	{
    	SDL_UpdateTexture(texture, NULL, pixels, 640 * sizeof(Uint32));

        SDL_PollEvent(&event);
 
        switch (event.type)
        	{
	        case SDL_QUIT:
	            quit = 1;
	        break;
	        case SDL_KEYDOWN: //This key input code is temporary, and should be more efficient/hardware specific code
			    switch (event.key.keysym.sym)
			    	{
			    	case SDLK_0:  KEYMAP[0x0] = 1; break;
			    	case SDLK_1:  KEYMAP[0x1] = 1; break;
			    	case SDLK_2:  KEYMAP[0x2] = 1; break;
			    	case SDLK_3:  KEYMAP[0x3] = 1; break;
			    	case SDLK_4:  KEYMAP[0x4] = 1; break;
			    	case SDLK_5:  KEYMAP[0x5] = 1; break;
			    	case SDLK_6:  KEYMAP[0x6] = 1; break;
			    	case SDLK_7:  KEYMAP[0x7] = 1; break;
			    	case SDLK_8:  KEYMAP[0x8] = 1; break;
			    	case SDLK_9:  KEYMAP[0x9] = 1; break;
			    	case SDLK_a:  KEYMAP[0xa] = 1; break;
			    	case SDLK_b:  KEYMAP[0xb] = 1; break;
			    	case SDLK_c:  KEYMAP[0xc] = 1; break;
			    	case SDLK_d:  KEYMAP[0xd] = 1; break;
			    	case SDLK_e:  KEYMAP[0xe] = 1; break;
			    	case SDLK_f:  KEYMAP[0xf] = 1; break;
			    	}
			break;
			case SDL_KEYUP: //Same goes for this
			    switch (event.key.keysym.sym)
			    	{
			        case SDLK_0:  KEYMAP[0x0] = 0; break;
			    	case SDLK_1:  KEYMAP[0x1] = 0; break;
			    	case SDLK_2:  KEYMAP[0x2] = 0; break;
			    	case SDLK_3:  KEYMAP[0x3] = 0; break;
			    	case SDLK_4:  KEYMAP[0x4] = 0; break;
			    	case SDLK_5:  KEYMAP[0x5] = 0; break;
			    	case SDLK_6:  KEYMAP[0x6] = 0; break;
			    	case SDLK_7:  KEYMAP[0x7] = 0; break;
			    	case SDLK_8:  KEYMAP[0x8] = 0; break;
			    	case SDLK_9:  KEYMAP[0x9] = 0; break;
			    	case SDLK_a:  KEYMAP[0xa] = 0; break;
			    	case SDLK_b:  KEYMAP[0xb] = 0; break;
			    	case SDLK_c:  KEYMAP[0xc] = 0; break;
			    	case SDLK_d:  KEYMAP[0xd] = 0; break;
			    	case SDLK_e:  KEYMAP[0xe] = 0; break;
			    	case SDLK_f:  KEYMAP[0xf] = 0; break;

			    	}
			break;
        	}
        
		//Execute
		execute();
 		timer();
		
		//Draw
        int x,y,sx,sy;
		for(y=0;y<32;y++)
			{
			for(x=0;x<64;x++)
				{
				if(DISPLAY[x][y] == 1)
					{
					for(sy=0;sy<upscale;sy++)
						{
						for(sx=0;sx<upscale;sx++)
							{
							pixels[(x*upscale) + (y*upscale)*64*upscale + sx + (sy*64*upscale)] =  0xFFFFFFFF;	
							}
						}	
					}
				if(DISPLAY[x][y] == 0)
					{
					for(sy=0;sy<upscale;sy++)
						{
						for(sx=0;sx<upscale;sx++)
							{
							pixels[(x*upscale) + (y*upscale)*64*upscale + sx + (sy*64*upscale)] =  0x000000FF;	
							}
						}	
					}
				}
			}
    
	    SDL_RenderClear(renderer);
	    SDL_RenderCopy(renderer, texture, NULL, NULL);
	    SDL_RenderPresent(renderer);
    	}
 	SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
 
    return 0;
	}

void emuinit()
	{
	//Setting the seed for RNG
	srand(time(NULL));
	
	//Copy 0 to all spaces in the specified arrays
	memset(RAM,0,sizeof(RAM));
	memset(DISPLAY,0,sizeof(DISPLAY));
	memset(STACK,0,sizeof(STACK));
	memset(REGISTERS,0,sizeof(REGISTERS));
	
	int i;
	//Copy fontset into main memory
	for(i=0;i<80;i++)
		{
		RAM[i] = FONTSET[i];
		}
	
	//Read ROM file
	GAME = fopen("INVADERS.c8", "rb");
	if(!GAME)
		{
        printf("ERROR WHILE OPENING GAME FILE\n\n");
        fflush(stdin); 
        getchar();
        exit(1);
    	}
		
	fread(RAM+0x200,1,4096-0x200,GAME); //Copy game data, to RAM+0x200, one byte at a time, with a total size of 4096-0x200, from GAME
	
	PC = 0x200; //Default program start address
	SP = 0; //At the bottom of the stack 'container', but as there is no data in the stack, it is at the 'top' of  the stack.
	
	int delay;	
	}

void timer()
	{
	if(DT > 0) //Delay timer
		{
		DT--;
		}	
	if(ST > 0) //Sound timer
		{
		ST--;	
		}
	}
	
void execute()
	{
	int INSTRUCTION[4]; //Instruction hex digits
	int i,j; //Variables to use in loops, etc.
	int randomnum = rand() % 0xFF; //RNG "rolling the dice"
	int drawbit;
	int drawx, drawy, drawflag;
	uint16_t sum;

	//Divide each Hex digit of opcode to an array
	for(i=0;i<4;i++)
		{
		uint16_t inst = RAM[PC]*0x100 + RAM[PC+1]; //Get full opcode/instruction
		INSTRUCTION[i] = ((inst << i*4) & 0xFFFF)/0x1000; 
		}
	
	int nnn = INSTRUCTION[3]*0x1 + INSTRUCTION[2]*0x10 + INSTRUCTION[1]*0x10*0x10; //The last 12-bits of the opcode, for convinience
	int kk = INSTRUCTION[3]*0x1 + INSTRUCTION[2]*0x10; //The last 8-bits of the opcode, for convinience
	
	//Debug purposes
	printf("\nExecuting opcode: %X%X%X%X at %04X, Pre-execution status: I: %04X, SP: %02X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3],PC,I,SP);
	
	//The main switch where the 'decode' and 'execute' steps of the cycle happens
	switch(INSTRUCTION[0])
		{
		case 0:
			switch(INSTRUCTION[3])
				{
				case 0:
					memset(DISPLAY,0,sizeof(DISPLAY)); //CLEAR SCREEN
					PC += 2; //Move on to the next instruction
				break;
				case  0xE:
					PC = STACK[SP--]; //RETURN FROM SUBROUTINE
					PC += 2; //Move on to the next instruction
				break;
				default:
					printf("\nUNIDENTIFIED OPCODE!: %X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
					PC += 2; //Move on to the next instruction
				}	
		break;
		case 1: //JUMP TO ADDRESS NNN
			PC = nnn;
		break;
		case 2:
			STACK[++SP] = PC; //SAVE CURRENT ADDRESS AND CALL SUBROUTINE
			PC = nnn;
		break;
		case 3:
			if(REGISTERS[INSTRUCTION[1]] == kk) //SKIP NEXT INSTRUCTION IF VX = KK
				{
				PC += 2;	
				}
			PC += 2; //Move on to the next instruction
		break;
		case 4:
			if(REGISTERS[INSTRUCTION[1]] != kk) //SKIP NEXT INSTRUCTION IF VX != KK
				{
				PC += 2;	
				}
			PC += 2; //Move on to the next instruction
		break;
		case 5:
			if(REGISTERS[INSTRUCTION[1]] == REGISTERS[INSTRUCTION[2]]) //SKIP NEXT INSTRUCTION IF VX = VY
				{
				PC += 2;	
				}
			PC += 2; //Move on to the next instruction
		break;
		case 6:
			REGISTERS[INSTRUCTION[1]] = kk; //VX = KK
			PC += 2; //Move on to the next instruction
		break;
		case 7:
			REGISTERS[INSTRUCTION[1]] += kk; //VX = VX + KK
			PC += 2; //Move on to the next instruction
		break;
		case 8:
			switch(INSTRUCTION[3])
				{
				case 0:
					REGISTERS[INSTRUCTION[1]] = REGISTERS[INSTRUCTION[2]]; //VX = VY
					PC += 2; //Move on to the next instruction
				break;
				case 1:
					REGISTERS[INSTRUCTION[1]] |= REGISTERS[INSTRUCTION[2]]; //VX = VX OR VY
					PC += 2; //Move on to the next instruction
				break;
				case 2:
					REGISTERS[INSTRUCTION[1]] &= REGISTERS[INSTRUCTION[2]]; //VX = VX AND VY
					PC += 2; //Move on to the next instruction
				break;
				case 3:
					REGISTERS[INSTRUCTION[1]] ^=  REGISTERS[INSTRUCTION[2]]; //VX = VX XOR VY
					PC += 2; //Move on to the next instruction
				break;
				case 4: //VX = VX + VY, VF = CARRY
					REGISTERS[0xF] = 0;	
					sum = REGISTERS[INSTRUCTION[1]] + REGISTERS[INSTRUCTION[2]]; //sum is 16-bit, so it can handle >255 numbers
					if(sum > 255)
						{
						REGISTERS[0xF] = 1;
						}
					REGISTERS[INSTRUCTION[1]] += REGISTERS[INSTRUCTION[2]];
					PC += 2; //Move on to the next instruction
				break;
				case 5: //VX = VX - VY, VF = BORROW
					REGISTERS[0xF] = 0;	
					if(REGISTERS[INSTRUCTION[1]] > REGISTERS[INSTRUCTION[2]])
						{
						REGISTERS[0xF] = 1;	
						}
					REGISTERS[INSTRUCTION[1]] -= REGISTERS[INSTRUCTION[2]];
					PC += 2; //Move on to the next instruction
				break;
				case 6:
					REGISTERS[0xF] = REGISTERS[INSTRUCTION[1]] & 0x1; //STORE LEAST SIGNIFICANT BIT IN 0xF REGISTER
					REGISTERS[INSTRUCTION[1]] /= 2; //VX >> 1
					PC += 2; //Move on to the next instruction
				break;
				case 7: //VX = VY - VX, VF = BORROW
					REGISTERS[0xF] = 0;	
					if(REGISTERS[INSTRUCTION[2]] > REGISTERS[INSTRUCTION[1]])
						{
						REGISTERS[0xF] = 1;	
						}
					REGISTERS[INSTRUCTION[1]] = REGISTERS[INSTRUCTION[2]] - REGISTERS[INSTRUCTION[1]];
					PC += 2; //Move on to the next instruction
				break;
				case 0xE:
					REGISTERS[0xF] = (REGISTERS[INSTRUCTION[1]] & 0x80)/0x80; //STORE MOST SIGNIFICANT BIT IN 0xF REGISTER
					REGISTERS[INSTRUCTION[1]] *= 2; //VX << 1
					PC += 2; //Move on to the next instruction
				break;
				default:
					printf("\nUNIDENTIFIED OPCODE!: %X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
					PC += 2; //Move on to the next instruction
				}
		break;
		case 9:
			if(REGISTERS[INSTRUCTION[1]] != REGISTERS[INSTRUCTION[2]])
				{
				PC += 2; //SKIP NEXT INSTRUCTION IF VX = VY
				}
			PC += 2; //Move on to the next instruction
		break;
		case 0xA:
			I = nnn; //I = NNN
			PC += 2; //Move on to the next instruction
		break;
		case 0xB:
			PC = nnn + REGISTERS[0x0]; //JUMP TO NNN + 0x0 REGISTER
		break;
		case 0xC:
			REGISTERS[INSTRUCTION[1]] = randomnum & kk; //VX = RANDOM NUMBER AND KK
			PC += 2; //Move on to the next instruction
		break;
		case 0xD:
			//DRAW GRAPHICS
			for(i=0;i<INSTRUCTION[3];i++) //REPEAT (HEIGHT) TIMES
				{
				for(j=0;j<8;j++) //FOR ALL BITS
					{
					drawbit = ((RAM[I+i] << j) & 0xFF)/0x80;
					if (drawbit == 1)
						{
						drawflag = 1;
						drawx = REGISTERS[INSTRUCTION[1]]+j;
						drawy = REGISTERS[INSTRUCTION[2]]+i;
						if((drawx <= 63) && (drawy <= 31))
							{
							if(DISPLAY[drawx][drawy] != 0)
								{
								REGISTERS[0xF] = 1;
								}
							DISPLAY[drawx][drawy] ^= 1;
							}
						}
					}	
				}
			PC += 2; //Move on to the next instruction
		break;
		case 0xE:
			switch(INSTRUCTION[2])
				{
				case 9:
					if(KEYMAP[REGISTERS[INSTRUCTION[1]]] == 1)
						{
						PC += 2; //SKIP NEXT INSTRUCTION IF KEY NO. X IS PRESSED
						}
					PC += 2; //Move on to the next instruction
				break;
				case 0xA:
					if(KEYMAP[REGISTERS[INSTRUCTION[1]]] == 0)
						{
						PC += 2; //SKIP NEXT INSTRUCTION IF KEY NO. X IS NOT PRESSED
						}
					PC += 2; //Move on to the next instruction
				break;	
				default:
					printf("\nUNIDENTIFIED OPCODE!: %X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
					PC += 2; //Move on to the next instruction
				}
		break;
		case 0xF:
			switch(INSTRUCTION[2])
				{
				case 0:
					switch(INSTRUCTION[3])
						{
						case 7:
							REGISTERS[INSTRUCTION[1]] = DT; //VX = DT
							PC += 2; //Move on to the next instruction
						break;
						case 0xA: //WAIT UNTIL ANY KEYPRESS
							for(i=0;i<16;i++)
								{
								if(KEYMAP[i] == 1)
									{
									REGISTERS[INSTRUCTION[1]] = i;
									i = 16; //Escape from loop
									PC += 2; //Only move on to the next instruction when the conditions are met
									}
								}
						break;
						default:
							printf("\nUNIDENTIFIED OPCODE!: %X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
							PC += 2; //Move on to the next instruction
						}
				break;
				case 1:
					switch(INSTRUCTION[3])
						{
						case 5:
							DT = REGISTERS[INSTRUCTION[1]]; //DT = VX
							PC += 2; //Move on to the next instruction
						break;
						case 8:
							ST = REGISTERS[INSTRUCTION[1]]; //ST = VX
							PC += 2; //Move on to the next instruction
						break;
						case 0xE:
							I += REGISTERS[INSTRUCTION[1]]; //I += VX
							PC += 2; //Move on to the next instruction
						break;
						default:
							printf("\nUNIDENTIFIED OPCODE!: %X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
							PC += 2; //Move on to the next instruction
						}
				break;
				case 2: //I = ADDRESS OF VX DIGIT SPRITE
					I = REGISTERS[INSTRUCTION[1]]*5; //Since I set the location for the fontsets, I need to tell the interpreter where it is
					PC += 2;
				break;
				case 3: //STORE BCD OF VX
					RAM[I] = REGISTERS[INSTRUCTION[1]]/100;
					RAM[I+1] = (REGISTERS[INSTRUCTION[1]]/10)%10;
					RAM[I+2] = REGISTERS[INSTRUCTION[1]]%10;
					PC += 2; //Move on to the next instruction
				break;
				case 5: //STORE V0 ~ VX STARTING AT I
					for(i=0;i<=INSTRUCTION[1];i++) //Be careful! I need to INCLUDE Vx! 
						{
						RAM[I+i] = REGISTERS[i];	
						}
					I = I + INSTRUCTION[1] + 1;
					
					PC += 2; //Move on to the next instruction
				break;
				case 6: //STORE TO V0 ~ VX STARTING AT I
					for(i=0;i<=INSTRUCTION[1];i++) //Be careful! I need to INCLUDE Vx! 
						{
						REGISTERS[i] = RAM[I+i];	
						}
						
					I = I + INSTRUCTION[1] + 1;
					
					PC += 2; //Move on to the next instruction
				break;
				default:
					printf("\nUNIDENTIFIED OPCODE!: %X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
					PC += 2; //Move on to the next instruction
				}
		break;
		default:
			printf("\nUNIDENTIFIED OPCODE!: %X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
			PC += 2; //Move on to the next instruction
		}
	
	}
