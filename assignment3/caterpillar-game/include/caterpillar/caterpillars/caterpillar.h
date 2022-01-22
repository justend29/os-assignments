#pragma once
/**
 * @file caterpillar.h
 * @author Justen Di Ruscio
 * @brief Contains declarations of the Caterpillars and all related attributes
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/task.h>
#include <jd/list.h>

#include <caterpillar/game/game_console.h>
#include <caterpillar/game/constants.h>

#define CATERPILLAR_HEIGHT 2
#define SEGMENT_WIDTH 1
#define CATERPILLAR_ANIM_TILES 4

// move direction
typedef enum CaterpillarDirection {
  dir_Left = LEFT,
  dir_Right = RIGHT
} CaterpillarDirection;

typedef struct CaterpillarSegment {
  CaterpillarDirection direction;
  int animTile;
  int row;
  int col;
} CaterpillarSegment;

typedef struct Caterpillar {
  pthread_mutex_t locationMutex;
  int speedTicks;
  int numSegments;
  pthread_mutex_t bulletMutex;
  List shotBullets;
  CaterpillarSegment segments[GAME_COLS];
} Caterpillar;

typedef struct RunCaterpillarArg {
  Task *sleepGame;
  Caterpillar *caterpillar;
  Task* runCaterpillarTask;
  pthread_mutex_t *killedCatLock;
  List* killedCaterpillars;
} RunCaterpillarArg;

/**
 * @brief Clears a segment of the console at the provided location large enough to clear one caterpillar segment
 *
 * @param row
 * @param col
 */
void clearConsoleSegment(const int row, const int col);

/**
 * @brief Initializes the provided caterpillar
 *
 * @param c caterpillar
 * @param dir start direction
 * @param headRow head start row
 * @param headCol head start column
 * @param numSegments num segments in caterpillar
 * @param speedTicks ticks specifying init. speed
 * @return int errno
 */
int initCaterpillar(Caterpillar*const c, const CaterpillarDirection dir,
const int headRow, const int headCol, const int numSegments, const int
speedTicks);

/**
 * @brief Destroys the provided caterpillar
 *
 * @param c caterpillar
 * @return int errno
 */
int destroyCaterpillar(Caterpillar *const c);

/**
 * @brief Destroys the provided task and enclosed caterpillar
 *
 * @param task run caterpillar task
 */
void destroyCaterpillarTask(void* task);

/**
 * @brief Thread start function to run a caterpillar
 *
 * @param data RunCaterpillarArg
 * @return void* errno
 */
void* runCaterpillar(void* data);
