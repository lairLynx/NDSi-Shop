#include "settings.h"

#include "config.h"
#include "utils/filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SETTINGS_FILENAME "settings.cfg"
#define LEGACY_APPDATA_DIR "/NDS-Shop"

struct Settings settings;

bool defaultSettings(void)
{
    if (dirExists("/roms")) {
        sprintf(settings.dlPath, "/roms");
        settings.dlUseDirs = true;
    } else {
        sprintf(settings.dlPath, APPDATA_DIR);
        settings.dlUseDirs = false;
    }

    settings.colorScheme = COLOR_SCHEME_1;
    settings.lang = LANG_EN;

    return true;
}

bool loadSettings(void)
{
    FILE* fp = fopen(APPDATA_DIR "/" SETTINGS_FILENAME, "r");
    if (!fp)
        fp = fopen(LEGACY_APPDATA_DIR "/" SETTINGS_FILENAME, "r"); // Settings from before the data directory rename
    if (!fp)
        return false;

    char line[2048];
    char name[1024];
    char value[1024];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;

        // Skip malformed lines instead of discarding every setting
        if (sscanf(line, "%1023[^=]=%1023[^\n]", name, value) != 2)
            continue;

        int valueInt = atoi(value);

        if (strcmp(name, "dlPath") == 0 && dirExists(value))
            snprintf(settings.dlPath, sizeof(settings.dlPath), "%s", value);
        else if (strcmp(name, "useDirs") == 0 && (valueInt == 0 || valueInt == 1))
            settings.dlUseDirs = valueInt;
        else if (strcmp(name, "colorScheme") == 0 && (valueInt > 0 && valueInt < COLOR_SCHEMES_COUNT + 1))
            settings.colorScheme = valueInt - 1;
        else if (strcmp(name, "lang") == 0 && (valueInt >= 0 && valueInt < LANGS_COUNT))
            settings.lang = valueInt;
    }

    fclose(fp);

    return true;
}

bool saveSettings(void)
{
    FILE* fp = fopen(APPDATA_DIR "/" SETTINGS_FILENAME, "w");
    if (!fp)
        return false;

    fprintf(fp, "dlPath=%s\n", settings.dlPath);
    fprintf(fp, "useDirs=%d\n", settings.dlUseDirs);
    fprintf(fp, "colorScheme=%d\n", settings.colorScheme + 1);
    fprintf(fp, "lang=%d\n", settings.lang);

    fclose(fp);
    return true;
}
