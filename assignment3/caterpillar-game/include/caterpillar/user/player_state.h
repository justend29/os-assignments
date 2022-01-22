#pragma once
/**
 * @file player_state.h
 * @author Justen Di Ruscio
 * @brief Functions related to updating player state
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <caterpillar/user/player.h>

/**
 * @brief Updates the provided player's lives by the offset
 *
 * @param p player
 * @param offset
 * @param lives lives after update
 * @return int errno
 */
int updatePlayerLives(Player *const p, const int offset, int* const lives);

/**
 * @brief Updates the provided player's score by offset
 *
 * @param p player
 * @param offset
 * @return int errno
 */
int updatePlayerScore(Player *const p, const int offset);
