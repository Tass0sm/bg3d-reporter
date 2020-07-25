#include <stdint.h>
#include <stdio.h>

typedef struct {
  char headerString[16];
  uint32_t version;
} BG3DHeaderType;

typedef struct {
  uint32_t width, height;
  uint32_t bufferSize;
} BG3DTextureHeader;

typedef struct {
  uint32_t materialNum;
  uint32_t flags;
  uint32_t numPoints;
  uint32_t numTriangles;
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

