#include "commands.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "../../error/error.h"
#include "../../fat32/directory.h"
#include "../../fat32/fat.h"
#include "../../fat32/fat32.h"

void doDir(const fat32_header *const header, const uint32_t curDirClus) {
  const char fooName[] = "doDir";

  // Arg Validity
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return;
  }

  // List current directory contents spread across all pertinent clusters
  printf("\nDIRECTORY LISTING\nVOL_ID: %s\n\n", header->volumeId);

  // Read first directory entry
  fat32_directory dir;
  bool dirFound = nextDirEntry(header, &dir, &curDirClus);
  if (errno != 0) {
    fprintf(stderr,
            "Failure reading first directory entry of cluster %u in %s\n",
            curDirClus, fooName);
    return;
  }

  while (dirFound) {
    if (dir.DIR_Attr != ATTR_VOLUME_ID) {
      // Form dir name
      char dirEntryName[DIR_NAME_LENGTH + 1];
      dirName(dirEntryName, (char *)dir.DIR_Name);
      const bool isDir = dir.DIR_Attr == ATTR_DIRECTORY;

      // Print directory entry
      printf("%s%s%s\t%u\n", isDir ? "<" : "", dirEntryName, isDir ? ">" : "",
             dir.DIR_FileSize);
    }

    // Read next directory entry
    dirFound = nextDirEntry(header, &dir, NULL);
    if (errno != 0) {
      fprintf(stderr, "Failure reading next directory entry in %s\n", fooName);
      return;
    }
  }

  // Print out footer
  const uint64_t bytesPerCluster =
      header->bootSector.BPB_BytesPerSec * header->bootSector.BPB_SecPerClus;
  printf("--Bytes Free: %lu\n--DONE\n",
         bytesPerCluster * header->fsInfo.FSI_Free_Count);
}
