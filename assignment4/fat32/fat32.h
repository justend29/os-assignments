#pragma once
/**
 * @file fat32.h
 * @author Justen Di Ruscio
 * @brief Contains declarations relating to FAT32 filesystem as an entity
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <inttypes.h>
#include <stdbool.h>

#include "boot_sector.h"
#include "fat32_header.h"
#include "fsinfo.h"

/**
 * @brief Reads FAT32 header from disk image located at file descriptor, fd. Allocates on heap; the returned pointer should be cleaned by cleanupHeader. Sets errno on failure and returns NULL
 *
 * @param fd file descriptor of opened disk image file
 * @return fat32_header* parsed FAT32 header
 */
fat32_header *readHeader(const int fd);

/**
 * @brief Cleans header returned from readHeader
 *
 * @param header FAT32 header to clean
 */
void cleanupHeader(fat32_header *const header);

/**
 * @brief Determines if disk image type is FAT32 by following steps on page 15 of FAT document; uses count of clusters measurement. Sets errno on error of this function
 *
 * @param header FAT32 header
 * @return true volume is FAT32 type
 * @return false volume is not FAT32 type; may be FAT12, FAT16, or neither
 */
bool isFat32Volume(const fat32_header *const header);

/**
 * @brief Returns byte offset from start of file where sector of sectorNum is located. Sets errno on error of this function
 *
 * @param header FAT32 header
 * @param sectorNum sector number to resolve byte offset in file for
 * @return uint64_t byte offset from start of file where sector of sectorNum starts
 */
uint64_t seekToSector(const fat32_header* const header, const uint64_t sectorNum);

/**
 * @brief Returns the sector number of the first sector in the cluster clusterNum. Sets errno on error of this function.
 *
 * @param bs FAT32 boot sector; part of the FAT32 header
 * @param clusterNum cluster number to get sector number for
 * @return uint64_t sector number of the first sector in cluster clusterNum
 */
uint64_t firstSectorNumOfCluster(const fat32_bootSector *const bs,
                                 const uint32_t clusterNum);

/**
 * @brief Iterates over FAT data structure and calculates the number of free clusters. Sets errno on error of this function and returns 0
 *
 * @param header FAT32 header
 * @return uint32_t number of free clusters on the volume
 */
uint32_t numFreeClusters(const fat32_header *const header);
