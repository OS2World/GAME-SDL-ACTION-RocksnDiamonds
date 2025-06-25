#include "yellifish.h"
#include <assert.h>
#include <math.h>
#include "guru_meditation.h"
#include "SFont.h"
#include "globals.h"
#include "random_gen.h"
#include "datafun.h"
#include "sparkles.h"
#include "sound.h"

static sdl_surfaces_t *yellifish_tentacles;
static sdl_surfaces_t *yellifish_head;
static SDL_Surface *yellifish_head_explosion_map;

/*! \brief load yellifish parameter from IFF file
 *
 * The data is stored in the FORM.ROCK YLLI chunk. The chunk has the
 * following structure:
 *
 * 0000 UWORD	minimum level before yellifish appear
 * 0002 ULONG	yellifish probability * 2^32
 * 0006 UWORD	yellifish modulo, if level modulo this value is zero then no yellifish appear
 * 0008 UWORD	x-distance in pixel before yellifish attack
 * 000A UWORD	minimum tentacle length in pixel
 * 000C UWORD	maximum tentacle length in pixel
 * 000E ULONG	delta velocity * 2^32, e.g. 1173957729 = 0.2733333336655
 * 0012 ULONG	maximum velocity / 16 * 2^32
 * 0016 UWORD	head X-offset
 * 0018 UWORD	head Y-offset
 * 
 * \param yelli pointer to yellifish subsystem structure
 * \param iff iff context
 * \return NULL on error
 */
static yellifishsubsystem_t *load_yellifish_parameter(yellifishsubsystem_t *yelli, uiff_ctx_t iff) {
  int32_t chunksize;
  double d;

  chunksize = uiff_find_chunk_wflags(&iff, MakeID('Y', 'L', 'L', 'I'), IFF_FIND_REWIND);
  assert(printf("FORM.ROCK YLLI size = $%08X\n", chunksize));
  if(chunksize >= 26) { //Chunk found and is big enough.
    yelli->minimum_level = read16(iff.f);
    d = read32(iff.f);
    yelli->yelliprobability = d / UINT32_MAX;
    yelli->level_modulo = read16(iff.f);
    assert(printf("\tyelli->minimum_level=%d yelli->yelliprobability=%e yelli->level_modulo=%d\n",
		  yelli->minimum_level,
		  yelli->yelliprobability,
		  yelli->level_modulo));
    yelli->attack_distance = read16(iff.f);
    yelli->minimum_length = read16(iff.f);
    yelli->maximum_length = read16(iff.f);
    d = read32(iff.f);
    yelli->delta_velocity = d / UINT32_MAX;
    d = read32(iff.f);
    yelli->maximum_velocity = d * 16 / UINT32_MAX;
    assert(printf("\tyelli->delta_velocity=%e, yelli->maximum_velocity=%e\n", yelli->delta_velocity, yelli->maximum_velocity));
    yelli->headoffsetx = read16(iff.f);
    yelli->headoffsety = read16(iff.f);
    assert(printf("\tyelli->headoffsetx=%hd, headoffsety=%d\n", yelli->headoffsetx, yelli->headoffsety));
  } else {
    guru_alert(GM_FLAGS_DEADEND, GURU_SEC_yellifish | GM_SS_BootStrap | GM_GE_OpenRes, &load_yellifish_parameter);
    return NULL;
  }
  return yelli;
}

yellifishsubsystem_t *init_yellifish_subsystem(uiff_ctx_t iff) {
  yellifishsubsystem_t *yellifish_data_struct = NULL;

  yellifish_data_struct = calloc(1, sizeof(struct Yellifishsubsystem));
  if(!yellifish_data_struct) {
    guru_alert(GM_FLAGS_DEADEND, GURU_SEC_yellifish | GM_SS_BootStrap | GM_GE_NoMemory, &init_yellifish_subsystem);
  } else {
    yellifish_tentacles = load_images_ck("yellifishtentacle.%02hX.png", 0xFF, 0xFF, 0xFF);
#ifndef NDEBUG
    printf("yellifisch_tentacles = %p\n", yellifish_tentacles);
#endif
    if(!yellifish_tentacles) {
      guru_alert(GM_FLAGS_DEADEND, GURU_SEC_yellifish | GM_SS_Graphics | GM_GE_OpenRes, &yellifish_tentacles);
    } else {
      yellifish_head = load_images_no_convert("yellifish.%02hX.png", -1, -1, -1);
#ifndef NDEBUG
      printf("yellifish_head = %p\n", yellifish_head);
#endif
      if(!yellifish_head) {
	guru_alert(GM_FLAGS_DEADEND, GURU_SEC_yellifish | GM_SS_Graphics | GM_GE_OpenRes, &yellifish_head);
      }
      yellifish_head_explosion_map = load_image("yellifish.explosion_map.png", 0, 0xFF, 0);
      if(!yellifish_head_explosion_map) {
	guru_alert(GM_FLAGS_DEADEND, GURU_SEC_yellifish | GM_SS_Graphics | GM_GE_OpenRes, &yellifish_head_explosion_map);
      }
    }
  }
  return load_yellifish_parameter(yellifish_data_struct, iff);
}


