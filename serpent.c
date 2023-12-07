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

/* Game variables */
bool is_alive = true;               /* variable to track if snake is alive */
int SPEED = 100;                    /* speed of the game */
int MAX_SPEED = 60;                 /* max speed of the game */
/* */

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
    /* Command line parsing */
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
    /* */

    /* Initialize board */
    board_t board = {
        .border = BOARD_CHAR,
        .boardHeight = SCREEN_HEIGHT,
        .boardWidth = SCREEN_WIDTH
    };
    
    /* Initialize the snake doubly-linked list */
    snake = startSnake(&board);
    
    /* Initialize the apple */
    apple = startApple(&board);
    
    /* Initialize window settings with ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    /* Check if the board size is greater than the screen size */
    if (board.boardWidth > COLS || board.boardHeight > LINES) {
        endwin();
        printf("ERROR: Board size is larger than the terminal window.\n");
        return 1;
    }
    
    while (is_alive) {
        /* Take arrow key inputs */
        int c = getch();
        if (c != ERR) {
            handleInput(c);
        }
    
        /* Update the snake position */
        moveSnake(&board);
    
        /* Redraw the frame */
        draw(&board);
    
        /* Refresh the window */
        refresh();
        usleep(SPEED * 800L);
    }
    
    /* Game score */
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

void appendSnakeNode(snake_t *new_snake) {
    /* Allocate memory for a new snake node */
    snake_node *node_ptr = malloc(sizeof(snake_node));
    
    /* Set the previous pointer of the new node to the current tail of the snake */
    node_ptr->prev = new_snake->tail;
    
    /* The new node is the last in the linked list, so its next pointer is NULL */
    node_ptr->next = NULL;
    
    /* Connect the current tail's next pointer to the new node */
    new_snake->tail->next = node_ptr;

    /* Update the snake's tail to point to the new node, making it the new tail */
    new_snake->tail = node_ptr;
}

snake_t *startSnake(board_t *board) {
    /* Allocate memory for a new snake */
    snake_t *new_snake = malloc(sizeof(snake_t));
    
    /* Allocate memory for the head of the snake */
    snake_node *head = malloc(sizeof(snake_node));
    
    /* Initialize the snake's head and tail to the allocated head node */
    new_snake->head = head;
    new_snake->tail = head;
    
    /* Set the head's previous and next pointers to NULL, as it's the only node in the beginning */
    head->prev = NULL;
    head->next = NULL;
    
    /* Set the initial position of the snake's head at the center of the board */
    head->pX = board->boardWidth / 2;
    head->pY = board->boardHeight / 2;
    
    /* Initialize a node pointer to the head for iterating through the snake's body */
    snake_node *node = head;

    /* Create additional nodes to form the initial snake body */
    for (int i = 1; i < START_SNAKE_SIZE; i++) {
        /* Add a new node to the snake, updating the node pointer */
        appendSnakeNode(new_snake);
        node = node->next;

        /* Copy the position of the previous node to maintain a straight line */
        node->pX = node->prev->pX;
        node->pY = node->prev->pY + 1;
    }
    
    /* Set the initial direction of the snake to move upward */
    new_snake->dX = 0;
    new_snake->dY = -1;
    
    /* Return the initialized snake */
    return new_snake;
}

void freeSnake() {
    /* Initialize pointers to traverse the snake linked list */
    snake_node *snake_current = snake->head;
    snake_node *snake_next = snake->head->next;

    /* Traverse the snake linked list and free each node */
    while (snake_next != NULL) {
        /* Free the current node */
        free(snake_current);

        /* Move to the next node */
        snake_current = snake_next;
        snake_next = snake_next->next;
    }

    /* Free the last node */
    free(snake_current);

    /* Free the snake structure itself */
    free(snake);

    /* Set the global snake pointer to NULL to avoid dangling references */
    snake = NULL;
}

