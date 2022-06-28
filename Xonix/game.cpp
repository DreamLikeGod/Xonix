#include "game.h"

#include <random>
#include <ctime>

void initMap(Map& map)
{
	map.col = (GAME_HEIGHT - RECORDS_OFFSET) / BLOCK_WIDTH;
	map.row = GAME_WIDTH / BLOCK_WIDTH;

	for (int i = 0; i < map.row; i++)
	{
		for (int j = 0; j < map.col; j++)
		{
			map.blocks[i][j].x = i;
			map.blocks[i][j].y = j;
			if (i == 0 || j == 0 || i == map.row - 1 || j == map.col - 1)
			{
				map.blocks[i][j].status = typeBlock::CAPTURED;
			}
			else
			{
				map.blocks[i][j].status = typeBlock::NONCAPTURED;
			}
		}
	}
}

void initRecordsBox(RecordsBox& rBox)
{
	rBox.border.y = GAME_HEIGHT - RECORDS_OFFSET;
	rBox.border.x = 0;
	rBox.border.w = GAME_WIDTH;
	rBox.border.h = RECORDS_OFFSET;

	rBox.box.x = rBox.border.x + 2;
	rBox.box.y = rBox.border.y + 2;
	rBox.box.w = rBox.border.w - 4;
	rBox.box.h = rBox.border.h - 4;

	rBox.score = 0;
	rBox.percent = 0;

	initFont(rBox.font);
	SDL_Color color = { 0, 0, 0 };
	rBox.textScore = TTF_RenderUTF8_Blended(rBox.font, "Очки: 0", color);
	rBox.textPerc = TTF_RenderUTF8_Blended(rBox.font, "Захват: 0/100", color);
}

void initWinLose(SDL_Texture*& win, SDL_Texture*& lose, SDL_Renderer* renderer)
{
	SDL_Surface* tmp = SDL_LoadBMP("resources/win.bmp");
	SDL_SetColorKey(tmp, SDL_TRUE, SDL_MapRGB(tmp->format, 255, 255, 255));

	win = SDL_CreateTextureFromSurface(renderer, tmp);
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP("resources/lose.bmp");
	SDL_SetColorKey(tmp, SDL_TRUE, SDL_MapRGB(tmp->format, 255, 255, 255));

	lose = SDL_CreateTextureFromSurface(renderer, tmp);
	SDL_FreeSurface(tmp);
}

void renderMap(SDL_Renderer* renderer, Map& map)
{
	SDL_Rect rect;
	rect.w = rect.h = BLOCK_WIDTH;
	for (int i = 0; i < map.row; i++)
	{
		for (int j = 0; j < map.col; j++)
		{
			rect.x = map.blocks[i][j].x * BLOCK_WIDTH;
			rect.y = map.blocks[i][j].y * BLOCK_WIDTH;

			switch (map.blocks[i][j].status)
			{
			case typeBlock::CAPTURED:
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
				break;
			case typeBlock::NONCAPTURED:
				SDL_SetRenderDrawColor(renderer, 0, 162, 232, 0);
				break;
			case typeBlock::PROCESSED:
				SDL_SetRenderDrawColor(renderer, 4, 210, 227, 0);
				break;
			}
			SDL_RenderFillRect(renderer, &rect);
		}
	}
}

void renderRBox(SDL_Renderer* renderer, RecordsBox& rBox)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(renderer, &rBox.border);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0); /*цвет подложки рекордов*/
	SDL_RenderFillRect(renderer, &rBox.box);

	SDL_Texture* tScore = SDL_CreateTextureFromSurface(renderer, rBox.textScore);
	SDL_Rect sRect = { 50, GAME_HEIGHT - RECORDS_OFFSET + 25, 100, 50 };
	SDL_RenderCopy(renderer, tScore, nullptr, &sRect);
	SDL_DestroyTexture(tScore);

	SDL_Texture* tPerc = SDL_CreateTextureFromSurface(renderer, rBox.textPerc);
	SDL_Rect pRect = { GAME_WIDTH / 2, GAME_HEIGHT - RECORDS_OFFSET + 25, 200, 50 };
	SDL_RenderCopy(renderer, tPerc, nullptr, &pRect);
	SDL_DestroyTexture(tPerc);
}

void renderGame(SDL_Renderer* renderer, Map& map, Player& player, Enemy& enemies, RecordsBox& rBox)
{
	renderMap(renderer, map);
    player.render(renderer);
    enemies.render(renderer);
	renderRBox(renderer, rBox);
}

