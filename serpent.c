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

/* global variables */
bool isAlive = true;    /* variable to track if snake is alive */
bool isRunning = true;
unsigned int SPEED = 100;         /* speed of the game */
unsigned int MAX_SPEED = 60;      /* max speed of the game */
unsigned int terminalRows;
unsigned int terminalCols;
int startY;
int startX;
unsigned int score;
/* */

/* Initialize structs */
Snake *snake;
Apple *apple;
/* */

/* Function prototypes */
// structure functions
Snake *startSnake();
Apple *startApple();
void appendSnakeNode(Snake *new_snake);
void freeSnake();
int snakeSize();
// game logic
void updateSnake();
void updateApple();
bool snakeCollision(int x, int y, bool excludeHead);
bool appleCollision(int x, int y);
// game i-o
void handleInput(int key);
void drawGame();
// game functions
void initializeGame();
void gameLoop();
void run();
void mainMenu(WINDOW *menuScreen, int menuType);
void cleanup();
// command line functions
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

    initializeGame();

    cleanup();

    return 0;
}

/* Function responsible of initializing the snake strucutre */
Snake *startSnake() {
    /* Allocate memory for a new snake */
    Snake *new_snake = malloc(sizeof(Snake));
    
    /* Allocate memory for the head of the snake */
    SnakeNode *head = malloc(sizeof(SnakeNode));
    
    /* Initialize the snake's head and tail to the allocated head node */
    new_snake->head = head;
    new_snake->tail = head;
    
    /* Set the head's previous and next pointers to NULL, as it's the only node in the beginning */
    head->prev = NULL;
    head->next = NULL;
    
    /* Set the initial position of the snake's head at the center of the board */
    head->pX = SCREEN_WIDTH / 2;
    head->pY = SCREEN_HEIGHT / 2;
    
    /* Initialize a node pointer to the head for iterating through the snake's body */
    SnakeNode *node = head;

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
    new_snake->direction = UP;
    
    /* Return the initialized snake */
    return new_snake;
}

/* Function responsible of initializing the apple structure */
Apple *startApple() {
    /* Allocate memory for a new apple */
    Apple *new_apple = malloc(sizeof(Apple));

    /* Seed the random number generator with the current time */
    srand(time(NULL));

    /* Generate random coordinates for the new apple, ensuring it does not overlap with the snake */
    do {
        new_apple->pX = (random() % (SCREEN_WIDTH - 2)) + 1;
        new_apple->pY = (random() % (SCREEN_HEIGHT - 2)) + 1;
    } while (snakeCollision(new_apple->pX, new_apple->pY, false));

    /* Return the initialized apple */
    return new_apple;
}

/* Function responsible of adding a new node to the tail of the snake */
void appendSnakeNode(Snake *new_snake) {
    /* Allocate memory for a new snake node */
    SnakeNode *node_ptr = malloc(sizeof(SnakeNode));
    
    /* Set the previous pointer of the new node to the current tail of the snake */
    node_ptr->prev = new_snake->tail;
    
    /* The new node is the last in the linked list, so its next pointer is NULL */
    node_ptr->next = NULL;
    
    /* Connect the current tail's next pointer to the new node */
    new_snake->tail->next = node_ptr;

    /* Update the snake's tail to point to the new node, making it the new tail */
    new_snake->tail = node_ptr;
}

