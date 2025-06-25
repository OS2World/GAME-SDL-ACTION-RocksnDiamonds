#include "globals.h"
#include "blubats.h"
#include "sparkles.h"
#include "guru_meditation.h"
#include "highscore_io.h"
#include "random_gen.h"
#include "ship.h"
#include "sprite.h"
#include "sound.h"
#include <SDL/SDL_image.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#ifdef DEBUG
#include <stdio.h>
#endif

//! Maximum number of blubat droppingss
#define MAX_BLUBAT_DROPPINGS 24
// Maximum number of possible doctrines
#define MAX_DOCTRINES 16
// Maximum length of any doctrine (to help detect runaway arguments)
#define MAX_DOCTRINE_LENGTH 8

static float blubat_attack_prob = 0.99862;	//!< 1 - attack probability
static int max_blubats = 7; //!< maximum number of blubats

/*! \brief positions for "spline" in doctrine
 */
struct doctrine_pos {
  float x[4];			//!< The X positions
  float y[4];			//!< The Y positions
};

/*! \brief a doctrine
 *
 * A doctrine consists of doctrine positions and a size.
 */
struct doctrine {
  struct doctrine_pos *positions;	//!< pointer to position array
  unsigned size;		//!< number of elements
};

struct doctrine doctrines[MAX_DOCTRINES];	//!< array with doctrines
unsigned int number_of_doctrines = 0;	//!< how many doctrines do we have?

/*! \brief Structure describing a Blubat
 */
struct blubat {
  struct sprite sprite;
  struct doctrine *doctrine;	//!< pointer to doctrine position array
  unsigned int docpos;		//!< position in doctrine

  float t;			//!< parameter for "spline": t in [0..1]
  float x;			//!< last valid x position
  float y;			//!< last valid y position

  float attack_probability;	//!< probability of an attack
  short loaded_droppings;	//!< number of droppings a blubat has
};

/*! \brief Blubat dropping
 */
struct blubat_dropping {
  float x;			//!< last valid x position
  float y;			//!< last valid y position
  float dx;			//!< velocity, x component
  float dy;			//!< velocity, y component

  struct sprite sprite;
};

//------------------------------------------------------------------

static struct doctrine_pos doctrine0[] = {
  {{-0.2, 2, -1, 1.2},
   {-0.2, 1, 1, -0.2}},
  {{666}, {NAN}}
};

static struct doctrine_pos doctrine1[] = {
  {{-0.3, 1.4, -0.4, 1.2},
   {-0.3, 0, 1, 1.2}},
  {{666}, {NAN}}
};

static struct doctrine_pos doctrine2[] = {
  {{1.2, -0.5, 1.5, -0.2},
   {1, 0, 0, 1}},
  {{666}, {NAN}}
};

static struct doctrine_pos doctrine3[] = {
  {{1.2, 1, 0, -0.2},
   {1, 0, 0, 1}},
  {{666}, {NAN}}
};

static struct doctrine_pos doctrine4[] = {
  {{-0.2, 1, 1, 0.5},
   {-0.2, 0, 1, 1.2}},
  {{0.5, 0, 0, 1.5},
   {1.2, 1, 0, 0.5}},
  {{666}, {NAN}}
};

static struct doctrine_pos doctrine5[] = {
  {{0.4, 0.6, 0.8, 0.3},
   {1.2, -1.4, 2.4, -0.4}},
  {{0.3, 0.7, 0.3, -0.5},
   {-0.4, 0.8, 1.3, 0.33}},
  {{666}, {NAN}}
};

static struct doctrine_pos doctrine6[] = {
  {{-0.2, 1.6, 0.8, 0.7},
   {0.2, 0.2, 0.5, 1.4}},
  {{0.7, 0.7, 1.1, -0.2},
   {1.4, -0.8, 1.1, 0.8}},
  {{-0.2, 0.6, 0.7, -0.2},
   {0.8, 0.7, 0.3, 0.4}},
  {{666}, {NAN}}
};

static struct doctrine_pos doctrine7[] = {
  {{-0.2, 2.2, 0.8, -0.1},
   {0.6, 0.5, 0.9, 1.1}},
  {{666}, {NAN}}
};

