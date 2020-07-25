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
  size_t resultSize;

  resultSize = fread(header.headerString, 1, 16, pFile);

  if (resultSize < 16) {
    perror("Error Reading BG3D Header String.\n");
    return;
  }

  resultSize = fread(&(header.version), 1, 4, pFile);

  if (resultSize < 4) {
    perror("Error Reading BG3D Header Version.\n");
    return;
  }
  			
  if ((header.headerString[0] != 'B') || (header.headerString[1] != 'G') ||
      (header.headerString[2] != '3') || (header.headerString[3] != 'D')) {
    perror("Error: BG3D file has invalid header.\n");
    return;
  }


  printf("Header: %s\n", header.headerString);
  printf("Version: %u\n", header.version);
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

    printf("Tag: %u\n", tag);

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

  printf("Flags: %u\n", flags);
}

void readMaterialDiffuseColor (FILE * pFile) {
  size_t count;
  uint32_t color[4];

  count = sizeof(uint32_t) * 4;
  size_t result = fread(color, 1, 16, pFile);

  printf("Result: %zu\n", result);

  if (result < count) {
    perror("Error Reading Diffuse Color\n");
    return;
  }

  for (int i = 0; i < 4; i++) {
    color[i] = htobe32(color[i]);
  }

  printf("Diffuse Color r: %x\n", color[0]);
  printf("Diffuse Color g: %x\n", color[1]);
  printf("Diffuse Color b: %x\n", color[2]);
  printf("Diffuse Color a: %x\n", color[3]);
}

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

  printf("Width: %u\n", header.width);
  printf("Height: %u\n", header.height);
  printf("Size: %u\n", header.bufferSize);

  count = header.bufferSize;

  void * buffer = malloc(count);

  result = fread(buffer, 1, count, pFile);

  if (result < count) {
    perror("Error Reading Texture Pixels.\n");
    return;
  }

  free(buffer);
}

BG3DMeshHeader * readNewMesh (FILE * pFile) {
  BG3DMeshHeader * geoHeader = (BG3DMeshHeader *) malloc(sizeof(geoHeader));

  size_t count = sizeof(geoHeader->materialNum);
  size_t result = fread(&(geoHeader->materialNum), 1, count, pFile);

  if (result < count) {
    perror("Error Reading Material Num.\n");
    return NULL;
  }

  geoHeader->materialNum = htobe32(geoHeader->materialNum);

  count = sizeof(geoHeader->flags);
  result = fread(&(geoHeader->flags), 1, count, pFile);

  if (result < count) {
    perror("Error Reading Flags.\n");
    return NULL;
  }

  geoHeader->flags = htobe32(geoHeader->flags);

  count = sizeof(geoHeader->numPoints);
  result = fread(&(geoHeader->numPoints), 1, count, pFile);

  if (result < count) {
    perror("Error Reading Num Points.\n");
    return NULL;
  }

  geoHeader->numPoints = htobe32(geoHeader->numPoints);

  count = sizeof(geoHeader->numTriangles);
  result = fread(&(geoHeader->numTriangles), 1, count, pFile);

  if (result < count) {
    perror("Error Reading Num Triangles.\n");
    return NULL;
  }

  geoHeader->numTriangles = htobe32(geoHeader->numTriangles);

  printf("Material Num: %u\n", geoHeader->materialNum);
  printf("Flags: %u\n", geoHeader->flags);
  printf("Num Points: %u\n", geoHeader->numPoints);
  printf("Num Triangles: %u\n", geoHeader->numTriangles);

  return geoHeader;
}

void readVertexArray (FILE * pFile, BG3DMeshHeader * header) {
  uint32_t numPoints = header->numPoints;

  size_t count = numPoints * 12;
  void * vertexArray = malloc(count);

  size_t result = fread(vertexArray, 1, count, pFile);

  if (result < count) {
    perror("Error Reading the Vertex Array.\n");
    return;
  }

  free(vertexArray);
}

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

void readVertexColorArray (FILE * pFile, BG3DMeshHeader * header) {

}

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


void readGroup (void) {

}

void endGroup (void) {

}
