#pragma once
/**
 * @file fat.h
 * @author Justen Di Ruscio
 * @brief Contains declarations pertaining specifically to the FAT data structure of FAT32 filesystem
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "fat32_header.h"

#define EMPTY_CLUSTER 0x00000000
#define EOC_CLUSTER 0x0FFFFFFF
#define BAD_CLUSTER 0x0FFFFFF7

/**
 * @brief Validates the signature of the FAT data structure by checking values of 0th and 1st FAT entries. Sets errno on error of this function
 *
 * @return true FAT signature is valid
 * @return false FAT signature is invalid
 */
bool fatSignatureValid();

// sets errno on error & returns -1
// result is 32bit unsigned

/**
 * @brief Returns the value in the FAT entry associated with clusterNum or -1 on error with setting errno.
 *
 * @param header FAT32 header
 * @param clusterNum cluster number to get FAT entry value for
 * @return int64_t value of FAT entry
 */
int64_t fatEntry(const fat32_header *const header, const uint32_t clusterNum);

/**
 * @brief Reads entire cluster located at clusterNum into cluster. Sets errno on error
 *
 * @param header FAT32 header
 * @param clusterNum cluster number to read
 * @param cluster location to place cluster contents. Should point to array of size bs->BPB_BytesPerSec * bs->BPB_SecPerClus, where bs is the bootSector in the header.
 */
void readClusterBytes(const fat32_header *const header,
                             const uint32_t clusterNum, uint8_t *cluster);
