#include <stdio.h>
#include <string.h>

#include "bg3d.c"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    perror("Error: Provide a BG3D File");
    return 1;
  }

  char * filename = argv[1];

  FILE * pFile;
  pFile = fopen(filename, "r");

  if (pFile == NULL) {
    perror("Error Opening File.\n");
    return 1;
  }

  readHeader(pFile);
  parseFile(pFile);

  fclose(pFile);
  
  return 0;
}