static struct doctrine_pos doctrine8[] = {
  {{0.55, 1.4, -0.4, 0.45},
   {-0.1, 0.6, 0.6, -0.1}},
  {{0.55, 1.1, -0.1, 0.35},
   {-0.1, 1.6, 1.6, -0.1}},
  {{0.35, 0.1, 0.1, 0.3},
   {-0.1, 1.3, -1.1, -0.1}},
  {{0.35, 0.2, 0.2, -0.1},
   {-0.1, 0.6, 0.7, 0.7}},
  {{666}, {NAN}}
};


sdl_surfaces_t *surf_blubats;
static struct blubat *blubats;
static struct blubat *blu_end;

static sdl_surfaces_t *surf_blubat_droppings;
static struct blubat_dropping blubat_droppings[MAX_BLUBAT_DROPPINGS];
static struct blubat_dropping *bludrop_end = blubat_droppings;

static float calc_doctrine(float t, const float pos[4]) {
  float t_ = 1 - t;
  float t_2 = t_ * t_;
  float t_3 = t_2 * t_;
  float t2 = t * t;
  float t3 = t2 * t;

  assert(pos != NULL);
  return pos[0] * t_3 + 3 * pos[1] * t_2 * t + 3 * pos[2] * t_ * t2 +
    pos[3] * t3;
}

#ifdef DEBUG
static const char *doctrine2str(struct doctrine_pos *d) {
  static char buf[128];
  float *x = d->x;
  float *y = d->y;

  sprintf(buf, "(%.6e, %.6e), (%.6e, %.6e), (%.6e, %.6e), (%.6e, %.6e)",
	  *x, *y, x[1], y[1], x[2], y[2], x[3], y[3]
    );
  return buf;
}

static struct doctrine_pos *str2doctrine(const char *docstr) {
  int i;
  struct doctrine_pos *d = malloc(sizeof(struct doctrine_pos));
  float *x = d->x;
  float *y = d->y;

  if(d) {
    i = sscanf(docstr, "(%e, %e), (%e, %e), (%e, %e), (%e, %e)",
	       x, y, x + 1, y + 1, x + 2, y + 2, x + 3, y + 3);
    if(i != 8) {
      free(d);
      d = NULL;
    }
  }
  return d;
}
#endif

/*
 * FORM.ROCK FORM.BBAT is a FORM chunk with further data for blubats.
 *
 * . . APBB attack probability for blubats
 *		0000 UWORD (1-probability)*65535
 *
 * . . PARA Blubat parameter
 *		0000 UWORD maximum number of blubats
 */

