#include "rocks.h"
#include "sparkles.h"
#include "datafun.h"
#include "greeblies.h"
#include "guru_meditation.h"
#include "random_gen.h"
#include "globals.h"
#include "sound.h"
#include <assert.h>
#include <math.h>

float rockrate; //!< How many rocks are created per time unit?
float rockspeed; //!< How fast are the rocks?
float countdown = 0; //!< countdown before next rock creation

struct rock_struct rock[MAX_ROCKS];

sdl_surfaces_t *surf_rocks = NULL; //!< All the rocky asteroid surfaces.
sdl_surfaces_t *surf_dead_rocks = NULL; //!< All the dead asteroid surfaces.
sdl_surfaces_t *surf_lithiumrocks = NULL; //!< All the heavy-duty asteroid surfaces.
sdl_surfaces_t *surf_dead_lithiumrocks = NULL; //!< All the dead/hot heavy-duty asteroid surfaces.
sdl_surfaces_t *surf_icerocks = NULL; //!< All the ice asteroid surfaces.
sdl_surfaces_t *surf_dead_icerocks = NULL; //!< All the dead/hot ice asteroid surfaces.


SDL_Surface *get_rock_surface(int rockidx, int hot) {
  sdl_surfaces_t *surfs;
  unsigned short imgidx = rock[rockidx].type_number;

  assert(rock[rockidx].type_class <= 2);
  switch(rock[rockidx].type_class) {
  case 2:
    surfs = hot == 1 ? surf_dead_icerocks : surf_icerocks;
    break;
  case 1:
    surfs = hot == 1 ? surf_dead_lithiumrocks : surf_lithiumrocks;
    break;
  default:
    //Defaults to normal rocks so that the game does (somehow) function.
    surfs = hot == 1 ? surf_dead_rocks : surf_rocks;
    break;
  }
  if(imgidx >= surfs->num_surfaces) {
    Uint32 addr = rockidx;
    if(hot) {
      addr |= 0x8000;
    }
    addr |= (imgidx & 0xFF) << 24;
    addr |= (surfs->num_surfaces & 0xFF) << 16;
    guru_meditation(GM_FLAGS_RECOVERY | GM_FLAGS_CHOICE | GM_FLAGS_ABORTIFY, GM_SS_Misc | GM_GE_BadParm | GURU_SEC_rocks, (void *)(unsigned long)addr);
    imgidx = 0;
  }
  return surfs->surfaces[imgidx];
}


void heat_rock_up(int rockidx, float mvmt) {
  struct rock_struct *rptr;
  float plume_phi;
  
  if(rockidx < 0 || rockidx >= MAX_ROCKS) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Misc | GM_GE_BadParm | GURU_SEC_rocks, (void *)(long)rockidx);
  }
  rptr = rock + rockidx;
  rock[rockidx].heat += mvmt / rock[rockidx].heat_capacity;
  if(rock[rockidx].type_class == 2) {
    SDL_Surface *image = get_rock_surface(rockidx, 0);
    float size = (image->w + image->h) / 2.0;
    create_plumedots(9, rptr->x + image->w / 2, rptr->y + image->h / 2, rptr->plume_angle, 2, 17.27);
    plume_phi = (rptr->plume_angle + 180) / 360.0 * 2 * M_PI;
    rptr->xvel += 1.442 * cosf(plume_phi) / size;
    rptr->yvel += 1.442 * -sinf(plume_phi) / size;
  }
}


void create_rock() {
  // rockspeed current default speed of the rock, modified by a random amount
  struct rock_struct *rockptr;

  for(rockptr = rock; rockptr < rock + MAX_ROCKS; ++rockptr) {
    if(!rockptr->active && (rockptr->greeb == 0 || the_greeblies[rockptr->greeb].landed)) {
      rockptr->x = (float) xsize;
      rockptr->y = rnd() * ysize;
      rockptr->xvel = -(rockspeed) * (1 + rnd());
      rockptr->yvel = (rockspeed / 3.0) * (rnd() - 0.5);
      rockptr->heat = 0;
      rockptr->active = 1;
      rockptr->greeb = 0; //TODO: really zero?

      if(level > 3 && rnd() < 0.0713645) { //Start with third level...
	rockptr->type_class = 2;
	rockptr->type_number = random() % surf_icerocks->num_surfaces;
	rockptr->heat_capacity = 1.0 / 3;
	rockptr->plume_angle = 360 * rnd();
      } else if(level > 5 && rnd() < 0.068073) { //Start with fifth level...
	rockptr->type_class = 1;
	rockptr->type_number = random() % surf_lithiumrocks->num_surfaces;
	rockptr->heat_capacity = 34.0 / 49.0;
      } else {
	rockptr->type_class = 0;
	rockptr->type_number = random() % MAX_ROCK_IMAGESS;
	rockptr->heat_capacity = 1.0 / 3;
      }
      break; //Done with the loop!
    }
  }
#ifdef DEBUG
  //printf("create_rock(): rock=%p MAXrock=%p rockptr=%p\n", rock, rock + MAX_ROCKS, rockptr);
#endif
}

