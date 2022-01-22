#pragma once
/**
 * @file bullet.h
 * @author Justen Di Ruscio
 * @brief Declarations for bullets and bullet shooting
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <pthread.h>

#include <jd/task.h>
#include <jd/threadpool.h>

#include <caterpillar/user/player.h>

#define BULLET_HEIGHT 1
#define BULLET_WIDTH  1

typedef enum BulletType {
  b_Player,
  b_Caterpillar
} BulletType;

typedef struct Bullet {
  BulletType bulletType;
  pthread_mutex_t locationMutex;
  int row;
  int col;
} Bullet;

typedef struct RunShootBulletArg {
  Bullet *bullet;
  ListNode *shootNode;
  void* shooter;
} RunShootBulletArg;

/**
 * @brief Initializes a bullet. Sets errno on error
 *
 * @param b bullet to initialize
 * @param row row to start
 * @param col col to start
 * @param bType type of bullet
 * @return int errno
 */
int initBullet(Bullet* const b, const int row, const int col, const BulletType bType);

/**
 * @brief Completely deletes the bullet, shoot task, and bullet in shootNode. Erases it from the provided shooter's list. Sets errno on error
 *
 * @param shooter Player of Caterpillar
 * @param shootNode node in shot list of shooter
 * @param bullet bullet in shootNode
 * @return int errno
 */
int deleteBullet(void *const shooter, ListNode *const shootNode,
                 Bullet *const bullet);

/**
 * @brief Destroys the provided bullet
 *
 * @param b bullet
 * @return int errno
 */
int destroyBullet(Bullet* const b);

/**
 * @brief Shoots a bullet from the provided location. Sets errno on error
 *
 * @param row start row
 * @param col start column
 * @param bType type of bullet to shoot
 * @param shooter Player or Caterpillar to shoot bullet
 * @return int errno
 */
int shootBullet(const int row, const int col, const BulletType bType, void* const shooter);

/**
 * @brief Deletes the provided bullet task and the enclosed bullet. Used as a deleter.
 *
 * @param bulletTask Task of shot bullet to delete
 */
void deleteBulletTask(void* bulletTask);
