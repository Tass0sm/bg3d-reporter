#include <stdio.h>
#include <string.h>

#include "arg.c"
#include "bg3d.c"

int main(int argc, char *argv[]) {
	setArgState(argc, argv);

	FILE * pFile;
	pFile = fopen(inputPath, "r");

	if (pFile == NULL) {
		perror("Error Opening File.\n");
		die();
	}

	readHeader(pFile);
	parseFile(pFile);

	fclose(pFile);

	extern uint8_t argState;
	if (argState & 2) {
		// get json string
		extern json_object * outputJSON;
		const char * jsonString = json_object_to_json_string(outputJSON);

		// get the json output file name
		extern char * outputName;
		char outputPathJSON[100] = "";
		snprintf(outputPathJSON, 100, "%s.gltf", outputName);

		// open, write, close
		FILE * pOutFile = fopen(outputPathJSON, "w");
		fprintf(pOutFile, "%s\n", jsonString);
		fclose(pOutFile);

		//free
		json_object_put(outputJSON);
	}

	return 0;
}
