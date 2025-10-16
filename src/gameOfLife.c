#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

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
#define DEAD 0
#define KNUTH_HASH 2654435761u
#define CLEAR_SCREEN "\x1b[2J\x1b[H"
#define RENDER_ALIVE_CELL "\x1b[38;2;%d;%d;%dm#\x1b[0m"
#define RENDER_UTF8_ALIVE_CELL "\x1b[38;2;%d;%d;%dm\xE2\x96\x89\x1b[0m"
//                          COLOR(RGB) + CELL + COLOR_RESET

typedef enum Color {
    GRADIENT = 0,
    LIFE_SPAN,
    RANDOMIZE,
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    NONE
} _COLOR;

typedef enum Input {
    ARROW_UP = 1,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    ENTER,
    SPACE,
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_C,
    KEY_N,
    KEY_R,
    ESC,
    BACKSPACE,
} _INPUT;

typedef struct gameOfLife {
    int width;
    int height;

    uint16_t generation;
    uint16_t population;

    int cursorX;
    int cursorY;

    uint16_t* world;
    uint16_t worldSpeed;

    bool pause;
    bool utf8Support;
    enum Color colorMode;
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
            gameOfLife->world[ ((y+i) * gameOfLife->width) + (x+j) ]) 
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
                    case 'a': case 'A':keyPress = KEY_A;break;
                    case 's': case 'S':keyPress = KEY_S;break;
                    case 'd': case 'D':keyPress = KEY_D;break;
                    case 'c': case 'C':keyPress = KEY_C;break;
                    case 'n': case 'N':keyPress = KEY_N;break;
                    case 'r': case 'R':keyPress = KEY_R;break;
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
                    case 'a': case 'A':keyPress = KEY_A;break;
                    case 's': case 'S':keyPress = KEY_S;break;
                    case 'd': case 'D':keyPress = KEY_D;break;
                    case 'c': case 'C':keyPress = KEY_C;break;
                    case 'n': case 'N':keyPress = KEY_N;break;
                    case 'r': case 'R':keyPress = KEY_R;break;
                    case 127: case 8:keyPress = BACKSPACE; break;
                }
            }
        }
    #endif
    return keyPress;
}

char* getColorModeName(GAMEOFLIFE* gameOfLife){
    switch(gameOfLife->colorMode){
        case GRADIENT: return "Gradient";
        case LIFE_SPAN: return "LifeSpan";
        case RANDOMIZE: return "Randomize";
        case WHITE: return "White";
        case RED: return "Red";
        case GREEN: return "Green";
        case BLUE: return "Blue";
        case YELLOW: return "Yellow";
    }
    
    return "";
}

char* getColorModeColor(GAMEOFLIFE* gameOfLife){
    switch (gameOfLife->colorMode){
    case GRADIENT:
        return gameOfLife->utf8Support?
        "\x1b[38;2;0;0;255m\xE2\x96\xA0\x1b[38;2;128;0;128m\xE2\x96\xA0\x1b[38;2;255;0;128m\xE2\x96\xA0":
        "\x1b[38;2;0;0;255m#\x1b[38;2;128;0;128m#\x1b[38;2;255;0;128m#";
        break;
    case LIFE_SPAN:
        return gameOfLife->utf8Support?
        "\x1b[38;2;0;255;0m\xE2\x96\xA0\x1b[38;2;128;128;0m\xE2\x96\xA0\x1b[38;2;255;0;0m\xE2\x96\xA0":
        "\x1b[38;2;0;255;0m#\x1b[38;2;128;128;0m#\x1b[38;2;255;0;0m#";
        break;
    case RANDOMIZE:
        return gameOfLife->utf8Support?
        "\x1b[38;2;0;0;0m\xE2\x8D\xB0\xE2\x8D\xB0\xE2\x8D\xB0":
        "\x1b[38;2;0;0;0m???";
        break;
    case RED:
        return gameOfLife->utf8Support?
        "\x1b[38;2;255;0;0m\xE2\x96\xA0":
        "\x1b[38;2;255;0;0m#";
        break;
    case GREEN:
        return gameOfLife->utf8Support?
        "\x1b[38;2;0;255;0m\xE2\x96\xA0":
        "\x1b[38;2;0;255;0m#";
        break;
    case BLUE:
        return gameOfLife->utf8Support?
        "\x1b[38;2;0;0;255m\xE2\x96\xA0":
        "\x1b[38;2;0;0;255m#";
        break;
    case YELLOW:
        return gameOfLife->utf8Support?
        "\x1b[38;2;255;255;0m\xE2\x96\xA0":
        "\x1b[38;2;255;255;0m#";
        break;
    case WHITE:
    default:
        return gameOfLife->utf8Support?
        "\x1b[38;2;255;255;255m\xE2\x96\xA0":
        "\x1b[38;2;255;255;255m#";
        break;
    }
    return "";
}

