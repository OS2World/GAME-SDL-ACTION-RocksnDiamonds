#include <assert.h>
#include <math.h>
#include "sparkles.h"
#include "random_gen.h"
#include "globals.h"
#include "guru_meditation.h"
#include "u-iff.h"

/*
 * Sparkles are small colour changing dots which are painted directly
 * onto a surface. Important information like the colours defined are
 * read from a IFF FORM ROCK file.
 *
 * FORM ROCK		Rockdodger iff file
 * . FORM SPRK		sparkles form
 * . . MDOT		maximum number of dots chunk
 * . . CMAP		colour map chunk (as known from ILBM files)
 *			must occur at least two times
 *
 * MDOT chunk
 * ----------
 *
 * uint32	maximum number of dots/sparkles
 * uint16	further flags for sparkles engine (= 0)
 *		  (bit 0 = 1: debug output on overflow)
 */

#ifdef DEBUG
#include <stdio.h>
static unsigned long max_bangdots_used = 0;
#endif

sparkle_life_colours_t hot_colours;
sparkle_life_colours_t cool_colours;
static sparkle_t *sparkles;
static sparkle_t *sparkles_end;
uint32_t max_bang_dots = MAX_BANG_DOTS;
uint16_t mdot_flags = 0;

sparkle_t *init_sparkles(uiff_ctx_t iff) {
  int32_t size;
  int32_t cidx;
  sparkle_life_colours_t splcls;
  sparkle_t *ret = NULL;
  int index = -1;

  //This destroys the old context group-information but we do not need it any longer anyway.
  size = uiff_find_group_ctx(&iff, IFF_FIND_REWIND, FORM, MakeID('S', 'P', 'R', 'K'));
  assert(printf("FORM.ROCK FORM.SPRK size = $%08X\n", size));
  if(size >= 0) {
    if((size = uiff_find_chunk_ctx(&iff, MakeID('M', 'D', 'O', 'T'))) >= 4) {
      //Chunk found and it is long enough...
      max_bang_dots = read32(iff.f);
      assert(printf("FORM.ROCK FORM.SPRK MDOT size = $%08X mdot = $%08X\n", size, max_bang_dots));
    }
    if(size >= 6) {
      //Chunk found and it is long enough...
      mdot_flags = read16(iff.f);
      assert(printf("FORM.ROCK FORM.SPRK MDOT size = $%08X mdot_flags = $%04hX\n", size, mdot_flags));
    }
    uiff_rewind_group(&iff);
    index = 0;
    while((size = uiff_find_chunk_ctx(&iff, MakeID('C', 'M', 'A', 'P'))) > 0) {
      assert(printf("FORM.ROCK FORM.SPRK CMAP size = $%08X\n", size));
      if(size % 3 != 0) return NULL;
      splcls.cl_len = size / 3;
      if((splcls.clist = malloc((splcls.cl_len + 1/*Space for duplicate*/) * sizeof(SDL_Color))) == NULL) return NULL;
      for(cidx = 0; cidx < splcls.cl_len && ftell(iff.f) < iff.ckend; ++cidx) {
	splcls.clist[cidx].r = fgetc(iff.f);
	splcls.clist[cidx].g = fgetc(iff.f);
	splcls.clist[cidx].b = fgetc(iff.f);
      }
      assert(printf("splcls.cl_len = $%x splcls.clist = %p\n", splcls.cl_len, splcls.clist));
      splcls.clist[splcls.cl_len] = splcls.clist[splcls.cl_len - 1];
#ifdef DEBUG
      int i;
      for(i = 0; i < splcls.cl_len; ++i) {
	printf("\t $%02x $%02x $%02x\n", (unsigned)splcls.clist[i].r, (unsigned)splcls.clist[i].g, (unsigned)splcls.clist[i].b);
      }
#endif
      switch(index++) {
      case 0:
	hot_colours = splcls;
	break;
      case 1:
	cool_colours = splcls;
	break;
      default:
	break; //We are OK
      }
    }
  }
  if(index < 2) return NULL; /* We need at least two colour maps. */
  if(!(sparkles = malloc(sizeof(sparkle_t) * max_bang_dots))) {
    perror("Can not allocate sparkles");
    return NULL;
  } else sparkles_end = sparkles;
  ret = sparkles;
  assert(printf("ret = %p\n", ret));
  return ret;
}

