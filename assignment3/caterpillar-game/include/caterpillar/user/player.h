#pragma once
/**
 * @file player.h
 * @author Justen Di Ruscio
 * @brief Declarations related to caterpillar game player, including space ship
 * on screen
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include <jd/task.h>
#include <jd/list.h>

#define PLAYER_WIDTH 3
#define PLAYER_HEIGHT 3

typedef enum PlayerState
{
	GAME,
	DEAD,
	GAMEOVER
} PlayerState;


typedef struct Player
{
	// initial location for restarts
	int startCol;
	int startRow;

  // bullets fired by this player
	pthread_mutex_t bulletMutex;
	List shotBullets;

	// player state
	pthread_mutex_t stateMutex; // lives, score
	PlayerState state;
	int lives;
	int score;
	int animTile;

	// location of player
  pthread_mutex_t locationMutex;
	int row;
	int col;
} Player;

typedef struct RunPlayerArg {
	Task* const sleepGame;
	Player* const p;
} RunPlayerArg;

/**
 * @brief Initializes provided player
 *
 * @param p player
 * @param startRow
 * @param startCol
 * @param lives
 * @return int errno
 */
int initPlayer(Player *const p, const int startRow, const int startCol,
               const int lives);

/**
 * @brief destroys provided player
 *
 * @param p player
 * @return int errno
 */
int destroyPlayer(Player *const p);

/**
 * @brief Thread start function to run player
 *
 * @return void* errno
 */
void* runPlayer(void*);

/**
 * @brief Moves player to provided location and updates console
 *
 * @param p player
 * @param row new row
 * @param col new column
 * @param lock flag to lock player location or not`
 * @return int errno
 */
int movePlayer(Player *const p, int row, int col, const bool lock);
