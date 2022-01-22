/**
 * @file cd.c
 * @author Justen Di Ruscio
 * @brief Contains function handler definition for CD command
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "commands.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "../../error/error.h"
#include "../../fat32/directory.h"
#include "../../fat32/fat.h"

uint32_t doCD(const fat32_header *const header, const uint32_t curDirClus,
              const char *const buffer) {
  const char fooName[] = "doCD";

  // Argument Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return 0;
  }

  const char *const directoryName = getArg1(buffer);
  if (errno != 0) {
    return 0;
  }

  // Read first directory entry
  fat32_directory dir;
  uint32_t dirClusterNum = nextDirEntry(header, &dir, &curDirClus);
  if (errno != 0) {
    fprintf(stderr,
            "Failure reading first directory entry of cluster %u in %s\n",
            curDirClus, fooName);
    return 0;
  }

  // Find directory with specified name and return cluster of its contents
  while (dirClusterNum) {
    char currentDirName[DIR_NAME_LENGTH + 1];
    dirName(currentDirName, (char *)dir.DIR_Name);
    if (strcmp(currentDirName, directoryName) == 0) { // found dir
      if (dir.DIR_Attr & ATTR_DIRECTORY) {
        uint32_t contentClusterNum =
            dir.DIR_FstClusHI << 16 | (uint32_t)dir.DIR_FstClusLO;
        if (contentClusterNum == 0) { // cd'd to root dir
          contentClusterNum = header->bootSector.BPB_RootClus;
        }
        return contentClusterNum;
      } else {
        fprintf(stderr, "%s is not a directory\n", directoryName);
        errno = ENOTDIR;
        return 0;
      }
    }

    // Read next directory entry
    dirClusterNum = nextDirEntry(header, &dir, NULL);
    if (errno != 0) {
      fprintf(stderr, "Failure reading next directory entry in %s\n", fooName);
      return 0;
    }
  }

  fprintf(stderr, "%s does not exist\n", directoryName);
  errno = ENOENT;
  return 0;
}
