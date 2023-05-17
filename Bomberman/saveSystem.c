#include "saveSystem.h"
#include <stdlib.h>
bool saveSystem_init()
{
	if (!saveFile)
	{
		fopen_s(&saveFile, "data/save.bmb", "r+");
		if (saveFile)
			printf("[SAVE SYSTEM] Save System Initialized\n");
		else
		{
			printf("[SAVE SYSTEM] Save File Not Found! Creating One.\n");
			fopen_s(&saveFile, "data/save.bmb", "w+");
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
		bool keepread = true;
		int currLine = 0;
		do
		{
			fgets(buf, LINE_LENGTH, saveFile);
			trimText(buf, LINE_LENGTH);
			//printf("Checking: %s\n", buf);
			if (currLine == lineNumber)
			{
				keepread = false;
				printf("[SAVE SYSTEM] Returning Line %d: '%s'\nREMEMBER TO FREE YOUR MEMORY AFTER FINISHING OPERATIONS\n", lineNumber, buf);
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
	char* loadS = saveSystem_readLine(0);
	printf("[SAVE SYSTEM] Attempting to convert '%s' to INT\n", loadS);
	tret = atoi(loadS);
	printf("[SAVE SYSTEM] Loading Level: %d\n", tret);
	free(loadS);
	return tret;
}