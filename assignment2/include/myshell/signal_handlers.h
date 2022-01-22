#pragma once
/**
 * @file signal_handlers.h
 * @author Justen Di Ruscio (3624673)
 * @brief Function symbols of public functions to handle signals
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <signal.h>

/**
 * @brief Executes specific signal handling function based on sigNum
 *
 * @param sigNum the signal number received by the process
 */
void handleSignal(const int sigNum);
