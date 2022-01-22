/**
 * @file bullet_list.c
 * @author Justen Di Ruscio
 * @brief Function definitions for bullet_list.h
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "bullet_list.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include <caterpillar/bullets/registry.h>
#include <caterpillar/caterpillars/caterpillar.h>
#include <caterpillar/user/player.h>

// ========================= Support Functions =====================
/**
 * @brief erases all of the bullets from all of the caterpillar's lists
 *
 * @return int errno
 */
static int eraseCaterpillarsBullets() {
  const char fooName[] = "eraseCaterpillarsBullets";

  // for each running caterpillar
  const ListNode *caterpillarNode = registry.runningCaterpillars->head;
  while (caterpillarNode != (ListNode *)NULL) {
    const Task *const catTask = (*(Task **)caterpillarNode->data);
    Caterpillar *const caterpillar =
        ((RunCaterpillarArg *)catTask->fooArg)->caterpillar;

    // lock current caterpillar's bullet list before iterating its bullets
    errno = pthread_mutex_lock(&caterpillar->bulletMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure locking a Caterpillar's bullet mutex in %s\n",
              fooName);
      return false;
    }

    // erase shot bullets
    list_freeNodes(&caterpillar->shotBullets);

    errno = pthread_mutex_unlock(&caterpillar->bulletMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure unlocking a Caterpillar's bullet mutex in %s\n",
              fooName);
      return false;
    }

    caterpillarNode = caterpillarNode->next;
  }

  return errno;
}

// ======================== Public Bullet List Functions ====================
int eraseBulletFromShooterList(void *const shooter, ListNode *const bulletNode,
                               const BulletType type, const bool lock) {
  const char fooName[] = "removeBulletFromShooterList";

  // Argument Validity Check
  errno = 0;
  if (shooter == (void *)NULL) {
    fprintf(stderr, "argument 'shooter' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }
  if (bulletNode == (void *)NULL) {
    fprintf(stderr,
            "argument 'bulletNode' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Resolve shooter's bullets
  List *shooterBullets;
  pthread_mutex_t *shooterBulletMutex;
  if (type == b_Player) {
    Player *const p = (Player *)shooter;
    shooterBullets = &p->shotBullets;
    shooterBulletMutex = &p->bulletMutex;
  } else {
    Caterpillar *const c = (Caterpillar *)shooter;
    shooterBullets = &c->shotBullets;
    shooterBulletMutex = &c->bulletMutex;
  }

  // Remove bullet from shooter's list
  if (lock) {
    errno = pthread_mutex_lock(shooterBulletMutex);
    if (errno != 0) {
      fprintf(stderr, "unable to lock shooter's bullet mutex in %s\n", fooName);
      return errno;
    }
  }

  bool erased = true;
  if (list_containsNode(shooterBullets, bulletNode)) {
    erased = list_eraseNode(shooterBullets, bulletNode);
    if (!erased) {
      fprintf(stderr, "Failure erasing bullet from shooter's list in %s\n",
              fooName);
    }
  }

  if (lock) {
    errno = pthread_mutex_unlock(shooterBulletMutex);
    if (errno != 0) {
      fprintf(stderr, "unable to unlock shooter's bullet mutex in %s\n",
              fooName);
    }
  }
  return errno;
}

int addBulletToShooterList(void *const shooter, ListNode *const bulletNode,
                           const BulletType type) {
  const char fooName[] = "addBulletToShooterList";

  // Argument Validity Check
  errno = 0;
  if (shooter == (void *)NULL) {
    fprintf(stderr, "argument 'shooter' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }
  if (bulletNode == (void *)NULL) {
    fprintf(stderr,
            "argument 'bulletNode' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Resolve shooter's bullets
  List *shooterBullets;
  pthread_mutex_t *shooterBulletMutex;
  if (type == b_Player) {
    Player *const p = (Player *)shooter;
    shooterBullets = &p->shotBullets;
    shooterBulletMutex = &p->bulletMutex;
  } else {
    Caterpillar *const c = (Caterpillar *)shooter;
    shooterBullets = &c->shotBullets;
    shooterBulletMutex = &c->bulletMutex;
  }

  // Add bullet to shooter's list
  errno = pthread_mutex_lock(shooterBulletMutex);
  if (errno != 0) {
    fprintf(stderr, "unable to lock shooter's bullet mutex in %s\n", fooName);
    return errno;
  }

  const bool pushed = list_pushNodeBack(shooterBullets, bulletNode);
  if (!pushed) {
    fprintf(stderr, "Failure pushing bullet to shooter's list in %s\n",
            fooName);
  }

  errno = pthread_mutex_unlock(shooterBulletMutex);
  if (errno != 0) {
    fprintf(stderr, "unable to unlock shooter's bullet mutex in %s\n", fooName);
  }
  return errno;
}

int eraseAllBullets() {
  const char fooName[] = "eraseAllBullets";
  errno = 0;
  // Error if caterpillars/player haven't been registered
  if (registry.runningCaterpillars == NULL || registry.runningCatLock == NULL ||
      registry.player == NULL) {
    fprintf(stderr, "Game components must be registered before using %s\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // lock running caterpillars before iterating over them
  errno = pthread_mutex_lock(registry.runningCatLock);
  if (errno != 0) {
    fprintf(stderr, "Unable to lock running caterpillars mutex in %s\n",
            fooName);
    return false;
  }

  // erase player's bullets
  list_freeNodes(&registry.player->shotBullets);

  // erase caterpillar's bullets
  errno = eraseCaterpillarsBullets();
  if (errno != 0) {
    fprintf(stderr, "Failure erasing caterpillar bullets in %s\n", fooName);
    return errno;
  }

  errno = pthread_mutex_unlock(registry.runningCatLock);
  if (errno != 0) {
    fprintf(stderr, "Unable to unlock running caterpillars mutex in %s\n",
            fooName);
  }
  return errno;
}
