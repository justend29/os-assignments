/**
 * @file fat.c
 * @author Justen Di Ruscio
 * @brief Contains declarations pertaining specifically to the FAT data
 * structure of FAT32 filesystem
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "fat.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "../error/error.h"
#include "../fat32/fat32.h"

#define ENTRY_MASK 0x0FFFFFFF
#define FAT32_OFFSET_SHIFT 2

bool fatSignatureValid(const fat32_header *const header) {
  const char fooName[] = "fatSignatrueValid";

  // Argument Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return false;
  }

  // Verify cluster 0 signature
  int64_t entry = fatEntry(header, 0);
  if (entry == -1) {
    fprintf(stderr, "Failure obtaining contents of FAT entry %i in %s\n", 0,
            fooName);
    return false; // errno set by fatEntry
  }
  uint32_t entry32 = (uint32_t)entry;
  const uint8_t setLow8 = 0xFF;
  const bool low8ArentMedia =
      (entry32 & setLow8) != header->bootSector.BPB_Media;
  const bool high20ArentSet = ((entry32 | setLow8) & ENTRY_MASK) != ENTRY_MASK;
  if (low8ArentMedia || high20ArentSet) {
    return false;
  }

  // Verify cluster 1 signature
  entry = fatEntry(header, 1);
  if (entry == -1) {
    fprintf(stderr, "Failure obtaining contents of FAT entry %i in %s\n", 1,
            fooName);
    return false; // errno set by fatEntry
  }
  const uint32_t setLow20 = 0x3FFFFFF;
  const bool low20NotSet = ((uint32_t)entry & setLow20) != setLow20;
  if (low20NotSet) {
    return false;
  }

  return true;
}

int64_t fatEntry(const fat32_header *const header, const uint32_t clusterNum) {
  const char fooName[] = "fatEntry";

  // Argument Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return -1;
  }

  const fat32_bootSector *const bs = &header->bootSector;

  // Find fat entry for given cluster
  const uint64_t fatOffset = clusterNum << FAT32_OFFSET_SHIFT;
  const uint16_t bytesPerSec = bs->BPB_BytesPerSec;
  const uint64_t fatEntrySecNum =
      bs->BPB_RsvdSecCnt + (fatOffset / bytesPerSec);
  const uint16_t fatEntryOffset = fatOffset % bytesPerSec;

  // seek to sector of fat entry
  seekToSector(header, fatEntrySecNum);
  if (errno != 0) {
    fprintf(
        stderr,
        "Failure seeking to sector %lu associated with cluster number %u in "
        "%s\n",
        fatEntrySecNum, clusterNum, fooName);
    return -1; // errno set by seekToSector
  }

  // read sector
  uint8_t sectorBuff[bytesPerSec];
  const ssize_t bytesRead = read(header->fileDes, sectorBuff, bytesPerSec);
  if (bytesRead == -1) {
    fprintf(stderr, "Failure reading sector number %lu for cluster %i in %s",
            fatEntrySecNum, clusterNum, fooName);
    printf("%d\n", errno);
    return -1; // errno set by read
  }

  // extract cluster contents from sector contents
  const uint32_t *const clusterLocation =
      (uint32_t *)&sectorBuff[fatEntryOffset];
  const uint32_t clusterContents = (*clusterLocation) & ENTRY_MASK;

  return clusterContents;
}

void readClusterBytes(const fat32_header *const header,
                      const uint32_t clusterNum, uint8_t *cluster) {
  const char fooName[] = "readClusterBytes";
  const fat32_bootSector *const bs = &header->bootSector;

  // Seek to start of first sector of curDirClus
  const uint64_t sectorNum = firstSectorNumOfCluster(bs, clusterNum);
  if (errno != 0) {
    fprintf(stderr, "Failure calculating first sector of cluster %u in %s\n",
            clusterNum, fooName);
    return; // errno set by firstSectorNumOfCluster
  }
  seekToSector(header, sectorNum);
  if (errno != 0) {
    fprintf(stderr,
            "Failure seeking to sector %lu in %s to display directory "
            "contents of cluster %u\n",
            sectorNum, fooName, clusterNum);
    return; // errno set by seekToSector
  }

  // Read entire cluster
  const uint32_t bytesPerCluster = bs->BPB_BytesPerSec * bs->BPB_SecPerClus;
  const ssize_t bytesRead = read(header->fileDes, cluster, bytesPerCluster);
  if (bytesRead == -1) {
    fprintf(stderr, "Failure reading cluster %u in %s\n", clusterNum, fooName);
    return; // errno set by read
  }
}
