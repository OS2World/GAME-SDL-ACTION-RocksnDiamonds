#include <assert.h>
#include "guru_meditation.h"
#include "mood_item.h"
#include "globals.h"
#include "random_gen.h"
#include "datafun.h"
#if DEBUG > 0
#include <stdio.h>
#endif

#define MOOD_ITEM_SPEED 0.04
#define MOOD_ITEM_TIME_GAP 29179

static sdl_surfaces_t* mood_item_surfaces;
static SDL_Surface *current_item_surface = NULL; //NULL means none active
SDL_Surface *mood_icon_surface = NULL;
static float mood_item_x;
static float mood_item_y;
static Uint32 mood_item_last_ticks = 0;
static Uint32 mood_item_time_gap = MOOD_ITEM_TIME_GAP;
static float mood_item_speed = MOOD_ITEM_SPEED;

/*
 * Parameters for the mood items are stored in the FORM.ROCK MOOD
 * chunk. The chunk has the following structure:
 *
 * 0000 UWORD	mood item time-gap
 * 0002 UWORD	mood item speed * 65536 / 4 - 1, default is $028B
 */

SDL_Surface *init_mood_item(uiff_ctx_t iff) {
  int32_t chunksize;
  unsigned int i;

  chunksize = uiff_find_chunk_wflags(&iff, MakeID('M', 'O', 'O', 'D'), IFF_FIND_REWIND);
  assert(printf("FORM.ROCK MOOD size = $%08X\n", chunksize));
  if(chunksize >= 2) {
    mood_item_time_gap = read16(iff.f);
    printf("\ttime gap = $%04X\n", (unsigned)mood_item_time_gap);
  }
  if(chunksize >= 4) {
    i = read16(iff.f);
    mood_item_speed = i + 1;
    mood_item_speed *= 4.0 / 65536;
    printf("\titem speed = $%04X %12.6e\n", i, mood_item_speed);
  } else guru_meditation(GM_FLAGS_GREEN | GM_FLAGS_ABORTIFY, GM_SS_Graphics | GM_GE_BadParm | GURU_SEC_mood_item, &mood_item_speed);
  if((mood_item_surfaces = load_images_ck("mood_item.%02hx.png", 0, 255, 0)) == NULL) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_mood_item, &init_mood_item);
    return NULL;
  }
  if((mood_icon_surface = load_image("icon.mood_item.png", 0, 255, 0)) == NULL) {
    destroy_mood_item();
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_mood_item, &mood_icon_surface);
    return NULL;
  }
  return mood_icon_surface;
}

void destroy_mood_item(void) {
  assert(printf("destroy mood item %p %p\n", mood_icon_surface, mood_item_surfaces));
  if(mood_icon_surface) SDL_FreeSurface(mood_icon_surface);
  mood_icon_surface = NULL;
  if(mood_item_surfaces) destroy_sdl_surfaces(mood_item_surfaces);
  mood_item_surfaces = NULL;
}

void draw_mood_item(SDL_Surface *target) {
  SDL_Rect dest;

  if(!current_item_surface && (rnd() < 1e-3) && (last_ticks - mood_item_last_ticks > mood_item_time_gap)) {
    current_item_surface = mood_item_surfaces->surfaces[(int)(mood_item_surfaces->num_surfaces * rnd())];
    mood_item_x = xsize;
    mood_item_y = rnd() * (ysize - current_item_surface->h);
  } else if(current_item_surface) { /* We still need to check if an item is active. */
    dest.x = mood_item_x;
    dest.y = mood_item_y;
    dest.w = current_item_surface->w;
    dest.h = current_item_surface->h;
    SDL_BlitSurface(current_item_surface, NULL, target, &dest);
    mood_item_x -= mood_item_speed;
    if(mood_item_x < -current_item_surface->w) {
      current_item_surface = NULL;
      mood_item_last_ticks = last_ticks;
    }
  }
}
