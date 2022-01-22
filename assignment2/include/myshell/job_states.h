#pragma once
/**
 * @file job_states.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to process states and transitioning between them.
 * Vectors of PIDs to track states of child processes and a useful wait
 * function.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/vector.h>
#include <sys/types.h>

extern Vector fgPids;         // processes in foreground
extern Vector suspendedPids;  // suspended processes
extern Vector bgPids;         // processes in background

/**
 * @brief Waits for all PIDs listed in fgPids using WUNTRACED flag. Updates job
 * state vectors based on status of stopped process. Sets errno upon error.
 *
 * @return true successfully waited and managed process states
 * @return false failed - an error occurred
 */
bool waitForForegroundPids();
