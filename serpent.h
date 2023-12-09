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
#define SCREEN_WIDTH     50         /* the virtual screen width */
#define SCREEN_HEIGHT    20	        /* the virtual screen height */
/* */

/* Possible directions for the snake */
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

/* Snake node structure (for dobly linked list) */
typedef struct SnakeNode {
    int pX, pY;                     /* represents the node's position on the board */
    struct SnakeNode *next, *prev; /* pointers to the next and previous nodes */
} SnakeNode;

/* Snake structure (dobly linked list) */
typedef struct Snake {
    Direction direction;
    SnakeNode *head, *tail;    /* snake's nodes, dobly linked list */
} Snake;

/* Apple structure */
typedef struct Food {
    int pX, pY;    /* represents the apple's position on the board */
} Apple;

#endif //SERPENT_H
