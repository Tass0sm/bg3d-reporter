#include "bg3d.h"

json_object * outputJSON;

void readHeader (FILE * pFile) {
	BG3DHeaderType header;
	size_t result;

	result = fread(header.headerString, 1, 16, pFile);

	if (result < 16) {
		perror("Error Reading BG3D Header String.\n");
		die();
	}

	result = fread(&(header.version), 1, 4, pFile);

	if (result < 4) {
		perror("Error Reading BG3D Header Version.\n");
		die();
	}
  			
	if ((header.headerString[0] != 'B') || (header.headerString[1] != 'G') ||
	    (header.headerString[2] != '3') || (header.headerString[3] != 'D')) {
		perror("Error: BG3D file has invalid header.\n");
		die();
	}

	extern uint8_t argState;
	if (argState & 1) {
		printf("Header: %s\n", header.headerString);
		long pos = ftell(pFile) - result;
		printf("%8lx: %u (version)\n", pos, header.version);
	}

	if (argState & 2) {
		extern json_object * outputJSON;
		outputJSON = json_object_new_object();

		json_object * asset = json_object_new_object();
		json_object * version = json_object_new_string("2.0");

		json_object_object_add(asset, "version", version);
		json_object_object_add(outputJSON, "asset", asset);
	}
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
			die();
		}

		tag = htobe32(tag);

		extern uint8_t argState;
		if (argState & 1) {
			long pos = ftell(pFile) - result;
			printf("%8lx: %u (tag)\n", pos, tag);
		}

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
			die();
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
		die();
	}

	flags = htobe32(flags);

	extern uint8_t argState;
	if (argState & 1) {
		long pos = ftell(pFile) - result;
		printf("%8lx: %u (flags)\n", pos, flags);
	}
}

// Tag 1
void readMaterialDiffuseColor (FILE * pFile) {
	size_t count;
	uint32_t color[4];

	count = sizeof(uint32_t) * 4;
	size_t result = fread(color, 1, 16, pFile);

	if (result < count) {
		perror("Error Reading Diffuse Color\n");
		die();
	}

	for (int i = 0; i < 4; i++) {
		color[i] = htobe32(color[i]);
	}

	extern uint8_t argState;
	if (argState & 1) {
		long pos = ftell(pFile) - result;
		printf("%8lx: %x (diffuse color r)\n", pos, color[0]);
		printf("%8lx: %x (diffuse color g)\n", pos + 4, color[1]);
		printf("%8lx: %x (diffuse color b)\n", pos + 8, color[2]);
		printf("%8lx: %x (diffuse color a)\n", pos + 12, color[3]);
	}
}

// Tag 2
void readMaterialTextureMap (FILE * pFile) {
	BG3DTextureHeader header;
	size_t count = sizeof(header);

	size_t result = fread(&header, 1, count, pFile);

	if (result < count) {
		perror("Error Reading Texture Header.\n");
		die();
	}

	header.width = htobe32(header.width);
	header.height = htobe32(header.height);
	header.bufferSize = htobe32(header.bufferSize);

	extern uint8_t argState;
	if (argState & 1) {
		long pos = ftell(pFile) - result;
		printf("%8lx: %u (width)\n", pos, header.width);
		printf("%8lx: %u (height)\n", pos + 4, header.height);
		printf("%8lx: 0x%x (size)\n", pos + 8, header.bufferSize);
		printf("%8lx: Beginning of Texture Data\n", pos + result);
	}

	count = header.bufferSize;
	void * buffer = malloc(count);

	result = fread(buffer, 1, count, pFile);

	if (result < count) {
		perror("Error Reading Texture Pixels.\n");
		die();
	}

	if (argState & 2) {
		// get the texture output file name
		extern char * outputName;
		char outputPathTexture[100] = "";
		snprintf(outputPathTexture, 100, "%s.bmp", outputName);

		// create bitmap structure
		Bitmap * b = bm_create(header.width, header.height);

		char * pColorComponent = (char *) buffer;
		for (int r = 0; r < header.width; r++) {
			for (int c = 0; c < header.height; c++) {
				char aComp = 0xff;
				char rComp = *(pColorComponent++);
				char gComp = *(pColorComponent++);
				char bComp = *(pColorComponent++);
				unsigned int color = aComp << 24 | rComp << 16 | gComp << 8 | bComp;

				bm_set(b, r, c, color);
			}
		}

		// write bitmap and free it.
		bm_save(b, outputPathTexture);
		bm_free(b);
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
		die();
	}

	geoHeader->materialNum = htobe32(geoHeader->materialNum);
	geoHeader->flags = htobe32(geoHeader->flags);
	geoHeader->numPoints = htobe32(geoHeader->numPoints);
	geoHeader->numTriangles = htobe32(geoHeader->numTriangles);

	extern uint8_t argState;
	if (argState & 1) {
		long pos = ftell(pFile) - 16;
		printf("%8lx: %u (materialNum)\n", pos, geoHeader->materialNum);
		printf("%8lx: %u (flags)\n", pos + 4, geoHeader->flags);
		printf("%8lx: %u (numPoints)\n", pos + 8, geoHeader->numPoints);
		printf("%8lx: %u (numTriangles)\n", pos + 12, geoHeader->numTriangles);
	}

	if (argState & 2) {
		extern json_object * outputJSON;

		json_object * buffers = json_object_new_array();
		json_object * currentBuffer = json_object_new_object();

		json_object_object_add(asset, "version", version);
		json_object_object_add(outputJSON, "asset", asset);
	}

	return geoHeader;
}

// Tag 6
void readVertexArray (FILE * pFile, BG3DMeshHeader * header) {
	uint32_t numPoints = header->numPoints;
	size_t count = numPoints * 12;
	void * vertexArray = malloc(count);

	size_t result = fread(vertexArray, 1, count, pFile);

	if (result < count) {
		perror("Error Reading the Vertex Array.\n");
		die();
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
		die();
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
		die();
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
		die();
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
		die();
	}

	free(triList);
}

// Tag 3
void readGroup (void) {

}

// Tag 4
void endGroup (void) {

}
