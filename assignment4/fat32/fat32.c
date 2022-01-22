/**
 * @file fat32.c
 * @author Justen Di Ruscio
 * @brief Contains definitions relating to FAT32 filesystem as an entity, and
 * all definitions for functions relating to FAT32 components
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */
#define _FILE_OFFSET_BITS 64

#include "fat32.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../error/error.h"
#include "directory.h"
#include "fat.h"

#define FIRST_DATA_CLUSTER_NUM 2

/**
 * @brief Set the Header Volume Id of the provided header object. Sets errno on
 * error of this function and if no VOLUME_ID file is found in the root
 * directory
 *
 * @param header FAT32 header
 */
static void setHeaderVolumeId(fat32_header *const header) {
  const char fooName[] = "setVolumeId";

  // Argument Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return;
  }

  const fat32_bootSector *const bs = &header->bootSector;
  const uint32_t rootCluster = bs->BPB_RootClus;

  // Read first directory entry
  fat32_directory dir;
  bool dirFound = nextDirEntry(header, &dir, &rootCluster);
  if (errno != 0) {
    fprintf(stderr,
            "Failure reading first directory entry of root cluster %u in %s\n",
            rootCluster, fooName);
    return;
  }

  while (dirFound) {
    // Set volume ID if directory entry is volume ID file
    if (dir.DIR_Attr & ATTR_VOLUME_ID) {
      dirName((char *)header->volumeId, (char *)dir.DIR_Name);
      return;
    }

    // Read next directory entry
    dirFound = nextDirEntry(header, &dir, NULL);
    if (errno != 0) {
      fprintf(stderr, "Failure reading next directory entry in %s\n", fooName);
      return;
    }
  }

  fprintf(stderr, "Failure in %s: unable to find file specifying VOLUME_ID\n",
          fooName);
  errno = ENOENT;
}

// ==================== Public Functions ====================

fat32_header *readHeader(const int fd) {
  const char fooName[] = "readHeader";

  // Allocate memory for header
  const char *const headerBytes = malloc(sizeof(fat32_header));
  if (headerBytes == NULL) {
    fprintf(stderr, "Unable to allocate memory for fat32_header in %s\n",
            fooName);
    return NULL; // errno set by malloc
  }
  fat32_header *const header = (fat32_header *)headerBytes;

  // Store file descriptor in header
  header->fileDes = fd;

  // Read boot sector from disk image sector 0
  fat32_bootSector *const bootSector = &header->bootSector;
  const size_t bsSize = sizeof(header->bootSector);
  ssize_t bytesRead = read(fd, bootSector, bsSize);
  if (bytesRead == -1) {
    fprintf(stderr,
            "Failure reading boot sector from disk image into header in  %s",
            fooName);
    free(header);
    return NULL; // errno set by read
  }

  // Perform validation checks on read boot sector
  bool sigValid = bootSectorSignatureValid(bootSector);
  if (errno != 0) {
    fprintf(stderr, "Failure while validating boot sector signature in %s\n",
            fooName);
    free(header);
    return NULL; // errno set by bootSectorSignatureValid
  }
  if (!sigValid) {
    fprintf(stderr, "Unable to verify boot sector signature in %s\n", fooName);
    errno = ENOMEDIUM;
    free(header);
    return NULL;
  }

  const bool isFat32 = isFat32Volume(header);
  if (errno != 0) {
    fprintf(stderr, "Failure while validating volume is FAT32 in %s\n",
            fooName);
    free(header);
    return NULL; // errno set by isFat32Volume
  }
  if (!isFat32) {
    fprintf(stderr, "Read sector in %s is not part of a FAT32 volume\n",
            fooName);
    errno = EMEDIUMTYPE;
    free(header);
    return NULL;
  }

  // Read FSInfo sector from disk
  const uint32_t fsInfoSectorNum = bootSector->BPB_FSInfo;
  seekToSector(header, fsInfoSectorNum);
  if (errno != 0) {
    fprintf(stderr, "Failure seeking to FSInfo sector on disk image in %s\n",
            fooName);
    free(header);
    return NULL;
  }
  fat32_fsInfo *const fsInfoSector = &header->fsInfo;
  const size_t fsInfoSize = sizeof(header->fsInfo);
  bytesRead = read(fd, fsInfoSector, fsInfoSize);
  if (bytesRead == -1) {
    fprintf(stderr,
            "Failure reading FSInfo sector from disk image into header in %s\n",
            fooName);
    free(header);
    return NULL; // errno set by read;
  }

  // Perform validation check on read fsinfo sector
  sigValid = fsInfoSectorSignatureValid(fsInfoSector);
  if (errno != 0) {
    fprintf(stderr, "Failure while validating FSINFO signature in %s\n",
            fooName);
    free(header);
    return NULL; // errno set by fsInfoSectorSignatureValid
  }
  if (!sigValid) {
    fprintf(stderr, "Read FSInfo sector has invalid signature %s\n", fooName);
    errno = EMEDIUMTYPE;
    free(header);
    return NULL;
  }

  // Perform validation on FAT
  sigValid = fatSignatureValid(header);
  if (errno != 0) {
    fprintf(stderr, "Failure while validating FAT signature in %s\n", fooName);
    free(header);
    return NULL; // errno set by fatSignatureValid
  }
  if (!sigValid) {
    fprintf(stderr, "FAT signature is invalid in %s\n", fooName);
    free(header);
    return NULL;
  }

  // Set volume id from file in root directory
  setHeaderVolumeId(header);
  if (errno != 0) {
    fprintf(stderr, "Failure setting header volume ID in %s\n", fooName);
    return NULL;
  }

  // Calculate and store free space
  printf("Calculating free space...\n");
  uint32_t numFree = numFreeClusters(header);
  if (errno != 0) {
    fprintf(stderr, "Failure calculating free space in %s\n", fooName);
    return NULL;
  }
  fsInfoSector->FSI_Free_Count = numFree;

  return header;
}