void *init_blubats(uiff_ctx_t iff) {
  unsigned int i;
  struct doctrine_pos **docarrptr;
  struct doctrine_pos *docptr;
  static struct doctrine_pos *doc_pos_arrs[] = {
    doctrine0, doctrine1, doctrine2, doctrine3, doctrine4, doctrine5,
    doctrine6, doctrine7, doctrine8
  };
  int32_t size;

  size = uiff_find_group_ctx(&iff, IFF_FIND_REWIND, FORM, MakeID('B', 'B', 'A', 'T'));
  assert(printf("FORM.BBAT size = $%08X\n", size));
  if(size >= 0) {
    if((size = uiff_find_chunk_ctx(&iff, MakeID('A', 'P', 'B', 'B'))) >= 2) {
      //Chunk found and it is long enough...
      i = read16(iff.f);
      assert(printf("FORM.BBAT APBB size = $%08X i = $%08X\n", size, i));
      blubat_attack_prob = 1 - (i / (float)((1 << 16) - 1));
      assert(printf("\tnew attack probability is %e\n", blubat_attack_prob));
    }
    size = uiff_find_chunk_wflags(&iff, MakeID('P', 'A', 'R', 'A'), IFF_FIND_REWIND);
    assert(printf("FORM.BBAT PARA size = $%04X\n", (int)size));
    if(size >= 2) {
      max_blubats = read16(iff.f);
      assert(printf("\tmax_blubats = %d\n", max_blubats));
    }
  } else {
    guru_alert(GM_FLAGS_DEADEND, GURU_SEC_blubats | GM_SS_BootStrap | GM_GE_OpenRes, &init_blubats);
    return NULL;
  }

  blubats = calloc(max_blubats, sizeof(struct blubat));
  if(!blubats) {
    guru_alert(GM_FLAGS_DEADEND, GURU_SEC_blubats | GM_SS_BootStrap | GM_GE_NoMemory, &blubats);
    return NULL;
  } else blu_end = blubats;
  number_of_doctrines = 0;
  for(i = 0; i < max_blubats; ++i) {
    blubats[i].doctrine = NULL;
  }
  if((surf_blubats = load_images_ck("blubat.%02hx.xpm", -1, -1, -1)) == NULL) {
    guru_alert(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError, (void*)0x4242);
    return NULL;
  }
  if((surf_blubat_droppings = load_images_ck("bb-dropping.%02hx.xpm", -1, -1, -1)) == NULL) {
    guru_alert(GM_FLAGS_DEADEND, GM_SS_Graphics | GM_GE_IOError, (void*)0x42424452);
    return NULL;
  }
  for(docarrptr = doc_pos_arrs;
      docarrptr <
      doc_pos_arrs + sizeof(doc_pos_arrs) / sizeof(struct doctrine_pos *);
      ++docarrptr) {
    docptr = *docarrptr;
    for(i = 0; i < MAX_DOCTRINE_LENGTH; ++i) {
      if(docptr[i].x[0] == 666 || docptr[i].y[0] == NAN) {
	assert(number_of_doctrines < MAX_DOCTRINES);
	doctrines[number_of_doctrines].size = i;
	doctrines[number_of_doctrines++].positions = docptr;
#ifdef DEBUG
	printf("doctrine %ld ptr=%p, size=$%02X\n", docarrptr - doc_pos_arrs,
	       docptr, i);
#endif
	break;
      }
    }
    assert(i < MAX_DOCTRINE_LENGTH);
  }
  assert(number_of_doctrines != 0);
  return docarrptr;
}

/*! \brief Check if a float is smaller then epsilon.
 *
 * If f is smaller than epsilon it will be set to epsilon
 *
 * \param f pointer to a float, can be modifiedchanged
 * \param epsilon value to check for
 */
static inline void no_smaller_than(float *f, float epsilon) {
  assert(epsilon > 0);
  if(fabsf(*f) < epsilon) {
    *f = *f < 0 ? -epsilon : epsilon;
  }
}


static void activate_one_dropping(const struct blubat *blubat) {
  if(bludrop_end - blubat_droppings >= MAX_BLUBAT_DROPPINGS)
    return;
  struct blubat_dropping *bptr = bludrop_end++;
  float x = blubat->x;
  float y = blubat->y;
  const float maxdx = 35;
  const float maxdy = 30;

  bptr->x = x;
  bptr->y = y;
  bptr->dx = (shipdata.xship + surfaces_ship->surfaces[0]->w / 2 - x) / 51.1;
  if(bptr->dx > maxdx) {
    x = bptr->dx / maxdx;	//Scaling factor, otherwise our blubats always miss...
    bptr->dx = maxdx;
    bptr->dy *= x;
  }
  bptr->dy = (shipdata.yship + surfaces_ship->surfaces[0]->h / 2 - y) / 51.1;
  if(bptr->dy > maxdy) {
    y = bptr->dy / maxdy;	//Scaling factor
    bptr->dx *= y;
    bptr->dy = maxdy;
  }
  //If droppings are too slow, speed them up (otherwise they never leave the screen)
  no_smaller_than(&bptr->dx, 0.15);
  no_smaller_than(&bptr->dy, 0.15);
  init_sprite2(&bptr->sprite, surf_blubat_droppings, 130, last_ticks);
}


int activate_one_blubat() {
  if(blu_end - blubats >= max_blubats)
    return 0;
  struct blubat *bptr = blu_end++;

  bptr->doctrine = doctrines + (unsigned) (rnd() * number_of_doctrines);
  bptr->docpos = 0;
  bptr->attack_probability = blubat_attack_prob;
  bptr->loaded_droppings = rnd() * 3 + 1;
  bptr->t = 0;
  bptr->x = calc_doctrine(0, bptr->doctrine->positions->x) * xsize;
  bptr->y = calc_doctrine(0, bptr->doctrine->positions->y) * ysize;
  init_sprite2(&bptr->sprite, surf_blubats, 100, last_ticks);
#ifdef DEBUG
  unsigned i;
  printf("bptr=%p blu_end=%p doctrine=%p x=%9.4e y=%9.4e\n", bptr, blu_end,
	 bptr->doctrine, bptr->x, bptr->y);
  for(i = 0; i < bptr->doctrine->size; ++i) {
    printf("\t%s\n", doctrine2str(bptr->doctrine->positions + i));
  }
#endif
  return 1;
}