bool snakeOccupies(int x, int y, bool excludeHead) {
    /* Initialize a pointer to traverse the snake linked list */
    snake_node *snake_ptr;

    /* Determine the starting point in the linked list based on whether the head should be excluded */
    if (excludeHead) {
        snake_ptr = snake->head;
    } else {
        snake_ptr = snake->head->next;
    }

    /* Traverse the snake linked list */
    while (snake_ptr != NULL) {
        /* Check if the current node's position matches the specified coordinates */
        if (snake_ptr->pX == x && snake_ptr->pY == y) {
            /* The snake occupies the specified position */
            return true;
        }

        /* Move to the next node */
        snake_ptr = snake_ptr->next;
    }

    /* The snake does not occupy the specified position */
    return false;
}

int snakeSize() {
    /* Initialize a pointer to traverse the snake linked list */
    snake_node *snake_ptr = snake->head;

    /* Initialize a counter to keep track of the number of nodes in the snake linked list */
    int counter = 0;

    /* Traverse the snake linked list and count each node */
    while (snake_ptr != NULL) {
        counter++;

        /* Move to the next node */
        snake_ptr = snake_ptr->next;
    }

    /* Return the total size of the snake, which is the number of nodes in the linked list */
    return counter;
}

apple_t *startApple(board_t *board) {
    /* Allocate memory for a new apple */
    apple_t *new_apple = malloc(sizeof(apple_t));

    /* Seed the random number generator with the current time */
    srand(time(NULL));

    /* Generate random coordinates for the new apple, ensuring it does not overlap with the snake */
    do {
        new_apple->pX = (random() % (board->boardWidth - 2)) + 1;
        new_apple->pY = (random() % (board->boardHeight - 2)) + 1;
    } while (snakeOccupies(new_apple->pX, new_apple->pY, false));

    /* Return the initialized apple */
    return new_apple;
}

void moveApple(board_t *board) {
    /* Variables to store the new coordinates for the apple */
    int new_x, new_y;

    /* Generate new random coordinates for the apple, ensuring it does not overlap with the snake */
    do {
        new_x = (random() % (board->boardWidth - 2)) + 1;
        new_y = (random() % (board->boardHeight - 2)) + 1;
    } while (snakeOccupies(new_x, new_y, true));

    /* Update the position of the existing apple to the new coordinates */
    apple->pX = new_x;
    apple->pY = new_y;
}

bool appleOccupies(int x, int y) {
    /* Check if the specified coordinates match the position of the apple */
    if (apple->pX == x && apple->pY == y) {
        /* The apple occupies the specified position */
        return true;
    }

    /* The apple does not occupy the specified position */
    return false;
}

void handleInput(int key) {
    /* Handle different key inputs to change the snake's direction */
    switch (key) {
        case KEY_UP:
            /* If the snake is not currently moving down, change its direction to up */
            if (snake->dY == 0) {
                snake->dY = -1;
                snake->dX = 0;
            }
            break;
        case KEY_DOWN:
            /* If the snake is not currently moving up, change its direction to down */
            if (snake->dY == 0) {
                snake->dY = 1;
                snake->dX = 0;
            }
            break;
        case KEY_RIGHT:
            /* If the snake is not currently moving left, change its direction to right */
            if (snake->dX == 0) {
                snake->dX = 1;
                snake->dY = 0;
            }
            break;
        case KEY_LEFT:
            /* If the snake is not currently moving right, change its direction to left */
            if (snake->dX == 0) {
                snake->dX = -1;
                snake->dY = 0;
            }
            break;
        default:
            /* Do nothing for other keys */
            refresh();
            break;
    }
}

