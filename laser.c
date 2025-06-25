#include "laser.h"
#include "config.h"
#include "random_gen.h"

void draw_random_dots(SDL_Surface *surf, int ypos, int x_min, int x_max) {
  RD_VIDEO_TYPE c, *rawpixel;
  int i;
  int x, y;

  SDL_LockSurface(surf);
  rawpixel = (RD_VIDEO_TYPE *) surf->pixels;
  c = SDL_MapRGB(surf->format, rnd() * 128, 128 + rnd() * 120, rnd() * 128);

  for(i = 0; i < (x_max - x_min) * 5; i += 10) {
    x = rnd() * (x_max - x_min) + x_min;
    y = ypos + (rnd() - 0.5) * 1.5;
    rawpixel[surf->pitch / sizeof(RD_VIDEO_TYPE) * y + x] = c;
  }
  SDL_UnlockSurface(surf);
}
