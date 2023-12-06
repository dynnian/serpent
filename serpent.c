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
#include <ncurses.h>
#include "serpent.h"
/* */

/* Absolute value macro */
#define ABS(x) (x) < 0 ? -(x) : (x)
/* */

// Variable to track if snake is alive
bool is_alive = true;

/* Initialize structs */
snake_t *snake;
apple_t *apple;
/* */

/* Function prototypes */
void appendSnakeNode(snake_t *new_snake);
snake_t *startSnake(board_t *board);
void freeSnake();
bool snakeOccupies(int x, int y, bool excludeHead);
int snakeSize();
apple_t *startApple(board_t *board);
void moveApple(board_t *board);
bool appleOccupies(int x, int y);
void handleInput(int key);
void moveSnake(board_t *board);
void draw(board_t *board);
void argControls();
void argHelp();
void argVersion();
/* */

int main (int argc, char **argv) {
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

    // Initialize board
    board_t gameBoard = {
        .border = BOARD_CHAR,
        .boardHeight = SCREEN_HEIGHT,
        .boardWidth = SCREEN_WIDTH
    };
    
    /* Initialize the snake doubly-linked list */
    snake = startSnake(&gameBoard);
    
    /* Initialize the apple */
    apple = startApple(&gameBoard);
    
    /* Initialize window settings with ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    
    while (is_alive) {
        /* Take arrow key inputs */
        int c = getch();
        if (c != ERR) {
            handleInput(c);
        }
    
        /* Update the snake position */
        moveSnake(&gameBoard);
    
        /* Redraw the frame */
        draw(&gameBoard);
    
        /* Refresh the window */
        refresh();
        usleep(SPEED * 800L);
    }
    
    int score = snakeSize();
    
    /* Free memory allocated by snake */
    freeSnake();
    
    /* Free memory allocated by apple */
    free(apple);
    
    /* End ncurses window */
    endwin();
    
    /* Print final score out to terminal */
    printf("Game over. Score: %d", score - START_SNAKE_SIZE);
    return 0;
}

void appendSnakeNode(snake_t * new_snake) {
    snake_node *node_ptr = malloc(sizeof(snake_node));
    
    node_ptr->prev = new_snake->tail;
    node_ptr->next = NULL;
    
    new_snake->tail->next = node_ptr;
    
    new_snake->tail = node_ptr;
}

snake_t *startSnake(board_t *board) {
    snake_t *new_snake = malloc(sizeof(snake_t));
    
    snake_node *head = malloc(sizeof(snake_node));
    
    new_snake->head = head;
    new_snake->tail = head;
    
    head->prev = NULL;
    head->next = NULL;
    head->pX = board->boardWidth / 2 + 1;
    head->pY = board->boardHeight / 2 + 1;
    
    snake_node *node = head;
    for (int i = 1; i < START_SNAKE_SIZE; i++) {
        appendSnakeNode(new_snake);
        node = node->next;
        node->pX = node->prev->pX;
        node->pY = node->prev->pY + 1;
    }
    
    new_snake->dX = 0;
    new_snake->dY = -1;
    
    return new_snake;
}

void freeSnake() {
    snake_node *snake_current = snake->head;
    snake_node *snake_next = snake->head->next;

    while (snake_next != NULL) {
        free(snake_current);
        snake_current = snake_next;
        snake_next = snake_next->next;
    }

    free(snake_current);
    free(snake);
    snake = NULL;
}

bool snakeOccupies(int x, int y, bool excludeHead) {
    snake_node *snake_ptr;
    
    if (excludeHead) {
        snake_ptr = snake->head;
    } else {
        snake_ptr = snake->head->next;
    }
    
    while (snake_ptr != NULL) {
    
        if (snake_ptr->pX == x && snake_ptr->pY == y) {
            return true;
        }
    
        snake_ptr = snake_ptr->next;
    }
    
    return false;
}

int snakeSize() {
    snake_node *snake_ptr = snake->head;
    int counter = 0;
    
    while (snake_ptr != NULL) {
        counter++;
        snake_ptr = snake_ptr->next;
    }
    
    return counter;
}

