#pragma once
#include <stdbool.h>

bool initFat(void);
char* getRomDir(void);
char* getRomPath(void); // Full path of the running .nds, or empty if unknown