int check_blubat_hit(int ylaser, int maxx) {
  struct blubat *bluptr;

  for(bluptr = blubats; bluptr < blu_end; ++bluptr) {
    //Just to be sure...
    if((bluptr->doctrine) == NULL)
      continue;
    float x = bluptr->x;
    float y = bluptr->y;
    if(ylaser >= y && ylaser < y + 16) {
      if(x <= maxx && shipdata.xship < x) {
#ifdef DEBUG
	printf("bluptr=%p blu_end=%p ylaser=%d maxx=%d x=%e y=%e\n", bluptr,
	       blu_end, ylaser, maxx, bluptr->x, bluptr->y);
#endif
	return bluptr - blubats;
      }
    }
  }
  return -1;
}


void deactivate_all_blubats(void) {
  blu_end = blubats;
}

static void display_droppings(SDL_Surface * surf_screen) {
  float x, y;
  struct blubat_dropping *dropptr;

  for(dropptr = blubat_droppings; dropptr < bludrop_end; ++dropptr) {
    x = dropptr->x;
    y = dropptr->y;
    draw_sprite(&dropptr->sprite, x, y, surf_screen, last_ticks);
  }
}

void display_blubats(SDL_Surface * surf_screen) {
  struct blubat *bluptr;
  float x, y;

  for(bluptr = blubats; bluptr < blu_end; ++bluptr) {
    if((bluptr->doctrine) != NULL) {
      x = bluptr->x;
      y = bluptr->y;
      draw_sprite(&bluptr->sprite, x, y, surf_screen, last_ticks);
    }
  }
  display_droppings(surf_screen);
}


void kill_blubat(unsigned int hit) {
  struct blubat *bluptr = blubats + hit;
  int x = (int) bluptr->x;
  int y = (int) bluptr->y;

  bluptr->doctrine = NULL;
  play_sound(1 + (int) (rnd() * 3));
  makebangdots(x, y, 0, 0,
	       get_current_sprite_surface(&bluptr->sprite, last_ticks), 15,
	       1);
  inc_score(x, y, BLUBAT_KILL_POINTS);
}


static void move_all_droppings() {
  float x, y;
  struct blubat_dropping *bptr;
  struct blubat_dropping *target = blubat_droppings;

  for(bptr = blubat_droppings; bptr < bludrop_end; ++bptr) {
    x = bptr->x + bptr->dx * movementrate;
    y = bptr->y + bptr->dy * movementrate;
    if(x > xsize || y > ysize ||
       x < -surf_blubat_droppings->surfaces[0]->w || y < -surf_blubat_droppings->surfaces[0]->h)
      continue;

    bptr->x = x;
    bptr->y = y;
    if(target != bptr) {
      *target = *bptr;
    }
    ++target;
  }
  bludrop_end = target;
}

