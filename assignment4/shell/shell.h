#pragma once
/**
 * @file shell.h
 * @author Justen Di Ruscio
 * @brief Contains declarations of public functions necessary for assignment 4 shell
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

/**
 * @brief Input loop of shell
 *
 * @param fd file descriptor of disk image provided to fat32 program
 */
void shellLoop(int fd);
