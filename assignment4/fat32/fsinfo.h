#pragma once
/**
 * @file fsinfo.h
 * @author Justen Di Ruscio
 * @brief Contains declarations pertaining to FSINFO data structure in FAT32 filesystem
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdint.h>

// FSINFO data structure constants
#define FSI_RESERVED1_NUM_BYTES 480
#define FSI_RESERVED2_NUM_BYTES 12

// Serialization structure containing exact FSINFO contents in FAT32 file system
#pragma pack(push)
#pragma pack(1)

typedef struct fat32_fsInfo_struct {
  uint32_t FSI_LeadSig;
  uint8_t FSI_Reserved1[FSI_RESERVED1_NUM_BYTES];
  uint32_t FSI_StrucSig;
  uint32_t FSI_Free_Count;
  uint32_t FSI_Nxt_Free;
  uint8_t FSI_Reserved2[FSI_RESERVED2_NUM_BYTES];
  uint32_t FSI_TrailSig;
} fat32_fsInfo;

#pragma pack(pop)

/**
 * @brief Validates signature of read FSINFO data structure. Sets errno on error of this function
 *
 * @param fsInfo FSINFO data structure to validate signature of
 * @return true signature is valid
 * @return false signature is invalid or error encountered
 */
bool fsInfoSectorSignatureValid(const fat32_fsInfo *const fsInfo);
