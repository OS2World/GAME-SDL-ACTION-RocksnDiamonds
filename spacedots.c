#include "guru_meditation.h"
#include "globals.h"
#include "spacedots.h"
#include "random_gen.h"
#include <assert.h>

#define MAX_CICADA_SPACEDOTS 0x0800

/*! \file spacedots.c
 * Space dots are the stars in the background. Currently there are two
 * different engines to draw all the stars. Constants will be read
 * from the IFF file.
 *
 * FORM ROCK		Rockdodger iff file
 * . FORM SPDT		space dots form
 * . . NDOT		Number of space dots in the normal generation
 *			engine.
 *
 * NDOT chunk
 * ----------
 *
 * uint16	number of space dots (normal generation engine)
 */


/*! \brief Initialise space dots.
 *
 * The information is written into the spacedots_t union.
 *
 * \param surf_screen The surface which is displayed.
 * \param spacedots Structure to be initialised.
 * \param max_space_dots Number of spacedots to allocate.
 * \return 0 = failure, 1 = OK
 */
int init_space_dots(spacedots_t *spacedots, SDL_Surface *surf_screen, unsigned int max_space_dots) {
  int i, intensity;

  spacedots->num_spacedots = max_space_dots;
  spacedots->spacedots = malloc(sizeof(struct spacedot) * spacedots->num_spacedots);
  if(spacedots->spacedots == NULL) {
    fprintf(stderr, "Can not allocate space for %d spacedots!\n", spacedots->num_spacedots);
    return 0;
  }
  for(i = 0; i < spacedots->num_spacedots; i++) {
    float r = rnd_next_float();	// We are in init and can take our time.

    spacedots->spacedots[i].x = rnd() * (xsize - 5);
    spacedots->spacedots[i].y = rnd() * (ysize - 5) + 2.5;

    r = r * r;
    spacedots->spacedots[i].dx = -r * 4;
    // -1/((1-r)+.3);
    intensity = (int) (r * 180 + 50);
    spacedots->spacedots[i].color =
      SDL_MapRGB(surf_screen->format, intensity, intensity, intensity);
#if DEBUG > 0
    printf("r=%E spacedots->spacedots.dx=%E\n", r, spacedots->spacedots[i].dx);
#endif
  }
  return 1;
}


unsigned short read_number_of_dots(uiff_ctx_t *iff) {
  unsigned short ndots = MAX_SPACE_DOTS;
  int32_t size = uiff_find_group_ctx(iff, IFF_FIND_REWIND, FORM, MakeID('S', 'P', 'D', 'T'));

  assert(printf("FORM.ROCK FORM.SPDT size = $%08lX\n", (unsigned long)size));
  if(size >= 0) {
    if((size = uiff_find_chunk_ctx(iff, MakeID('N', 'D', 'O', 'T'))) >= 2) {
      //Chunk found and it is long enough...
      ndots = read16(iff->f);
      assert(printf("FORM.ROCK FORM.SPDT MDOT size = $%08lX ndot = $%04hX\n", (unsigned long)size, ndots));
    } else {
      guru_meditation(GM_FLAGS_RECOVERY, GM_SS_BootStrap | GM_GE_BadParm | GURU_SEC_spacedots, &read_number_of_dots);
    }
  } else {
    guru_meditation(GM_FLAGS_RECOVERY, GM_SS_BootStrap | GM_GE_OpenLib | GURU_SEC_spacedots, &read_number_of_dots);
  }
  return ndots;
}


void draw_space_dots_pixel(spacedots_t *spacedots, SDL_Surface *surf_screen) {
  int i;
  RD_VIDEO_TYPE *rawpixel = (RD_VIDEO_TYPE *) surf_screen->pixels;

  assert(spacedots->type == 0);
  struct spacedot *sdot = spacedots->spacedots;
  assert(sdot != NULL);
  SDL_LockSurface(surf_screen);
  for(i = 0; i < spacedots->num_spacedots; ++i) {
    rawpixel[surf_screen->pitch / sizeof(RD_VIDEO_TYPE) * sdot[i].y + (int) (sdot[i].x)] = sdot[i].color;
    sdot[i].x += sdot[i].dx * movementrate;
    if(sdot[i].x < 0)
      sdot[i].x = xsize;
  }
  SDL_UnlockSurface(surf_screen);
}


/*
 * List of Prime Numbers up to 1000.
 * 
 *  2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59,
 * 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131,
 * 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
 * 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271,
 * 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353,
 * 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433,
 * 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509,
 * 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
 * 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677,
 * 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769,
 * 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859,
 * 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953,
 * 967, 971, 977, 983, 991, 997
 */

