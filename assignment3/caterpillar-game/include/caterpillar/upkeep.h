/**
 * @file upkeep.h
 * @author Justen Di Ruscio
 * @brief Upkeep game declarations
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once

#include <pthread.h>

#include <jd/task.h>
#include <jd/list.h>

#include <caterpillar/user/player.h>

// provide to runUpkeep
typedef struct UpkeepArg {
  Player* const p;
  Task* const sleepGame;
  pthread_mutex_t *const killedCatLock;
  List* const killedCaterpillars;
  char** const banner;
} UpkeepArg;

/**
 * @brief thread start function to upkeep game
 *
 * @param data UpkeepArg
 * @return void* errno
 */
void* runUpkeep(void* data);