void shutdown_yellifish_subsystem(yellifishsubsystem_t *yellifishs) {
  if(!yellifishs || !yellifish_tentacles || !yellifish_head || !yellifish_head_explosion_map) {
    if(guru_alert(GM_FLAGS_RECOVERY | GM_FLAGS_CHOICE, GURU_SEC_yellifish | GM_SS_RAM | GM_GE_BadParm, &shutdown_yellifish_subsystem) != 1) return;
  }
#ifndef NDEBUG
  printf("Freeing yellifish=%p, yellifish_tentacles=%p, yellifish_head=%p.\n", yellifishs, yellifish_tentacles, yellifish_head);
#endif
  free(yellifishs);
  destroy_sdl_surfaces(yellifish_tentacles);
  destroy_sdl_surfaces(yellifish_head);
  SDL_FreeSurface(yellifish_head_explosion_map);
}


void init_yellifish(yellifishsubsystem_t *ysys, yellifish_t *yelli, int posx, int posy) {
  int i, j;
  float spd;

  assert(yelli != NULL);
  memset(yelli, 0, sizeof(struct Yellifish));
  yelli->length = ysys->minimum_length;
  yelli->posx = posx;
  yelli->posy = posy;
  for(j = 0; j < YELLIFISH_TENTACLENO; ++j) {
    spd = rnd_next_float() * 2.1039 + 0.67779;
    if(rnd_next_float() < 0.41221221) spd *= -1;
    yelli->tspeed[j] = spd;
    for(i = 0; i < YELLIFISH_TENTACLESEGMENTS; ++i) {
      yelli->tentacleparts[j][i] = rnd_next_float() * yellifish_tentacles->num_surfaces;
      //printf("yelli->tentacleparts[%d][%d] = %d\n", j, i, (int)(yelli->tentacleparts[j][i]));
    }
  }
  yelli->active = 1;
  init_sprite2(&yelli->ysprite, yellifish_head, 217, last_ticks);
}


static SDL_Surface *yellifish_get_tentacle_surface(yellifish_t *yelli, int tentaclenum, int tentaclepart) {
  return yellifish_tentacles->surfaces[yelli->tentacleparts[tentaclenum][tentaclepart % YELLIFISH_TENTACLESEGMENTS]];
}

static SDL_Surface *yellifish_get_current_tentacle_surface(yellifish_t *yelli, int tentaclenum) {
  return yellifish_get_tentacle_surface(yelli, tentaclenum, yelli->partoffset[tentaclenum]);
}


yellifish_t *maybe_activate_yellifish(yellifishsubsystem_t *yelli, int level, int x, int y) {
  if((level > yelli->minimum_level) &&
     (level % yelli->level_modulo != 0)
     && (rnd() < yelli->yelliprobability * movementrate)) {
    /*1.81117733e-3*/
    return activate_yellifish(yelli, x, y);
  }
  return NULL;
}


yellifish_t *activate_yellifish(yellifishsubsystem_t *yelli, int x, int y) {
  yellifish_t *yptr = yelli->yellifishs;
  yellifish_t *yend = yptr + YELLIFISH_MAXNUM;

  while(yptr < yend) {
    if(!yptr->active) {
      init_yellifish(yelli, yptr, x, y);
#ifndef NDEBUG
      printf("yptr=%p (%d,%d)\n", yptr, x, y);
#endif
      return yptr;
    }
    ++yptr;
  }
  return NULL;
}


void display_yellifishs(yellifishsubsystem_t *yelli, SDL_Surface *surf_screen) {
  yellifish_t *yptr = yelli->yellifishs;
  yellifish_t *yend = yptr + YELLIFISH_MAXNUM;

  while(yptr < yend) {
    if(yptr->active) display_yellifish(yelli, yptr, surf_screen);
    ++yptr;
  }
}


void display_yellifish(yellifishsubsystem_t *ysys, yellifish_t *yelli, SDL_Surface *surf_screen) {
  SDL_Rect src;
  SDL_Surface *yellisurf;
  int day_of_the_tentacle; //Number of the current tentacle
  SDL_Rect dest;

  src.x = 0;
  src.w = 16;
  for(day_of_the_tentacle = 0; day_of_the_tentacle < YELLIFISH_TENTACLENO; ++day_of_the_tentacle) {
    float length = yelli->length;
    float offset = yelli->offset[day_of_the_tentacle];
    int tentaclepart = yelli->partoffset[day_of_the_tentacle];
    float posx = yelli->posx;
    float posy = yelli->posy;
    while(length > 0) {
      src.y = offset;
      src.h = length;
      yellisurf = yellifish_get_tentacle_surface(yelli, day_of_the_tentacle, tentaclepart);
      dest.x = posx;
      dest.y = posy;
      SDL_BlitSurface(yellisurf, &src, surf_screen, &dest);
      posy += yellisurf->h - offset;
      length -= yellisurf->h - offset;
      offset = 0;
      tentaclepart++;
    }
    yelli->offset[day_of_the_tentacle] += yelli->tspeed[day_of_the_tentacle] * movementrate;
    if(yelli->offset[day_of_the_tentacle] >= yellifish_get_current_tentacle_surface(yelli, day_of_the_tentacle)->h) {
      yelli->offset[day_of_the_tentacle] -= yellifish_get_current_tentacle_surface(yelli, day_of_the_tentacle)->h;
      yelli->partoffset[day_of_the_tentacle]++;
    } else if(yelli->offset[day_of_the_tentacle] < 0) {
      --yelli->partoffset[day_of_the_tentacle];
      yelli->offset[day_of_the_tentacle] += yellifish_get_current_tentacle_surface(yelli, day_of_the_tentacle)->h;
    }
  }
  draw_sprite(&yelli->ysprite, yelli->posx - ysys->headoffsetx, yelli->posy - ysys->headoffsety, surf_screen, last_ticks);
}


