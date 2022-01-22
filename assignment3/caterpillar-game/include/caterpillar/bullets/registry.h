#pragma once
/**
 * @file registry.h
 * @author Justen Di Ruscio
 * @brief A registry of game components for bullets to reference so the game components don't have to be passed around to the many bullets
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <pthread.h>

#include <jd/list.h>
#include <jd/threadpool.h>
#include <jd/task.h>

#include <caterpillar/user/player.h>

// Basic registry of game instances such that they don't need to be passed to
// shootBullet
typedef struct GameRegistryForBullet {
  Player *player;
  pthread_mutex_t *runningCatLock;
  List *runningCaterpillars;
  pthread_mutex_t *killedCatLock;
  List *killedCaterpillars;
  ThreadPool *tp;
  Task *sleepGame;
} GameRegistry;

extern GameRegistry registry;

void registerPlayerForBullets(Player* p);
void registerCaterpillarsForBullets(List* runningCaterpillars);
void registerThreadPoolForBullets(ThreadPool* const tp);
void registerSleepTaskForBullets(Task *const sleepGame);
void registerRunningCatLockForBullets(pthread_mutex_t *const runningCatLock);
void registerKilledCatLockForBullets(pthread_mutex_t *const killedCatLock);
void registerKilledCaterpillarsForBullets(List *const killedCaterpillars);