Uint32 get_life_colour(float life, sparkle_life_colours_t *fcols, SDL_Surface *surf) {
  int index;
  float mixing;
  Uint8 r, g, b;

  assert(fcols->cl_len > 0);
  if(life < 0 || life > 1) {
#ifdef DEBUG
    fprintf(stderr, "life=%e, fcols=%p, surf=%p\n", (double)life, fcols, surf);
#endif
    guru_meditation(GM_FLAGS_RECOVERY, GM_SS_Graphics | GM_GE_BadParm | GURU_SEC_bangdots, &get_life_colour);
    life = life < 0 ? 0 : 1;
  }
  index = floorf(life * fcols->cl_len);
  mixing = 1 - (fcols->cl_len * life - index);
  //Now you hopefully see, why we substracted one...
  r = mixing * fcols->clist[index].r + (1 - mixing) * fcols->clist[index + 1].r;
  g = mixing * fcols->clist[index].g + (1 - mixing) * fcols->clist[index + 1].g;
  b = mixing * fcols->clist[index].b + (1 - mixing) * fcols->clist[index + 1].b;
  //fprintf(stderr, "life=%e, fcols=%p, surf=%p index=%d mixing=%e r=%x g=%x b=%x\n", (double)life, fcols, surf, index, (double)mixing, (int)r, (int)g, (int)b);
  return SDL_MapRGB(surf->format, r, g, b);
}

/*
 * \return pointer to new sparkle
 */
sparkle_t *create_colour_sparkle(float x, float y, float dx, float dy, float decay, RD_VIDEO_TYPE col) {
#ifdef DEBUG
  unsigned long nmax;
  nmax = sparkles_end - sparkles;
  if(nmax > max_bangdots_used) {
    max_bangdots_used = nmax;
    printf("new: max_bangdots_used = $%lX\n", max_bangdots_used);
  }
#endif
  if(sparkles_end - sparkles >= max_bang_dots) {
    guru_meditation(GM_FLAGS_GREEN, GM_SS_Intuition | GM_GE_ProcCreate | GURU_SEC_bangdots, &create_colour_sparkle);
  } else {
    sparkles_end->x = x;
    sparkles_end->y = y;
    sparkles_end->dx = dx;
    sparkles_end->dy = dy;
    sparkles_end->life = 1.0;
    sparkles_end->decay = decay;
    sparkles_end->col = col;
    sparkles_end->spty = 0;
    return sparkles_end++;
  }
  return NULL;
}

/*
 * \return pointer to new sparkle
 */
sparkle_t *create_sparkle(float x, float y, float dx, float dy, float decay, sparkle_life_colours_t *col) {
#ifdef DEBUG
  unsigned long nmax;
  nmax = sparkles_end - sparkles;
  if(nmax > max_bangdots_used + 0x100) {
    max_bangdots_used = nmax;
    printf("new: max_bangdots_used = $%lX\n", max_bangdots_used);
  }
  //In debug mode always guru meditate.
  mdot_flags |= 1;
#endif
  assert(hot_colours.clist != NULL);
  assert(cool_colours.clist != NULL);
  if(sparkles_end - sparkles >= max_bang_dots) {
    if((mdot_flags & 0x0001) != 0) {
      guru_meditation(GM_FLAGS_GREEN, GM_SS_Intuition | GM_GE_ProcCreate | GURU_SEC_bangdots, &create_sparkle);
    }
  } else {
    assert(sparkles_end < sparkles + max_bang_dots);
    sparkles_end->x = x;
    sparkles_end->y = y;
    sparkles_end->dx = dx;
    sparkles_end->dy = dy;
    sparkles_end->life = 1.0;
    sparkles_end->decay = decay;
    sparkles_end->fading = col;
    sparkles_end->spty = 1;
    return sparkles_end++;
  }
  return NULL;
}


void draw_sparkle(sparkle_t *sparkle, SDL_Surface *surf) {
  int x = sparkle->x;
  int y = sparkle->y;
  RD_VIDEO_TYPE col;
  RD_VIDEO_TYPE *rawpixel = (RD_VIDEO_TYPE *) surf->pixels;

  switch(sparkle->spty) {
  case 0:
    col = sparkle->col;
    break;
  case 1:
    col = get_life_colour(sparkle->life, sparkle->fading, surf);
    sparkle->oldfadecol = col;
    break;
  default:
    col = guru_meditation(GM_FLAGS_RECOVERY | GM_FLAGS_AUTOTIMEOUT | GM_FLAGS_ABORTIFY | GM_FLAGS_CHOICE, GM_SS_Graphics | GM_GE_BadParm | GURU_SEC_bangdots, &draw_sparkle);
  }
  rawpixel[surf->pitch / sizeof(RD_VIDEO_TYPE) * y + x] = col;
}