void moveSnake(board_t *board) {
    /* Check if the snake is moving (either horizontally or vertically) */
    if ((ABS(snake->dX) > 0) || (ABS(snake->dY) > 0)) {
        /* Calculate the new coordinates for the head and tail of the snake */
        int new_head_x = snake->head->pX + snake->dX;
        int new_head_y = snake->head->pY + snake->dY;
        int new_tail_x = snake->tail->pX + snake->dX;
        int new_tail_y = snake->tail->pY + snake->dY;

        /* Check if the new head position does not overlap with an apple */
        if (!appleOccupies(new_head_x, new_head_y)) {
            /* Move the tail to the new head position */
            snake->tail->pX = new_head_x;
            snake->tail->pY = new_head_y;

            /* Adjust the linked list to maintain the snake's continuity */
            snake->tail->next = snake->head;
            snake->head->prev = snake->tail;
            snake->tail = snake->tail->prev;
            snake->tail->next = NULL;
            snake->head->prev->prev = NULL;
            snake->head = snake->head->prev;

        } else {
            /* If the head overlaps with an apple (the snake ate an apple), move the apple to a new position */
            moveApple(board);

            /* Increase the speed if it hasn't reached the maximum */
            if (SPEED <= MAX_SPEED) {
                SPEED -= 1;
            }

            /* Create a new head node and update its position */
            snake_node *new_head = malloc(sizeof(snake_node));
            new_head->pX = new_head_x;
            new_head->pY = new_head_y;
            new_head->prev = NULL;
            new_head->next = snake->head;

            /* Update the linked list to include the new head */
            snake->head->prev = new_head;
            snake->head = new_head;

            /* Add a new node to the snake's body */
            appendSnakeNode(snake);

            /* Set the position for the tail node */
            snake->tail->pX = new_tail_x;
            snake->tail->pY = new_tail_y;
        }

        /* Check for collision with the game borders or itself */
        if (snakeOccupies(new_head_x, new_head_y, false) ||
            (new_head_x == 0) || (new_head_x == board->boardWidth - 1) ||
            (new_head_y == 0) || (new_head_y == board->boardHeight - 1)) {
            /* If there is a collision, set the snake as not alive */
            is_alive = false;
        }
    }
}

void draw(board_t *board) {
    /* Clear the terminal screen */
    erase();

    /* Draw the borders of the game board */
    for (int i = 0; i < board->boardHeight; i++) {
        for (int j = 0; j < board->boardWidth; j++) {
            if (i == 0 || i == board->boardHeight - 1) {
                /* Draw the top and bottom borders */
                mvprintw(i, j, "%c", board->border);
            } else if (j == 0 || j == board->boardWidth - 1) {
                /* Draw the left and right borders */
                mvprintw(i, j, "%c", board->border);
            }
        }
    }

    /* Draw the snake on the game board */
    snake_node *snake_ptr = snake->head;
    if (snake->dX == -1) {
        /* Draw the snake head facing left */
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_HEAD_L);
    } else if (snake->dX == 1) {
        /* Draw the snake head facing right */
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_HEAD_R);
    } else if (snake->dY == -1) {
        /* Draw the snake head facing up */
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_HEAD_U);
    } else if (snake->dY == 1) {
        /* Draw the snake head facing down */
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_HEAD_D);
    }
    /* Move to the next node */
    snake_ptr = snake_ptr->next;

    /* Draw the snake body */
    while(snake_ptr != NULL) {
        mvaddch(snake_ptr->pY, snake_ptr->pX, SNAKE_BODY);
        /* Move to the next node */
        snake_ptr = snake_ptr->next;
    }

    /* Draw the apple on the game board */
    mvaddch(apple->pY, apple->pX, FOOD);

    /* Display the game score */
    mvprintw(board->boardHeight + 3, 0, "Score: %d", snakeSize() - START_SNAKE_SIZE);
}

/* Function for displaying the game controls in the command line */
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

/* Function to display the help message in the command line */
void argHelp() {
    printf("Usage: %s [OPTIONS]\n", NAME);
    printf("Play the all time classic snake game in the console.\n\n");
    printf("Options:\n");
    printf("\t-c, --show-controls  Show the controls for the game.\n");
    printf("\t-h, --help           Display this help message and exit.\n");
    printf("\t-v, --version        Display version and exit.\n");
}

/* Function to display the version in the command line */
void argVersion() {
    printf("%s. version: %.1lf\n", NAME, VERSION);
    printf("Author: Darius Drake\n");
    printf("License: GPL v3\n");
    printf("Contribute:\n");
    printf("  The source code is available on GitHub -> https://github.com/d4r1us-drk/serpent\n");
    printf("  Feel free to contribute with ideas, issues or pull requests.\n");
}