/* Function responsible of freeing the memory for the snake structure */
void freeSnake() {
    /* Initialize pointers to traverse the snake linked list */
    SnakeNode *snake_current = snake->head;
    SnakeNode *snake_next = snake->head->next;

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

/* Function responsible of saving the size of the snake */
int snakeSize() {
    /* Initialize a pointer to traverse the snake linked list */
    SnakeNode *snake_ptr = snake->head;

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

/* Function responsible of handling the snake's movement */
void updateSnake() {
    /* Check if the snake is moving (either horizontally or vertically) */
    if ((ABS(snake->direction == LEFT ? -1 : snake->direction == RIGHT ? 1 : 0) > 0) ||
        (ABS(snake->direction == UP ? -1 : snake->direction == DOWN ? 1 : 0) > 0)) {
        /* Calculate the new coordinates for the head and tail of the snake */
        int new_head_x = snake->head->pX + (snake->direction == LEFT ? -1 : snake->direction == RIGHT ? 1 : 0);
        int new_head_y = snake->head->pY + (snake->direction == UP ? -1 : snake->direction == DOWN ? 1 : 0);
        int new_tail_x = snake->tail->pX + (snake->direction == LEFT ? -1 : snake->direction == RIGHT ? 1 : 0);
        int new_tail_y = snake->tail->pY + (snake->direction == UP ? -1 : snake->direction == DOWN ? 1 : 0);

        /* Check if the new head position does not overlap with an apple */
        if (!appleCollision(new_head_x, new_head_y)) {
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
            updateApple();

            /* Increase the speed if it hasn't reached the maximum */
            if (SPEED >= MAX_SPEED) {
                SPEED -= 1;
            }

            /* Create a new head node and update its position */
            SnakeNode *new_head = malloc(sizeof(SnakeNode));
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
        if (snakeCollision(new_head_x, new_head_y, false) ||
            (new_head_x == 0) || (new_head_x == SCREEN_WIDTH - 1) ||
            (new_head_y == 0) || (new_head_y == SCREEN_HEIGHT - 1)) {
            /* If there is a collision, set the snake as not alive */
            isAlive = false;
        }
    }
}

/* Function responsible of moving the apple to a new position */
void updateApple() {
    /* Variables to store the new coordinates for the apple */
    int new_x, new_y;

    /* Generate new random coordinates for the apple, ensuring it does not overlap with the snake */
    do {
        new_x = (random() % (SCREEN_WIDTH - 2)) + 1;
        new_y = (random() % (SCREEN_HEIGHT - 2)) + 1;
    } while (snakeCollision(new_x, new_y, true));

    /* Update the position of the existing apple to the new coordinates */
    apple->pX = new_x;
    apple->pY = new_y;
}

/* Function responsible of checking if the snake collided with itself */
bool snakeCollision(int x, int y, bool excludeHead) {
    /* Initialize a pointer to traverse the snake linked list */
    SnakeNode *snake_ptr;

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

/* Function to chech if the snake ate an apple */
bool appleCollision(int x, int y) {
    /* Check if the specified coordinates match the position of the apple */
    if (apple->pX == x && apple->pY == y) {
        /* The apple occupies the specified position */
        return true;
    }

    /* The apple does not occupy the specified position */
    return false;
}

/* Function responsible of handling user input */
void handleInput(int key) {
    /* Handle different key inputs to change the snake's direction */
    switch (key) {
        case KEY_UP:
            /* If the snake is not currently moving down, change its direction to up */
            if (snake->direction != DOWN) {
                snake->direction = UP;
            }
            break;
        case KEY_DOWN:
            /* If the snake is not currently moving up, change its direction to down */
            if (snake->direction != UP) {
                snake->direction = DOWN;
            }
            break;
        case KEY_RIGHT:
            /* If the snake is not currently moving left, change its direction to right */
            if (snake->direction != LEFT) {
                snake->direction = RIGHT;
            }
            break;
        case KEY_LEFT:
            /* If the snake is not currently moving right, change its direction to left */
            if (snake->direction != RIGHT) {
                snake->direction = LEFT;
            }
            break;
        default:
            /* Do nothing for other keys */
            refresh();
            break;
    }
}

/* Function responsible of drawing each object in the screen */
void drawGame() {
    /* Clear the terminal screen */
    erase();

    // Calculate the center coordinates
    int startY = (terminalRows - SCREEN_HEIGHT) / 2;
    int startX = (terminalCols - SCREEN_WIDTH) / 2;

    WINDOW *gameBoard = newwin(SCREEN_HEIGHT, SCREEN_WIDTH, startY, startX);
    box(gameBoard, 0, 0);
    refresh();
    wrefresh(gameBoard);

    /* Draw the snake's head on the game board */
    SnakeNode *snake_ptr = snake->head;
    switch (snake->direction) {
        case LEFT:
            mvaddch(snake_ptr->pY + startY, snake_ptr->pX + startX, SNAKE_HEAD_L);
            break;
        case RIGHT:
            mvaddch(snake_ptr->pY + startY, snake_ptr->pX + startX, SNAKE_HEAD_R);
            break;
        case UP:
            mvaddch(snake_ptr->pY + startY, snake_ptr->pX + startX, SNAKE_HEAD_U);
            break;
        case DOWN:
            mvaddch(snake_ptr->pY + startY, snake_ptr->pX + startX, SNAKE_HEAD_D);
            break;
    }

    /* Move to the next node */
    snake_ptr = snake_ptr->next;

    /* Draw the snake's body */
    while(snake_ptr != NULL) {
        mvaddch(snake_ptr->pY + startY, snake_ptr->pX + startX, SNAKE_BODY);
        /* Move to the next node */
        snake_ptr = snake_ptr->next;
    }

    /* Draw the apple on the game board */
    mvaddch(apple->pY + startY, apple->pX + startX, FOOD);

    /* Display the game score */
    mvprintw(startY, startX + 1, "Score: %d", snakeSize() - START_SNAKE_SIZE);
}

/* Game loop function */
void gameLoop() {
    /* Take arrow key inputs */
    int c = getch();
    if (c != ERR) {
        handleInput(c);
    }

    /* Update the snake position */
    updateSnake();

    /* Redraw the frame */
    drawGame();

    /* Refresh the window */
    refresh();
    
    /* Introduce a delay for the game loop */
    usleep(SPEED * 800L);
}

/* Function responsible of initializing the game */
void initializeGame() {
    /* Initialize window settings with ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    getmaxyx(stdscr, terminalRows, terminalCols);

    startY = (terminalRows - SCREEN_HEIGHT) / 2;
    startX = (terminalCols - SCREEN_WIDTH) / 2;

    /* Initialize the snake doubly-linked list */
    snake = startSnake();
    
    /* Initialize the apple */
    apple = startApple();

    run();
}

/* Starting point of the game */
void run() {
    int choice;

    WINDOW * menuScreen = newwin(SCREEN_HEIGHT, SCREEN_WIDTH, startY, startX);
    refresh();
    wrefresh(menuScreen);

    while (isRunning) {
        /* Display main menu */
        mainMenu(menuScreen, 1);

        /* Input validation loop */
        do {
            /* Use getch to get a single character */
            choice = wgetch(menuScreen);

        } while (choice < '1' || choice > '3');

        switch (choice) {
            case '1':
                if (!isAlive) {
                    snake = startSnake();
                    isAlive = true;
                }
                /* Start the game loop */
                while (isAlive) {
                    gameLoop();
                }
                score = snakeSize() - START_SNAKE_SIZE;
                mainMenu(menuScreen, 3);
                break;
            case '2':
                /* Show controls */
                mainMenu(menuScreen, 2);
                continue;
                break;
            case '3':
                /* Exit the game */
                isRunning = false;
                break;
            default:
                break;
        }
    }
}

/* Function to display the main menu */
void mainMenu(WINDOW *menuScreen, int menuType) {
    int menuY = (SCREEN_HEIGHT - startY) / 5;
    int menuX = (SCREEN_WIDTH - startX) / 2;

    wclear(menuScreen);
    box(menuScreen, 0, 0);
    mvwprintw(menuScreen, menuY - 1, menuX - 2, "                          ____      ");
    mvwprintw(menuScreen, menuY,     menuX - 2, " ________________________/ O  \\___/");
    mvwprintw(menuScreen, menuY + 1, menuX - 2, "<_____________________________/   \\");
    mvwprintw(menuScreen, menuY + 2, menuX - 2, " __                            _    ");
    mvwprintw(menuScreen, menuY + 3, menuX - 2, "/ _\\ ___ _ __ _ __   ___ _ __ | |_ ");
    mvwprintw(menuScreen, menuY + 4, menuX - 2, "\\ \\ / _ \\ '__| '_ \\ / _ \\ '_ \\| __|");
    mvwprintw(menuScreen, menuY + 5, menuX - 2, "_\\ \\  __/ |  | |_) |  __/ | | | |_ ");
    mvwprintw(menuScreen, menuY + 6, menuX - 2, "\\__/\\___|_|  | .__/ \\___|_| |_|\\__|");
    mvwprintw(menuScreen, menuY + 7, menuX - 2, "             |_|                    ");

    switch (menuType) {
        case 1:
            /* Print the main menu inside the menuScreen window */
            mvwprintw(menuScreen, menuY + 9,  menuX, "\tMain Menu");
            mvwprintw(menuScreen, menuY + 10, menuX, "\t  1. Start Game");
            mvwprintw(menuScreen, menuY + 11, menuX, "\t  2. Show Controls");
            mvwprintw(menuScreen, menuY + 12, menuX, "\t  3. Exit Game");
            mvwprintw(menuScreen, menuY + 13, menuX, "\tPress a key [1-3]...");
            wrefresh(menuScreen);
            break;
        case 2:
            /* Display controls */
            mvwprintw(menuScreen, menuY + 9,  menuX, "\tControls");
            mvwprintw(menuScreen, menuY + 10, menuX, "\t  Arrow Up: Move Up");
            mvwprintw(menuScreen, menuY + 11, menuX, "\t  Arrow Down: Move Down");
            mvwprintw(menuScreen, menuY + 12, menuX, "\t  Arrow Left: Move Left");
            mvwprintw(menuScreen, menuY + 13, menuX, "\t  Arrow Right: Move Right");
            mvwprintw(menuScreen, menuY + 14, menuX, "\tPress a key to go back...");
            wrefresh(menuScreen);
            wgetch(menuScreen);
            break;
        case 3:
            /* Display the final score on the main menu */
            mvwprintw(menuScreen, menuY + 13, menuX, "\tFinal Score: %d", score);
            mvwprintw(menuScreen, menuY + 14, menuX, "\tPress a key to go back...");
            wgetch(menuScreen);
            break;
    }
}

/* Function responsible of cleaning the memory */
void cleanup() {
    /* Free memory allocated by snake */
    freeSnake();

    /* Free memory allocated by apple */
    free(apple);

    /* End ncurses window */
    endwin();
}

/* Function for displaying the game controls in the command line */
void argControls() {
    printf("%s version: %.1lf\n", NAME, VERSION);
    printf("Controls:\n");
    printf("\tArrow Up: move up\n");
    printf("\tArrow Down: move down\n");
    printf("\tArrow Left: move to the left\n");
    printf("\tArrow Right: move to the right\n");
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
