/**
 * @file main.c
 * @author Justen Di Ruscio
 * @brief Main program of assignment 4; the fat32 filesystem reader
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shell/shell.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *const file = argv[1];
  const int fd = open(file, O_RDWR);
  if (fd == -1) {
    perror("opening file: ");
    exit(EXIT_FAILURE);
  }

  shellLoop(fd);

  close(fd);

  return EXIT_SUCCESS;
}
