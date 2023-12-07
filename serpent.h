#ifndef SERPENT_H
#define SERPENT_H

 /* Program information */
#define NAME    "serpent"
#define VERSION 0.1
/* */

/* Absolute value macro */
#define ABS(x) (x) < 0 ? -(x) : (x)
/* */

/* Global variables/constants */
#define START_SNAKE_SIZE 5          /* snake's initial size */
#define SNAKE_BODY       '*'        /* snake's body */
#define SNAKE_HEAD_U     'v'        /* head when going up */
#define SNAKE_HEAD_D     '^'        /* head when going down */
#define SNAKE_HEAD_L     '>'        /* head when going left */
#define SNAKE_HEAD_R     '<'        /* head when going right  */
#define FOOD             '@'        /* normal food */
#define BOARD_CHAR       '#'        /* character at corners of border */
#define SCREEN_WIDTH     40         /* the virtual screen width */
#define SCREEN_HEIGHT    30	        /* the virtual screen height */
/* */

/* Board structure */
typedef struct board_t {
    char border;                /* will use BOARD_CHAR */
    unsigned int boardHeight;   /* will use SCREEN_HEIGHT */
    unsigned int boardWidth;    /* will use SCREEN_WIDTH */
} board_t;

/* Snake node structure (for dobly linked list) */
typedef struct snake_node {
    int pX, pY;                     /* represents the node's position on the board */
    struct snake_node *next, *prev; /* pointers to the next and previous nodes */
} snake_node;

/* Snake structure (dobly linked list) */
typedef struct snake_t {
    int dX, dY;                 /* represents the snake's direction */
    snake_node *head, *tail;    /* snake's nodes, dobly linked list */
} snake_t;

/* Apple structure */
typedef struct apple_t {
    int pX, pY; /* represents the apple's position on the board */
} apple_t;

#endif //SERPENT_H
