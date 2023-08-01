#define SDL_MAIN_HANDLED 1
#define BOARD_SIZE 1500
#define NUM_IMAGES 9
#define IMAGE_SIZE 4
#define TIME_BTWN_FRAMES 16

#define COLOR_LIMIT 150
#define RENDER_OFFSET_X IMAGE_SIZE - (BOARD_SIZE * IMAGE_SIZE / 2) + (SCREEN_WIDTH / 2)
#define RENDER_OFFSET_Y IMAGE_SIZE - (BOARD_SIZE * IMAGE_SIZE / 2) + (SCREEN_HEIGHT / 2)
//#define LIMIT_FRAMERATE true
//#define CELL_LIMIT 30000

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <random>
#include <vector>
#include "SDL.h"
#include <SDL_image.h>
#include <chrono>

using namespace std;

SDL_Surface** texStorage;

class Cell
{
	public:
	Cell(SDL_Renderer* render)
	{
		texture = SDL_CreateTextureFromSurface(render, texStorage[rand() % NUM_IMAGES]);
		SDL_SetTextureColorMod(texture, rand() % COLOR_LIMIT, rand() % COLOR_LIMIT, rand() % COLOR_LIMIT);
		
		construct(render);
	}
	
	Cell(SDL_Renderer* render, SDL_Texture* text)
	{
		texture = text;
		construct(render);
	}
	
	~Cell()
	{
		//SDL_DestroyTexture(texture);
	}
	
	void construct(SDL_Renderer* render)
	{
		ren = render;
		rect.x = rect.y = 0;
		rect.w = rect.h = IMAGE_SIZE;
		
		// 15 10
		timeToDeath = (rand() % 6) + 7;
		resetRepro();
	}
	
	void resetRepro()
	{
		// 10 10
		// 15 7
		timeToRepro = (rand() % 7) + 5;
	}
	
	void setPos(int x, int y)
	{
		rect.x = x;
		rect.y = y;
	}
	/*
	int* getPos()
	{
		int Pos[2] = {rect.x, rect.y};
		return Pos;
	}
	*/
	void setTexture(SDL_Texture* text)
	{
		SDL_DestroyTexture(texture);
		texture = text;
	}
	
	SDL_Texture* getTexture()
	{
		return texture;
	}
	
	bool isDead()
	{
		return timeToDeath <= 0;
	}
	
	void timeStep()
	{
		timeToDeath--;
		timeToRepro--;
	}
	
	void kill(bool** board)
	{
		// I found out why there was a diagonal...
		// board[rect.x][rect.y] = false;
		
		board[rect.y][rect.x] = false;
	}
	
	
	
	void reproduce(vector<Cell*>* cells, bool** board)
	{
		/*
			0 = up
			1 = right
			2 = down
			3 = left
		*/
		
		// If not time to repro, then skip this
		if (timeToRepro > 0) return;
		
		# ifdef CELL_LIMIT
		if (cells->size() > CELL_LIMIT) 
		{
			resetRepro();
			return;
		}
		# endif
		
		for (int i = 0; i < 4; i++)
		{
			if ((rand() % 2) == 1)
			{
				int newX = rect.x, newY = rect.y;
				switch(i)
				{
					case 0:
						newY -= 1;
						break;
					case 1:
						newX += 1;
						break;
					case 2:
						newY += 1;
						break;
					case 3:
						newX -= 1;
						break;
				}
				
				SDL_Rect tempRect;
				tempRect.x = newX * RENDER_OFFSET_X;
				tempRect.y = newY * RENDER_OFFSET_Y;
				
				if ( tempRect.x > 0 && tempRect.x < SCREEN_WIDTH &&
					tempRect.y > 0 && tempRect.y < SCREEN_HEIGHT &&
					!board[newY][newX])
					
				{
					// Successful reproduction!
					Cell* newCell;
					
					// Set texture to parent's if it failed to speciate
					if ( (rand() % 1000) > 2 )
						newCell = new Cell(ren, texture);
					else
						newCell = new Cell(ren);
					
					board[newY][newX] = true;
					newCell->setPos(newX, newY);
					
					cells->push_back(newCell);
				}
			}
		}
		
		resetRepro();
	}
	
