#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//RNG!!
#include <time.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

/*
##########################################################
REMEMBER TO COPY THE sdl.dll TO THE OUTPUT FOLDER!!!!!!!!!
##########################################################
*/

#include "SDL2/SDL.h"

//CHIP 8 STARTS HERE

static uint8_t DISPLAY[64][32];

//Fontset, brought from external code
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
static FILE* GAME;
static uint8_t RAM[4096];
/*
[http://www.multigesture.net/wp-content/uploads/mirror/goldroad/chip8.shtml]
I have set up the memory as the char variable type (uint8_t), that is, 1 byte.
Why have I done this when all chip 8 instructions are two bytes long??
Well, if part of the code is data (ie. sprites) and there is not an
even number of bytes, then instructions will become unaligned and
you will not be able to read them properly if you allocate memory
in two byte portions.
*/

//If the container (RAM) is aligned with 2 bytes, odd-number sized data (such as sprites) can produce blank spaces.
//If the container is made up of the minimal unit (1 byte), No blank spaces will be formed.

//*****THIS IS WHY WE ADD 2 TO THE PROGRAM COUNTER EVERY TIME AN OPCODE EXECUTES!!!!*****
static uint8_t REGISTERS[16]; 
static uint16_t I;
static uint16_t PC;
static uint8_t SP;
static uint16_t STACK[16];
static uint8_t DT;
static uint8_t ST;

uint32_t pixels[640 * 320];

//Initialise fuctions (Prevents implicit use warnings)
void emuinit(void);
void execute(void);
void timer(void);
void draw(void);
void vdraw(void);
	
