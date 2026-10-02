#include "box.h"

#include "software.h"
#include <gl2d.h>

struct GuiBox {
    size_t width;
    size_t height;
    size_t posX;
    size_t posY;
    u16 color;
    size_t borderSize;
    u16 borderColor;
};

GuiBox newGuiBox(size_t width, size_t height, u16 color)
{
    GuiBox gb = malloc(sizeof(struct GuiBox));
    gb->width = width;
    gb->height = height;
    gb->posX = 0;
    gb->posY = 0;
    gb->color = color;
    gb->borderSize = 0;
    gb->borderColor = 0;

    return gb;
}

void freeGuiBox(GuiBox gb)
{
    free(gb);
}

void setGuiBoxWidth(GuiBox gb, size_t width)
{
    gb->width = width;
}

void setGuiBoxHeight(GuiBox gb, size_t height)
{
    gb->height = height;
}

size_t getGuiBoxBorderSize(GuiBox gb)
{
    return gb->borderSize;
}

void setGuiBoxPos(GuiBox gb, size_t posX, size_t posY)
{
    gb->posX = posX;
    gb->posY = posY;
}

void setGuiBoxBorder(GuiBox gb, size_t borderSize, u16 borderColor)
{
    gb->borderSize = borderSize;
    gb->borderColor = borderColor;
}

void drawGuiBoxPos(GuiBox gb, size_t posX, size_t posY)
{
    if (guiSoftwareIsActive()) {
        if (gb->borderSize && gb->borderColor && gb->color) {
            guiSoftwareFillRect(posX, posY, gb->width, gb->height, gb->borderColor);
            guiSoftwareFillRect(posX + gb->borderSize, posY + gb->borderSize,
                gb->width - gb->borderSize * 2, gb->height - gb->borderSize * 2, gb->color);
        } else if (gb->color) {
            guiSoftwareFillRect(posX, posY, gb->width, gb->height, gb->color);
        } else if (gb->borderSize && gb->borderColor) {
            guiSoftwareFillRect(posX, posY, gb->width, gb->borderSize, gb->borderColor);
            guiSoftwareFillRect(posX, posY + gb->height - gb->borderSize, gb->width, gb->borderSize, gb->borderColor);
            guiSoftwareFillRect(posX, posY, gb->borderSize, gb->height, gb->borderColor);
            guiSoftwareFillRect(posX + gb->width - gb->borderSize, posY, gb->borderSize, gb->height, gb->borderColor);
        }
        return;
    }

    if (gb->borderSize && gb->borderColor && gb->color) {
        glBoxFilled(
            posX,
            posY,
            posX + gb->width - 1,
            posY + gb->height - 1,
            gb->borderColor);
        glBoxFilled(
            posX + gb->borderSize,
            posY + gb->borderSize,
            posX + gb->width - gb->borderSize - 1,
            posY + gb->height - gb->borderSize - 1,
            gb->color);
    } else if (gb->color) {
        glBoxFilled(
            posX,
            posY,
            posX + gb->width - 1,
            posY + gb->height - 1,
            gb->color);
    } else if (gb->borderSize && gb->borderColor) {
        for (size_t i = 0; i < gb->borderSize; i++) {
            glBox(
                posX + i,
                posY + i,
                posX + gb->width - i - 2,
                posY + gb->height - i - 2,
                gb->borderColor);
        }
    }
}

void drawGuiBox(GuiBox gb)
{
    drawGuiBoxPos(gb, gb->posX, gb->posY);
}