static void way_of_the_exploding_rock(struct rock_struct *rockp, int last_rock, SDL_Surface *image, SDL_Surface *surf_screen) {
  int g;
  short x, y;

  rockp->active = 0;
  play_sound(1 + (int) (rnd() * 3));
  switch(rockp->type_class) {
  case 2: //Ice rocks are different
    x = rockp->x + image->w / 2;
    y = rockp->y + image->h / 2;
    create_plumedots(1659, x, y, 180, 360, 10.9312);
    create_plumedots(539, x, y, 180, 360, 2.73002);
    create_plumedots(367, x, y, 90, 109, 4.656);
    create_plumedots(367, x, y, 270, 109, 4.656);
    create_plumedots(467, x + rockp->xvel, y + rockp->yvel, 90, 109, 1.7656);
    create_plumedots(467, x + rockp->xvel, y + rockp->yvel, 270, 109, 1.7656);
    create_plumedots(426, x - rockp->xvel, y - rockp->yvel, 90, 109, 1.37656);
    create_plumedots(426, x - rockp->xvel, y - rockp->yvel, 270, 109, 1.37656);
    break;
  default:
    makebangdots(rockp->x, rockp->y, rockp->xvel, rockp->yvel,
		 image, 10, 3);
  }

  // If a greeblie was going to this rock
  if((g = rockp->greeb)) {
    // ...and had landed, then the greeblie explodes too.
    if(the_greeblies[g].landed)
      kill_greeb(g);
    else {
      // If the greeblie is not yet landed, then it must now
      // find another rock to jump to. Choose the last active
      // rock found.
      rockp->greeb = 0;
      rock[last_rock].greeb = g;
      the_greeblies[g].target_rock_number = last_rock;
    }
  }
#ifdef DEBUG
  printf("explode rock: %3ld %9.2e %9.2e %9.2e %9.2e %04X %04X %9.2e %9.2e %04x %04X\n", rockp - rock, rockp->x, rockp->y, rockp->xvel, rockp->yvel, rockp->type_class, rockp->type_number, rockp->heat, rockp->heat_capacity, rockp->greeb, rockp->plume_angle);
#endif
}

void display_rocks(SDL_Surface *surf_screen) {
  int i;
  static int last_rock = 1;
  SDL_Rect dest;

  dest.w = 0;
  dest.h = 0;
  for(i = 0; i < MAX_ROCKS; i++) {
    struct rock_struct *rcki = &rock[i];
    if(rcki->active) {
      SDL_Surface *image = get_rock_surface(i, 0);
      if(!rcki->greeb)
	last_rock = i;
      dest.x = (int) rcki->x;
      dest.y = (int) rcki->y;

      // Draw the rock
      SDL_BlitSurface(image, NULL, surf_screen, &dest);

      // Draw the heated part of the rock, in an alpha which reflects the
      // amount of heat in the rock.
      if(rcki->heat > 0) {
	SDL_Surface *deadrock = get_rock_surface(i, 1);
	SDL_SetAlpha(deadrock, SDL_SRCALPHA, rcki->heat * 255 / image->h);
	dest.x = (int) rcki->x;	// kludge
	SDL_BlitSurface(deadrock, NULL, surf_screen, &dest);
	if(rnd() < 0.3)
	  rcki->heat -= movementrate;
      }
      // If the rock is heated past a certain point, the water content of
      // the rock flashes to steam, releasing enough energy to destroy
      // the rock in spectacular fashion.
      if(rcki->heat > image->h) {
	way_of_the_exploding_rock(rcki, last_rock, image, surf_screen);
      }
    }
    // Draw any greeblies attached to (or going to) the rock, whether the rock is active or not.
    if(rcki->greeb && the_greeblies[rcki->greeb].target_rock_number == i)
      display_greeb(&the_greeblies[rcki->greeb], surf_screen);
  }
}