void update_yellifishs(yellifishsubsystem_t *yelli, float preyx, float preyy) {
  yellifish_t *yptr = yelli->yellifishs;
  yellifish_t *yend = yptr + YELLIFISH_MAXNUM;
#ifdef DEBUG
  //char buf[80];
#endif

  while(yptr < yend) {
    if(yptr->active > 0) {
      yptr->posx += yptr->velx * movementrate;
      if(yptr->posx < preyx) yptr->velx += yelli->delta_velocity * movementrate;
      else if(yptr->posx > preyx) yptr->velx -= yelli->delta_velocity * movementrate;
      if(yptr->velx > yelli->maximum_velocity) yptr->velx *= .892444;
      yptr->posy += yptr->vely * movementrate;
      if(yptr->posy < preyy) yptr->vely += yelli->delta_velocity;
      else if(yptr->posy > preyy) yptr->vely -= yelli->delta_velocity;
      if(yptr->vely > yelli->maximum_velocity) yptr->vely *= .892444;
      switch(yptr->tentacle_active) {
      case 1:
	yptr->length += movementrate * 4.141;
	if(yptr->length > yelli->maximum_length) {
	  yptr->tentacle_active = -1;
	}
	break;
      case -1:
	yptr->length -= movementrate * 4.171;
	if(yptr->length < yelli->minimum_length) {
	  yptr->length = yelli->minimum_length;
	  yptr->tentacle_active = 0;
	}
	break;
      case 0:
	if(fabsf(yptr->posx - preyx) < yelli->attack_distance) {
	  yptr->tentacle_active = 1;
	  play_sound(10);
	}
	break;
      default:
	guru_alert(GM_FLAGS_RECOVERY, GURU_SEC_yellifish | GM_SS_Misc | GURU_SEC_yellifish | GM_GE_BadParm, &update_yellifishs);
      }
    } else if(yptr->active < 0) {
      yptr->vely += yelli->delta_velocity * 2;
      yptr->posx += yptr->velx * movementrate;
      yptr->posy += yptr->vely * movementrate;
      //yptr->velx *= .9589131313;
      yptr->length *= .9589131313;
      if(yptr->posy > ysize) yptr->active = 0;
    }
    yptr++;
  }
#ifdef DEBUG
  //yptr = yelli->yellifishs;
  /* snprintf(buf, sizeof(buf), "%f %f, %f %f", yptr->posx, yptr->posy, yptr->velx, yptr->vely); */
  //printf("%14.7e %14.7e, %14.7e %14.7e\n", yptr->posx, yptr->posy, yptr->velx, yptr->vely);
  /* PutString(surf_screen, 10, 480, buf); */
  /* SDL_Flip(surf_screen); */
#endif
}

yellifish_t *check_yellifish_hit(yellifishsubsystem_t *yelli, float minlaserx, float maxlaserx, float lasery) {
  yellifish_t *yptr;

  for(yptr = yelli->yellifishs; yptr < yelli->yellifishs + YELLIFISH_MAXNUM; ++yptr) {
    if(yptr->active > 0) {
      if((minlaserx < yptr->posx && yptr->posx < maxlaserx) &&
	 (lasery >= yptr->posy && lasery < yptr->posy + yellifish_head->surfaces[0]->h)) {
	yptr->active = -1;
	return yptr;
      }
    }
  }
  return NULL;
}

void kill_yellifish(yellifish_t *yptr) {
#ifdef DEBUG
  printf("killing yptr=%p\n", yptr);
#endif
  assert(yellifish_head_explosion_map);
  makebangdots(yptr->posx - 4, yptr->posy - 8, yptr->velx, yptr->vely,
	       yellifish_head_explosion_map, 39,
	       1);
  play_sound(11);
}

void deactivate_all_yellifish(yellifishsubsystem_t *yelli) {
  yellifish_t *yptr;

  for(yptr = yelli->yellifishs; yptr < yelli->yellifishs + YELLIFISH_MAXNUM; ++yptr) {
    yptr->active = 0;
  }
}
