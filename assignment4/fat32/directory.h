#pragma once
/**
 * @file directory.h
 * @author Justen Di Ruscio
 * @brief Contains declarations pertaining to FAT32 directory entries
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdint.h>

#include "fat32_header.h"

// Directory entry constant values
#define FREE_DIR_ENTRY_NAME 0xE5
#define LAST_DIR_ENTRY_NAME 0x00

// Attribute value constants
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME                                                         \
  (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK (ATTR_LONG_NAME | ATTR_DIRECTORY | ATTR_ARCHIVE)

// Serialization struct representing exact boot sector
#pragma pack(push)
#pragma pack(1)

typedef struct fat32_directory {
  uint8_t DIR_Name[DIR_NAME_LENGTH];
  uint8_t DIR_Attr;
  uint8_t DIR_NTRes;
  uint8_t DIR_CrtTimeTenth;
  uint16_t DIR_CrtTime;
  uint16_t DIR_CrtDate;
  uint16_t DIR_LstAccDate;
  uint16_t DIR_FstClusHI;
  uint16_t DIR_WrtTime;
  uint16_t DIR_WrtDate;
  uint16_t DIR_FstClusLO;
  uint32_t DIR_FileSize;
} fat32_directory;

#pragma pack(pop)

/**
 * @brief Fills cleanName with a null-terminated, cleaned directory name from rawName. Follows rules specified on page 23 of FAT filesystems document.
 *
 * @param cleanName cleaned version of rawName
 * @param rawName direct DIR_Name from fat32_directory
 */
void dirName(char cleanName[DIR_NAME_LENGTH+1], const char rawName[DIR_NAME_LENGTH]);

/**
 * @brief Reads the directory entry name as provided from the shell buffer through bufferName, and converts it to a FAT32 short name following the rules outlined on page 24 of Microsoft's FAT document
 *
 * @param shortName resultant short name of bufferName
 * @param bufferName pointer to null-terminated arg1, the directory entry name, from the shell's buffer
 */
void toShortDirName(char shortName[DIR_NAME_LENGTH], const char* const bufferName);


/**
 * @brief Fills nextDirectory with the next directory entry in the cluster chain starting at startClusterNum. Should be first called with startClusterNum populated, and then subsequent calls should leave this field NULL to transition to subsequent directory entries. Stores static variables that are reset on first call with startClusterNum; i.e. not thread safe.
 *
 * @param header FAT32 volume header
 * @param nextDirectory location where next directory in chain will copied
 * @param startClusterNum first cluster in chain
 * @return uint32_t When another directory entry was available, and nextDirectory was populated, this is the cluster number where the directory resides. Otherwise, at the end of the cluster chain, 0 will be returned
 */
uint32_t nextDirEntry(const fat32_header *const header,
                  fat32_directory *const nextDirectory,
                  const uint32_t *const startClusterNum);