void move_all_blubats() {
  struct blubat *bluptr;
  float t;

  for(bluptr = blubats; bluptr < blu_end; ++bluptr) {
    //We have to check as a blubat may be killed already!
    if((bluptr->doctrine) == NULL) {
#ifdef DEBUG
	printf("bluptr=%p blu_end=%p x=%e y=%e, swapping with ", bluptr, blu_end, bluptr->x, bluptr->y);
#endif
      /*
       * Reduce end pointer until it is the current one (we are done
       * then) or reduce end pointer if this is also a NULL doctrine.
       */
      while(--blu_end != bluptr) {
	if(blu_end->doctrine != NULL) break;
      }
#ifdef DEBUG
	printf("blu_end=%p\n", bluptr);
#endif
      //Copy the old end to the current one.
      if(bluptr < blu_end) {
	*bluptr = *blu_end;
      } else {
	break; //The whole loop as we are done now.
      }
      assert(blu_end >= bluptr);
    }
    t = bluptr->t + 1.0 / 200 * movementrate;
    if(t <= 1.0) {
      assert(bluptr->doctrine != NULL);
      bluptr->x =
	calc_doctrine(t, bluptr->doctrine->positions[bluptr->docpos].x) * xsize;
      bluptr->y =
	calc_doctrine(t, bluptr->doctrine->positions[bluptr->docpos].y) * ysize;
      bluptr->t = t;
    } else {
      //Get next part of the spline!
      //Set time to beginning again.
      bluptr->t = 0;
      //Get next spline part.
      if(++bluptr->docpos >= bluptr->doctrine->size) {
	//This was the last part, remove this blubat!
	bluptr->doctrine = NULL;
      }
    }
    //Check if blubat start to drop something
    if(bluptr->loaded_droppings > 0 && bluptr->x >= 0 && bluptr->x < xsize
       && bluptr->y >= 0 && bluptr->y < ysize) {
      if(rnd() >= bluptr->attack_probability) {
	--bluptr->loaded_droppings;
	activate_one_dropping(bluptr);
	play_sound(6 + (int) (rnd() * 4));
	bluptr->attack_probability = blubat_attack_prob; //Reset attack probability
      } else {
	bluptr->attack_probability *= blubat_attack_prob;	//Increase
      }
    }
  }
  move_all_droppings();
  //See if http://www.ferzkopp.net/Software/SDL_gfx-2.0/ is of any help...
  /*
   * /usr/share/doc/libsdl-sge/html/index.html
   * /usr/share/doc/libsdl-gfx1.2-4/html/index.html 
   */
  //http://cone3d.gamedev.net/cgi-bin/index.pl?page=tutorials/gfxsdl/tut4
}


void shutdown_blubats(void) {
#ifdef DEBUG
  unsigned i;

  printf("blubats=%p\n", blubats);
  for(i = 0; i < max_blubats; ++i) {
    printf("\t%cblubat[%X]: doctrine=%p docpos=%03X t=%12.6e x=%e y=%e\n",
	   blubats + i < blu_end ? '!' : ' ',
	   i,
	   blubats[i].doctrine,
	   blubats[i].docpos, blubats[i].t, blubats[i].x, blubats[i].y);
  }
  printf("blu_end=%p\n", blu_end);
#endif
  free(blubats);
  destroy_sdl_surfaces(surf_blubats);
  destroy_sdl_surfaces(surf_blubat_droppings);
}

#ifdef IAMMAIN
int main(int argc, char **argv) {
  float x, y, t;
  int i;
  FILE *f;
  //cc -D IAMMAIN blubats.c -lSDL -lSDL_mixer -lSDL_image random_gen.o globals.o sprite.o sound.o bangdots.o datafun.o ship.o highscore_io.o
  struct doctrine_pos doctrine0 = {
    {-1, 2, -2, 1},
    {-1, 1, 1, -1}
  };

  struct doctrine_pos dtemp;
  float xs[10];
  float *x_ptr = xs;

  //i = sscanf("(0.1, -0.2), (0.3, 0.4)", "(%e, %e), (%e, %e)", x_ptr++, x_ptr++, x_ptr++, x_ptr++);
  i =
    sscanf("(0.1, -0.2), (0.3, 0.4)", "(%e, %e), (%e, %e)", xs, xs + 1,
	   xs + 2, xs + 3);
  printf("i=%d %e %e %e %e\n", i, xs[0], xs[1], xs[2], xs[3]);
  printf("%s\n", doctrine2str(&doctrine0));
  /*
   * for(i = 0; i <= 40; ++i) {
   * t = i / 40.0;
   * x = calc_doctrine(t, doctrine3_1.x);
   * y = calc_doctrine(t, doctrine3_1.y);
   * printf("%e %e %e\n", t, x, y);
   * }
   * for(i = 0; i <= 40; ++i) {
   * t = i / 40.0;
   * x = calc_doctrine(t, doctrine3_2.x);
   * y = calc_doctrine(t, doctrine3_2.y);
   * printf("%e %e %e\n", t + 1, x, y);
   * }
   */
  f = fopen("input.iff", "r");
  if(f) {
    printf("%p\n", f);
  }
  return 0;
}
#endif
