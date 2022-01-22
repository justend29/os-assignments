#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <jd/threadpool.h>

#include "../../distribute/console.h"
#include <caterpillar/game/game.h>

#define NUM_INIT_THREADS 16
#define INSTRUCTIONS_SLEEP_TICKS 150

void printInstructions() {
  printf(
      "======== INSTRUCTIONS ========\n"
      "w - move up\n"
      "a - move left\n"
      "s - move down\n"
      "d - move right\n"
      "space - shoot\n"
      "Hit a caterpillar and it will split into two or die if it's too short.\n"
      "Kill all caterpillars to win game.\n");
}

int main() {
  srand(time(0));
  ThreadPool tp;
  tp_init(&tp, NUM_INIT_THREADS);

  printInstructions();
  sleepTicks(INSTRUCTIONS_SLEEP_TICKS);
  caterpillarRun(&tp);

  tp_destroy(&tp);
  return EXIT_SUCCESS;
}
