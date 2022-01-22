#pragma once
/**
 * @file bullet_collisions.h
 * @author Justen Di Ruscio
 * @brief Contains utility functions to detect bullet collisions
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdbool.h>

#include <jd/list.h>

#include <caterpillar/caterpillars/caterpillar.h>
#include <caterpillar/user/player.h>
#include <caterpillar/bullets/bullet.h>

/**
 * @brief Detects if the provided bullet hit the provided caterpillar segment provided caterpillar segment without locking either. sets errno on error
 *
 * @param b bullet
 * @param seg caterpillar segment
 * @return true bullet hit caterpillar segment
 * @return false bullet didn't hit caterpillar segment
 */
bool hitCaterpillarSegment(const Bullet *const b,
                           const CaterpillarSegment *const seg);

/**
 * @brief Detects if the provided bullet hit any of the caterpillars in the provided list. Sets hitCaterpillar node to the hit caterpillar node upon a collision. sets errno on error
 *
 * @param bullet
 * @param runningCaterpillars list of caterpillars
 * @param hitCaterpillarNode set to the caterpillar node that the bullet hit
 * @return true bullet hit one of the caterpillars
 * @return false bullet hit no caterpillars
 */
bool hitCaterpillars(Bullet *const bullet,
                     List *const runningCaterpillars, ListNode** hitCaterpillarNode);

/**
 * @brief detects if any bullet shot by the player has hit the provided caterpillar. Sets hitBulletNode to the bullet node that hit the caterpillar. Sets errno on error
 *
 * @param caterpillar
 * @param hitBulletNode points to the bullet node that hit the caterpillar, if a collision occurred
 * @return true a collision occurred
 * @return false a collision didn't occur
 */
bool anyBulletHitCaterpillar(Caterpillar* const caterpillar, ListNode** hitBulletNode);

/**
 * @brief Detects if the provided bullet hit the provided player without locking either. sets errno on error
 *
 * @param bullet
 * @param player
 * @return true bullet hit player
 * @return false bullet didn't hit player
 */
bool hitPlayer(Bullet * const bullet, Player* const player);


/**
 * @brief Performs the same action as hitPlayer, but locks the player's locks
 *
 * @param bullet
 * @param player
 * @return true bullet hit player
 * @return false bullet didn't hit player
 */
bool hitPlayerLocked(Bullet * const bullet, Player* const player);

/**
 * @brief detects if any caterpillar bullet hit the player. Sets hitBulletNode to the bullet node that collided. Sets errno on error.
 *
 * @param player
 * @param shooter
 * @param hitBulletNode bullet node that hit the player
 * @return true a caterpillar bullet hit the player
 * @return false no bullet hit player
 */
bool anyBulletHitPlayer(Player* const player, Caterpillar** shooter, ListNode** hitBulletNode);
