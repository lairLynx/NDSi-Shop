#include "video.h"

#include "software.h"
#include <gl2d.h>
#include <nds.h>

void initGuiVideo(void)
{
    videoSetMode(MODE_0_3D);
    lcdMainOnTop();
    guiSoftwareVideoInit();

    glScreen2D();

    vramSetBankA(VRAM_A_TEXTURE_SLOT0);
    vramSetBankB(VRAM_B_TEXTURE_SLOT1);
    vramSetBankD(VRAM_D_TEXTURE_SLOT3);
    vramSetBankE(VRAM_E_TEX_PALETTE);
    vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
    vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5);
}

void guiLoop(void)
{
    glFlush(0);
    cothread_yield_irq(IRQ_VBLANK);
    guiSoftwareSwapFrame();
}
