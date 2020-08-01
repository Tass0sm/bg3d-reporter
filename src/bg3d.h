#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <endian.h>
#include <json-c/json_object.h>

#include "common.c"

#define BMPH_IMPLEMENTATION
#include "bmph.h"

#define OTTOMATIC

typedef struct {
  char headerString[16];
  uint32_t version;
} BG3DHeaderType;

typedef struct {
  uint32_t width, height;
#ifdef OTTOMATIC
  uint32_t unknwn1, unknwn2;
#endif // OTTOMATIC
  uint32_t bufferSize;
#ifdef OTTOMATIC
  char unknwn3[16];
#endif // OTTOMATIC
} BG3DTextureHeader;

typedef struct {
  uint32_t materialNum;
  uint32_t flags;
#ifdef OTTOMATIC
  char unknwn1[20];
#endif // OTTOMATIC
  uint32_t numPoints;
  uint32_t numTriangles;
#ifdef OTTOMATIC
  char unknwn2[16];
#endif // OTTOMATIC
} BG3DMeshHeader;

enum {
  BG3D_TAGTYPE_MATERIALFLAGS		=	0,
  BG3D_TAGTYPE_MATERIALDIFFUSECOLOR	=	1,
  BG3D_TAGTYPE_TEXTUREMAP		=	2,
  BG3D_TAGTYPE_GROUPSTART		=	3,
  BG3D_TAGTYPE_GROUPEND			=	4,	
  BG3D_TAGTYPE_GEOMETRY			=	5,
  BG3D_TAGTYPE_VERTEXARRAY		=	6,
  BG3D_TAGTYPE_NORMALARRAY		=	7,
  BG3D_TAGTYPE_UVARRAY			=	8,
  BG3D_TAGTYPE_COLORARRAY		=	9,
  BG3D_TAGTYPE_TRIANGLEARRAY		= 	10,
  BG3D_TAGTYPE_ENDFILE			=	11
};

void readHeader (FILE *);
void parseFile (FILE *);

void readMaterialFlags (FILE *);
void readMaterialDiffuseColor (FILE *);
void readMaterialTextureMap (FILE *);
void readGroup (void);
void endGroup (void);

BG3DMeshHeader * readNewMesh (FILE *);
void readVertexArray (FILE *, BG3DMeshHeader *);
void readNormalArray (FILE *, BG3DMeshHeader *);
void readUVArray (FILE *, BG3DMeshHeader *);
void readVertexColorArray (FILE *, BG3DMeshHeader *);
void readTriangleArray (FILE *, BG3DMeshHeader *);

void preLoadTextureMaterials (void);