int main(int argc, char *argv[])
	{
	//#####SDL RELATED INIT#####
	int quit = 0;
    SDL_Event event;
 
    SDL_Init(SDL_INIT_VIDEO);
 
    SDL_Window * window = SDL_CreateWindow("chip8emu testing",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, 0);
 	
 	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 640, 320);
 	
    memset(pixels, 200, 640 * 320 * sizeof(Uint32));
    
    emuinit();
    while (!quit)
    	{
    	SDL_UpdateTexture(texture, NULL, pixels, 640 * sizeof(Uint32));
        //SDL_WaitEvent(&event); //This thing just 'waits' for an event to happen
        SDL_PollEvent(&event); //This thing runs the code
 
        switch (event.type)
        	{
	        case SDL_QUIT:
	            quit = 1;
	        break;
	        case SDL_KEYDOWN:
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
			case SDL_KEYUP:
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
        
        int x,y,sx,sy;
		for(y=0;y<32;y++)
			{
			for(x=0;x<64;x++)
				{
				if(DISPLAY[x][y] == 1)
					{
					for(sy=0;sy<10;sy++)
						{
						for(sx=0;sx<10;sx++)
							{
							pixels[(x*10) + (y*10)*640 + sx + (sy*640)] =  0xFFFFFFFF;	
							}
						}	
					}
				if(DISPLAY[x][y] == 0)
					{
					for(sy=0;sy<10;sy++)
						{
						for(sx=0;sx<10;sx++)
							{
							pixels[(x*10) + (y*10)*640 + sx + (sy*640)] =  0x000000FF;	
							}
						}	
					}
				}
			}
			
		//vdraw();
		
 		execute();
 		timer();
 		
    
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
	//RNG setting the seed
	srand(time(NULL));
	
	//Copy 0 to all spaces in the specified arrays
	memset(RAM,0,sizeof(RAM));
	memset(DISPLAY,0,sizeof(DISPLAY));
	memset(STACK,0,sizeof(STACK));
	memset(REGISTERS,0,sizeof(REGISTERS));
	
	int i;
	//Copy fontset into the RAM
	for(i=0;i<80;i++)
		{
		RAM[i] = FONTSET[i];
		}
	
	//Read file
	GAME = fopen("PONG.c8", "rb");
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
	if(DT > 0)
		{
		DT--;
		}	
	if(ST > 0)
		{
		ST--;	
		}
	}

void vdraw()
	{
	int i,j;
	printf("\n###########################################################################################################\n\n");
	for(i=0;i<32;i++)
		{
		for(j=0;j<64;j++)
			{
			printf("%d ",pixels[j+i*640]);	
			}
		printf("\n");
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
	
	
	for(i=0;i<4;i++)
		{
		//*****VERY IMPORTANT!!!!!*****
		uint16_t inst = RAM[PC]*0x100 + RAM[PC+1]; //As each opcode is consuming 2 'slots' (bytes) of RAM, a full 2-byte opcode should be made by combining two bytes together
		//Assigning each 4-bit hex code of the opcode to an array
		INSTRUCTION[i] = ((inst << i*4) & 0xFFFF
		/*For example when I shift 1011 1 bit left, 10110 will be produced. Therefore I need to mask it with an AND operator.
		Also, even though inst is a 16-bit variable, inst << i*4 will create a NEW piece of temporary data, so I cannot avoid the masking process.*/
		)/0x1000; //I could just do inst & 0xFFFF, inst & 0x0FFF... and so on, but that would require and additional for loop 
		}
	
	int nnn = INSTRUCTION[3]*0x1 + INSTRUCTION[2]*0x10 + INSTRUCTION[1]*0x10*0x10; //The last 12-bits of the opcode, for convinience
	int kk = INSTRUCTION[3]*0x1 + INSTRUCTION[2]*0x10; //The last 8-bits of the opcode, for convinience
	
	//printf("\nExecuting opcode: %X%X%X%X at %04X, I: %04X, SP: %02X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3],PC,I,SP);
	//printf("Registers: ");
	//printf("%X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
	for(i=0;i<16;i++)
		{
		//printf("V%X: %02X, ",i,REGISTERS[i]);	
		}
	//printf("\n\n");
	
	switch(INSTRUCTION[0])
		{
		case 0:
			switch(INSTRUCTION[3])
				{
				case 0:
					//CLEAR DISPLAY
					memset(DISPLAY,0,sizeof(DISPLAY));
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
					REGISTERS[INSTRUCTION[1]] = REGISTERS[INSTRUCTION[1]] | REGISTERS[INSTRUCTION[2]]; //VX = VX OR VY
					PC += 2; //Move on to the next instruction
				break;
				case 2:
					REGISTERS[INSTRUCTION[1]] = REGISTERS[INSTRUCTION[1]] & REGISTERS[INSTRUCTION[2]]; //VX = VX AND VY
					PC += 2; //Move on to the next instruction
				break;
				case 3:
					REGISTERS[INSTRUCTION[1]] = REGISTERS[INSTRUCTION[1]] ^ REGISTERS[INSTRUCTION[2]]; //VX = VX XOR VY
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
					REGISTERS[INSTRUCTION[1]] = REGISTERS[INSTRUCTION[1]] - REGISTERS[INSTRUCTION[2]];
					PC += 2; //Move on to the next instruction
				break;
				case 6:
					REGISTERS[0xF] = 0;	
					if((REGISTERS[INSTRUCTION[1]] & 0x1) == 1)
						{
						REGISTERS[0xF] = 1;	
						}
					//REGISTERS[INSTRUCTION[1]] /= 2;
					REGISTERS[INSTRUCTION[1]] = (REGISTERS[INSTRUCTION[2]] >> 1) & 0xFF;
					PC += 2; //Move on to the next instruction
				break;
				case 7:
					REGISTERS[0xF] = 0;	
			
					if(REGISTERS[INSTRUCTION[2]] > REGISTERS[INSTRUCTION[1]])
						{
						REGISTERS[0xF] = 1;	
						}
					REGISTERS[INSTRUCTION[1]] = REGISTERS[INSTRUCTION[2]] - REGISTERS[INSTRUCTION[1]];
					PC += 2; //Move on to the next instruction
				break;
				case 0xE:
					REGISTERS[0xF] = 0;
					if((REGISTERS[INSTRUCTION[1]] & 0x80) == 0x80)
						{
						REGISTERS[0xF] = 1;	
						}
					//REGISTERS[INSTRUCTION[1]] *= 2;
					REGISTERS[INSTRUCTION[1]] = (REGISTERS[INSTRUCTION[2]] << 1) & 0xFF;
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
				PC += 2;	
				}
			PC += 2; //Move on to the next instruction
		break;
		case 0xA:
			I = nnn;
			PC += 2; //Move on to the next instruction
		break;
		case 0xB:
			PC = nnn + REGISTERS[0x0];
		break;
		case 0xC:
			REGISTERS[INSTRUCTION[1]] = randomnum & kk;
			PC += 2; //Move on to the next instruction
		break;
		case 0xD:
			//DRAW GRAPHICS
			for(i=0;i<INSTRUCTION[3];i++)
				{
				for(j=0;j<8;j++)
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
						PC += 2;	
						}
					PC += 2; //Move on to the next instruction
				break;
				case 0xA:
					if(KEYMAP[REGISTERS[INSTRUCTION[1]]] == 0)
						{
						PC += 2;	
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
							REGISTERS[INSTRUCTION[1]] = DT;
							PC += 2; //Move on to the next instruction
						break;
						case 0xA:
							for(i=0;i<16;i++)
								{
								if(KEYMAP[i] == 1)
									{
									REGISTERS[INSTRUCTION[1]] = i;
									i = 16; //escape!
									PC += 2; //Since this PC increment statement is inside the loop, we can make the program 'hang'
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
							DT = REGISTERS[INSTRUCTION[1]];
							PC += 2; //Move on to the next instruction
						break;
						case 8:
							ST = REGISTERS[INSTRUCTION[1]];
							PC += 2; //Move on to the next instruction
						break;
						case 0xE:
							I += REGISTERS[INSTRUCTION[1]];
							PC += 2; //Move on to the next instruction
						break;
						default:
							printf("\nUNIDENTIFIED OPCODE!: %X%X%X%X\n",INSTRUCTION[0],INSTRUCTION[1],INSTRUCTION[2],INSTRUCTION[3]);
							PC += 2; //Move on to the next instruction
						}
				break;
				case 2:
					I = REGISTERS[INSTRUCTION[1]]*5; //Since I set the location for the fontsets, I need to tell the interpreter where it is
					PC += 2;
				break;
				case 3:
					RAM[I] = REGISTERS[INSTRUCTION[1]]/100;
					RAM[I+1] = (REGISTERS[INSTRUCTION[1]]/10)%10;
					RAM[I+2] = REGISTERS[INSTRUCTION[1]]%10;
					PC += 2; //Move on to the next instruction
				break;
				case 5:
					for(i=0;i<=INSTRUCTION[1];i++) //Be careful! I need to INCLUDE Vx! 
						{
						RAM[I+i] = REGISTERS[i];	
						}
					
					I = I + INSTRUCTION[1] + 1;
					
					PC += 2; //Move on to the next instruction
				break;
				case 6:
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
