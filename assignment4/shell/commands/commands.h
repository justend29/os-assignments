#pragma once
/**
 * @file commands.h
 * @author Justen Di Ruscio
 * @brief Contained in this file are the function declarations for handlers of shell commands. Additionally, functions necessary to service the shell commands with these handlers are included.
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../fat32/fat32.h"

/**
 * @brief Prints volume info for FAT32 file system
 *
 * @param header FAT32 header
 */
void printInfo(const fat32_header* const header);
void doDir(const fat32_header *const header, const uint32_t curDirClus);

/**
 * @brief Returns first cluster of directory specified as arg1 in buffer. Sets errno on error and returns 0
 *
 * @param header FAT32 header
 * @param curDirClus cluster number of current location; where specified directory is located
 * @param buffer command line of shell that includes desired directory as arg1
 * @return uint32_t the cluster number of specified directory or 0 on error
 */
uint32_t doCD(const fat32_header *const header, const uint32_t curDirClus,
              const char *const buffer);

/**
 * @brief Downloads the file specified in arg1 of buffer to the CWD of the program. Sets errno on error
 *
 * @param header FAT32 header
 * @param curDirClus first cluster of directory where specified file resides
 * @param buffer command line of shell that includes desired file as arg1
 */
void doGet(const fat32_header *const header, const uint32_t curDirClus,
                const char *const buffer);

/**
 * @brief Uploads file specified in arg1 of bufferRaw to the fat32 diskimage.
 *
 * @param header FAT32 header
 * @param curDirClus cluster number of first cluster of current directory in FAT32 filesystem. Location where file will be uploaded
 * @param buffer shell command line in capital letters only. Will be used to set uploaded file name
 * @param bufferRaw direct shell command line as provided by user. Will be used to read file to upload
 */
void doUpload(const fat32_header* const header, const uint32_t curDirClus, const char* const buffer, const char* const bufferRaw);

/**
 * @brief Returns arg1 of command line from shell. Sets errno on error
 *
 * @param buffer command line of shell
 * @return char* arg1
 */
char *getArg1(const char *const buffer);