int init_cicada_spacedots(spacedots_t *spacedots, SDL_Surface *surf_screen) {
#ifdef DEBUG
  printf("Entering %s@%p.\n", __func__, &init_cicada_spacedots);
#endif
  SDL_Surface *surf;
  int current_playfield, j, x, y;
  SDL_Color colors[256];
  int widths[] = {307, 337, 349, 367, 373, 379, 383, 389};
#if (NUMBER_CICADA_PLAYFIELDS > 8)
#error "Not enough primes!"
#endif

  assert(surf_screen->w == xsize);
  assert(surf_screen->h == ysize);

  if((spacedots->cicadadots = malloc(sizeof(struct cicada_spacedots_playfield) * NUMBER_CICADA_PLAYFIELDS)) == NULL) return 0;
  spacedots->num_cicada_playfields = NUMBER_CICADA_PLAYFIELDS;

  /* Fill colors with colour information */
  for(j = 0; j < 256; j++) {
    float jj = j / 255.0 * 180.0 + 40;
    colors[j].r = jj;
    colors[j].g = jj;
    colors[j].b = jj;
  }

  struct cicada_spacedots_playfield *cicada_spacedots_playfields = spacedots->cicadadots;
  for(current_playfield = 0; current_playfield < NUMBER_CICADA_PLAYFIELDS; ++current_playfield) {
    if((surf = SDL_CreateRGBSurface(SDL_HWSURFACE, widths[current_playfield], ysize, 8, 0, 0, 0, 0)) == NULL) return 0;
#ifdef DEBUG
    printf("playfield %d at %p\n", current_playfield, surf);
#endif    
    SDL_LockSurface(surf);
    SDL_SetColors(surf, colors, 0, 256);
    cicada_spacedots_playfields[current_playfield].playfield = surf;
    cicada_spacedots_playfields[current_playfield].speed = current_playfield * .885 + .162;
    cicada_spacedots_playfields[current_playfield].pos = 0;

    unsigned char *pixels = surf->pixels;
    float maxcol = (float)(current_playfield + 1)/ NUMBER_CICADA_PLAYFIELDS;
    float mincol = (float)(current_playfield) / NUMBER_CICADA_PLAYFIELDS;
#ifdef DEBUG
    printf("mincol=%e maxcol=%e\n", mincol, maxcol);
#endif
    for(j = 0; j < MAX_CICADA_SPACEDOTS / (NUMBER_CICADA_PLAYFIELDS + 1); ++j) {
      x = rnd_next_float() * surf->w;
      y = rnd_next_float() * (ysize - 5) + 2.5;
      float r = (mincol + rnd_next_float() * (maxcol - mincol));
      assert(r >= 0 && r < 1);
      pixels[y * surf->w + x] = 256 * r;
    }
    SDL_SetColorKey(surf, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(surf->format, 0, 0, 0));
    SDL_UnlockSurface(surf);
  }
  return 1;
}


void draw_cicada_dots(spacedots_t *spacedots, SDL_Surface *surf_screen) {
  int i;
  float pos;
  SDL_Rect rect = { 0, 0, 0, 0 };

  assert(spacedots->type == 1);
  for(i = 0; i < spacedots->num_cicada_playfields; ++i) {
    struct cicada_spacedots_playfield *playf = spacedots->cicadadots + i;
    
    for(pos = playf->pos; pos < xsize; pos += playf->playfield->w) {
      rect.x = pos;
      SDL_BlitSurface(playf->playfield, NULL, surf_screen, &rect);
    }
    if((playf->pos -= playf->speed * movementrate) < -playf->playfield->w) playf->pos = 0;
  }
}


spacedots_t *init_space_dots_engine(SDL_Surface *surf_screen, short type, uiff_ctx_t iff) {
  unsigned short number_of_dots;
  spacedots_t *spacedots = calloc(sizeof(spacedots_t), 1);
  int ret = 0;

  if(spacedots) {
    spacedots->type = type;
    switch(spacedots->type) {
    case 0:
      number_of_dots = read_number_of_dots(&iff);
      ret = init_space_dots(spacedots, surf_screen, number_of_dots);
      assert(spacedots->spacedots != NULL);
      assert(spacedots->num_spacedots == number_of_dots);
      break;
    case 1:
      ret = init_cicada_spacedots(spacedots, surf_screen);
      break;
    default:
      return NULL;
    }
    if(!ret) {
#ifdef DEBUG
      fprintf(stderr, "Spacedot initialisation failed!\n");
#endif
      free(spacedots);
      spacedots = NULL;
    }
  } else {
    //We return NULL and the error is handled in main().
    guru_alert(GM_FLAGS_DEADEND, GM_SS_Misc|GM_GE_NoMemory|GURU_SEC_spacedots, init_space_dots_engine);
  }
  return spacedots;
}


void draw_space_dots(spacedots_t *spacedots, SDL_Surface *surf_screen) {
  switch(spacedots->type) {
  case 0:
    draw_space_dots_pixel(spacedots, surf_screen);
    break;
  case 1:
    draw_cicada_dots(spacedots, surf_screen);
    break;
  default:
    assert(0);
  }
}


void destroy_space_dots(spacedots_t *spacedots) {
  //Null pointer is harmless...
  if(spacedots == NULL) return;
  switch(spacedots->type) {
  case 0:
    free(spacedots->spacedots);
    break;
  case 1:
    free(spacedots->cicadadots);
    break;
  case -1: //Do not care about double destroy...
    return;
  default:
    /* In debug mode please crash, otherwise just cross your fingers
       and hope for the best... */
    assert(0);
  }
  spacedots->type = -1;
  free(spacedots);
}