void cleanupHeader(fat32_header *const header) { free(header); }

bool isFat32Volume(const fat32_header *const header) {
  const char fooName[] = "isFat32Volume";

  // Argument Validity
  errno = 0;
  if (header == NULL) {
    fprintf(stderr, "argument 'header' of %s must point to the fat header\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Determine FAT Type
  const fat32_bootSector *const bs = &header->bootSector;
  // signature matching value in doc indicates optional fat32 fields present
  if (bs->BS_BootSig != 0x29) {
    fprintf(stderr, "This program requires the FAT32 volume to contain the "
                    "optional three FAT32 fields\n");
    return false;
  }

  // count of sectors occupied by root directory
  const uint32_t rootDirSectors =
      ((bs->BPB_RootEntCnt * 32) + (bs->BPB_BytesPerSec - 1)) /
      bs->BPB_BytesPerSec;
  if (rootDirSectors != 0) {
    return false; // immediately false if not 0
  }

  // count of sectors in data region of volume
  const uint16_t fatSize =
      bs->BPB_FATSz16 != 0 ? bs->BPB_FATSz16 : bs->BPB_FATSz32;
  const uint16_t totSecs =
      bs->BPB_TotSec16 != 0 ? bs->BPB_TotSec16 : bs->BPB_TotSec32;
  const uint32_t numDataSecs =
      totSecs -
      (bs->BPB_RsvdSecCnt + (bs->BPB_NumFATs * fatSize) + rootDirSectors);

  // count of clusters
  const uint32_t numClusters = numDataSecs / bs->BPB_SecPerClus;

  // resolve FAT type
  if (numClusters < 4085) {
    // volume is FAT12
    return false;
  } else if (numClusters < 65525) {
    // volume is FAT16
    return false;
  } else {
    // volume is FAT32
    return true;
  }
}

bool bootSectorSignatureValid(const fat32_bootSector *const bs) {
  const char fooName[] = "bootSectorSignatureValid";

  // Arg Validity
  argValidityCheck(bs, "bs", fooName);
  if (errno != 0) {
    return false; // errno set by argValidityCheck
  }

  // Check Signatures against values specified in doc
  return bs->BS_SigA == 0x55 && bs->BS_SigB == 0xAA;
}

bool fsInfoSectorSignatureValid(const fat32_fsInfo *const fsInfo) {
  const char fooName[] = "fsInfoSectorSignatureValid";

  // Arg Validity
  argValidityCheck(fsInfo, "fsInfo", fooName);
  if (errno != 0) {
    return false; // errno set by argValidityCheck
  }

  // Check Signatures against values specified in doc
  return fsInfo->FSI_LeadSig == 0x41615252 &&
         fsInfo->FSI_StrucSig == 0x61417272 &&
         fsInfo->FSI_TrailSig == 0xAA550000;
}

uint64_t seekToSector(const fat32_header *const header,
                      const uint64_t sectorNum) {
  const char fooName[] = "seekToSector";

  // Arg Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return 0; // errno set by argValidityCheck
  }

  // Seek byte
  const fat32_bootSector *const bs = &header->bootSector;
  const off_t byteOffset =
      lseek(header->fileDes, bs->BPB_BytesPerSec * sectorNum, SEEK_SET);
  if (byteOffset == -1) {
    fprintf(stderr, "Failure seeking to sector number %lu in %s\n", sectorNum,
            fooName);
  }

  return byteOffset;
}

uint64_t firstSectorNumOfCluster(const fat32_bootSector *const bs,
                                 const uint32_t clusterNum) {
  const char fooName[] = "firstSectorNumOfCluster";

  // Arg Validity
  argValidityCheck(bs, "bs", fooName);
  if (errno != 0) {
    return false; // errno set by argValidityCheck
  }

  const uint64_t firstDataSector =
      bs->BPB_RsvdSecCnt + (bs->BPB_NumFATs * bs->BPB_FATSz32);
  return (clusterNum - 2) * bs->BPB_SecPerClus + firstDataSector;
}

void dirName(char cleanName[DIR_NAME_LENGTH + 1],
             const char rawName[DIR_NAME_LENGTH]) {
  const int8_t readingMain = 0;
  const int8_t hitSpaces = 1;
  const int8_t foundExtension = 2;
  int8_t state = readingMain;
  uint8_t cleanLocation = 0;

  // Fill clean name using state machine reading rawName
  for (uint8_t i = 0; i < DIR_NAME_LENGTH; ++i) {
    if (state == readingMain && i == 7) {
      state = foundExtension;
    }
    if (state == readingMain) {
      if (rawName[i] == ' ') {
        state = hitSpaces;
      } else {
        cleanName[cleanLocation++] = rawName[i];
        state = readingMain;
      }
    } else if (state == hitSpaces) {
      if (rawName[i] != ' ') {
        cleanName[cleanLocation++] = '.';
        cleanName[cleanLocation++] = rawName[i];
        state = foundExtension;
      } else {
        state = hitSpaces;
      }
    } else if (state == foundExtension) {
      if (rawName[i] != ' ') {
        cleanName[cleanLocation++] = rawName[i];
        state = foundExtension;
      } else {
        break;
      }
    }
  }

  // Terminate clean name
  cleanName[cleanLocation] = '\0';
}

uint32_t nextDirEntry(const fat32_header *const header,
                      fat32_directory *const nextDirectory,
                      const uint32_t *const startClusterNum) {
  const char fooName[] = "nextDirEntry";
  static uint32_t dirNum = 0;
  static uint32_t clusterNum = 0;
  if (startClusterNum != NULL) {
    clusterNum = *startClusterNum;
    dirNum = 0;
  }

  // Arg Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return 0;
  }
  if (clusterNum < FIRST_DATA_CLUSTER_NUM) {
    fprintf(stderr, "Must call %s with 'startClusterNum' value >= %u\n",
            fooName, FIRST_DATA_CLUSTER_NUM);
    errno = EPERM;
    return 0;
  }

  // List current directory contents spread across all pertinent clusters
  const fat32_bootSector *const bs = &header->bootSector;
  const uint32_t bytesPerCluster = bs->BPB_BytesPerSec * bs->BPB_SecPerClus;

  uint32_t nextCluster = fatEntry(header, clusterNum);
  if (errno != 0) {
    fprintf(stderr, "Failure following cluster chain in %s\n", fooName);
    return 0;
  }

  while (clusterNum != EOC_CLUSTER) {
    if (clusterNum != BAD_CLUSTER) {
      // Read entire cluster contents
      uint8_t cluster[bytesPerCluster];
      readClusterBytes(header, clusterNum, cluster);
      if (errno != 0) {
        fprintf(stderr, "Failure reading cluster %u contents in %s\n",
                clusterNum, fooName);
        return 0;
      }

      // Iterate over directory entries until a valid one is found
      // Once one is found, set nextDirectory to it and store
      // nextDirectory for following call
      const uint32_t dirsPerCluster = bytesPerCluster / sizeof(fat32_directory);
      fat32_directory *dir = (fat32_directory *)cluster + dirNum;
      for (; dirNum < dirsPerCluster; ++dirNum) {
        if (dir->DIR_Name[0] == LAST_DIR_ENTRY_NAME) {
          break;
        } else if ((dir->DIR_Name[0] != FREE_DIR_ENTRY_NAME) &&
                   (dir->DIR_Attr != ATTR_LONG_NAME) &&
                   (dir->DIR_Attr != ATTR_LONG_NAME_MASK)) {
          *nextDirectory = *dir;
          ++dirNum;
          return clusterNum;
        }
        ++dir;
      }
      dirNum = 0;
    }
    clusterNum = nextCluster;
    nextCluster = fatEntry(header, clusterNum);
    if (errno != 0) {
      fprintf(stderr, "Failure following cluster chain in %s\n", fooName);
      return 0;
    }
  }
  return 0;
}

