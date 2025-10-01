#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

/*
 * Constants 
 */
#define ALIVE '#'
#define DEAD  ' '

#define CLEAR_SCREEN "\x1b[2J\x1b[H"
#define RENDER_ALIVE_CELL "\x1b[38;2;%d;%d;%dm#\x1b[0m"
#define RENDER_UTF8_ALIVE_CELL "\x1b[38;2;%d;%d;%dm\xE2\x96\x89\x1b[0m"
//                          COLOR(RGB) + CELL + COLOR_RESET

typedef enum Input {
    ARROW_UP = 1,
    ARROW_DOWN = 2,
    ARROW_LEFT = 3,
    ARROW_RIGHT = 4,
    ENTER = 5,
    SPACE = 6,
    KEY_W = 7,
    KEY_S = 8,
    KEY_D = 9,
    KEY_C = 10,
    KEY_N = 11,
    ESC = 12,
    BACKSPACE = 13,
} _INPUT;

typedef struct gameOfLife {
    int width;
    int height;

    int generation;
    int population;

    int cursorX;
    int cursorY;

    char* world;
    int worldSpeed;

    bool pause;
    bool utf8Support;
} GAMEOFLIFE;

/*
 * Util Functions 
 */
int getNeighborsCount(GAMEOFLIFE* gameOfLife, int index){
    int count = 0;
    int x = index % gameOfLife->width;
    int y = index / gameOfLife->width;

    for(int i = -1; i <= 1; i++){
    for(int j = -1; j <= 1; j++){
        if(i == 0 && j == 0) continue;

        if(
            (x+j >= 0 && x+j < gameOfLife->width) &&
            (y+i >= 0 && y+i < gameOfLife->height) && 
            gameOfLife->world[ ((y+i) * gameOfLife->width) + (x+j) ] == ALIVE) 
            count++;
    }
    }

    return count;
}

int getInput(){
    int keyPress = 0;
    #if defined(_WIN32) || defined(_WIN64)
        if (_kbhit()){
            int ch = _getch();
            if (ch == 0 || ch == 224) {
                ch = _getch();
                switch (ch) {
                    case 72:keyPress =  ARROW_UP; break;
                    case 80:keyPress =  ARROW_DOWN;break;
                    case 75:keyPress =  ARROW_LEFT;break;
                    case 77:keyPress =  ARROW_RIGHT;break;
                    case 83: keyPress = BACKSPACE;break;
                }
            }else{
                switch (ch){
                    case 27:keyPress = ESC;break;
                    case 13:keyPress =  ENTER;break;
                    case ' ':keyPress =  SPACE;break;
                    case 8:keyPress = BACKSPACE;break;
                    case 'w': case 'W':keyPress = KEY_W;break;
                    case 's': case 'S':keyPress = KEY_S;break;
                    case 'd': case 'D':keyPress = KEY_D;break;
                    case 'c': case 'C':keyPress = KEY_C;break;
                    case 'n': case 'N':keyPress = KEY_N;break;
                }
            }
        }
    #else
        struct termios oldt, newt;
        int ch;
        int oldf;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if (ch != EOF) {
            ungetc(ch, stdin);
            int ch = getchar();
            if (ch == 27) {
                if (getchar() == '[') {
                    switch (getchar()) {
                        case 'A': keyPress =  ARROW_UP;break;
                        case 'B': keyPress =  ARROW_DOWN;break;
                        case 'C': keyPress =  ARROW_RIGHT;break;
                        case 'D': keyPress =  ARROW_LEFT;break;
                    }
                }else{
                    keyPress = ESC;
                }
            }else{
                switch (ch){
                    case '\n':keyPress = ENTER;break;
                    case ' ':keyPress = SPACE;break;
                    case 'w': case 'W':keyPress = KEY_W;break;
                    case 's': case 'S':keyPress = KEY_S;break;
                    case 'd': case 'D':keyPress = KEY_D;break;
                    case 'c': case 'C':keyPress = KEY_C;break;
                    case 'n': case 'N':keyPress = KEY_N;break;
                    case 127: case 8:keyPress = BACKSPACE; break;
                }
            }
        }
    #endif
    return keyPress;
}

