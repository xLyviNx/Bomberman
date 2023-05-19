#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#define LINE_LENGTH 32

FILE* saveFile;

bool saveSystem_init();
void saveSystem_close();

char* saveSystem_readLine(int);
int saveSystem_LoadLevel();

bool trimText(char*, int);
void saveSystem_printAtLine(int, const char*);
void saveSystem_resetData();
void saveSystem_setStartData();