#include "style.h"
#include "../settings.h"

u16 guiGetColor(ColorEnum color)
{
    return colorSchemes[settings.colorScheme][color];
}