apple_t *startApple(board_t *board) {
    apple_t *new_apple = malloc(sizeof(apple_t));
    srandom(time(NULL));
    
    do {
        new_apple->pX = random() % board->boardWidth + 1;
    } while (new_apple->pX == board->boardHeight / 2 + 1);
    
    new_apple->pY = random() % board->boardHeight + 1;
    
    return new_apple;
}


void moveApple(board_t *board) {
    int new_x;
    int new_y;
    
    do {
        new_x = random() % board->boardWidth + 1;
        new_y = random() % board->boardHeight + 1;
    
    } while (snakeOccupies(new_x, new_y, true));
    
    apple->pX = new_x;
    apple->pY = new_y;
}

bool appleOccupies(int x, int y) {
    if (apple->pX == x && apple->pY == y) {
        return true;
    }
    return false;
}

void handleInput(int key) {
    switch(key) {
        case KEY_UP:
            if (snake->dY == 0) {
                snake->dY = -1;
                snake->dX = 0;
                }
            break;
        case KEY_DOWN:
            if (snake->dY == 0) {
                snake->dY = 1;
                snake->dX = 0;
            }
            break;
        case KEY_RIGHT:
            if (snake->dX == 0) {
                snake->dX = 1;
                snake->dY = 0;
            }
            break;
        case KEY_LEFT:
            if (snake->dX == 0) {
                snake->dX = -1;
                snake->dY = 0;
            }
            break;
        default:
            refresh();
            break;
    }
}

void moveSnake (board_t *board) {
    if ((ABS(snake->dX) > 0) || (ABS(snake->dY) > 0)) {
        if (!appleOccupies(snake->head->pX + snake->dX, snake->head->pY + snake->dY)) {
            snake->tail->pX = snake->head->pX + snake->dX;
            snake->tail->pY = snake->head->pY + snake->dY;

            snake->tail->next = snake->head;
            snake->head->prev = snake->tail;

            snake->tail = snake->tail->prev;
            snake->tail->next = NULL;
            snake->head->prev->prev = NULL;

            snake->head = snake->head->prev;

        } else {
            moveApple(board);
            snake_node *new_head = malloc(sizeof(snake_node));
            new_head->pX = snake->head->pX + snake->dX;
            new_head->pY = snake->head->pY + snake->dY;
            new_head->prev = NULL;
            new_head->next = snake->head;
            snake->head->prev = new_head;
            snake->head = new_head;
            appendSnakeNode(snake);
        }
    
        if (snakeOccupies(snake->head->pX, snake->head->pY, false)) {
            is_alive = false;
        }
        
        if ((snake->head->pX == 0) || (snake->head->pX == board->boardWidth + 1)) {
            is_alive = false;
        }
        
        if ((snake->head->pY == 0) || (snake->head->pY == board->boardHeight + 1)) {
            is_alive = false;
        }
    }
}

void draw(board_t *board) {
    mvprintw(0, 0, "%c", board->border);

    for (int i = 0; i < board->boardHeight; i++) {
        mvaddch(i + 1, 0, board->border);
        mvaddch(i + 1, board->boardWidth + 1, board->border);

        for (int j = 0; j < board->boardWidth; j++) {
            mvaddch(i + 1, j + 1, ' ');
        }
    }

    snake_node *snake_ptr = snake->head;

    if (snake->dX == -1) {
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_HEAD_L);
    } else if (snake->dX == 1) {
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_HEAD_R);
    } else if (snake->dY == -1) {
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_HEAD_U);
    } else if (snake->dY == 1) {
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_HEAD_D);
    }
    snake_ptr = snake_ptr->next;

    while(snake_ptr != NULL) {
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_BODY);
        snake_ptr = snake_ptr->next;
    }

    mvaddch(apple->pY, apple->pX, FOOD);

    mvprintw(board->boardHeight + 1, 0, "%c", board->border);
    mvprintw(board->boardHeight + 3, 0, "Score: %d", snakeSize() - START_SNAKE_SIZE);
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
