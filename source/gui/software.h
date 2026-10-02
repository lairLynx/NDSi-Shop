#pragma once

#include <nds/ndstypes.h>
#include <stdbool.h>
#include <stddef.h>

void guiSoftwareVideoInit(void);
void guiSoftwareBeginFrame(u16 clearColor);
void guiSoftwarePresentFrame(void);
void guiSoftwareSwapFrame(void);
bool guiSoftwareIsActive(void);
void guiSoftwareSetActive(bool active);
void guiSoftwareFillRect(size_t x, size_t y, size_t width, size_t height, u16 color);
void guiSoftwareDrawGlyph(const u8* atlas, size_t atlasWidth, size_t sourceX, size_t sourceY,
    size_t width, size_t height, size_t x, size_t y, u16 color);
void guiSoftwareBlit(const u16* pixels, size_t sourceWidth, size_t sourceHeight,
    size_t x, size_t y, size_t width, size_t height, bool tint, u16 tintColor);