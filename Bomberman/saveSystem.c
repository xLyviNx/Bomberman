#include "saveSystem.h"
#include <stdlib.h>
#define SAVEFILE_LOCATION "data/save.bmb"
#define TEMPFILE_LOCATION "data/save.tmp"
bool saveSystem_init()
{
	if (!saveFile)
	{
		fopen_s(&saveFile, SAVEFILE_LOCATION, "r+");
		if (saveFile)
			printf("[SAVE SYSTEM] Save System Initialized\n");
		else
		{
			printf("[SAVE SYSTEM] Save File Not Found! Creating One.\n");
			fopen_s(&saveFile, SAVEFILE_LOCATION, "w+");
			saveSystem_setStartData();
			
		}
		if (!saveFile)
			printf("[SAVE SYSTEM] Save System Initialization FAILED!\n");
	}
	else
		printf("[SAVE SYSTEM] Save System ALREADY INITIALIZED!\n");
	return (saveFile != NULL);
}
void saveSystem_close()
{
	if (saveFile) {
		fclose(saveFile);
		saveFile = NULL;
	}
}

char* saveSystem_readLine(int lineNumber)
{
	char* buf = (char*)malloc(LINE_LENGTH * sizeof(char));
	if (saveFile && buf) {
		fseek(saveFile, 0, SEEK_SET);
		bool keepread = true;
		int currLine = 1;
		do
		{
			fgets(buf, LINE_LENGTH, saveFile);
			trimText(buf, LINE_LENGTH);
			printf("Checking (%d): %s\n", currLine, buf);
			if (currLine == lineNumber)
			{
				keepread = false;
				printf("[SAVE SYSTEM] Returning Line %d: '%s'\nREMEMBER TO FREE YOUR MEMORY AFTER FINISHING OPERATIONS\n", currLine, buf);
			}
			if (keepread)
			{
				if (feof(saveFile))
				{
					keepread = false;
					buf[0] = '\0';
					printf("[SAVE SYSTEM] Could not find the line.\n");
				}
			}
			currLine++;

		} while (keepread);
	}
	return buf != NULL ? buf : NULL;

}
bool trimText(char* text, int n)
{
	for (int i = 0; i < n - 1; i++)
	{
		if (text[i] == '\n' && text[i + 1] == '\0')
		{
			text[i] = '\0';
			return true;
		}
	}
	return false;
}
int saveSystem_LoadLevel()
{
	int tret = 1;
	char* loadS = saveSystem_readLine(1);
	printf("[SAVE SYSTEM] Attempting to convert '%s' to INT\n", loadS);
	tret = atoi(loadS);
	printf("[SAVE SYSTEM] Loading Level: %d\n", tret);
	free(loadS);
	return tret;
}

void saveSystem_printAtLine(int lineNumber, const char* content)
{
	if (!saveFile)
	{
		printf("[SAVE SYSTEM] ERROR! Not initialized (saveLine)\n");
		return;
	}
	fseek(saveFile, 0, SEEK_SET);
	int currLine = 1;
	char buf[LINE_LENGTH] = "";
	//strcpy_s(stringToPut, LINE_LENGTH, content);
	FILE* tempfile = NULL;
	fopen_s(&tempfile, TEMPFILE_LOCATION, "w");
	if (tempfile)
	{
		while (!feof(saveFile) && fgets(buf, LINE_LENGTH, saveFile) != NULL)
		{
			if (currLine == lineNumber)
			{
				strcpy_s(buf, LINE_LENGTH * sizeof(char), content);
				//printf("line: %d, '%s'\n", currLine, buf);
			}
			//printf("Putting '%s'\n", buf);
			fputs(buf, tempfile);
			int lastl = strlen(buf) - 1;
			if (buf[lastl] != '\n' && buf[lastl] != '\0' && !feof(saveFile)) {
				fputc('\n', tempfile);
				//printf("CBUF: '%c' (+1: %d)\n", buf[lastl], buf[lastl+1]);
			}
			currLine++;
		}
		bool atEnd = false;
		while (currLine <= lineNumber && currLine>1)
		{
			atEnd = true;
			fputc('\n', tempfile);
			currLine++;
		}
		if (atEnd || currLine==1)
		{
			strcpy_s(buf, LINE_LENGTH * sizeof(char), content);
			fputs(buf, tempfile);
			int lastl = strlen(buf) - 1;
			if (buf[lastl] != '\n' && buf[lastl] != '\0' && !feof(saveFile)) {
				fputc('\n', tempfile);
				//printf("CBUF: '%c' (+1: %d)\n", buf[lastl], buf[lastl + 1]);
			}
		}
		puts("[SAVE SYSTEM] Reloading the SAVE SYSTEM (replacing files)");
		fclose(tempfile);
		saveSystem_close();
		remove(SAVEFILE_LOCATION);
		rename(TEMPFILE_LOCATION, SAVEFILE_LOCATION);
		remove(TEMPFILE_LOCATION);
		saveSystem_init();
	}
	else
	{
		puts("[SAVE SYSTEM] ERROR CREATING TEMP FILE.");
	}
}

void saveSystem_resetData()
{
	puts("[SAVE SYSTEM] RESETTING DATA");
	saveSystem_close();
	remove(SAVEFILE_LOCATION);
	saveSystem_init();
	saveSystem_setStartData();
}
void saveSystem_setStartData()
{
	saveSystem_printAtLine(1, "1");
}