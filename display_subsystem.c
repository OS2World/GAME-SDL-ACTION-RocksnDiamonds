#include "sparkles.h"
#include "display_subsystem.h"
#include "globals.h"
#include "engine_exhaust.h"
#include "mood_item.h"
#include "sound.h"
#include <math.h>

#define END_LEVEL_MARGIN 0.98

/*! \brief Draw the black infinity of space
 *
 * The space, the final frontier....
 */
void draw_infinite_black() {
  float t;
  float l = level - floorf(level);
  Uint32 c = 0;

  if((l > END_LEVEL_MARGIN) && (state == GAME_PLAY)) {
    t = 4 * (1 - (1 - l) / (1 - END_LEVEL_MARGIN));
    if(last_levelup_snd < level) {
      last_levelup_snd = ceilf(level);
      play_sound(4);
    }
    switch ((int) t) {
    case 0:
      // ramp up red
      c = SDL_MapRGB(surf_screen->format, (int) (255 * t), 0, 0);
      break;
    case 1:
      // ramp down red, ramp up green
      c =	SDL_MapRGB(surf_screen->format, (int) (255 * (1 - t)), (int) (255 * (t - 1)), 0);
      break;
    case 2:
      // ramp down green, ramp up blue
      c = SDL_MapRGB(surf_screen->format, 0, (int) (255 * (3 - t)), (int) (255 * (t - 3)));
      break;
    case 3:
      // ramp down blue
      c = SDL_MapRGB(surf_screen->format, 0, 0, (int) (255 * (4 - t)));
      break;
    }
  }
  SDL_FillRect(surf_screen, NULL, c);
}


void draw_ghostly_rock_dodger(SDL_Surface *surf_t_rock, SDL_Surface *surf_t_dodger) {
  SDL_Rect src, dest;

  src.w = surf_t_rock->w;
  src.h = surf_t_rock->h;
  src.x = 0;
  src.y = 0;
  dest.w = src.w;
  dest.h = src.h;
  dest.x = (xsize - src.w) / 2 + cosf(fadetimer / 6.5) * 10;
  dest.y = (ysize / 2 - src.h) / 2 + sinf(fadetimer / 5) * 10;
  SDL_SetAlpha(surf_t_rock, SDL_SRCALPHA, (int) (170 + 85 * sinf(fadetimer)));
  SDL_BlitSurface(surf_t_rock, &src, surf_screen, &dest);
  src.w = surf_t_dodger->w;
  src.h = surf_t_dodger->h;
  dest.w = src.w;
  dest.h = src.h;
  dest.x = (xsize - src.w) / 2 + sinf(fadetimer / 6.5) * 10;
  dest.y =
    (ysize / 2 - src.h) / 2 + surf_t_rock->h + 20 +
    sinf((fadetimer + 1) / 5) * 10;
  SDL_SetAlpha(surf_t_dodger, SDL_SRCALPHA,
	       (int) (170 + 85 * sinf(fadetimer - 1.0)));
  SDL_BlitSurface(surf_t_dodger, &src, surf_screen, &dest);
}


void drawdots(SDL_Surface * s) {
  // Draw the background stars (aka space dots - they can't possibly be stars
  // because then we'd be moving past them at something like 10,000 times the
  // speed of light)
  draw_space_dots(current_spacedot_engine, s);
  // Draw the mood item...
  draw_mood_item(s);
  // Draw all the engine dots
  SDL_LockSurface(s);
  draw_engine_dots(s);
  // Draw all outstanding bang dots and plumes
  draw_sparkles(s);
  SDL_UnlockSurface(s);
}


void draw_background_objects() {
  // Draw a fully black background
  draw_infinite_black();
  // Draw the background dots
  drawdots(surf_screen);
}

