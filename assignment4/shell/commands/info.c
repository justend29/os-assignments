/**
 * @file info.c
 * @author Justen Di Ruscio
 * @brief Contains function handler definition for INFO command and any other
 * support functions required by handler
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "commands.h"

#include "../../error/error.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define BYTES_IN_MEG 1000000    // conversion ratio of # bytes in 1 megabyte
#define BYTES_IN_GIG 1000000000 // conversion ratio of # bytes in 1 gigabyte

/**
 * @brief Prints the device info section for the FAT32 volume. Sets errno on
 * error
 *
 * @param bs FAT32 header
 */
static void printDeviceInfo(const fat32_bootSector *const bs) {
  const char fooName[] = "printDeviceInfo";

  // Set fields from boot sector values
  const char *const oemName = bs->BS_OEMName;

  // copy max num chars in label
  const unsigned labelLength = 12;
  char label[labelLength];
  strncpy(label, bs->BS_VolLab, labelLength);
  label[labelLength - 1] = '\0';

  // copy max num chars in file system type
  const unsigned fsLength = 6;
  char fsTypeStr[fsLength];
  strncpy(fsTypeStr, bs->BS_FilSysType, fsLength);

  // convert media type number to string with explanation in brackets
  const unsigned maxMediaLength = 17;
  char mediaType[maxMediaLength];
  const uint8_t mediaVal = bs->BPB_Media;
  char *mediaBracket = "fixed";
  if (mediaVal != 0xF8) {
    if (mediaVal == 0xF0) {
      mediaBracket = "removable";
    } else {
      mediaBracket = "unknown";
    }
  }
  int numBytes =
      snprintf(mediaType, maxMediaLength, "0x%x (%s)", mediaVal, mediaBracket);
  if (numBytes < 0) {
    fprintf(stderr, "Failure converting BPB_Media to string in %s\n", fooName);
    errno = EIO;
    return;
  }

  // calculate total volume size
  const uint64_t sizeInt = (uint64_t)bs->BPB_BytesPerSec * bs->BPB_TotSec32;
  const uint32_t megaBytes = (uint32_t)((double)sizeInt / BYTES_IN_MEG);
  const float gigaBytes = (double)sizeInt / BYTES_IN_GIG;

  // convert drive number to string and put explanation in brackets
  const unsigned driveLength = 18;
  const uint8_t driveInt = bs->BS_DrvNum;
  char *driveBracket = "";
  if (driveInt == 0x80) {
    driveBracket = "hard disk";
  } else if (driveInt == 0x00) {
    driveBracket = "floppy disk";
  }
  char driveNum[driveLength];
  numBytes = snprintf(driveNum, driveLength, "%u (%s)", driveInt, driveBracket);
  if (numBytes < 0) {
    fprintf(stderr, "Failure converting BS_DrvNum to string in %s\n", fooName);
    errno = EIO;
    return;
  }
  driveNum[numBytes] = '\0';

  // Print Results
  printf("---- Device Info ----\n"
         "OEM Name: %s\n"
         "Label: %s\n"
         "File System Type: %s\n"
         "Media Type: %s\n"
         "Size: %zu bytes (%i MiB, %5.3f GiB)\n"
         "Drive Number: %s\n",
         oemName, label, fsTypeStr, mediaType, sizeInt, megaBytes, gigaBytes,
         driveNum);
}

/**
 * @brief Prints the file system geometry section for the FAT32 volume
 *
 * @param bs FAT32 boot sector; part of FAT32 header
 */
static void printGeometry(const fat32_bootSector *const bs) {
  // Print boot sector fields
  printf("\n--- Geometry ---\n"
         "Bytes per Sector: %u\n"
         "Sectors Per Cluster: %u\n"
         "Total Sectors: %u\n"
         "Geom: Sectors per Track %u\n"
         "Geom: Heads: %u\n"
         "Hidden Sectors: %u\n",
         bs->BPB_BytesPerSec, bs->BPB_SecPerClus, bs->BPB_TotSec32,
         bs->BPB_SecPerTrk, bs->BPB_NumHeads, bs->BPB_HiddSec);
}

/**
 * @brief Prints generic file system info for the FAT32 volume
 *
 * @param header FAT32 header
 */
static void printFilesystemInfo(const fat32_header *const header) {
  const fat32_bootSector *const bs = &header->bootSector;
  const int mirror = bs->BPB_ExtFlags & 0x01;
  printf("\n--- FS Info ---\n"
         "Volume ID: %s\n"
         "Version: %u.%u\n"
         "Reserved Sectors: %u\n"
         "Number of FATs: %u\n"
         "FAT Size: %u\n"
         "Mirrored FAT: %u (%s)\n"
         "Boot Sector Backup Sector No: %u\n",
         header->volumeId, bs->BPB_FSVerHigh, bs->BPB_FSVerLow,
         bs->BPB_RsvdSecCnt, bs->BPB_NumFATs, bs->BPB_FATSz32, mirror,
         mirror ? "no" : "yes", bs->BPB_BkBootSec);
}

// ======================== Public Functions ==================

void printInfo(const fat32_header *const header) {
  const char fooName[] = "printInfo";

  // Arg Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return; // errno set by argValidityCheck
  }

  // Print Info
  printDeviceInfo(&header->bootSector);
  if (errno != 0) {
    fprintf(stderr, "Failed printing device information in %s\n", fooName);
    return; // errno set by printDeviceInfo
  }
  printGeometry(&header->bootSector);
  printFilesystemInfo(header);
}
