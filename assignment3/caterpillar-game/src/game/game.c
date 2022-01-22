/**
 * @file game.c
 * @author Justen Di Ruscio
 * @brief Main entry point for caterpillar game
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/game/game.h>

#include <pthread.h>
#include <stdio.h>

#include <jd/error.h>
#include <jd/list.h>
#include <jd/task.h>
#include <jd/threadpool.h>

#include <caterpillar/bullets/bullet.h>
#include <caterpillar/bullets/registry.h>
#include <caterpillar/caterpillars/caterpillar.h>
#include <caterpillar/caterpillars/spawn_caterpillars.h>
#include <caterpillar/game/constants.h>
#include <caterpillar/game/game_console.h>
#include <caterpillar/upkeep.h>
#include <caterpillar/user/player.h>
#include <caterpillar/user/user_input.h>

void *noop(void *data) { return data; }

// ======================= Public Game Function ========================
void caterpillarRun(ThreadPool *const threadPool) {
  const char fooName[] = "caterpillarRun";
  char *banner = "DONE - LOSE";

  if (gameConsoleInit()) {
    registerThreadPoolForBullets(threadPool);

    // ========== VARS TO SLEEP GAME WHILE RUNNING =============
    // Task used to sleep game until completion
    // being used with noop to effectively create a promise & future pair
    // instead of a packaged task & future pair
    Task sleepGame;
    errno = task_init(&sleepGame, noop, NULL, NULL);
    if (errno != 0) {
      fprintf(stderr, "Unable to initialize sleep game task in %s\n", fooName);
      task_destroy(&sleepGame);
      handleExitError(errno);
    }
    registerSleepTaskForBullets(&sleepGame);
    errno = task_execute(&sleepGame);
    if (errno != 0) {
      fprintf(stderr, "Unable to execute sleep game task in %s\n", fooName);
      task_destroy(&sleepGame);
      handleExitError(errno);
    }

    // ========== PLAYER THREAD =============
    Player p;
    errno =
        initPlayer(&p, PLAYER_START_ROW, PLAYER_START_COL, PLAYER_START_LIVES);
    if (errno != 0) {
      fprintf(stderr, "Unable to initialize player in %s\n", fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      handleExitError(errno);
    }
    registerPlayerForBullets(&p);
    Task playerTask;
    void *runPlayerResult;
    RunPlayerArg playerArg = {.p = &p, .sleepGame = &sleepGame};
    errno = task_init(&playerTask, runPlayer, &playerArg, &runPlayerResult);
    if (errno != 0) {
      fprintf(stderr, "Unable to initialize player in %s\n", fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      handleExitError(errno);
    }
    errno = tp_enqueueImmediate(threadPool, &playerTask);
    if (errno != 0) {
      fprintf(stderr, "Unable to add player task to thread pool in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      handleExitError(errno);
    }

    // ========== SCREEN REFRESHER THREAD =============
    Task screenRefresherTask;
    void *screenRefreshResult;
    RefreshArg refreshArg = {.sleepGame = &sleepGame};
    errno = task_init(&screenRefresherTask, runScreenRefresher, &refreshArg,
                      &screenRefreshResult);
    if (errno != 0) {
      fprintf(stderr, "Unable to initialize screen refresher task in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      handleExitError(errno);
    }
    errno = tp_enqueueImmediate(threadPool, &screenRefresherTask);
    if (errno != 0) {
      fprintf(stderr, "Unable to add refresher task to threadpool in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      handleExitError(errno);
    }

    // ========== USER INPUT THREAD =============
    Task userInputTask;
    void *userInputResult;
    UserInputArg inputArg = {.p = &p, .sleepGame = &sleepGame};
    errno =
        task_init(&userInputTask, acceptUserInput, &inputArg, &userInputResult);
    if (errno != 0) {
      fprintf(stderr, "Unable to initialize user input thread in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      task_destroy(&userInputTask);
      handleExitError(errno);
    }
    errno = tp_enqueueImmediate(threadPool, &userInputTask);
    if (errno != 0) {
      fprintf(stderr, "Unable to user input task to threadpool in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      task_destroy(&userInputTask);
      handleExitError(errno);
    }

    // ========== LIST OF RUNNING CATERPILLARS =============
    pthread_mutex_t runningCatLock;
    errno = pthread_mutex_init(&runningCatLock, NULL);
    if (errno != 0) {
      fprintf(stderr, "Unable initialize running caterpillar lock in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      task_destroy(&userInputTask);
      pthread_mutex_destroy(&runningCatLock);
      handleExitError(errno);
    }
    registerRunningCatLockForBullets(&runningCatLock);
    List runningCaterpillars = list_constructEmpty(sizeof(Task *));
    runningCaterpillars.elementDeleter = destroyCaterpillarTask;
    registerCaterpillarsForBullets(&runningCaterpillars);

    // ========== LIST OF KILLED CATERPILLARS =============
    pthread_mutex_t killedCatLock;
    errno = pthread_mutex_init(&killedCatLock, NULL);
    if (errno != 0) {
      fprintf(stderr, "Unable initialize killed caterpillar lock in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      task_destroy(&userInputTask);
      pthread_mutex_destroy(&runningCatLock);
      list_freeNodes(&runningCaterpillars);
      pthread_mutex_destroy(&killedCatLock);
      handleExitError(errno);
    }
    registerKilledCatLockForBullets(&killedCatLock);
    List killedCaterpillars = list_constructEmpty(sizeof(Task *));
    registerKilledCaterpillarsForBullets(&killedCaterpillars);
    killedCaterpillars.elementDeleter = destroyCaterpillarTask;

    // ========== CATERPILLAR SPAWNING THREAD =============
    Task spawnCatsTask;
    void *spawnCatsResult;
    SpawnCaterpillarsArg spawnCatsArg = {
        .runningCatLock = &runningCatLock,
        .runningCaterpillars = &runningCaterpillars,
        .killedCatLock = &killedCatLock,
        .killedCaterpillars = &killedCaterpillars,
        .threadPool = threadPool,
        .sleepGame = &sleepGame};
    errno = task_init(&spawnCatsTask, spawnCaterpillars, &spawnCatsArg,
                      &spawnCatsResult);
    if (errno != 0) {
      fprintf(stderr, "Unable initialize spawn caterpillar task in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      task_destroy(&userInputTask);
      pthread_mutex_destroy(&runningCatLock);
      list_freeNodes(&runningCaterpillars);
      pthread_mutex_destroy(&killedCatLock);
      list_freeNodes(&killedCaterpillars);
      task_destroy(&spawnCatsTask);
      handleExitError(errno);
    }
    errno = tp_enqueueImmediate(threadPool, &spawnCatsTask);
    if (errno != 0) {
      fprintf(stderr,
              "Unable to add spawn caterpillar task to threadpool in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      task_destroy(&userInputTask);
      pthread_mutex_destroy(&runningCatLock);
      list_freeNodes(&runningCaterpillars);
      pthread_mutex_destroy(&killedCatLock);
      list_freeNodes(&killedCaterpillars);
      task_destroy(&spawnCatsTask);
      handleExitError(errno);
    }

    // ========== UPKEEP THREAD =============
    Task upkeepTask;
    void *upkeepResult;
    UpkeepArg upkeepArg = {.p = &p,
                           .killedCatLock = &killedCatLock,
                           .killedCaterpillars = &killedCaterpillars,
                           .banner = &banner,
                           .sleepGame = &sleepGame};
    errno = task_init(&upkeepTask, runUpkeep, &upkeepArg, &upkeepResult);
    if (errno != 0) {
      fprintf(stderr, "Unable to initialize upkeep task in %s\n", fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      task_destroy(&userInputTask);
      pthread_mutex_destroy(&runningCatLock);
      list_freeNodes(&runningCaterpillars);
      pthread_mutex_destroy(&killedCatLock);
      list_freeNodes(&killedCaterpillars);
      task_destroy(&spawnCatsTask);
      task_destroy(&upkeepTask);
      handleExitError(errno);
    }
    errno = tp_enqueueImmediate(threadPool, &upkeepTask);
    if (errno != 0) {
      fprintf(stderr, "Unable to add upkeep task to threadpool in %s\n",
              fooName);
      task_destroy(&sleepGame);
      destroyPlayer(&p);
      task_destroy(&playerTask);
      task_destroy(&screenRefresherTask);
      task_destroy(&userInputTask);
      pthread_mutex_destroy(&runningCatLock);
      list_freeNodes(&runningCaterpillars);
      pthread_mutex_destroy(&killedCatLock);
      list_freeNodes(&killedCaterpillars);
      task_destroy(&spawnCatsTask);
      task_destroy(&upkeepTask);
      handleExitError(errno);
    }

    // ========== SLEEP GAME WHILE RUNNING =============
    // Sleep current thread until game completion using Task
    //  task_getResult will sleep until task_markComplete is called
    task_getResult(&sleepGame);
    task_destroy(&sleepGame);

    putBanner(banner);

    // ========== FINISH TASKS AND GET RESULTS =============
    task_getResult(&playerTask);
    task_getResult(&screenRefresherTask);
    task_getResult(&userInputTask);
    task_getResult(&upkeepTask);
    task_getResult(&spawnCatsTask);

    // ========== DESTROY GAME VARIABLES =============
    task_destroy(&playerTask);
    task_destroy(&upkeepTask);
    task_destroy(&screenRefresherTask);
    task_destroy(&userInputTask);
    task_destroy(&spawnCatsTask);

    destroyPlayer(&p);
    list_freeNodes(&runningCaterpillars);
    list_freeNodes(&killedCaterpillars);
    pthread_mutex_destroy(&runningCatLock);
    pthread_mutex_destroy(&killedCatLock);

    finalKeypress(); /* wait for final key before killing curses and game */
  }

  gameConsoleFinish();
}