	void render()
	{
		SDL_Rect tempRect;
		tempRect.x = rect.x * RENDER_OFFSET_X;
		tempRect.y = rect.y * RENDER_OFFSET_Y;
		tempRect.w = tempRect.h = IMAGE_SIZE;
		
		 // if ( tempRect.x > 0 && tempRect.x < SCREEN_WIDTH &&
				 // tempRect.y > 0 && tempRect.y < SCREEN_HEIGHT )
				
			SDL_RenderCopyEx(ren, texture, NULL, &tempRect, 0, NULL, SDL_FLIP_NONE);
	}
	
	private:
	SDL_Rect rect;
	SDL_Texture* texture;
	SDL_Renderer* ren;
	
	int timeToDeath;
	int timeToRepro;
};

int main (int len, char** args)
{
	srand((unsigned) time(NULL));
	
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	
	SDL_Window* win = SDL_CreateWindow("CDoodle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (win == nullptr)
	{
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	#ifdef LIMIT_FRAMERATE
	int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	#else
	int flags = SDL_RENDERER_ACCELERATED;
	#endif
	
	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, flags);
	if (ren == nullptr)
	{
		SDL_DestroyWindow(win);
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	
	//Initialize PNG loading
	if( (IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG) != IMG_INIT_PNG )
	{
		std::cout <<  "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}
	
	
	// Load textures for cells
	SDL_Surface* texs[9];
	
	for (int j = 1; j <= NUM_IMAGES; j++)
	{
		ostringstream str;
		str << "./images/cell" << j << ".png";
		texs[j-1] = IMG_Load(str.str().c_str() );
		// int ran = rand() % 255;
		// SDL_SetTextureColorMod(texs[j-1], rand() % 255, rand() % 255, rand() % 255);
		// SDL_SetTextureAlphaMod(texs[j-1], 255);
		// SDL_SetTextureBlendMode(texs[j-1], SDL_BLENDMODE_BLEND);
	}
	// Save pointer to textures in global
	texStorage = texs;
	
	// Initialize variables for board and cells
	SDL_Event event;
	bool** board;
	

	board = (bool**)calloc(sizeof(bool), BOARD_SIZE);
	
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		//cout << i << endl;
		board[i] = (bool*)calloc(sizeof(bool), BOARD_SIZE);
	}
	
	vector<Cell*> cells;
	
	const int firstCellLoc = BOARD_SIZE/2;
	board[firstCellLoc][firstCellLoc] = true;
	Cell* firstCell = new Cell(ren);
	firstCell->setPos(firstCellLoc, firstCellLoc);
	cells.push_back(firstCell);
	
	SDL_Texture* firstColTex = firstCell->getTexture();
	Uint8 r,g,b;
	SDL_GetTextureColorMod(firstColTex, &r, &g, &b);
	int flasher = 0;
	int flasherDown = 1;
	chrono::high_resolution_clock::time_point frameStart;
	chrono::milliseconds timeOnFrame (0);
	
	int flasherSpeed = (1000 / TIME_BTWN_FRAMES) / 11;
	
	while (true)
	{
		frameStart = chrono::high_resolution_clock::now();
		
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				SDL_Quit();
				return 1;
			}
		}
		
		//First clear the renderer
		SDL_RenderClear(ren);
		
		// Handle first colony flashing
		flasher += flasherSpeed * flasherDown;
		
		if (flasher >= 255 - flasherSpeed)
			flasherDown = -1;
		else if (flasher <= 0 + flasherSpeed)
			flasherDown = 1;
		
		SDL_SetTextureColorMod(firstColTex, (int)fmax(r, flasher), (int)fmax(g, flasher), (int)fmax(b, flasher));
		
		
		// Take a time step!
		for (unsigned i = 0; i < cells.size(); i++)
		{
			cells[i]->timeStep();
			
			// Be a little more forgiving in the beginning 
			if (cells[i]->isDead() && cells.size() > 3)
			{
				cells[i]->kill(board);
				delete cells[i];
				cells.erase(cells.begin() + i--);
				continue;
			}
			
			cells[i]->reproduce(&cells, board);
			cells[i]->render();
		}
	
		//Update the screen
		SDL_RenderPresent(ren);
		
		timeOnFrame = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - frameStart);
		std::cout << "Time spent on frame: " << timeOnFrame.count() << endl;
		//SDL_Delay(TIME_BTWN_FRAMES - (int)timeOnFrame.count());
	}
	
	
	
	
	
	
	// system("pause");
	return 0;
}