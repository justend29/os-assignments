#include "commands.h"

#include <errno.h>
#include <stdio.h>

#include "../../error/error.h"

void doUpload(const fat32_header *const header, const uint32_t curDirClus,
              const char *const buffer, const char *const bufferRaw) {
  const char fooName[] = "doUpload";

  // Argument validity checks
  argValidityCheck(header, "header", fooName);
  if (errno != 0) {
    return;
  }
  argValidityCheck(buffer, "buffer", fooName);
  if (errno != 0) {
    return;
  }
  argValidityCheck(bufferRaw, "bufferRaw", fooName);
  if (errno != 0) {
    return;
  }

  // Open file to upload in directory of program execution
  const char *const localFileName = getArg1(bufferRaw);
  if (errno != 0) {
    fprintf(stderr, "Failure reading file name from shell command line in %s\n",
            fooName);
    return; // errno set by getArg1
  }

  FILE *localFile = fopen(localFileName, "r");
  if (localFile == NULL) {
    fprintf(stderr, "Failure opening file %s to upload in %s\n", );
    return; // errno set by fopen
  }
}
