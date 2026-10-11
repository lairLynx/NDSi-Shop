#include "main.h"

#include "config.h"
#include "filesystem.h"
#include "gui/text.h"
#include "gui/video.h"
#include "menu.h"
#include "networking.h"
#include "settings.h"
#include "splash.h"
#include "utils/filesystem.h"
#include <fat.h>
#include <nds.h>

Database db = NULL;

static void sleepSeconds(u32 seconds)
{
    for (u32 i = 0; i < seconds * 60; i++)
        cothread_yield_irq(IRQ_VBLANK);
}

// The splash covers the initialization. Problems are reported on the bottom screen console
static void handleInit(bool result, const char* errorMessage, u32 sleepTime, bool exitOnError)
{
    if (result)
        return;

    consoleDemoInit();
    printf("\n\t\t\t" APP_NAME " " APP_VERSION "\n%s", errorMessage);
    sleepSeconds(sleepTime);

    if (exitOnError)
        exit(1);
}

int main(int argc, char** argv)
{
    splashBegin();

    handleInit(initFat(), "\n\nFailed to initialize filesystem\n", 5, true);
    handleInit(initNetworking() == NETWORKING_INIT_SUCCESS, "\n\nFailed to initialize networking:\nwifi connection failed\n", 3, false);
    handleInit((createDir(APPDATA_DIR) && createDir(CACHE_DIR)), "\n\nFailed to initialize directories\n", 5, true);
    handleInit((defaultSettings() && loadSettings()), "\n\nLoaded default settings\n", 3, false);

    splashEnd();

    initGuiVideo();
    initGuiFont();

    menuBegin(MENU_REPOS);

    return 0;
}