uint32_t numFreeClusters(const fat32_header *const header) {
  const char fooName[] = "numFreeClusters";
  uint32_t clusterNum = FIRST_DATA_CLUSTER_NUM;

  // Arg Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return 0;
  }

  // Iterate FAT entries of data clusters and accumulate free entries
  uint32_t numFree = 0;
  const fat32_bootSector *const bs = &header->bootSector;
  const uint32_t totNumClusters =
      ((uint64_t)bs->BPB_TotSec32 - (bs->BPB_NumFATs + bs->BPB_FATSz32)) /
          bs->BPB_SecPerClus +
      FIRST_DATA_CLUSTER_NUM;

  uint32_t nextCluster = fatEntry(header, clusterNum);
  if (errno != 0) {
    fprintf(stderr, "Failure following cluster chain in %s\n", fooName);
    return 0;
  }

  while (clusterNum < totNumClusters) {
    numFree += (uint32_t)(nextCluster == EMPTY_CLUSTER);
    ++clusterNum;
    nextCluster = fatEntry(header, clusterNum);
    if (errno != 0) {
      fprintf(stderr, "Failure reading FAT entry for cluster %u in %s\n",
              clusterNum, fooName);
      return 0;
    }
  }
  return numFree;
}

void toShortDirName(char shortName[DIR_NAME_LENGTH],
                    const char *const bufferName) {
  const char fooName[] = "toShortDirName";

  // Arg Validity
  argValidityCheck(bufferName, "bufferName", fooName);

  unsigned long length = strlen(bufferName);
  char longName[length];
  strcpy(longName, bufferName);

  if (length > DIR_NAME_LENGTH) {
    longName[DIR_NAME_LENGTH - 1] = '~';
  }

  for (unsigned i = 0, shortLoc = 0; i < DIR_NAME_LENGTH; ++i) {
    const char longChar = longName[i];
    if (longChar == '.') {
      if (i < 8) {
        shortName[shortLoc++] = ' ';
      }
    }
  }
}
