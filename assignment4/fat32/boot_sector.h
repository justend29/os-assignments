#pragma once
/**
 * @file boot_sector.h
 * @author Justen Di Ruscio
 * @brief Contains declarations related to the boot sector of FAT32 file system
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <inttypes.h>

/* boot sector constants */
#define BS_OEMName_LENGTH 8
#define BS_VolLab_LENGTH 11
#define BS_FilSysType_LENGTH 8

// Serialization struct representing exact boot sector
#pragma pack(push)
#pragma pack(1)

struct fat32BS_struct {
	char BS_jmpBoot[3];
	char BS_OEMName[BS_OEMName_LENGTH];
	uint16_t BPB_BytesPerSec;
	uint8_t BPB_SecPerClus;
	uint16_t BPB_RsvdSecCnt;
	uint8_t BPB_NumFATs;
	uint16_t BPB_RootEntCnt;
	uint16_t BPB_TotSec16;
	uint8_t BPB_Media;
	uint16_t BPB_FATSz16;
	uint16_t BPB_SecPerTrk;
	uint16_t BPB_NumHeads;
	uint32_t BPB_HiddSec;
	uint32_t BPB_TotSec32;
	uint32_t BPB_FATSz32;
	uint16_t BPB_ExtFlags;
	uint8_t BPB_FSVerLow;
	uint8_t BPB_FSVerHigh;
	uint32_t BPB_RootClus;
	uint16_t BPB_FSInfo;
	uint16_t BPB_BkBootSec;
	char BPB_reserved[12];
	uint8_t BS_DrvNum;
	uint8_t BS_Reserved1;
	uint8_t BS_BootSig;
	uint32_t BS_VolID;
	char BS_VolLab[BS_VolLab_LENGTH];
	char BS_FilSysType[BS_FilSysType_LENGTH];
	char BS_CodeReserved[420];
	uint8_t BS_SigA;
	uint8_t BS_SigB;
};

#pragma pack(pop)

typedef struct fat32BS_struct fat32_bootSector;



// checks signature of sector 0
// sets errno on error
/**
 * @brief Validates the signature of the provided boot sector. Sets errno on error of function operation
 *
 * @param bs boot sector to check signature of
 * @return true signature is valid
 * @return false signature is invalid
 */
bool bootSectorSignatureValid(const fat32_bootSector* const bs);
