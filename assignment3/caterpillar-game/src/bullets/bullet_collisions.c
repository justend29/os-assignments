#include <caterpillar/bullets/bullet_collisions.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include <jd/list.h>

#include <caterpillar/bullets/registry.h>

// Collision detection specialized to single char bullet
#if BULLET_HEIGHT == 1 && BULLET_WIDTH == 1

// ============================ Support Functions =============================
/**
 * @brief Detects overlap between provided bullet and caterpillar. Sets errno on
 * error. Doesn't lock anything
 *
 * @param bullet
 * @param cat
 * @return true bullet hit cat
 * @return false no hit
 */
bool hitCaterpillarFoo(const Bullet *const bullet,
                       const Caterpillar *const cat) {
  const char fooName[] = "hitCaterpillar";

  // Argument Validity Checks
  errno = 0;
  if (bullet == (Bullet *)NULL) {
    fprintf(stderr, "argument 'bullet' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (cat == (Caterpillar *)NULL) {
    fprintf(stderr, "argument 'cat' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Check if any of the caterpillar's segments overlap
  for (int segIdx = 0; segIdx < cat->numSegments; ++segIdx) {
    const CaterpillarSegment *const seg = cat->segments + segIdx;
    if (hitCaterpillarSegment(bullet, seg)) {
      return true;
    }
  }
  return false;
}

// ============================ Public Functions =============================
bool hitCaterpillarSegment(const Bullet *const b,
                           const CaterpillarSegment *const seg) {
  const char fooName[] = "hitCaterpillarSegment";

  // Argument Validity Check
  errno = 0;
  if (b == (Bullet *)NULL) {
    fprintf(stderr, "Argument 'b' of %s must point to a Bullet\b", fooName);
    errno = EPERM;
    return errno;
  }
  if (seg == (CaterpillarSegment *)NULL) {
    fprintf(stderr, "Argument 'c' of %s must point to a Caterpillar\b",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Check if they hit
  const int yDiff = b->row - seg->row;
  const bool verticalOverlap = yDiff >= 0 && yDiff < CATERPILLAR_HEIGHT;
  const bool horizontalOverlap = b->col == seg->col;
  return horizontalOverlap && verticalOverlap;
}

bool hitCaterpillars(Bullet *const bullet, List *const runningCaterpillars,
                     ListNode **hitCaterpillarNode) {
  const char fooName[] = "hitCaterpillars";

  // Argument Validity Checks
  if (bullet == (Bullet *)NULL) {
    fprintf(stderr, "argument 'bullet' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (runningCaterpillars == (List *)NULL) {
    fprintf(
        stderr,
        "argument 'runningCaterpillars' of %s must point to a valid address\n",
        fooName);
    errno = EPERM;
    return false;
  }

  // Check if any caterpillars are hit
  // lock location mutexes so neither can move
  errno = pthread_mutex_lock(registry.runningCatLock);
  if (errno != 0) {
    fprintf(stderr, "Failure to lock running caterpillars mutex in %s\n",
            fooName);
    return false;
  }
  errno = pthread_mutex_lock(&bullet->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to lock bullet's location mutex in %s\n", fooName);
    return false;
  }

  bool hit = false;
  ListNode *n = runningCaterpillars->head;
  while (n != (ListNode *)NULL) {
    Task *caterpillarTask = *(Task **)n->data;
    RunCaterpillarArg *catArg = ((RunCaterpillarArg *)caterpillarTask->fooArg);
    Caterpillar *caterpillar = catArg->caterpillar;

    errno = pthread_mutex_lock(&caterpillar->locationMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure locking location mutex of caterpillar in %s\n",
              fooName);
    }

    hit = hitCaterpillarFoo(bullet, catArg->caterpillar);

    if (hit) {
      *hitCaterpillarNode = n;
      break;
    } else {
      errno = pthread_mutex_unlock(&caterpillar->locationMutex);
      if (errno != 0) {
        fprintf(stderr,
                "Failure unlocking location mutex of caterpillar in %s\n",
                fooName);
      }
    }

    n = n->next;
  }

  errno = pthread_mutex_unlock(registry.runningCatLock);
  if (errno != 0) {
    fprintf(stderr, "Failure to unlock running caterpillars mutex in %s\n",
            fooName);
    return false;
  }

  if (!hit) {
    errno = pthread_mutex_unlock(&bullet->locationMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure to unlock bullet's location mutex in %s\n",
              fooName);
      return false;
    }
  }

  return hit;
}

bool anyBulletHitCaterpillar(Caterpillar *const caterpillar,
                             ListNode **hitBulletNode) {
  const char fooName[] = "anyBulletHitCaterpillar";
  bool hit = false;

  // Skip until player is registered
  if (registry.player == (Player *)NULL) {
    return false;
  }

  // Argument Validity Check
  errno = 0;
  if (caterpillar == (Caterpillar *)NULL) {
    fprintf(stderr,
            "argument 'caterpillar' of %s must point to a valid Caterpillar\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Check if any of the Player's shot bullet have hit caterpillar
  // lock player's bullets before iterating over them
  errno = pthread_mutex_lock(&registry.player->bulletMutex);
  if (errno != 0) {
    fprintf(stderr, "Unable to lock player's bullet mutex in %s\n", fooName);
    return false;
  }

  // for each of player's bullets, check if it hit caterpillar
  ListNode *bulletNode = registry.player->shotBullets.head;
  while (bulletNode != (ListNode *)NULL) {
    const Task *bulletTask = (*(Task **)bulletNode->data);
    Bullet *const b = ((RunShootBulletArg *)bulletTask->fooArg)->bullet;

    errno = pthread_mutex_lock(&b->locationMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure locking a bullet's location mutex in %s\n",
              fooName);
      return false;
    }

    const bool hit = hitCaterpillarFoo(b, caterpillar);

    errno = pthread_mutex_unlock(&b->locationMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure unlocking a bullet's location mutex in %s\n",
              fooName);
      return false;
    }

    if (hit) {
      *hitBulletNode = bulletNode;
      break;
    }

    bulletNode = bulletNode->next;
  }

  errno = pthread_mutex_unlock(&registry.player->bulletMutex);
  if (errno != 0) {
    fprintf(stderr, "Unable to unlock player's bullet mutex in %s\n", fooName);
    return false;
  }
  return hit;
}

bool hitPlayer(Bullet *const bullet, Player *const player) {
  const char fooName[] = "hitPlayer";

  // Argument Validity Checks
  errno = 0;
  if (bullet == (Bullet *)NULL) {
    fprintf(stderr, "argument 'bullet' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (player == (Player *)NULL) {
    fprintf(stderr, "argument 'player' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Check if bullet overlaps with player
  // lock location mutexes so neither can move
  errno = pthread_mutex_lock(&bullet->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to lock bullet's location mutex in %s\n", fooName);
    return false;
  }

  const int yDiff = bullet->row - player->row;
  const int xDiff = bullet->col - player->col;

  errno = pthread_mutex_unlock(&bullet->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to lock bullet's location mutex in %s\n", fooName);
    return false;
  }

  return xDiff >= 0 && yDiff >= 0 && yDiff < PLAYER_HEIGHT &&
         xDiff < PLAYER_WIDTH;
}

bool hitPlayerLocked(Bullet *const bullet, Player *const player) {
  const char fooName[] = "hitPlayerLocked";

  // Argument Validity Checks
  errno = 0;
  if (bullet == (Bullet *)NULL) {
    fprintf(stderr, "argument 'bullet' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (player == (Player *)NULL) {
    fprintf(stderr, "argument 'player' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Check if bullet overlaps with player
  // lock location mutexes so neither can move
  errno = pthread_mutex_lock(&player->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to lock player's location mutex in %s\n", fooName);
    return false;
  }
  errno = pthread_mutex_lock(&player->bulletMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to lock player's location mutex in %s\n", fooName);
    return false;
  }
  errno = pthread_mutex_lock(&bullet->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to lock bullet's location mutex in %s\n", fooName);
    return false;
  }

  const int yDiff = bullet->row - player->row;
  const int xDiff = bullet->col - player->col;
  const bool hit =
      xDiff >= 0 && yDiff >= 0 && yDiff < PLAYER_HEIGHT && xDiff < PLAYER_WIDTH;

  if (!hit) {
    errno = pthread_mutex_unlock(&player->locationMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure to unlock player's location mutex in %s\n",
              fooName);
      return false;
    }
    errno = pthread_mutex_unlock(&player->bulletMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure to unlock player's location mutex in %s\n",
              fooName);
      return false;
    }
  }
  errno = pthread_mutex_unlock(&bullet->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to unlock bullet's location mutex in %s\n",
            fooName);
    return false;
  }
  return hit;
}

bool anyBulletHitPlayer(Player *const player, Caterpillar **shooter,
                        ListNode **hitBulletNode) {
  const char fooName[] = "anyBulletHitPlayer";
  bool hit = false;

  // Skip if caterpillars haven't been registered
  if (registry.runningCaterpillars == (List *)NULL ||
      registry.runningCatLock == NULL) {
    return false;
  }

  // Argument Validity Check
  errno = 0;
  if (player == (Player *)NULL) {
    fprintf(stderr,
            "argument 'player' of %s must point to the current player\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Check each shot bullet for each caterpillar
  errno = pthread_mutex_lock(registry.runningCatLock);
  if (errno != 0) {
    fprintf(stderr, "Unable to lock running caterpillars mutex in %s\n",
            fooName);
    return false;
  }

  // for each caterpillar on the board
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

    // check collision on each bullet of current caterpillar
    ListNode *bulletNode = caterpillar->shotBullets.head;
    while (bulletNode != (ListNode *)NULL) {
      const Task *const bTask = (*(Task **)bulletNode->data);
      Bullet *const bullet = ((RunShootBulletArg *)bTask->fooArg)->bullet;
      if (hitPlayer(bullet, registry.player)) {
        hit = true;
        *hitBulletNode = bulletNode;
        *shooter = caterpillar;
        break;
      }
      bulletNode = bulletNode->next;
    }

    errno = pthread_mutex_unlock(&caterpillar->bulletMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure unlocking a Caterpillar's bullet mutex in %s\n",
              fooName);
      return false;
    }

    if (hit) {
      break;
    }

    caterpillarNode = caterpillarNode->next;
  }

  errno = pthread_mutex_unlock(registry.runningCatLock);
  if (errno != 0) {
    fprintf(stderr, "Unable to unlock running caterpillars mutex in %s\n",
            fooName);
    return false;
  }

  return hit;
}

#endif
