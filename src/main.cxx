#include <ncurses.h>
#include <iostream>
#include <chrono>
#include <time.h>
#include <vector>
#include <memory>
#include <random>
#include <unordered_map>

typedef struct Position
{
    int row, col;
} Position;

class Fruit
{
    public:
        Fruit(int row, int col) : Fruit()
        {
            p.row = row;
            p.col = col;
        }

        Fruit() : w(stdscr), image(ACS_DIAMOND) {}

        void draw(void) const
        {
            mvwaddch(w, p.row, p.col, image);
        }

        Position getPosition(void) const
        {
            return p;
        }

    private:
        Position p;
        WINDOW* w;
        int image;

};

class Block
{
    public:
        Block() : w(stdscr), image('o')
        {

        }

        Block(int row, int col) : Block()
        {
            p.row = row;
            p.col = col;
        }

        ~Block()
        {

        }

        void draw(void) const
        {
            mvwaddch(w, p.row, p.col, image);
        }

        void setPosition(const Position& p)
        {
            this->p.row = p.row;
            this->p.col = p.col;
        }

        Position getPosition() const
        {
            return p;
        }

    private:
        Position p;
        WINDOW* w;
        int image;

};

enum class Direction 
{
    Up,
    Down,
    Left,
    Right
};

class Snake
{
    public:
        Snake(int row, int col, Direction d)
        {
            blocks.push_back(Block(row, col));
            direction = d;
        }

        Direction getDirection(void)
        {
            return direction;
        }

        bool isCollision(void)
        {
            auto headPosition = blocks.front().getPosition();
            auto headRow = headPosition.row;
            auto headCol = headPosition.col;
            
            auto it = blocks.begin();
            it++;
            for (; it != blocks.end(); it++)
            {
                auto blockPosition = it->getPosition();
                if ((headRow == blockPosition.row) && (headCol == blockPosition.col))
                {
                    return true;
                }
            }
            return false;
        }

        void addBlock()
        {
            blocks.push_back(Block());
        }

        void draw()
        {
            for (const auto& b : blocks)
            {
                b.draw();
            }
        }

        void update(const Direction& d)
        {
            // if direction is reversed, need to reverse the snake, 
            // i.e. head is the at the tail and tail is at the head
            if (
                    ((Direction::Right == direction) && (Direction::Left  == d)) ||
                    ((Direction::Left  == direction) && (Direction::Right == d)) ||
                    ((Direction::Up    == direction) && (Direction::Down  == d)) ||
                    ((Direction::Down  == direction) && (Direction::Up    == d))
               )
            {
                std::reverse(blocks.begin(), blocks.end());
            }

            this->direction = d;

            // next block takes on the cooridnates of the previous one
            Position oldPosition = blocks.front().getPosition(),
                     newPosition = oldPosition;
            switch (direction)
            {
                case Direction::Up: 
                    newPosition.row--;
                    break;
                case Direction::Down: 
                    newPosition.row++;
                    break;
                case Direction::Left: 
                    newPosition.col--;
                    break;
                case Direction::Right: 
                    newPosition.col++;
                    break;
                default:
                    throw std::runtime_error("Undefined direction");
            }

            for (auto& b : blocks)
            {
                oldPosition = b.getPosition();
                b.setPosition(newPosition);
                newPosition = oldPosition;
            }
        }

        const std::vector<Block>& getBlocks() const
        {
            return blocks;
        }

    private:
        std::vector<Block> blocks;
        Direction direction;
};

class Game
{
    public:
        Game() : currentSpeed(initialSpeed), clockCnt(0), snakeDirection(Direction::Right), isFruitActive(false), isQuit(false), ch(-1)
        {
            initscr();
            curs_set(false);
            cbreak(); // disable line buffering
            nodelay(stdscr, true);
            noecho();
            keypad(stdscr, true);

            getmaxyx(stdscr, rows, cols);

            s = std::make_unique<Snake>(rows/2, 0, Direction::Right);
        }