void update_rocks(void) {
  int i;
  for(i = 0; i < MAX_ROCKS; i++)
    if(rock[i].active) {
      rock[i].x += rock[i].xvel * movementrate;
      rock[i].y += rock[i].yvel * movementrate;
      if(rock[i].x < -52.0)
	rock[i].active = 0;
    }
}


void *init_rocks(void) {
  int i;
  sdl_surfaces_t *surfs[] = { surf_rocks, surf_dead_rocks, surf_lithiumrocks, surf_dead_lithiumrocks, surf_icerocks, surf_dead_icerocks, NULL };  
  sdl_surfaces_t **ptrs;

  assert(surf_rocks == NULL && surf_dead_rocks == NULL);
  for(ptrs = surfs; *ptrs != NULL; ++ptrs) {
    if(!(*ptrs == NULL)) {
      guru_meditation(GM_FLAGS_RECOVERY | GM_FLAGS_ABORTIFY | GM_FLAGS_CHOICE, GM_SS_BootStrap | GM_GE_BadParm | GURU_SEC_rocks, &init_rocks);
    }
  }
  if((surf_rocks = load_images_ck("rock%hd.bmp", 0, 255, 0)) == NULL) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_rocks, (void*)0x524f434b);
  }
  if((surf_dead_rocks = load_images_ck("deadrock%hd.bmp", 0, 255, 0)) == NULL) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_rocks, (void*)~0x524f434b);
  }
  if((surf_lithiumrocks = load_images_ck("lithiumrock.%02hX.png", 0, 255, 0)) == NULL) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_rocks, (void*)0x4c495448);
  }
  if((surf_dead_lithiumrocks = load_images_ck("deadlithiumrock.%02hX.png", 0, 255, 0)) == NULL) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_rocks, (void*)~0x4c495448);
  }
  if((surf_icerocks = load_images_ck("icerock.%02hX.png", 0, 255, 0)) == NULL) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_rocks, (void*)0x494345);
  }
  if((surf_dead_icerocks = load_images_ck("deadicerock.%02hX.png", 0, 255, 0)) == NULL) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_rocks, (void*)~0x494345);
  }
  if(!surf_rocks || !surf_dead_rocks ||
     !surf_lithiumrocks || !surf_dead_lithiumrocks ||
     !surf_icerocks || !surf_dead_icerocks
     ) {
    return NULL;
  }
  for(ptrs = surfs; *ptrs != NULL; ++ptrs) {
    assert((*ptrs)->num_surfaces == (*(ptrs + 1))->num_surfaces);
    if(!((*ptrs)->num_surfaces == (*(ptrs + 1))->num_surfaces))
      guru_meditation(GM_FLAGS_RECOVERY | GM_FLAGS_ABORTIFY | GM_FLAGS_CHOICE, GM_SS_Misc | GM_GE_BadParm | GURU_SEC_rocks, *ptrs);
    ptrs = ptrs + 2;
  }
  assert(surf_rocks->num_surfaces >= MAX_ROCK_IMAGESS);
  for(i = 0; i < MAX_ROCKS; ++i) {
    rock[i].active = 0;
    rock[i].greeb = 0;
  }
  return surf_rocks;
}


void reset_rocks(void) {
  unsigned int i;

  rockspeed = 5.0 * xsize / 640.0;
  for(i = 0; i < MAX_ROCKS; i++)
    rock[i].active = 0;
  rockrate = 0.0;
  countdown = 1.111;
}


void ingame_maybe_rock() {
  if(state != GAME_PAUSED) {
    rockspeed = 4.885 * xsize / 640.0 + sqrtf(floorf(level)) / 5.0;
    rockrate = 0.13182 + sqrtf(floorf(level)) / 18.0;
    if(level - (int) level < 0.8) {
      if(state != GAME_PLAY && state != DEMO) {
	countdown -= 0.3 * movementrate;
      } else {
	countdown -= rockrate * movementrate;
      }
      while(countdown < 0) {
	countdown++;
	create_rock();
      }
    }
  }
}
