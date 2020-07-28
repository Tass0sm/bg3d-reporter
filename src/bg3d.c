#include <stdbool.h>
#include <stdlib.h>
#include <endian.h>

#include "bg3d.h"

void dump (const void * addr, const size_t len) {
  if (len == 0) {
    printf("ZERO LENGTH\n");
    return;
  }

  for (size_t i = 0; i < len; i++) {
    unsigned char c = *((unsigned char *) addr + i);

    for (int bit = 7; bit >= 0; bit--) {
      unsigned char n = c >> bit;

      if (n & 1) {
	printf("1");
      } else {
	printf("0");
      }
    }
    printf("\n");
  }
}

void readHeader (FILE * pFile) {
  BG3DHeaderType header;
  size_t result;

  result = fread(header.headerString, 1, 16, pFile);

  if (result < 16) {
    perror("Error Reading BG3D Header String.\n");
    return;
  }

  result = fread(&(header.version), 1, 4, pFile);

  if (result < 4) {
    perror("Error Reading BG3D Header Version.\n");
    return;
  }
  			
  if ((header.headerString[0] != 'B') || (header.headerString[1] != 'G') ||
      (header.headerString[2] != '3') || (header.headerString[3] != 'D')) {
    perror("Error: BG3D file has invalid header.\n");
    return;
  }


  printf("Header: %s\n", header.headerString);

  long pos = ftell(pFile) - result;
  printf("%8lx: %u (version)\n", pos, header.version);
}

void parseFile (FILE * pFile) {
  uint32_t tag;
  size_t count;
  bool done = false;
  BG3DMeshHeader * newObjHeader;

  do {
    count = sizeof(tag);
    size_t result = fread(&tag, 1, count, pFile);

    if (result < count) {
      perror("Error Reading a BG3D Tag.\n");
      return;
    }

    tag = htobe32(tag);

    long pos = ftell(pFile) - result;
    printf("%8lx: %u (tag)\n", pos, tag);

    switch (tag) {
    case BG3D_TAGTYPE_MATERIALFLAGS: {
      readMaterialFlags(pFile);
      break;
    }
    case BG3D_TAGTYPE_MATERIALDIFFUSECOLOR: {
      readMaterialDiffuseColor(pFile);
      break;
    }
    case BG3D_TAGTYPE_TEXTUREMAP: {
      readMaterialTextureMap(pFile);
      break;
    }
    case BG3D_TAGTYPE_GROUPSTART: {
      readGroup();
      break;
    }
    case BG3D_TAGTYPE_GROUPEND: {
      endGroup();
      break;
    }
    case BG3D_TAGTYPE_GEOMETRY: {
      newObjHeader = readNewMesh(pFile);
      break;
    }
    case BG3D_TAGTYPE_VERTEXARRAY: {
      readVertexArray(pFile, newObjHeader);
      break;
    }
    case BG3D_TAGTYPE_NORMALARRAY: {
      readNormalArray(pFile, newObjHeader);
      break;
    }
    case BG3D_TAGTYPE_UVARRAY: {
      readUVArray(pFile, newObjHeader);
      break;
    }
    case BG3D_TAGTYPE_COLORARRAY: {
      readVertexColorArray(pFile, newObjHeader);
      break;
    }
    case BG3D_TAGTYPE_TRIANGLEARRAY: {
      readTriangleArray(pFile, newObjHeader);
      break;
    }
    case BG3D_TAGTYPE_ENDFILE: {
      done = true;
      break;
    }
    default:
      perror("Error: Unrecognized Tag.\n");
      return;
    }    
  } while (!done);

  free(newObjHeader);
}

// Tag 0
void readMaterialFlags (FILE * pFile) {
  size_t count;
  uint32_t flags;

  count = sizeof(flags);
  size_t result = fread(&flags, 1, count, pFile);

  if (result < count) {
    perror("Error Reading Material Flags.\n");
    return;
  }

  flags = htobe32(flags);

  long pos = ftell(pFile) - result;
  printf("%8lx: %u (flags)\n", pos, flags);
}

// Tag 1
void readMaterialDiffuseColor (FILE * pFile) {
  size_t count;
  uint32_t color[4];

  count = sizeof(uint32_t) * 4;
  size_t result = fread(color, 1, 16, pFile);

  if (result < count) {
    perror("Error Reading Diffuse Color\n");
    return;
  }

  for (int i = 0; i < 4; i++) {
    color[i] = htobe32(color[i]);
  }

  long pos = ftell(pFile) - result;
  printf("%8lx: %x (diffuse color r)\n", pos, color[0]);
  printf("%8lx: %x (diffuse color g)\n", pos + 4, color[1]);
  printf("%8lx: %x (diffuse color b)\n", pos + 8, color[2]);
  printf("%8lx: %x (diffuse color a)\n", pos + 12, color[3]);
}

