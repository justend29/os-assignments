/**
 * @file shell.c
 * @author Justen Di Ruscio
 * @brief Contains main shell loop for fat32 program of assignment 4 and any
 * other necessary functions pertaining to shell itself
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../fat32/fat.h"
#include "../fat32/fat32.h"
#include "commands/commands.h"
#include "shell.h"

#define BUF_SIZE 256
#define CMD_INFO "INFO"
#define CMD_DIR "DIR"
#define CMD_CD "CD"
#define CMD_GET "GET"
#define CMD_PUT "PUT"

/**
 * @brief Prints error message in response to a single command failing
 *
 */
static void commandError(const char *const cmdName, const char *fooName) {
  fprintf(stderr, "Failed performing %s in %s: %s\n", cmdName, fooName,
          strerror(errno));
}

void shellLoop(const int fd) {
  const char fooName[] = "shellLoop";
  int running = true;
  uint32_t curDirClus;
  char buffer[BUF_SIZE];
  char bufferRaw[BUF_SIZE];

  fat32_header *const header = readHeader(fd);
  if (header == NULL)
    running = false;
  else { // valid, grab the root cluster
    curDirClus = header->bootSector.BPB_RootClus;
  }

  while (running) {
    printf(">");
    if (fgets(bufferRaw, BUF_SIZE, stdin) == NULL) {
      running = false;
      continue;
    }
    bufferRaw[strlen(bufferRaw) - 1] = '\0'; /* cut new line */
    for (unsigned i = 0; i < strlen(bufferRaw) + 1; i++)
      buffer[i] = toupper(bufferRaw[i]);

    if (strncmp(buffer, CMD_INFO, strlen(CMD_INFO)) == 0) {
      printInfo(header);
      if (errno != 0) {
        commandError(CMD_INFO, fooName);
      }
    } else if (strncmp(buffer, CMD_DIR, strlen(CMD_DIR)) == 0) {
      doDir(header, curDirClus);
      if (errno != 0) {
        commandError(CMD_DIR, fooName);
      }
    } else if (strncmp(buffer, CMD_CD, strlen(CMD_CD)) == 0) {
      const uint32_t newClusterNum = doCD(header, curDirClus, buffer);
      if (errno != 0) {
        commandError(CMD_CD, fooName);
      } else {
        curDirClus = newClusterNum;
      }
    } else if (strncmp(buffer, CMD_GET, strlen(CMD_GET)) == 0) {
      doGet(header, curDirClus, buffer);
      if (errno != 0) {
        commandError(CMD_GET, fooName);
      }
    } else if (strncmp(buffer, CMD_PUT, strlen(CMD_PUT)) == 0) {
      doUpload(header, curDirClus, buffer, bufferRaw);
      if (errno != 0) {
        commandError(CMD_PUT, fooName);
      }
      printf("Bonus marks!\n");
    } else
      printf("\nCommand not found\n");
  }
  printf("\nExited...\n");

  cleanupHeader(header);
}
