#include <math.h>
#include <assert.h>
#include "config.h"
#include "globals.h"
#include "engine_exhaust.h"
#include "random_gen.h"
#include "ship.h"

/*
 * Rewriting this part to use sparkles is not that easy as the engine
 * dots do not vanish when they collide with something.  Also they
 * start with a short life. TODO: An idea is needed in order to get a
 * smooth working version.
 */

#ifdef DEBUG
#include <stdio.h>
unsigned int max_edots_used = 0;
#endif

// Structure global variables
struct enginedots edot1[MAX_ENGINE_DOTS];

static struct enginedots *edots_begin = edot1;
static struct enginedots *edots_end = edot1;

void init_engine_dots() {
  edots_begin = edot1;
  edots_end = edot1;
}

void draw_engine_dots(SDL_Surface *s) {
  struct enginedots edot;
  struct enginedots *dotptr;
  struct enginedots *targetptr = edots_begin;
  int heatindex;
  RD_VIDEO_TYPE *rawpixel;

  rawpixel = (RD_VIDEO_TYPE *) s->pixels;
  for(dotptr = edots_begin; dotptr < edots_end; ++dotptr) {
    edot = *dotptr;
    edot.x += edot.dx * movementrate;
    edot.y += edot.dy * movementrate;
    if((edot.life -= movementrate * 3) < 0 ||
       edot.y < 0 || edot.y > ysize || edot.x < 0 || edot.x > xsize) {
      continue;
    }
    heatindex = edot.life * 6;
    rawpixel[(int) (s->pitch / sizeof(RD_VIDEO_TYPE) * (int) (edot.y)) + (int) (edot.x)] =
      heatindex > 3 * W ? heatcolor[3 * W - 1] : heatcolor[heatindex];
    *targetptr++ = edot;
  }
  edots_end = targetptr;
}

void create_engine_dots(int newdots) {
  unsigned int i;
  double theta, r, dx, dy;
  SDL_Surface *surf_ship = get_current_sprite_surface(&ship_sprite, last_ticks);

  for(i = 0; i < newdots * movementrate; ++i) {
    if(edots_end - edots_begin >= MAX_ENGINE_DOTS)
      break;
    theta = rnd() * M_PI * 2;
    r = rnd();
    dx = cosf(theta) * r;
    dy = sinf(theta) * r;
    
    edots_end->x = shipdata.xship + surf_ship->w / 2 - 14;
    edots_end->y = shipdata.yship + surf_ship->h / 2 + (rnd() - 0.5) * 5 - 1;
    edots_end->dx = 10 * (dx - 1.5) + shipdata.xvel;
    edots_end->dy = 1 * dy + shipdata.yvel;
    edots_end->life = 45 + rnd() * 5;
    edots_end++;
  }
#ifdef DEBUG
  i = edots_end - edots_begin;
  if(i > max_edots_used) {
    max_edots_used = i;
    printf("new: max_edots_used = $%X\n", max_edots_used);
  }
#endif
}


void create_engine_dots2(int newdots, int m) {
  int i;
  double theta, r, dx, dy;
  SDL_Surface *surf_ship = get_current_sprite_surface(&ship_sprite, last_ticks);

  // Don't create fresh engine dots when
  // the game is not being played and a demo is not beng shown
  if(state != GAME_PLAY && state != DEMO)
    return;

  for(i = 0; i < newdots; i++) {
    if(edots_end - edots_begin >= MAX_ENGINE_DOTS)
      break;
    theta = rnd() * M_PI * 2;
    r = rnd();
    dx = cosf(theta) * r;
    dy = sinf(theta) * r;

    edots_end->x = shipdata.xship + surf_ship->w / 2 + (rnd() - 0.5) * 1;
    edots_end->y = shipdata.yship + surf_ship->h / 2 + (rnd() - 0.5) * 1;
    switch (m) {
    case 0:
      edots_end->x -= 14;
      edots_end->dx = 5 * (dx - 1.5) + shipdata.xvel;
      edots_end->dy = 1 * dy + shipdata.yvel;
      break;
    case 1:
      edots_end->dy = 5 * (dy - 1.5) + shipdata.yvel;
      edots_end->dx = 1 * dx + shipdata.xvel;
      break;
    case 2:
      edots_end->x += 14;
      edots_end->dx = -5 * (dx - 1.5) + shipdata.xvel;
      edots_end->dy = 1 * dy + shipdata.yvel;
      break;
    case 3:
      edots_end->dy = 5 * (dy + 1.5) + shipdata.yvel;
      edots_end->dx = 1 * dx + shipdata.xvel;
      break;
    }
    edots_end->life = 45 + rnd() * 20;
    ++edots_end;
  }
}