// Tag 2
void readMaterialTextureMap (FILE * pFile) {
  BG3DTextureHeader header;
  size_t count = sizeof(header);

  size_t result = fread(&header, 1, count, pFile);

  if (result < count) {
    perror("Error Reading Texture Header.\n");
    return;
  }

  header.width = htobe32(header.width);
  header.height = htobe32(header.height);
  header.bufferSize = htobe32(header.bufferSize);

  long pos = ftell(pFile) - result;
  printf("%8lx: %u (width)\n", pos, header.width);
  printf("%8lx: %u (height)\n", pos + 4, header.height);
  printf("%8lx: 0x%x (size)\n", pos + 8, header.bufferSize);

  count = header.bufferSize;
  void * buffer = malloc(count);

  result = fread(buffer, 1, count, pFile);

  if (result < count) {
    perror("Error Reading Texture Pixels.\n");
    return;
  }

  free(buffer);
}

// Tag 5
BG3DMeshHeader * readNewMesh (FILE * pFile) {
  size_t count = sizeof(BG3DMeshHeader);
  BG3DMeshHeader * geoHeader = (BG3DMeshHeader *) malloc(count);
  size_t result = fread(geoHeader, 1, count, pFile);

  if (result < count) {
    perror("Error Reading Mesh Header.\n");
    return NULL;
  }

  geoHeader->materialNum = htobe32(geoHeader->materialNum);
  geoHeader->flags = htobe32(geoHeader->flags);
  geoHeader->numPoints = htobe32(geoHeader->numPoints);
  geoHeader->numTriangles = htobe32(geoHeader->numTriangles);

  long pos = ftell(pFile) - 16;
  printf("%8lx: %u (materialNum)\n", pos, geoHeader->materialNum);
  printf("%8lx: %u (flags)\n", pos + 4, geoHeader->flags);
  printf("%8lx: %u (numPoints)\n", pos + 8, geoHeader->numPoints);
  printf("%8lx: %u (numTriangles)\n", pos + 12, geoHeader->numTriangles);

  return geoHeader;
}

// Tag 6
void readVertexArray (FILE * pFile, BG3DMeshHeader * header) {
  uint32_t numPoints = header->numPoints;

  size_t count = numPoints * 12;

  printf("Count: %zu\n", count);

  void * vertexArray = malloc(count);

  size_t result = fread(vertexArray, 1, count, pFile);

  if (result < count) {
    perror("Error Reading the Vertex Array.\n");
    return;
  }

  free(vertexArray);
}

// Tag 7
void readNormalArray (FILE * pFile, BG3DMeshHeader * header) {
  uint32_t numPoints = header->numPoints;

  size_t count = numPoints * 12;
  void * normalArray = malloc(count);

  size_t result = fread(normalArray, 1, count, pFile);

  if (result < count) {
    perror("Error Reading the Vertex Array.\n");
    return;
  }

  free(normalArray);
}

// Tag 8
void readUVArray (FILE * pFile, BG3DMeshHeader * header) {
  uint32_t numPoints = header->numPoints;

  size_t count = numPoints * 8;
  void * uvList = malloc(count);

  size_t result = fread(uvList, 1, count, pFile);

  if (result < count) {
    perror("Error Reading the Vertex Array.\n");
    return;
  }

  free(uvList);
}

// Tag 9
void readVertexColorArray (FILE * pFile, BG3DMeshHeader * header) {
  uint32_t numPoints = header->numPoints;

  size_t count = numPoints * 4;
  void * vertexColorArray = malloc(count);

  size_t result = fread(vertexColorArray, 1, count, pFile);

  if (result < count) {
    perror("Error Reading the Vertex Color Array.\n");
    return;
  }

  free(vertexColorArray);
}

// Tag 10
void readTriangleArray (FILE * pFile, BG3DMeshHeader * header) {
  uint32_t numPoints = header->numTriangles;

  size_t count = numPoints * 12;
  void * triList = malloc(count);

  size_t result = fread(triList, 1, count, pFile);

  if (result < count) {
    perror("Error Reading the Triangle Array.\n");
    return;
  }

  free(triList);
}

// Tag 3
void readGroup (void) {

}

// Tag 4
void endGroup (void) {

}