void update_sparkles(SDL_Surface *surf) {
  Uint32 mycol, bgpixel;
  sparkle_t *sptr;
  sparkle_t *target;
  RD_VIDEO_TYPE *rawpixel;

  SDL_LockSurface(surf);
  rawpixel = (RD_VIDEO_TYPE *) surf->pixels;
  for(sptr = sparkles, target = sparkles; sptr < sparkles_end; ++sptr) {
    sptr->life = sptr->life - sptr->decay * movementrate;
    if(sptr->life < 0) continue; //It's dead, Jim.
    if(sptr->life < 0.87) {
      bgpixel = rawpixel[(int) (surf->pitch / sizeof(RD_VIDEO_TYPE) * (int) (sptr->y)) + (int) (sptr->x)];
      switch(sptr->spty) {
      case 0:
	mycol = sptr->col;
	break;
      case 1:
	//Use last generated one...
	mycol = sptr->oldfadecol;
	break;
      default:
	mycol = -1;
      }
      if(bgpixel != mycol) {
	// Continue and do not copy
	continue;
      }
    }
    sptr->x += sptr->dx * movementrate;
    sptr->y += sptr->dy * movementrate;
    if(sptr->x <= 0 || sptr->x >= xsize || sptr->y <= 0 || sptr->y >= ysize) {
      // If the dot has drifted outside the perimeter, kill it
      continue;
    }
    *target++ = *sptr;
  }
  SDL_UnlockSurface(surf);
  sparkles_end = target; //This points after the last one
}


void makebangdots(int xbang, int ybang, int xvel, int yvel, SDL_Surface * s,
		  int power, int amtdots) {
  int x, y, endcount;
  RD_VIDEO_TYPE *rawpixel, c;
  double theta, r;
  float sx, sy, dx, dy;
  int begin_generate;
  sparkle_t *sptr;

  begin_generate = SDL_GetTicks();
  SDL_LockSurface(s);
  rawpixel = (RD_VIDEO_TYPE *) s->pixels;
  endcount = 0;
  while(endcount < amtdots) {
    for(x = 0; x < s->w; x++) {
      for(y = 0; y < s->h; y++) {
	c = rawpixel[s->pitch / sizeof(RD_VIDEO_TYPE) * y + x];
	if(c && c != SDL_MapRGB(s->format, 0, 255, 0)) {
	  theta = rnd() * M_PI * 2;
	  r = 1 - (rnd() * rnd());
	  sx = x + xbang;
	  sy = y + ybang;
	  dx = (power / 50.0) * 45.0 * cos(theta) * r + xvel;
	  dy = (power / 50.0) * 45.0 * sin(theta) * r + yvel;
	  // Replace the last few bang dots with the pixels from the exploding object
	  if(endcount > 0) {
	    sptr = create_colour_sparkle(sx, sy, dx, dy, (rnd() * 3 + 1) / 100.0, c);
	  } else {
	    sptr = create_sparkle(sx, sy, dx, dy, (rnd() * 3 + 1) / 100.0, &hot_colours);
	  }
	  if(sptr == NULL) goto exitloop;
	}
      }
    }
    if(SDL_GetTicks() - begin_generate > 3)
      endcount++;
  }
exitloop:
  SDL_UnlockSurface(s);
}


void draw_sparkles(SDL_Surface * s) {
  sparkle_t *sptr;

  for(sptr = sparkles; sptr < sparkles_end; ++sptr) {
    draw_sparkle(sptr, s);
  }
}


void reset_sparkles(void) {
  sparkles_end = sparkles;
}


void create_plumedots(int num, short x, short y, short heading, short angle, float speed) {
  sparkle_t *sptr;
  float fheading;
  float speed_;

  /*
   * The -sinf is needed as the Y coordinate increases from top to
   * bottom and we would like to have the angles in the mathematical
   * sense. So 0 is to the right, 90 up, 180 left and 270 down.
   */
  while(--num > 0 && sparkles_end - sparkles < max_bang_dots) {
    fheading = heading + (rnd() - 0.5) * angle;
    fheading *= 2 * M_PI / 360.0;
    speed_ = speed * (0.3 + 0.8 * rnd());
    sptr = create_sparkle(x + (rnd() - 0.5) * 7,
			  y + (rnd() - 0.5) * 7,
			  cosf(fheading) * speed_,
			  -sinf(fheading) * speed_,
			  (rnd() * 3 + 1) / 100.0,
			  &cool_colours);
    if(sptr == NULL) break;
  }
}

