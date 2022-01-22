#pragma once
/**
 * @file game_console.h
 * @author Justen Di Ruscio
 * @brief Declarations for game console. Used as wrapper around distribute/console
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <pthread.h>
#include <stdbool.h>

#include <jd/task.h>
#include "../distribute/console.h"

extern pthread_mutex_t consoleMutex;

// provide to runScreenRefresher
typedef struct RefreshArg {
  Task* const sleepGame;
} RefreshArg;

/**
 * @brief initializes game console
 *
 * @return true success
 * @return false failure
 */
bool gameConsoleInit();

/**
 * @brief Call at end of game to clean game console and destroy consoleMutex
 *
 * @return int errno
 */
int gameConsoleFinish();

/**
 * @brief thread start function to refresh console
 *
 * @param data RefreshArg
 * @return void* errno
 */
void* runScreenRefresher(void* data);
