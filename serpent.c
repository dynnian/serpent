/* 
 * serpent.c
 *
 * Copyright 2023 Darius Drake
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
/* */

/* Program information */
#define NAME    "serpent"
#define VERSION 0.1
/* */

/* global variables/constants */
#define REFRESH_RATE 0.08               // refresh rate (in seconds)
#define START_SNAKE_LEN 5               // initial snake length
#define SNAKE_BODY '*'                  // snake's body
#define SNAKE_HEAD_U 'v'                // head when going up
#define SNAKE_HEAD_D '^'                // head when going down
#define SNAKE_HEAD_L '>'                // head when going left
#define SNAKE_HEAD_R '<'                // head when going right
#define FOOD '@'                        // normal food
#define FOOD_SCORE 1                    // score increase when snake eats food
#define BORDER_CORNER '+'               // character at corners of border
#define BORDER_VERT '|'                 // character for vertical border
#define BORDER_HORI '-'                 // character for horizontal border
#define SCREEN_WIDTH 40                 // the virtual screen width
#define SCREEN_HEIGHT 30                // the virtual screen height
/* */

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

typedef enum { RUNNING, PAUSED, STOPPED } GameState;

typedef struct {
    int X;
    int Y;
} Point;

typedef struct {
    char borderCor;
    char borderVer;
    char borderHor;
    unsigned int width;
    unsigned int height;
} Board;

typedef struct {
    GameState state;
    unsigned int points;
    char* player;
} Game;

typedef struct {
    Point position;
    char sprite;
    SnakeNode* next;
} SnakeNode;

typedef struct {
    Direction direction;
    unsigned int length;
    SnakeNode* head;
    SnakeNode* tail;
} Snake;

typedef struct {
    Point position;
    unsigned int points;
    char food;
} Food;

typedef struct {
    char borderCor;
    char borderVer;
    char borderHor;
    unsigned int width;
    unsigned int height;
} Board;

// Function prototypes
void initializeGame(Game *game, Board *board, Snake *snake, Food *food);
void clearScreen();
void waitForKeypress();
void displayLogo();
void displayMainMenu();
void displayControls();
void displayInfo();
void argControls();
void argHelp();
void argVersion();

int main (int argc, char **argv) {
    Game game;
    Board board;
    Food food;
    Snake snake;

    int option;

    static const char* short_options = "chv";
    static struct option long_options[] = {
        {"show-controls", no_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

    while ((option = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
        switch (option) {
            case 'c':
                argControls();
                break;
            case 'h':
                argHelp();
                break;
            case 'v':
                argVersion();
                break;
            case '?':
                fprintf(stderr, "Use '-h, --help' for help.\n");
                return 1;
        }
    }
    return 0;
}

void initializeGame(Game *game, Board *board, Snake *snake, Food *food) {
    game->state = RUNNING;
    game->points = 0;

    // Initialize snake direction and head
    snake->direction = RIGHT;
    snake->head = (SnakeNode*)malloc(sizeof(SnakeNode));
    snake->head->sprite = SNAKE_HEAD_R;
    snake->head->position.X = SCREEN_HEIGHT / 2;
    snake->head->position.Y = SCREEN_HEIGHT / 2;
    snake->head->next = NULL;

    // Initialize snake body
    SnakeNode *currentSegment = snake->head;
    for (int i = 1; i < START_SNAKE_LEN; ++i) {
        SnakeNode *newSegment = (SnakeNode*)malloc(sizeof(SnakeNode));
        newSegment->sprite = SNAKE_BODY;
        newSegment->position.X = snake->head->position.X - i;
        newSegment->position.Y = snake->head->position.Y;
        newSegment->next = NULL;

        currentSegment->next = newSegment;  // Link the current segment to the new one
        currentSegment = newSegment;       // Move to the new segment
    }

    // Set the tail of the snake
    snake->tail = currentSegment;
}

// Function for clearing the console
void clearScreen() {
    printf("\e[1;1H\e[2J");
}

// Function for waiting for keypress
void waitForKeypress() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    while (getchar() != '\n'); // Consume any remaining characters in the input buffer
    getchar(); // Wait for a key press
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

// Function for displaying the logo of the game
void displayLogo() {
    printf("                          ____      \n");
    printf(" ________________________/ O  \\___/\n");
    printf("<_____________________________/   \\\n");
    printf(" __                            _   \n");
    printf("/ _\\ ___ _ __ _ __   ___ _ __ | |_ \n");
    printf("\\ \\ / _ \\ '__| '_ \\ / _ \\ '_ \\| __|\n");
    printf("_\\ \\  __/ |  | |_) |  __/ | | | |_ \n");
    printf("\\__/\\___|_|  | .__/ \\___|_| |_|\\__|\n");
    printf("             |_|                   \n");
    printf("\n");
}

// Function for displaying the main menu
void displayMainMenu() {
    displayLogo();
    printf("Welcome to serpent!\n");
    printf("\n");
    printf("  Select an option:\n");
    printf("    1. Play\n");
    printf("    2. Show leader board\n");
    printf("    3. Show controls\n");
    printf("    4. Show Info\n");
    printf("    5. Exit\n");
    printf("\n");
    printf("  Choose [1-5]: ");
}

// Function for tisplaying the game controls
void displayControls() {
    displayLogo();
    printf("Controls.\n");
    printf("\n");
    printf("  Movement:\n");
    printf("    ↑: move up\n");
    printf("    ←: move to the left\n");
    printf("    →: move to the right\n");
    printf("    ↓: move down\n");
    printf("  Game:\n");
    printf("    q: quit\n");
    printf("    p: pause\n");
    printf("    r: restart\n");
    printf("\n");
    printf("Press any key to go back... ");
}

// Function for displaying the game info
void displayInfo() {
    displayLogo();
    printf("About this game.\n");
    printf("\n");
    printf("  Author: Darius Drake\n");
    printf("  License: GPL v3\n");
    printf("  Contribute:\n");
    printf("    The source code is available on GitHub -> https://github.com/d4r1us-drk/serpent\n");
    printf("    Feel free to contribute with ideas, issues or pull requests.\n");
    printf("\n");
    printf("Press any key to go back... ");
}

// Function for displaying the game controls in the command line
void argControls() {
    printf("%s controls.\n", NAME);
    printf("Movement:\n");
    printf("\t↑: move up\n");
    printf("\t←: move to the left\n");
    printf("\t→: move to the right\n");
    printf("\t↓: move down\n");
    printf("Game:\n");
    printf("\tq: quit\n");
    printf("\tp: pause\n");
    printf("\tr: restart\n");
}

// Function to display the help message in the command line
void argHelp() {
    printf("Usage: %s [OPTIONS]\n", NAME);
    printf("Play the all time classic snake game in the console.\n\n");
    printf("Options:\n");
    printf("\t-c, --show-controls  Show the controls for the game.\n");
    printf("\t-h, --help           Display this help message and exit.\n");
    printf("\t-v, --version        Display version and exit.\n");
}

// Function to display the version in the command line
void argVersion() {
    printf("%s. version: %.1lf\n", NAME, VERSION);
    printf("Author: Darius Drake\n");
    printf("License: GPL v3\n");
    printf("Contribute:\n");
    printf("  The source code is available on GitHub -> https://github.com/d4r1us-drk/serpent\n");
    printf("  Feel free to contribute with ideas, issues or pull requests.\n");
}