        void run(void)
        {
            auto prevTime = clock();
            while((not isQuit) and (not isCollision()))
            {
                auto currentTime = clock();
                auto diff = currentTime - prevTime;
                if (diff > static_cast<decltype(diff)>(CLOCKS_PER_SEC)/static_cast<decltype(diff)>(FPS))
                {

                    prevTime = currentTime;

                    clockCnt++;

                    getInput();

                    if (clockCnt >= currentSpeed)
                    {
                        update();
                        clear();
                        draw();

                        clockCnt = 0;
                    }

                    refresh();

                }
            }

            endwin();
        }

    private:
        bool isCollision(void)
        {
            auto headPosition = s->getBlocks().front().getPosition();
            return (headPosition.row > rows) or 
                   (headPosition.col > cols) or 
                   (headPosition.row < 0) or 
                   (headPosition.col < 0) or
                   s->isCollision();
        }

        void draw(void)
        {
            s->draw();
            if (isFruitActive)
            {
                f->draw();
            }
        }

        void update(void)
        {
            if (not isFruitActive)
            {
                generateFruit();
            }

            if (isSnakeCollisionWithFruit())
            {
                s->addBlock();
                f.reset();
                isFruitActive = false;

                if (currentSpeed > 0)
                {
                    currentSpeed--;
                }
            }

            s->update(snakeDirection);
        }

        bool isSnakeCollisionWithFruit(void)
        {
            if (isFruitActive)
            {
                auto snakeHead = s->getBlocks().front();
                auto snakePosition = snakeHead.getPosition();
                auto fruitPosition = f->getPosition();
                std::cout << fruitPosition.row << ":" << fruitPosition.col << "\n";

                if (snakePosition.row == fruitPosition.row && 
                    snakePosition.col == fruitPosition.col)
                {
                    return true;
                }
            }
            return false;
        }

        void generateFruit(void)
        {
            // scan for empty cells
            // generate an empty fruit in one of the empty cells
            std::unordered_map<int, bool> occupiedRows,
                                          occupiedCols;

            for (auto b : s->getBlocks())
            {
                Position p = b.getPosition();
                occupiedRows[p.row] = true;
                occupiedCols[p.col] = true;
            }

            auto findFreeCells = [](int cellCount, const std::unordered_map<int, bool>& occupiedCells)
            {
                std::vector<int> freeCells;
                for (int i = 0; i < cellCount; i++)
                {
                    if (occupiedCells.find(i) == occupiedCells.end())
                    {
                        freeCells.push_back(i);
                    }
                }
                return freeCells;
            };

            auto freeRows = findFreeCells(rows, occupiedRows);
            auto freeCols = findFreeCells(cols, occupiedCols);

            std::random_device rowDev, 
                               colDev; 
            
            std::mt19937 rowGen(rowDev()),
                         colGen(colDev());

            std::uniform_int_distribution<int> rowDist(0,  freeRows.size()- 1);
            std::uniform_int_distribution<int> colDist(0,  freeCols.size()- 1);
            
            int fruitRow = rowDist(rowGen);
            int fruitCol = colDist(colGen);
            
            f = std::make_unique<Fruit>(fruitRow, fruitCol);
            isFruitActive = true;
        }

        void getInput(void)
        {
            ch = getch();
            isQuit = 'q' == ch;

            switch(ch)
            {
                case KEY_UP: snakeDirection = Direction::Up; break;
                case KEY_DOWN: snakeDirection = Direction::Down; break;
                case KEY_LEFT: snakeDirection = Direction::Left; break;
                case KEY_RIGHT:snakeDirection = Direction::Right; break;
                default: break;
            }
        }

        int rows, cols;
        std::unique_ptr<Snake> s;
        const static unsigned FPS = 60;
        const static unsigned initialSpeed = FPS/3;
        unsigned currentSpeed;
        std::unique_ptr<Fruit> f;
        unsigned clockCnt;
        Direction snakeDirection;
        bool isFruitActive;
        bool isQuit;
        int ch;
        
};


int main()
{
    Game g;
    g.run();

    return 0;
}
