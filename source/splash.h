#pragma once

// Startup splash: the lairLynx Studios logo and credit on the top screen, the
// SDK credit on the bottom screen. Same look as the lairLynx Studios games.

// Draws the splash and fades it in. Call first, before anything else touches the video.
void splashBegin(void);

// Holds the splash until it has been up for a moment (any key or a touch skips
// ahead), then fades it out to black.
void splashEnd(void);
