#ifndef SERPENT_H
#define SERPENT_H

 /* Program information */
#define NAME    "serpent"
#define VERSION 0.1
/* */

/* global variables/constants */
#define START_SNAKE_SIZE 5          /* snake's initial size */
#define SNAKE_BODY       '*'        /* snake's body */
#define SNAKE_HEAD_U     'v'        /* head when going up */
#define SNAKE_HEAD_D     '^'        /* head when going down */
#define SNAKE_HEAD_L     '>'        /* head when going left */
#define SNAKE_HEAD_R     '<'        /* head when going right  */
#define FOOD             '@'        /* normal food */
#define BOARD_CHAR       '+'        /* character at corners of border */
#define SCREEN_WIDTH     30         /* the virtual screen width */
#define SCREEN_HEIGHT    20	        /* the virtual screen height */
#define SPEED 100                   /* speed of the game */
/* */

typedef struct board_t {
    char border;
    unsigned int boardHeight;
    unsigned int boardWidth;
} board_t;

typedef struct snake_node {
    int pX, pY;
    struct snake_node *next, *prev;
} snake_node;

typedef struct snake_t {
    int dX, dY;
    snake_node *head, *tail;
} snake_t;

typedef struct apple_t {
    int pX, pY;
} apple_t;

#endif //SERPENT_H
