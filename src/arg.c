#include <stdlib.h>
#include <stdint.h>

#include "common.c"

uint8_t argState;
char * inputPath;
char * outputName;

void setArgState(int argc, char *argv[]) {
	extern char * inputPath;
	extern char * outputName;
	extern uint8_t argState;

	if (argc < 2) {
		printf("Usage: tool inputPath.bg3d [-r] [-o outputName]\n");
		die();
	}

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			char c = argv[i][1];

			switch (c) {
			case 'r': {
				argState = argState | 0x01;
				break;
			}
			case 'o': {
				argState = argState | 0x02;

				if (i + 2 <= argc) {
					outputName = argv[++i];
				}

				break;
			}
			default:
				printf("Usage: tool inputPath.bg3d [-r] [-o outputName]\n");
				die();
				return;
			}
		} else {
			inputPath = argv[1];
		}

	}

	printf("Args: %x\n", argState);
}
