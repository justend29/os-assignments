/**
 * @file registry.c
 * @author Justen Di Ruscio
 * @brief A registry of game components for bullets to reference so the game
 * components don't have to be passed around to the many bullets
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/bullets/registry.h>

GameRegistry registry;

void registerCaterpillarsForBullets(List *runningCaterpillars) {
  registry.runningCaterpillars = runningCaterpillars;
}

void registerThreadPoolForBullets(ThreadPool *const tp) { registry.tp = tp; }

void registerSleepTaskForBullets(Task *const sleepGame) {
  registry.sleepGame = sleepGame;
}

void registerPlayerForBullets(Player *p) { registry.player = p; }

void registerRunningCatLockForBullets(pthread_mutex_t *const runningCatLock) {
  registry.runningCatLock = runningCatLock;
}

void registerKilledCatLockForBullets(pthread_mutex_t *const killedCatLock) {
  registry.killedCatLock = killedCatLock;
}

void registerKilledCaterpillarsForBullets(List *const killedCaterpillars) {
  registry.killedCaterpillars = killedCaterpillars;
}
