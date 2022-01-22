/**
 * @file get.c
 * @author Justen Di Ruscio
 * @brief Contains function handler definition for GET command and any other
 * functions required by handler
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
#include <unistd.h>

#include "../../error/error.h"
#include "../../fat32/directory.h"
#include "../../fat32/fat.h"

#define CWD_PATH_MAX_LEN 150

/**
 * @brief Returns min of arguments
 *
 * @param a first argument to compare
 * @param b second argument to compare
 * @return uint32_t either a or b, whichever is smaller
 */
static uint32_t min(const uint32_t a, const uint32_t b) {
  if (a <= b) {
    return a;
  }
  return b;
}

/**
 * @brief Follows cluster chain that specifies a file, starting at
 * startClusterNum, writing contents to a file in the program's CWD with the
 * name fileName. Sets errno on error
 *
 * @param header FAT32 header
 * @param fileName name of file to download cluster contents to
 * @param fileSize total file size of cluster chain in bytes
 * @param startClusterNum first cluster number of cluster chain
 */
static void downloadFile(const fat32_header *const header,
                         const char fileName[DIR_NAME_LENGTH + 1],
                         uint32_t fileSize, const uint32_t startClusterNum) {
  const char fooName[] = "downloadFile";
  uint32_t clusterNum = startClusterNum;
  // Arg Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return;
  }

  // Create path for
  FILE *destFile = fopen(fileName, "w");
  if (destFile == NULL) {
    fprintf(stderr, "Failed opening file %s in %s to download file into\n",
            fileName, fooName);
    return; // errno set by fopen
  }

  // Follow cluster chain starting at startClusterNum and write contents to file
  const fat32_bootSector *const bs = &header->bootSector;
  const uint32_t bytesPerCluster = bs->BPB_BytesPerSec * bs->BPB_SecPerClus;

  uint32_t nextCluster = fatEntry(header, clusterNum);
  if (errno != 0) {
    fprintf(stderr, "Failure following cluster chain in %s\n", fooName);
    fclose(destFile);
    return;
  }

  while (clusterNum != EOC_CLUSTER) {
    if (clusterNum != BAD_CLUSTER) {
      // Read entire cluster contents
      uint8_t cluster[bytesPerCluster];
      readClusterBytes(header, clusterNum, cluster);
      if (errno != 0) {
        fprintf(stderr, "Failure reading cluster %u contents in %s\n",
                clusterNum, fooName);
        fclose(destFile);
        return;
      }

      // Write cluster contents to file
      const uint32_t bytesToWrite = min(fileSize, bytesPerCluster);
      const size_t bytesWritten =
          fwrite(cluster, sizeof(uint8_t), bytesToWrite, destFile);
      if (bytesWritten != bytesToWrite) {
        fprintf(stderr,
                "Failure writing bytes in cluster %u to file %s in %s\n",
                clusterNum, fileName, fooName);
        fclose(destFile);
        return;
      }
      fileSize -= bytesWritten;
    }
    clusterNum = nextCluster;
    nextCluster = fatEntry(header, clusterNum);
    if (errno != 0) {
      fprintf(stderr, "Failure following cluster chain in %s\n", fooName);
      fclose(destFile);
      return;
    }
  }
  fclose(destFile);
}

// ================= Public Functions ====================

void doGet(const fat32_header *const header, const uint32_t curDirClus,
           const char *const buffer) {
  const char fooName[] = "doGet";

  // Arg Valididy
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return;
  }
  argValidityCheck(buffer, "buffer", fooName);
  if (errno != 0) {
    return;
  }

  const char *const fileName = getArg1(buffer);
  if (errno != 0) {
    return;
  }

  // Read first directory entry
  fat32_directory dir;
  uint32_t dirClusterNum = nextDirEntry(header, &dir, &curDirClus);
  if (errno != 0) {
    fprintf(stderr,
            "Failure reading first directory entry of cluster %u in %s\n",
            curDirClus, fooName);
    return;
  }

  // Find file with specified name and download its contents
  while (dirClusterNum) {
    char currentDirName[DIR_NAME_LENGTH + 1];
    dirName(currentDirName, (char *)dir.DIR_Name);
    if (strcmp(currentDirName, fileName) == 0) { // found file
      if (dir.DIR_Attr & ATTR_DIRECTORY) {
        fprintf(stderr, "%s is a directory\n", fileName);
        errno = EISDIR;
        return;
      } else if (dir.DIR_Attr & ATTR_VOLUME_ID) {
        fprintf(stderr, "%s is not a downloadable file\n", fileName);
        errno = ENOENT;
        return;
      } else {
        const uint32_t contentClusterNum =
            dir.DIR_FstClusHI << 16 | (uint32_t)dir.DIR_FstClusLO;
        downloadFile(header, currentDirName, dir.DIR_FileSize,
                     contentClusterNum);
        printf("Done.\n");
        return; // errno set by downloadFile
      }
    }

    // Read next directory entry
    dirClusterNum = nextDirEntry(header, &dir, NULL);
    if (errno != 0) {
      fprintf(stderr, "Failure reading next directory entry in %s\n", fooName);
      return;
    }
  }

  fprintf(stderr, "%s does not exist\n", fileName);
  errno = ENOENT;
  return;
}
