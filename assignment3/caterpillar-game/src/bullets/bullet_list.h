#pragma once
/**
 * @file bullet_list.h
 * @author Justen Di Ruscio
 * @brief Contains functions that are used to manage lists of bullets
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/list.h>

#include <caterpillar/bullets/bullet.h>

/**
 * @brief erases all caterpillar and player bullets
 *
 * @return int errno
 */
int eraseAllBullets();

/**
 * @brief erases the provided bullet node from the provided shooter's list. Sets errno on error
 *
 * @param shooter either a Caterpillar or Player
 * @param bulletNode bullet node from Caterpillar or Player list to delete
 * @param type type of bullet in bulletNode
 * @param lock flag to lock shooter's bullet mutex or not
 * @return int errno
 */
int eraseBulletFromShooterList(void *const shooter, ListNode *const bulletNode,
                               const BulletType type, const bool lock);

/**
 * @brief Adds the provided bullet node to the provided shooter's list of bullets. Sets errno on error
 *
 * @param shooter either a Player or Caterpillar
 * @param bulletNode bullet node to add to list
 * @param type type of bullet in bulletNode
 * @return int errno
 */
int addBulletToShooterList(void *const shooter, ListNode *const bulletNode,
                           const BulletType type);
