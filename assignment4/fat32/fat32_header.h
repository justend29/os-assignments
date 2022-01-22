#pragma once
/**
 * @file fat32_header.h
 * @author Justen Di Ruscio
 * @brief Contains definition of FAT32 filesystem header
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "boot_sector.h"
#include "fsinfo.h"

#define DIR_NAME_LENGTH 11 // length of directories in directory entry

#pragma pack(push)
#pragma pack(1)

struct fat32_header {
  int fileDes; // not part of fat32, but indicates fd of fat32 disk image
  uint8_t volumeId[DIR_NAME_LENGTH + 1]; // not part of fat32, but indicates volume id
  fat32_bootSector bootSector;
  fat32_fsInfo fsInfo;
};

#pragma pack(pop)

typedef struct fat32_header fat32_header;