void render(GAMEOFLIFE* gameOfLife){
    int index = 0;
    char* output = (char*) calloc((gameOfLife->height * gameOfLife->width * 27) + (gameOfLife->height * 1) + 1, sizeof(char));
    
    for (int i = 0; i < gameOfLife->width*gameOfLife->height + gameOfLife->height; ++i){
		if (i % gameOfLife->width == 0){
			// index += sprintf(output + index, "\x1b[%i;%iH", (i / gameOfLife->width) + 1, (i % gameOfLife->width) + 1);
			index += sprintf(output + index, "\n");
		}
        
        if(i < gameOfLife->width*gameOfLife->height){
            if(gameOfLife->world[i]){
                int x = i % gameOfLife->width;
                int y = i / gameOfLife->width;

                char* render = (gameOfLife->utf8Support)?RENDER_UTF8_ALIVE_CELL:RENDER_ALIVE_CELL;

                uint8_t r,g,b = 0;
                switch (gameOfLife->colorMode){
                    case GRADIENT:
                        r = (x*255)/(gameOfLife->width-1);
                        g = (y*255)/(gameOfLife->height-1);
                        b = 128;
                        break;
                    case LIFE_SPAN:
                        if(gameOfLife->world[i] + 100 <= gameOfLife->generation){ r = 255;g = b = 0; }
                        else if(gameOfLife->world[i] + 50 <= gameOfLife->generation){ r = g = 128;b = 0; }
                        else{ r = b = 0;g = 255; }
                        break;
                    case RANDOMIZE:
                        r = ((((gameOfLife->world[i] + i) * KNUTH_HASH) + 24) >>  8) & 0xFF;
                        g = ((((gameOfLife->world[i] + i) * KNUTH_HASH) + 16) >> 16) & 0xFF;
                        b = ((((gameOfLife->world[i] + i) * KNUTH_HASH) +  8) >> 24) & 0xFF;
                        break;
                    case RED:
                        r = 255; g = b = 0;
                        break;
                    case GREEN:
                        r = b = 0;g = 255;
                        break;
                    case BLUE:
                        r = g = 0; b = 255;
                        break;
                    case YELLOW:
                        r = g = 255; b = 0;
                        break;
                    case WHITE:
                    default:
                        r = g = b = 255;
                        break;
                }
                index += sprintf(output + index, render, r, g, b);
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

    char* stats = (char*) calloc(gameOfLife->width * 4, sizeof(char));
    int statsCount = sprintf(stats,
        gameOfLife->utf8Support? 
        "\x1b[48;5;2m GameOfLife.c \x1b[0m \x1b[47m\x1b[30m \xF0\x9F\x8C\x80 %d \x1b[0m \x1b[47m\x1b[30m \xF0\x9F\x8C\xB1 %d \x1b[0m \x1b[47m\x1b[30m \xE2\x8F\xA9 %d%% \x1b[0m \x1b[47m\x1b[30m \xF0\x9F\x8E\xA8 %s %s \x1b[0m %s":
        "\x1b[48;5;2m GameOfLife.c \x1b[0m \x1b[47m\x1b[30m Gen: %d \x1b[0m \x1b[47m\x1b[30m Pop: %d \x1b[0m \x1b[47m\x1b[30m Slow: %d%% \x1b[0m \x1b[47m\x1b[30m Color: %s %s \x1b[0m %s",
        gameOfLife->generation, 
        gameOfLife->population, 
        gameOfLife->worldSpeed / 30, 
        getColorModeName(gameOfLife),
        getColorModeColor(gameOfLife),
        gameOfLife->pause? "| \x1b[48;5;239m Paused \x1b[0m":""
    );
    
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
    uint16_t oldGen = gameOfLife->world[gameOfLife->cursorY * gameOfLife->width + gameOfLife->cursorX];
    gameOfLife->world[gameOfLife->cursorY * gameOfLife->width + gameOfLife->cursorX] = createLife? gameOfLife->generation + 1: DEAD;
    return (oldGen && !createLife) || (oldGen == DEAD && createLife);
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
    gameOfLife->worldSpeed = 300;
    gameOfLife->world = (uint16_t*) calloc(gameOfLife->height * gameOfLife->width, sizeof(uint16_t));
    memset(gameOfLife->world, DEAD, gameOfLife->height * gameOfLife->width * sizeof(uint16_t));
    
    return gameOfLife;
}

/*
 * Handler Functions 
 */
void handleWorld(GAMEOFLIFE* gameOfLife){
    int population = 0;
    bool isLifeUpdated = false;

    uint16_t* world = (uint16_t*) calloc(gameOfLife->height * gameOfLife->width, sizeof(uint16_t));
    memset(world, DEAD, gameOfLife->height * gameOfLife->width * sizeof(uint16_t));

    for(int i = 0; i < gameOfLife->width*gameOfLife->height; ++i){
        uint16_t cell = gameOfLife->world[i];
        int count = getNeighborsCount(gameOfLife, i);
        
        if(cell && (count <= 1 || count >= 4)){
            world[i] = DEAD;
            isLifeUpdated = true;
        }else if(!cell && count == 3){
            world[i] = gameOfLife->generation + 1;
            isLifeUpdated = true;
        }else{
            world[i] = gameOfLife->world[i];
        }
        
        if(world[i]) population++;
    }

    memcpy(gameOfLife->world,world, gameOfLife->width*gameOfLife->height * sizeof(uint16_t));
    gameOfLife->population = population;

    if(isLifeUpdated)
        gameOfLife->generation ++;
}

int handleInput(GAMEOFLIFE** gameOfLife){
    int keyPress = getInput();

    switch(keyPress){
        case ARROW_UP:   (*gameOfLife)->cursorY = (*gameOfLife)->cursorY != 0? (*gameOfLife)->cursorY - 1 : 0;break;
        case ARROW_DOWN: (*gameOfLife)->cursorY = (*gameOfLife)->cursorY != (*gameOfLife)->height - 1? (*gameOfLife)->cursorY + 1 : (*gameOfLife)->cursorY;break;
        case ARROW_LEFT: (*gameOfLife)->cursorX = (*gameOfLife)->cursorX != 0? (*gameOfLife)->cursorX - 1 : 0;break;
        case ARROW_RIGHT:(*gameOfLife)->cursorX = (*gameOfLife)->cursorX != (*gameOfLife)->width - 1? (*gameOfLife)->cursorX + 1 : (*gameOfLife)->cursorX;break;
        
        case ENTER:
        case BACKSPACE:
            if(createOrDeleteLife((*gameOfLife), keyPress == ENTER)) render((*gameOfLife));break;
        case SPACE:
            (*gameOfLife)->pause = !(*gameOfLife)->pause; render((*gameOfLife));break;
        
        case KEY_W:
            (*gameOfLife)->worldSpeed = (*gameOfLife)->worldSpeed - 30 <= 0 ? 30 : (*gameOfLife)->worldSpeed - 30;
            render((*gameOfLife));
            break;
        case KEY_S:
            (*gameOfLife)->worldSpeed = (*gameOfLife)->worldSpeed + 30 > 3000 ? 3000 : (*gameOfLife)->worldSpeed + 30;
            render((*gameOfLife));
            break;
        
        case KEY_A:
            (*gameOfLife)->colorMode = ((*gameOfLife)->colorMode + 1) % NONE;
            render((*gameOfLife));
            break;
        case KEY_D:
            (*gameOfLife)->pause = true;
            handleWorld((*gameOfLife));
            render((*gameOfLife));
            break;
        case KEY_R:
            for(int i = 0; i < (*gameOfLife)->width * (*gameOfLife)->height; i++){
                if((*gameOfLife)->world[i] == DEAD)
                    (*gameOfLife)->world[i] = (rand() % 100) < 10 ? (*gameOfLife)->generation + 1 :DEAD;
            }
            render((*gameOfLife));
            break;

        case KEY_C:
            free((*gameOfLife)->world);
            (*gameOfLife)->world = (uint16_t*) calloc((*gameOfLife)->height * (*gameOfLife)->width, sizeof(uint16_t));
            memset((*gameOfLife)->world, DEAD, (*gameOfLife)->height * (*gameOfLife)->width * sizeof(uint16_t));
            render((*gameOfLife));
            break;
        case KEY_N:
            free((*gameOfLife)->world);(*gameOfLife)->world = NULL;
            free((*gameOfLife));(*gameOfLife) = NULL;
            (*gameOfLife) = createWorld();
            render((*gameOfLife));
            break;
    }

    printf(
        "\x1b[%i;%iH", 
        (*gameOfLife)->cursorY + 1,
        (*gameOfLife)->cursorX + 1
    );

    return keyPress;
}

/*
 * Main Functions 
 */
int main(){
    srand(time(NULL));
    uint64_t gameItr = 0;
    bool isCellsDancing = true;
    GAMEOFLIFE* gameOfLife = createWorld();

    while(isCellsDancing){
        if(!gameOfLife->pause && gameItr % gameOfLife->worldSpeed == 0){
            handleWorld(gameOfLife);
            render(gameOfLife);
        }
        isCellsDancing = handleInput(&gameOfLife) != ESC;
        gameItr++;
    }

    free(gameOfLife->world);gameOfLife->world = NULL;
    free(gameOfLife);

    #if defined(_WIN32) || defined(_WIN64)
    fwrite(CLEAR_SCREEN, 1, strlen(CLEAR_SCREEN),stdout);
    #else
    write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
    #endif

    return EXIT_SUCCESS;
}
