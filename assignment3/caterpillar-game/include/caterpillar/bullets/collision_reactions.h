#pragma once
/**
 * @file collision_reactions.h
 * @author Justen Di Ruscio
 * @brief Contains declarations for actions to take when player/caterpillar get hit with bullet
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <jd/list.h>

#include <caterpillar/caterpillars/caterpillar.h>

/**
 * @brief Reacts to a player getting hit
 *
 * @return int errno
 */
int hitPlayerReaction();

/**
 * @brief Reacts to a caterpillar getting hit
 *
 * @param hitCaterpillarNode Node containing caterpillar that was hit
 * @param hitBulletNode Node containing bullet that hit caterpillar
 * @return int errno
 */
int hitCaterpillarReaction(ListNode* hitCaterpillarNode, ListNode* hitBulletNode);
