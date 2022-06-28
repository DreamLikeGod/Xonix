#ifndef XONIX_H

#include "constants.h"

#define BLOCK_WIDTH 20    /*ширина блока*/
#define MAX_ENEMY 5       /*максимально количество противников*/
#define SCORE_PER_BLOCK 5 /*очков за один захваченный блок*/

/*тип блока*/
enum class typeBlock 
{
	CAPTURED = 0, /*захвачено*/
	PROCESSED,    /*в процессе захвата*/
	NONCAPTURED,  /*не захвачено*/
	NONDRAW       /*не закрашивать*/
};


/*направления движения для игрока*/
enum class dirPlayerType
{
	left,
	right,
	top,
	down,
	none
};

/*направления движения*/
enum class dirEnemyType
{
	left = 0, /*лево*/
	right,    /*право*/
	Top,      /*вверх*/
	Down      /*вниз*/
};

/*структура полоски текущего рекорда*/
struct RecordsBox
{
	SDL_Rect border;
	SDL_Rect box;
	int score; /*очки*/
	int percent;     /*процент захвата карты*/
	SDL_Surface* textPerc;
	SDL_Surface* textScore;
	TTF_Font* font;
};

class gameObject {
public:
    int x;
    int y;
    void init();
    void render();
    bool update();
};

class Block : public gameObject {
public:
    typeBlock status; /*тип клетки*/
};

/*структура карты*/
struct Map
{
    int col; /*количество столбцов*/
    int row; /*количество строк*/
    Block blocks[GAME_WIDTH / BLOCK_WIDTH][(GAME_HEIGHT - RECORDS_OFFSET) / BLOCK_WIDTH]; /*массив блоков*/
};

/*перечисление разных уровней*/
enum class Difficulty
{
    Easy = 1,   /*легко*/
    Medium = 3, /*средне*/
    Hard = 5    /*сложно*/
};

class Enemy : public gameObject {
public:
    dirEnemyType dir_hor; /*движение по горионтали*/
    dirEnemyType dir_ver; /*движение по вертикали*/
    Enemy *mas;/*набор противников*/
    int counter;
    void init(Difficulty diff){
        switch (diff) {
            case Difficulty::Easy:
                counter = 1;
                break;
            case Difficulty::Medium:
                counter = 3;
                break;
            case Difficulty::Hard:
                counter = 5;
                break;
            default:
                break;
        }
        srand(unsigned(time(NULL)));
        mas = new Enemy[counter];
        for (size_t i = 0; i < size_t(diff); i++)
        {
            mas[i].x = 4 * int(i) + 2;
            mas[i].y = 4 * int(i) + 2;

            mas[i].dir_hor = dirEnemyType(rand() % 2);
            mas[i].dir_ver = dirEnemyType(rand() % 2 + 2);
        }
    }
    void render(SDL_Renderer* renderer) {
        for (size_t i = 0; i < size_t(counter); i++)
        {
            int circle_radius = 10;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            for (int w = 0; w < circle_radius * 2; w++)
            {
                for (int h = 0; h < circle_radius * 2; h++)
                {
                    int dx = circle_radius - w; /*горизонтальное смещение*/
                    int dy = circle_radius - h; /*вертикальное смещение*/
                    if ((dx * dx + dy * dy) <= (circle_radius * circle_radius))
                    {
                        SDL_RenderDrawPoint(renderer,
                            mas[i].x * BLOCK_WIDTH + BLOCK_WIDTH / 2 + dx,
                            mas[i].y * BLOCK_WIDTH + BLOCK_WIDTH / 2 + dy);
                    }
                }
            }
        }
    }
};

class Player : public gameObject {
    dirPlayerType dir; /*направление движения*/
public:
    void init(){
        this->x = 0;
        this->y = 1;
        this->dir = dirPlayerType::none;
    }
    void render(SDL_Renderer* renderer) {
        SDL_Rect border, p;
        border.x = this->x * BLOCK_WIDTH;
        border.y = this->y * BLOCK_WIDTH;
        border.w = border.h = BLOCK_WIDTH;
        p.x = border.x + 1;
        p.y = border.y + 1;
        p.w = p.h = BLOCK_WIDTH - 2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderFillRect(renderer, &border);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 0);
        SDL_RenderFillRect(renderer, &p);
    }
    bool update(Enemy* enemies, Map& map, RecordsBox& rBox);
    void updatePlayerInput(SDL_Event* event) {
        if (event->type == SDL_KEYDOWN)
        {
            if (event->key.keysym.sym == SDLK_UP)
            {
                this->dir = dirPlayerType::top;
            }
            if (event->key.keysym.sym == SDLK_DOWN)
            {
                this->dir = dirPlayerType::down;
            }
            if (event->key.keysym.sym == SDLK_LEFT)
            {
                this->dir = dirPlayerType::left;
            }
            if (event->key.keysym.sym == SDLK_RIGHT)
            {
                this->dir = dirPlayerType::right;
            }
        }
    }
};

/*инициализация карты*/
void initMap(Map& map);

/*иниализация рекордов*/
void initRecordsBox(RecordsBox& rBox);

/*инициализация экрана выигрыша, проигрыша*/
void initWinLose(SDL_Texture*& win, SDL_Texture*& lose, SDL_Renderer* renderer);

/*отрисовка карты*/
void renderMap(SDL_Renderer* renderer, Map& map);

/*отрисовка рекорда*/
void renderRBox(SDL_Renderer* renderer, RecordsBox& rBox);

/*отрисовка всего*/
void renderGame(SDL_Renderer* renderer, Map& map, Player& player, Enemy* enemies, RecordsBox& rBox);

/*отрисовка экрана выигрыша и проигрыша*/
void renderWinLose(SDL_Renderer* renderer, SDL_Texture* texture);