void render(GAMEOFLIFE* gameOfLife){
    int index = 0;
    char* output = (char*) calloc((gameOfLife->height * gameOfLife->width * 15) + (gameOfLife->height * 15) + 1, sizeof(char));
    
    for (int i = 0; i < gameOfLife->width*gameOfLife->height + gameOfLife->height; ++i){
		if (i % gameOfLife->width == 0){
			index += sprintf(output + index, "\x1b[%i;%iH", (i / gameOfLife->width) + 1, (i % gameOfLife->width) + 1);
		}
        
        if(i < gameOfLife->width*gameOfLife->height){
            if(gameOfLife->world[i] == ALIVE){
                int x = i % gameOfLife->width;
                int y = i / gameOfLife->width;

                if(gameOfLife->utf8Support)
                    index += sprintf(output + index, RENDER_UTF8_ALIVE_CELL, (x*255)/(gameOfLife->width-1), (y*255)/(gameOfLife->height-1), 128);
                else
                    index += sprintf(output + index, RENDER_ALIVE_CELL, (x*255)/(gameOfLife->width-1), (y*255)/(gameOfLife->height-1), 128);
                    // output[index++] = ALIVE;
            }else{
                output[index] = ' ';
                index++;
            }
        }
	}
    output[index] = '\0';


    #if defined(_WIN32) || defined(_WIN64)
    fwrite(CLEAR_SCREEN, 1, strlen(CLEAR_SCREEN),stdout);
    fwrite(output, 1, strlen(output),stdout);
    #else
    write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
    write(STDOUT_FILENO,output,strlen(output));
    #endif


    char* stats = (char*) calloc(gameOfLife->width + 1, sizeof(char));
    int statsCount = sprintf(stats, "Gen: %d | Pop: %d | Slow: %d | Pause: %d",gameOfLife->generation, gameOfLife->population, gameOfLife->worldSpeed, gameOfLife->pause);
    if(statsCount > gameOfLife->width){
        statsCount = sprintf(stats, "%d | %d | %d | %d",gameOfLife->generation, gameOfLife->population, gameOfLife->worldSpeed, gameOfLife->pause);
    }

    #if defined(_WIN32) || defined(_WIN64)
    fwrite(stats, 1, strlen(stats),stdout);
    #else
    write(STDOUT_FILENO,stats,strlen(stats));
    #endif
}

/*
 * Create Functions 
 */
bool createOrDeleteLife(GAMEOFLIFE* gameOfLife, bool createLife){
    char oldStatus = gameOfLife->world[gameOfLife->cursorY * gameOfLife->width + gameOfLife->cursorX];
    gameOfLife->world[gameOfLife->cursorY * gameOfLife->width + gameOfLife->cursorX] = createLife? ALIVE: DEAD;
    return (oldStatus == ALIVE && !createLife) || (oldStatus == DEAD && createLife);
}

GAMEOFLIFE* createWorld(){
    GAMEOFLIFE* gameOfLife = (GAMEOFLIFE*) calloc(1,sizeof(GAMEOFLIFE));

    #if defined(_WIN32) || defined(_WIN64)
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (!GetConsoleScreenBufferInfo(h, &csbi)) return NULL;
        gameOfLife->width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        gameOfLife->height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        UINT cp = GetConsoleOutputCP();
        gameOfLife->utf8Support = (cp == 65001);
    #else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return NULL;
        gameOfLife->width = w.ws_col;
        gameOfLife->height = w.ws_row;
        gameOfLife->utf8Support = true;
    #endif

    gameOfLife->height--; //For Showing Stats
    gameOfLife->worldSpeed = 100;
    gameOfLife->world = (char*) calloc(gameOfLife->height * gameOfLife->width, sizeof(char));
    memset(gameOfLife->world, DEAD, gameOfLife->height * gameOfLife->width * sizeof(char));
    
    return gameOfLife;
}

/*
 * Handler Functions 
 */