void renderWinLose(SDL_Renderer* renderer, SDL_Texture* texture)
{
	SDL_Rect rect = { 0,0,GAME_WIDTH, GAME_HEIGHT };
	SDL_RenderCopy(renderer, texture, nullptr, &rect);
	SDL_RenderPresent(renderer);
	SDL_Delay(2900);
}

int updateMap(Map& map, Enemy& enemies)
{
	int counter = 0;

	for (size_t i = 0; i < size_t(enemies.counter); i++)
	{
		draw(map, enemies.mas[i].x, enemies.mas[i].y);
	}
	
	for (int i = 0; i < map.row; i++)
	{
		for (int j = 0; j < map.col; j++)
		{
			if (map.blocks[i][j].status == typeBlock::NONDRAW)
			{
				map.blocks[i][j].status = typeBlock::NONCAPTURED;
			}
			else
			{
				if (map.blocks[i][j].status == typeBlock::NONCAPTURED 
					|| map.blocks[i][j].status == typeBlock::PROCESSED)
					counter++;
				map.blocks[i][j].status = typeBlock::CAPTURED;
			}
		}
	}
	return counter;
}

void updateText(int countNew, RecordsBox& rBox)
{
	rBox.score += SCORE_PER_BLOCK * countNew;
	int blocks = rBox.score / SCORE_PER_BLOCK;
	rBox.percent = (blocks * 100) / ((GAME_WIDTH / BLOCK_WIDTH - 2) * ((GAME_HEIGHT - RECORDS_OFFSET) / BLOCK_WIDTH - 2));

	char* score = (char*)calloc(60, sizeof(char));
	strcat(score, "Очки: ");
	char* text = (char*)calloc(20, sizeof(char));
	SDL_itoa(rBox.score, text, 10);
	strcat(score, text);

	SDL_FreeSurface(rBox.textScore);
	SDL_Color color = { 0, 0, 0 };
	rBox.textScore = TTF_RenderUTF8_Blended(rBox.font, score, color);

	char* perc = (char*)calloc(60, sizeof(char));
	SDL_itoa(rBox.percent, text, 10);
	strcat(perc, "Захват: ");
	strcat(perc, text);
	strcat(perc, "/100");
	SDL_FreeSurface(rBox.textPerc);
	rBox.textPerc = TTF_RenderUTF8_Blended(rBox.font, perc, color);

	free(score);
	free(text);
	free(perc);
}

void draw(Map& map, int x, int y)
{
	if (map.blocks[x][y].status == typeBlock::NONCAPTURED) map.blocks[x][y].status = typeBlock::NONDRAW;
	if (map.blocks[x][y - 1].status == typeBlock::NONCAPTURED) draw(map, x, y - 1);
	if (map.blocks[x][y + 1].status == typeBlock::NONCAPTURED) draw(map, x, y + 1);
	if (map.blocks[x - 1][y].status == typeBlock::NONCAPTURED) draw(map, x - 1, y);
	if (map.blocks[x + 1][y].status == typeBlock::NONCAPTURED) draw(map, x + 1, y);
}

bool checkWin(RecordsBox& rBox)
{
	if (rBox.percent >= 80)
		return true;
	return false;
}

void destructorGame(Difficulty& level, RecordsBox& rBox, SDL_Texture* win, SDL_Texture* lose)
{
	TTF_CloseFont(rBox.font);
	TTF_Quit();
	SDL_FreeSurface(rBox.textPerc);
	SDL_FreeSurface(rBox.textScore);
	SDL_DestroyTexture(win);
	SDL_DestroyTexture(lose);
	int easy;
	int medium;
	int hard;

	FILE* file;
	file = fopen("resources/records.txt", "r");
	if (file == nullptr)
	{
		return;
	}
	fscanf(file, "%d", &easy);
	fscanf(file, "%d", &medium);
	fscanf(file, "%d", &hard);

	switch (level)
	{
	case Difficulty::Easy:
		easy = (rBox.score > easy ? rBox.score : easy);
		break;
	case Difficulty::Medium:
		medium = (rBox.score > medium ? rBox.score : medium);
		break;
	case Difficulty::Hard:
		hard = (rBox.score > hard ? rBox.score : hard);
		break;
	}

	fclose(file);
	file = fopen("resources/records.txt", "w");

	fprintf(file, "%d\n%d\n%d", easy, medium, hard);

	fclose(file);
}