/*
обновление карты, закрашивается захваченная территория, 
возвращает количество захваченных блоков
*/
int updateMap(Map& map, Enemy* enemies);

/*обновление текста*/
void updateText(int countNew, RecordsBox& rBox);

bool Player::update(Enemy* enemies, Map& map, RecordsBox& rBox) {
    SDL_Rect nextPos = { 0,0,0,0 };
    switch (this->dir)
    {
        case dirPlayerType::none:
            return false;
        case dirPlayerType::top:
            nextPos.x = this->x;
            nextPos.y = this->y - 1;
            break;
        case dirPlayerType::down:
            nextPos.x = this->x;
            nextPos.y = this->y + 1;
            break;
        case dirPlayerType::left:
            nextPos.y = this->y;
            nextPos.x = this->x - 1;
            break;
        case dirPlayerType::right:
            nextPos.y = this->y;
            nextPos.x = this->x + 1;
            break;
    }
    if ((nextPos.x >= 0 && nextPos.x < GAME_WIDTH / BLOCK_WIDTH)
        && (nextPos.y >= 0 && nextPos.y < (GAME_HEIGHT - RECORDS_OFFSET) / BLOCK_WIDTH))
    {
        this->x = nextPos.x;
        this->y = nextPos.y;
        
        if (map.blocks[this->x][this->y].status == typeBlock::CAPTURED
            && this->dir != dirPlayerType::none)
        {
            int count = updateMap(map, enemies);
            updateText(count, rBox);
            this->dir = dirPlayerType::none;
            return false;
        }
        else if (map.blocks[this->x][this->y].status == typeBlock::NONCAPTURED)
        {
            map.blocks[this->x][this->y].status = typeBlock::PROCESSED;
            return false;
        }
        else if (map.blocks[this->x][this->y].status == typeBlock::PROCESSED)
        {
            return true;
        }
    }
    return false;
}
/*обновление шаров. True - если произошло столкновение*/
bool updateEnemy(Enemy& enemies,Map& map, Player& player) {
    bool isCollision = false;
    for (size_t i = 0; i < size_t(enemies.counter); i++)
    {
        SDL_Rect nextPos = { 0,0,0,0 };
        switch (enemies.mas[i].dir_hor) /*движение по горизонтали*/
        {
        case dirEnemyType::left:
            nextPos.x = enemies.mas[i].x - 1;
            break;
        case dirEnemyType::right:
            nextPos.x = enemies.mas[i].x + 1;
            break;
        }

        switch (enemies.mas[i].dir_ver) /*движение по вертикали*/
        {
            case dirEnemyType::Top:
            nextPos.y = enemies.mas[i].y - 1;
            break;
        case dirEnemyType::Down:
            nextPos.y = enemies.mas[i].y + 1;
            break;
        }

        if (enemies.mas[i].x == player.x && enemies.mas[i].y == player.y)
            isCollision = true; /*конец игры*/

        /*без столкновения*/
        if (map.blocks[nextPos.x][nextPos.y].status == typeBlock::NONCAPTURED)
        {
            enemies.mas[i].x = nextPos.x;
            enemies.mas[i].y = nextPos.y;
        }
        /*столкновение с уже захваченной территорией*/
        else if (map.blocks[nextPos.x][nextPos.y].status == typeBlock::CAPTURED)
        {
            bool flag = false;
            switch (enemies.mas[i].dir_hor) /*горизонтальная составляющая*/
            {
            case dirEnemyType::left:
                if (map.blocks[enemies.mas[i].x - 1][enemies.mas[i].y].status == typeBlock::CAPTURED)
                {
                    enemies.mas[i].dir_hor = dirEnemyType::right;
                    flag = true;
                }
                break;
            case dirEnemyType::right:
                if (map.blocks[enemies.mas[i].x + 1][enemies.mas[i].y].status == typeBlock::CAPTURED)
                {
                    enemies.mas[i].dir_hor = dirEnemyType::left;
                    flag = true;
                }
                break;
            }

            switch (enemies.mas[i].dir_ver) /*вертикальная составляющая*/
            {
            case dirEnemyType::Top:
                if (map.blocks[enemies.mas[i].x][enemies.mas[i].y - 1].status == typeBlock::CAPTURED)
                {
                    enemies.mas[i].dir_ver = dirEnemyType::Down;
                    flag = true;
                }
                break;
            case dirEnemyType::Down:
                if (map.blocks[enemies.mas[i].x][enemies.mas[i].y + 1].status == typeBlock::CAPTURED)
                {
                    enemies.mas[i].dir_ver = dirEnemyType::Top;
                    flag = true;
                }
                break;
            }

            if (!flag) /*если угол*/
            {
                if (enemies.mas[i].dir_hor == dirEnemyType::left)
                    enemies.mas[i].dir_hor = dirEnemyType::right;
                else
                    enemies.mas[i].dir_hor = dirEnemyType::left;

                if (enemies.mas[i].dir_ver == dirEnemyType::Top)
                    enemies.mas[i].dir_ver = dirEnemyType::Down;
                else
                    enemies.mas[i].dir_ver = dirEnemyType::Top;
            }
        }
        /*столкновение с территорией в процессе захвата*/
        else if (map.blocks[nextPos.x][nextPos.y].status == typeBlock::PROCESSED)
        {
            isCollision = true; /*конец игры*/
        }
    }
    return isCollision;
}

/*закрашивание территории*/
void draw(Map& map, int x, int y);

/*проверка выигрыша, true - если выигрыш*/
bool checkWin(RecordsBox& rBox);

/*очищение памяти*/
void destructorGame(Difficulty& level, RecordsBox& rBox, SDL_Texture* win, SDL_Texture* lose);

#endif // !XONIX_H