void handleWorld(GAMEOFLIFE* gameOfLife){
    int population = 0;
    bool isLifeUpdated = false;

    char* world = (char*) calloc(gameOfLife->height * gameOfLife->width, sizeof(char));
    memset(world, DEAD, gameOfLife->height * gameOfLife->width * sizeof(char));

    for(int i = 0; i < gameOfLife->width*gameOfLife->height; ++i){
        char cell = gameOfLife->world[i];
        bool isAlive = cell == ALIVE;
        int count = getNeighborsCount(gameOfLife, i);
        
        if(isAlive && (count <= 1 || count >= 4)){
            world[i] = DEAD;
            isLifeUpdated = true;
        }else if(!isAlive && count == 3){
            world[i] = ALIVE;
            isLifeUpdated = true;
        }else{
            world[i] = gameOfLife->world[i];
        }
        
        if(world[i] == ALIVE) population++;
    }

    strncpy(gameOfLife->world,world, sizeof(char) * gameOfLife->width*gameOfLife->height);
    gameOfLife->population = population;

    if(isLifeUpdated)
        gameOfLife->generation ++;
}

int handleInput(GAMEOFLIFE* gameOfLife){
    int keyPress = getInput();

    switch(keyPress){
        case ARROW_UP:   gameOfLife->cursorY = gameOfLife->cursorY != 0? gameOfLife->cursorY - 1 : 0;break;
        case ARROW_DOWN: gameOfLife->cursorY = gameOfLife->cursorY != gameOfLife->height - 1? gameOfLife->cursorY + 1 : gameOfLife->cursorY;break;
        case ARROW_LEFT: gameOfLife->cursorX = gameOfLife->cursorX != 0? gameOfLife->cursorX - 1 : 0;break;
        case ARROW_RIGHT:gameOfLife->cursorX = gameOfLife->cursorX != gameOfLife->width - 1? gameOfLife->cursorX + 1 : gameOfLife->cursorX;break;
        
        case ENTER:
            if(createOrDeleteLife(gameOfLife, true)) render(gameOfLife);break;
        case BACKSPACE:
            if(createOrDeleteLife(gameOfLife, false)) render(gameOfLife);break;
        case SPACE:
            gameOfLife->pause = !gameOfLife->pause; render(gameOfLife);break;
        
        case KEY_W:
            gameOfLife->worldSpeed = gameOfLife->worldSpeed - 25 <= 0 ? 25 : gameOfLife->worldSpeed - 25;
            render(gameOfLife);
            break;
        case KEY_S:
            gameOfLife->worldSpeed = gameOfLife->worldSpeed + 25 > 2000 ? 2000 : gameOfLife->worldSpeed + 25;
            render(gameOfLife);
            break;
        case KEY_D:
            gameOfLife->pause = true;
            handleWorld(gameOfLife);
            render(gameOfLife);
            break;
        case KEY_C:
            free(gameOfLife->world);
            gameOfLife->world = (char*) calloc(gameOfLife->height * gameOfLife->width, sizeof(char));
            memset(gameOfLife->world, DEAD, gameOfLife->height * gameOfLife->width * sizeof(char));
            render(gameOfLife);
            break;
        case KEY_N:
            free(gameOfLife->world);
            free(gameOfLife);
            gameOfLife = createWorld();
            render(gameOfLife);
            break;
    }

    printf(
        "\x1b[%i;%iH", 
        gameOfLife->cursorY + 1,
        gameOfLife->cursorX + 1
    );

    return keyPress;
}

/*
 * Main Functions 
 */
int main(){
    uint64_t gameItr = 0;
    bool isCellsDancing = true;
    GAMEOFLIFE* gameOfLife = createWorld();

    while(isCellsDancing){
        if(!gameOfLife->pause && gameItr % gameOfLife->worldSpeed == 0){
            handleWorld(gameOfLife);
            render(gameOfLife);
        }
        isCellsDancing = handleInput(gameOfLife) != ESC;
        gameItr++;
    }

    free(gameOfLife->world);
    free(gameOfLife);

    #if defined(_WIN32) || defined(_WIN64)
    fwrite(CLEAR_SCREEN, 1, strlen(CLEAR_SCREEN),stdout);
    #else
    write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
    #endif

    return EXIT_SUCCESS;
}
