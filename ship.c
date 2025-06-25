#include <math.h>
#include "config.h"
#include "sparkles.h"
#include "datafun.h"
#include "guru_meditation.h"
#include "ship.h"
#include "globals.h"
#include "random_gen.h"

struct black_point_struct {
  int x, y;
};
static struct black_point_struct black_point[MAX_BLACK_POINTS], *blackptr =
  black_point;

SDL_Surface *surf_small_ship;
sdl_surfaces_t *surfaces_ship;
struct sprite ship_sprite;
ship_t shipdata;


void draw_little_ships(int nships, SDL_Surface * surf_screen) {
  int i;
  SDL_Rect dest;

  for(i = 0; i < nships - 1; i++) {
    dest.w = surf_small_ship->w;
    dest.h = surf_small_ship->h;
    dest.x = (i + 1) * (surf_small_ship->w + 10);
    dest.y = 40;
    SDL_BlitSurface(surf_small_ship, NULL, surf_screen, &dest);
  }
}

void draw_ship(SDL_Surface * surf_screen) {
  static SDL_Rect dest;
  dest.w = 0; //src.w;
  dest.h = 0; //src.h;
  dest.x = (int) shipdata.xship;
  dest.y = (int) shipdata.yship;
  SDL_BlitSurface(get_current_sprite_surface(&ship_sprite, last_ticks), NULL, surf_screen, &dest);
}

void outline_detector() {
  char buf[256][256];
  int x, y;
  SDL_Surface *surf_ship = get_current_sprite_surface(&ship_sprite, last_ticks);

  SDL_LockSurface(surf_ship);
  RD_VIDEO_TYPE *raw_pixels = (RD_VIDEO_TYPE *) surf_ship->pixels;
  x = surf_ship->w - 1;
  y = surf_ship->h - 1;
  RD_VIDEO_TYPE background = raw_pixels[y * surf_ship->pitch / sizeof(RD_VIDEO_TYPE) + x];
  memset(buf, 0, sizeof(buf));
  for(y = 0; y < surf_ship->h; y++) {
    for(x = 0; x < surf_ship->w; x++) {
      buf[x + 1][y + 1] = raw_pixels[y * surf_ship->pitch / sizeof(RD_VIDEO_TYPE) + x] != background;
    }
  }
  for(y = 0; y < surf_ship->h + 2; y++) {
    for(x = 0; x < surf_ship->w + 2; x++) {
      if(buf[x][y] && (!buf[x][y - 1] || !buf[x][y + 1] || !buf[x - 1][y] || !buf[x + 1][y])) printf("%d %d\t", x - 1, y - 1);
    }
    putchar('\n');
  }
  SDL_UnlockSurface(surf_ship);
}

SDL_Surface *init_ship() {
  int i, j, numblack;
  SDL_Surface *surf_ship;

  // Load the spaceship surface from the spaceship file
  if((surfaces_ship = load_images_ck("ship.%02x.png", 0, 255, 0)) == NULL) {
    fprintf(stderr, "%s\n", SDL_GetError());
    guru_meditation(GM_FLAGS_DEADEND | GM_FLAGS_ABORTIFY, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_ship, (void*)0x53484950);
    //We never return from there...
  }
  init_sprite2(&ship_sprite, surfaces_ship, 117, initticks);

  /*
   * Remember that we assume that the ship has always *exactly* the
   * same size. If not you will get funny effects. And you will
   * explode all the time, of course.
   */
  surf_ship = surfaces_ship->surfaces[0];
  // Create the array of black points;
  SDL_LockSurface(surf_ship);
  RD_VIDEO_TYPE *raw_pixels = (RD_VIDEO_TYPE *) surf_ship->pixels;
  numblack = 0;
  for(i = 0; i < surf_ship->w; i++)
    for(j = 0; j < surf_ship->h; j++)
      if(raw_pixels[j * surf_ship->pitch / sizeof(RD_VIDEO_TYPE) + i] == 0) {
	blackptr->x = i;
	blackptr->y = j;
	blackptr++;
	if(++numblack > MAX_BLACK_POINTS)
	  return NULL;
      }
  SDL_UnlockSurface(surf_ship);
  //outline_detector();
  return surf_ship;
}

void draw_ship_shield(SDL_Surface * surf_screen) {
  // Show the freaky shields
  RD_VIDEO_TYPE *raw_pixels = (RD_VIDEO_TYPE *) surf_screen->pixels;
  struct black_point_struct *p;
  int x, y;
  RD_VIDEO_TYPE c;

  SDL_LockSurface(surf_screen);
  if(initialshield > 0) {
    initialshield -= movementrate;
    c = SDL_MapRGB(surf_screen->format, 0, 255, 255);
  } else {
    c = heatcolor[(int) shieldlevel];
    shieldlevel -= movementrate;
  }

  shieldpulse += 0.1;
  for(p = black_point; p < blackptr; p++) {
    x = p->x + (int) shipdata.xship + (rnd() + rnd() - 1) * sin(shieldpulse) * 4 + 1;
    y = p->y + (int) shipdata.yship + (rnd() + rnd() - 1) * sin(shieldpulse) * 4 + 1;
    if(x > 0 && y > 0 && x < xsize && y < ysize) {
      int offset = surf_screen->pitch / sizeof(RD_VIDEO_TYPE) * y + x;
      raw_pixels[offset] = c;
    }
  }
  SDL_UnlockSurface(surf_screen);
}


int ship_check_collision(SDL_Surface * surf_screen) {
  struct black_point_struct *p;
  RD_VIDEO_TYPE *raw_pixels;
  int bang = 0;

  SDL_LockSurface(surf_screen);
  raw_pixels = (RD_VIDEO_TYPE *) surf_screen->pixels;
  // When the shields are off, check that the black points 
  // on the ship are still black, and not covered up by rocks
  for(p = black_point; p < blackptr; p++) {
    int offset =
      surf_screen->pitch / sizeof(RD_VIDEO_TYPE) * (p->y + (int) shipdata.yship) + p->x + (int) shipdata.xship;
    if(raw_pixels[offset]) {
      // Set the bang flag, a collision with something happened
      bang = 1;
      break;
    }
  }
  SDL_UnlockSurface(surf_screen);
  return bang;
}

void ship_update_position(void) {
  SDL_Surface *surf_ship = get_current_sprite_surface(&ship_sprite, last_ticks);

  // FRICTION? In space? Oh well.
  shipdata.xvel *= powf(0.9, movementrate);
  shipdata.yvel *= powf(0.9, movementrate);
  // if (abs(shipdata.xvel)<0.00001) shipdata.xvel=0;
  // if (abs(shipdata.yvel)<0.00001) shipdata.yvel=0;

  // INERTIA
  shipdata.xship += shipdata.xvel * movementrate;
  shipdata.yship += shipdata.yvel * movementrate;

  // BOUNCE X  (okay, throwing all pretense of realism out the
  // window)
  if(shipdata.xship < 0 || shipdata.xship > xsize - surf_ship->w) {
    // BOUNCE from left and right wall
    shipdata.xship -= shipdata.xvel * movementrate;
    shipdata.xvel *= -0.99;
  }
  // BOUNCE Y
  if(shipdata.yship < 0 || shipdata.yship > ysize - surf_ship->h) {
    // BOUNCE from top and bottom wall
    shipdata.yship -= shipdata.yvel * movementrate;
    shipdata.yvel *= -0.99;
  }
}

void way_of_the_exploding_ship(void) {
  makebangdots(shipdata.xship, shipdata.yship, shipdata.xvel, shipdata.yvel, get_current_sprite_surface(&ship_sprite, last_ticks), 30, 3);
}
