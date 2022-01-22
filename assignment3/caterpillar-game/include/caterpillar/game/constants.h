#pragma once

/**
 * @file constants.h
 * @author Justen Di Ruscio
 * @brief Global game constants
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

// =============== GAME BOARD ===================
#define GAME_ROWS 24 // number of rows in game board
#define GAME_COLS 80 // number of columns in game board
#define BOUNDARY_ROW 16 // row with player boundary
extern char *GAME_BOARD[]; // dimensions must match macros above

// =============== SCORE BOARD ===================
#define SCORE_COLS 26 // number of columns from left to print score
#define SCORE_ROWS 0 // number of rows from top to print score
#define SCORE_MAX_LENGTH 9 // max number of digits available to print score in
#define LIFE_COLS  42 // number of columns from left to print lives
#define LIFE_ROWS 0 // number of rows from top to print lives
#define LIFE_MAX_LENGTH 10 // max number of digits available to print score in

// =============== PLAYER START GAME ===================
#define PLAYER_START_COL 37 // number of columns from left to start player
#define PLAYER_START_ROW 20 // number of rows from top to start player
#define PLAYER_START_LIVES 10 // number of lives player starts with

// =============== PLAYER RATE LIMITS ===================
#define MOVE_MIN_SECS 0.01       // limit 1 movement per 0.01 seconds
#define BULLET_MIN_SECS 0.25       // limit 1 bullet per 0.25 seconds

// =============== CATERPILLAR ===================
#define CATERPILLAR_INIT_SPEED 30 // delay for speed in ticks
#define CATERPILLAR_HIT_SPEED_MULTIPLIER 1.5 // speed multiple when hit
#define CATERPILLAR_TOP_ROW  2 // top row of caterpillar area
#define CATERPILLAR_MIN_LENGTH 5 // min length before it's killed
#define CATERPILLAR_MIN_SPEED 8 // min ticks to determine max speed

// =============== BULLET ===================
#define BULLET_SHOOT_RATE_TICKS 10 // ticks to sleep before moving bullet

// =============== SCORING ===================
#define HIT_CATERPILLAR_SCORE 1
#define HIT_PLAYER_LIVES -1
