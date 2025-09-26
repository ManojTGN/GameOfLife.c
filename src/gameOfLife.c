#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

#define CLEAR_SCREEN "\x1b[2J\x1b[H"

typedef struct gameOfLife {
    int width;
    int height;

    int generation;
    int population;

    int cursorX;
    int cursorY;

    char* world;
    bool pause;
} GAMEOFLIFE;

GAMEOFLIFE* createWorld(){
    GAMEOFLIFE* gameOfLife = (GAMEOFLIFE*) calloc(1,sizeof(GAMEOFLIFE));

    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (!GetConsoleScreenBufferInfo(h, &csbi)) return NULL;
        gameOfLife->width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        gameOfLife->height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return NULL;
        gameOfLife->width = w.ws_col;
        gameOfLife->height = w.ws_row;
    #endif

    gameOfLife->world = (char*) calloc(gameOfLife->height * gameOfLife->width, sizeof(char));
    memset(gameOfLife->world, ' ', gameOfLife->height * gameOfLife->width * sizeof(char));
    
    return gameOfLife;
}

void createLife(GAMEOFLIFE* gameOfLife){
    gameOfLife->world[
        gameOfLife->cursorY * gameOfLife->width + gameOfLife->cursorX
    ] = '#';
}

void handleWorld(GAMEOFLIFE* gameOfLife){

}

void handleInput(GAMEOFLIFE* gameOfLife){
    int keyPress = 0;
    #ifdef _WIN32
        if (!_kbhit()) return;

        int ch = _getch();
        if (ch == 0 || ch == 224) {
            ch = _getch();
            switch (ch) {
                case 72: //UP
                    keyPress =  1; 
                    break;
                case 80: //DOWN
                    keyPress =  2;
                    break;
                case 75: //LEFT
                    keyPress =  3;
                    break;
                case 77: //RIGHT
                    keyPress =  4;
                    break;
            }
        }else{
            switch (ch){
                case 13: //ENTER
                    keyPress =  5;
                    break;
                case ' ': //SPACE
                    keyPress =  6;
                    break;
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
            if (ch == 27) {  // ESC sequence
                if (getchar() == '[') {
                    switch (getchar()) {
                        case 'A': 
                            keyPress =  1;
                            break;
                        case 'B': 
                            keyPress =  2;
                            break;
                        case 'C': 
                            keyPress =  3;
                            break;
                        case 'D': 
                            keyPress =  4;
                            break;
                    }
                }
            }else{
                switch (ch){
                    case '\n':
                        keyPress = 5;
                        break;
                    case ' ':
                        keyPress = 6;
                        break;
                }
            }
        }
    #endif

    switch(keyPress){
        case 1:
            gameOfLife->cursorY = gameOfLife->cursorY != 0? gameOfLife->cursorY - 1 : 0;
            break;
        case 2:
            gameOfLife->cursorY = gameOfLife->cursorY != gameOfLife->height - 1? gameOfLife->cursorY + 1 : gameOfLife->cursorY;
            break;
        case 3:
            gameOfLife->cursorX = gameOfLife->cursorX != 0? gameOfLife->cursorX - 1 : 0;
            break;
        case 4:
            gameOfLife->cursorX = gameOfLife->cursorX != gameOfLife->width - 1? gameOfLife->cursorX + 1 : gameOfLife->cursorX;
            break;
        case 5:
            createLife(gameOfLife);
            break;
        case 6:
            gameOfLife->pause = !gameOfLife->pause;
            break;
    }
}

void render(GAMEOFLIFE* gameOfLife){
    int index = 0;
    char* output = (char*) calloc((gameOfLife->height * gameOfLife->width) + (gameOfLife->height * 10) + 1, sizeof(char));
    
    for (int i = 0; i < gameOfLife->width*gameOfLife->height + gameOfLife->height; ++i){
		if (i % gameOfLife->width == 0){
			index += sprintf(output + index, "\x1b[%i;%iH", (i / gameOfLife->width) + 1, (i % gameOfLife->width) + 1);
		}

		output[index] = gameOfLife->world[i];
		index++;
	}
    output[index] = '\0';

    write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
    write(STDOUT_FILENO,output,strlen(output));
}

int main(){
    int gameItr = 0;
    GAMEOFLIFE* gameOfLife = createWorld();

    while(1){
        if(gameItr % 100 == 0){
            render(gameOfLife);
            if(!gameOfLife->pause)  handleWorld(gameOfLife);
        }

        handleInput(gameOfLife);
	    printf(
            "\x1b[%i;%iH", 
            gameOfLife->cursorY + 1,
            gameOfLife->cursorX + 1
        );
        
        // sleep(1);
        gameItr++;
    }

    return 0;
}
