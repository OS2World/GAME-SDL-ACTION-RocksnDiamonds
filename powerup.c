#include <assert.h>
#include "powerup.h"
#include "datafun.h"
#include "guru_meditation.h"
#include "random_gen.h"
#include "globals.h"

static struct PowerupInformationForIni {
  const char *pname;
  const char *pfilename;
  short ckr, ckg, ckb; // colour keys
} powerupiniinfo[] = {
  { "NONE", NULL },
  { "Laserpowerups", "laserpowerup.%02X.png", 0, 0, 255 },
  { "Shieldpowerups", "shieldpowerup.%02X.png", 0, 0, 255 },
  { "Lifepowerups", "lifepowerup.%02X.png", 0, 255, 0 }
};

/*! \brief array with power surfaces
 *
 * Here we store all surfaces for the powerup animations. The order is
 * the same as the powerup numbers.
 */
sdl_surfaces_t *surf_powerups[POWERUP_MAXPOWERUP];

/*! \brief powerup state data
 */
static powerup_t powerup;


powerup_types_t get_current_powerup(void) {
  return powerup.active;
}


powerup_t *init_powerup() {
  int ip;
  struct PowerupInformationForIni *iniptr;

  for(ip = POWERUP_NONE + 1; ip < POWERUP_MAXPOWERUP; ++ip) {
    iniptr = powerupiniinfo + ip;
    printf("%s:\n", iniptr->pname);
    surf_powerups[ip] = load_images_ck(iniptr->pfilename, iniptr->ckr, iniptr->ckg, iniptr->ckb);
    if(surf_powerups[ip] == NULL) {
#ifndef DEBUG
      guru_meditation(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError | GURU_SEC_powerup, iniptr);
#endif
      return NULL;
    }
  }
  deactivate_powerup();
#ifdef DEBUG
  printf("surf_powerups[POWERUP_LASER]=%p surf_powerups[POWERUP_SHIELD]=%p surf_powerups[POWERUP_LIFE]=%p\n", surf_powerups[POWERUP_LASER], surf_powerups[POWERUP_SHIELD], surf_powerups[POWERUP_LIFE]);
#endif
  return &powerup;
}

void shutdown_powerups() {
  int ip;

  deactivate_powerup();
  for(ip = POWERUP_NONE + 1; ip < POWERUP_MAXPOWERUP; ++ip) {
    assert(surf_powerups[ip]);
    destroy_sdl_surfaces(surf_powerups[ip]);
#ifdef DEBUG
    printf("Free '%s' at %p.\n", powerupiniinfo[ip].pname, surf_powerups[ip]);
#endif
  }
}

SDL_Surface *get_powerup_surface() {
  SDL_Surface *source;
  int pownum;
  sdl_surfaces_t *surfaces = NULL;

  pownum = get_current_powerup();
  if(pownum <= POWERUP_NONE || pownum >= POWERUP_MAXPOWERUP) {
    guru_meditation(GM_FLAGS_DEADEND | GM_FLAGS_ABORTIFY, GM_SS_Graphics | GM_GE_BadParm | GURU_SEC_powerup, &get_powerup_surface);
    assert(0);
    return NULL;
  }
  surfaces = surf_powerups[pownum];
  assert(surfaces);
  source = surfaces->surfaces[(unsigned short)powerup.img_ctr % surfaces->num_surfaces];
  assert(source);
  return source;
}

void display_powerup(SDL_Surface * surf_screen) {
  SDL_Rect dest;
  SDL_Surface *source;

  dest.w = 0; //powersurf->w;
  dest.h = 0; //powersurf->h;
  dest.x = powerup.x;
  dest.y = powerup.y;
  source = get_powerup_surface();
  SDL_BlitSurface(source, NULL, surf_screen, &dest);
}

void deactivate_powerup() {
  powerup.active = POWERUP_NONE;
  powerup.countdown = POWERUPDELAY;
  powerup.img_ctr = 0;
}

void update_powerup() {
  if(powerup.active != POWERUP_NONE) {
    SDL_Surface *surf = get_powerup_surface();
    powerup.x += powerup.dx * movementrate;
    powerup.y += powerup.dy * movementrate;
    if(powerup.x < -surf->w
       || powerup.y < -surf->h
       || powerup.x > xsize
       || powerup.y > ysize) {
      deactivate_powerup();
    }
  } else {
    powerup.countdown -= movementrate;
    if(powerup.countdown < 0 && rnd() < .3) {
      powerup.x = xsize;
      powerup.dx = -(3 + rnd() * 5);
      powerup.dy = .5 + rnd() * 3;
      if((powerup.y = rnd() * ysize) >= ysize / 2)
	powerup.dy = -powerup.dy;
      if(rnd() < 0.051807) {
	powerup.active = POWERUP_LIFE;
      } else if(rnd() < .7) {
	powerup.active = POWERUP_LASER;
      } else {
	powerup.active = POWERUP_SHIELD;
      }
      assert(powerup.active != POWERUP_NONE);
    }
  }
  powerup.img_ctr += movementrate;
}
