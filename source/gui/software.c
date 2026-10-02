#include "software.h"

#include <nds.h>
#include <string.h>

#define SOFTWARE_WIDTH 256
#define SOFTWARE_HEIGHT 192
#define SOFTWARE_PAGE_BYTES (64 * 1024)
#define SOFTWARE_FRAME_BYTES (SOFTWARE_WIDTH * SOFTWARE_HEIGHT)

static u8 framebuffer[SOFTWARE_FRAME_BYTES];
static int bitmapBg;
static unsigned frontPage;
static bool softwareTargetActive;

static u8 colorToIndex(u16 color)
{
    u8 red = (color >> 0) & 0x1F;
    u8 green = (color >> 5) & 0x1F;
    u8 blue = (color >> 10) & 0x1F;

    return ((red >> 2) << 5) | ((green >> 2) << 2) | (blue >> 3);
}

void guiSoftwareVideoInit(void)
{
    vramSetBankC(VRAM_C_SUB_BG);
    videoSetModeSub(MODE_5_2D);
    bitmapBg = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
    bgSetPriority(bitmapBg, 0);

    for (unsigned index = 0; index < 256; index++) {
        unsigned red = (index >> 5) & 7;
        unsigned green = (index >> 2) & 7;
        unsigned blue = index & 3;
        BG_PALETTE_SUB[index] = RGB15(red * 31 / 7, green * 31 / 7, blue * 31 / 3);
    }

    frontPage = 0;
    softwareTargetActive = false;
    memset(framebuffer, 0, sizeof(framebuffer));
    memset((u8*)BG_GFX_SUB, 0, SOFTWARE_PAGE_BYTES * 2);
}

void guiSoftwareBeginFrame(u16 clearColor)
{
    memset(framebuffer, colorToIndex(clearColor), sizeof(framebuffer));
}

void guiSoftwarePresentFrame(void)
{
    unsigned backPage = frontPage ^ 1;
    u8* backBuffer = (u8*)BG_GFX_SUB + backPage * SOFTWARE_PAGE_BYTES;
    dmaCopy(framebuffer, backBuffer, sizeof(framebuffer));
}

void guiSoftwareSwapFrame(void)
{
    frontPage ^= 1;
    bgSetMapBase(bitmapBg, frontPage * 4);
}

bool guiSoftwareIsActive(void)
{
    return softwareTargetActive;
}

void guiSoftwareSetActive(bool active)
{
    softwareTargetActive = active;
}

void guiSoftwareFillRect(size_t x, size_t y, size_t width, size_t height, u16 color)
{
    if (x >= SOFTWARE_WIDTH || y >= SOFTWARE_HEIGHT || width == 0 || height == 0)
        return;

    if (width > SOFTWARE_WIDTH - x)
        width = SOFTWARE_WIDTH - x;
    if (height > SOFTWARE_HEIGHT - y)
        height = SOFTWARE_HEIGHT - y;

    u8 colorIndex = colorToIndex(color);
    for (size_t row = y; row < y + height; row++)
        memset(&framebuffer[row * SOFTWARE_WIDTH + x], colorIndex, width);
}

void guiSoftwareDrawGlyph(const u8* atlas, size_t atlasWidth, size_t sourceX, size_t sourceY,
    size_t width, size_t height, size_t x, size_t y, u16 color)
{
    if (!atlas)
        return;

    u8 colorIndex = colorToIndex(color);
    for (size_t row = 0; row < height; row++) {
        size_t targetY = y + row;
        if (targetY >= SOFTWARE_HEIGHT)
            continue;

        for (size_t column = 0; column < width; column++) {
            size_t targetX = x + column;
            if (targetX >= SOFTWARE_WIDTH || atlas[(sourceY + row) * atlasWidth + sourceX + column] == 0)
                continue;

            framebuffer[targetY * SOFTWARE_WIDTH + targetX] = colorIndex;
        }
    }
}

void guiSoftwareBlit(const u16* pixels, size_t sourceWidth, size_t sourceHeight,
    size_t x, size_t y, size_t width, size_t height, bool tint, u16 tintColor)
{
    if (!pixels || !sourceWidth || !sourceHeight || !width || !height)
        return;

    for (size_t drawY = 0; drawY < height; drawY++) {
        size_t targetY = y + drawY;
        if (targetY >= SOFTWARE_HEIGHT)
            continue;

        size_t sourceY = drawY * sourceHeight / height;
        for (size_t drawX = 0; drawX < width; drawX++) {
            size_t targetX = x + drawX;
            if (targetX >= SOFTWARE_WIDTH)
                continue;

            u16 pixel = pixels[sourceY * sourceWidth + drawX * sourceWidth / width];
            if (!(pixel & BIT(15)))
                continue;

            framebuffer[targetY * SOFTWARE_WIDTH + targetX] = colorToIndex(tint ? tintColor : pixel);
        }
    }
}