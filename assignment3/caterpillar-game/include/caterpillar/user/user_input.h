#pragma once
/**
 * @file user_input.h
 * @author Justen Di Ruscio
 * @brief Declarations related to accepting user input to control player
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/task.h>

#include <caterpillar/user/player.h>

// provide to accept user input
typedef struct UserInputArg {
  Task* const sleepGame;
  Player* const p;
} UserInputArg;

/**
 * @brief Thread start function to accept user input
 *
 * @param data UserInputArg
 * @return void* errno
 */
void *acceptUserInput(void *data);
