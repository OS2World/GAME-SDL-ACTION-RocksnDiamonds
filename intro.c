#include "intro.h"
#include "datafun.h"
#include "guru_meditation.h"
#include "random_gen.h"
#include "SFont.h"
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sparkles.h"
#include "blubats.h"
#include "display_subsystem.h"
#include "engine_exhaust.h"
#include "greeblies.h"
#include "globals.h"
#include "ship.h"
#include "sound.h"

/*
 * FORM.ROCK FORM.INTR is a FORM chunk with further data for the intro.
 *
 * . . TEXT	Chunk with further text for the scroller.
 *
 */


static struct {
  SDL_Color min;
  SDL_Color max;
} minmaxcolors[NUM_COPPERBARS] = {
  { { 0x30, 0, 0 }, { 0xcf, 0x40, 0x4f } }, //Red
  { { 0, 0x10, 0 }, { 0x11, 0xc0, 0xa } }, //Green
  { { 0, 0, 0x40 }, { 0x7, 0x8, 0xaf } }, //Blue
  { { 0xa, 0x1a, 0x13 }, { 0x12, 0xcd, 0xad } }, //Cyan
  { { 1, 1, 1 }, { 0xc0, 0xc1, 0xc0 } }, //Gray
  { { 0x10, 0x10, 0 }, { 0xc0, 0xbf, 0xa } } //Yellow
};

char intro_default_text[] = "Exploding Rock Productions presents \"Rock Dodger\" V"VERSION".    "
  "Press any key to continue!    "
  "       %s        "
  "Credits: Coding by Paul Holt, RPK. Music by Jack Beatmaster, Roz, Strobo, zake. Gfx: Paul Holt, RPK. Font: ripped from lib-sge.   "
  "  A little bit of history: In 1984 Paul Holt programmed the first version of the game in his computer science class. The game was later (in 2001) ported to Linux and SDL. In December 2010 I (RPK) joined the project and started cleaning up the code and added the eye blink to the greeblies. In 2015 we moved the main development site to https://bitbucket.org/rpkrawczyk/rockdodger. And here we are... Enjoy!       "
  ;
static char txtbuf[8001];


struct rockintro *init_intro(SDL_Surface *target, uiff_ctx_t *iff) {
  int i, j, x, y;
  int32_t chunksize;
  SDL_Color colors[16];
  SDL_Surface *surf;
  char readbuf[sizeof(txtbuf) - sizeof(intro_default_text) - 3];
  int xsize = target->w;
  struct rockintro *rockiptr = calloc(sizeof(struct rockintro), 1);

  if(rockiptr) {
    strcpy(txtbuf, "Intro data is kaputt...    Game may be borked...           ");
    chunksize = uiff_find_group_ctx(iff, IFF_FIND_REWIND, FORM, MakeID('I', 'N', 'T', 'R'));
    assert(printf("FORM.ROCK FORM.INTR size = $%08lX\n", (long)chunksize));
    if(chunksize <= 0) {
      guru_meditation(GM_FLAGS_RECOVERY | GM_FLAGS_CHOICE | GM_FLAGS_ABORTIFY, GM_SS_BootStrap | GM_GE_OpenRes | GURU_SEC_intro, &init_intro);
    } else {
      chunksize = uiff_find_chunk_wflags(iff, MakeID('T', 'E', 'X', 'T'), IFF_FIND_REWIND);
      assert(printf(". . TEXT size = $%08lX\n", (long)chunksize));
      if(chunksize > 0) {
	i = fread(readbuf, chunksize, 1, iff->f);
	if(i > 0 && i < sizeof(readbuf))
	  snprintf(txtbuf, sizeof(txtbuf), intro_default_text, readbuf);
      } else guru_meditation(GM_FLAGS_RECOVERY | GM_FLAGS_CHOICE | GM_FLAGS_ABORTIFY, GM_SS_BootStrap | GM_GE_IOError | GURU_SEC_intro, &init_intro);
    }

    if((surf = load_image("exploding_rock.png", 0, 0xff, 0)) != NULL) {
      rockiptr->intro_image = SDL_DisplayFormat(surf);
      SDL_FreeSurface(surf);
    } else return NULL;

    rockiptr->scroller = init_scroller(txtbuf, 440, 3.141, target->w);
    if(rockiptr->scroller == NULL) return NULL;
    for(i = 0; i < NUM_COPPERBARS; ++i) {
      rockiptr->positions[i] = 2 * M_PI / NUM_COPPERBARS * i;
      rockiptr->speeds[i] = 1.9 / 30.0;
    
      for(j = 0; j < 8; ++j) {
	SDL_Color col;
	unsigned u;

	u = (8 - j) * minmaxcolors[i].min.r;
	u += j * minmaxcolors[i].max.r;
	col.r = u / 8;
	u = (8 - j ) * minmaxcolors[i].min.g;
	u += j * minmaxcolors[i].max.g;
	col.g = u / 8;
	u = (8 - j ) * minmaxcolors[i].min.b;
	u += j * minmaxcolors[i].max.b;
	u /= 8;
	col.b = u;
	colors[j] = col;
      }
      
      surf = SDL_CreateRGBSurface(SDL_HWSURFACE, xsize, 16, 8, 0, 0, 0, 0);
      if(surf == NULL) return NULL;
      SDL_LockSurface(surf);
      SDL_SetColors(surf, colors, 0, 16);
      for(y = 0; y < 8; ++y) {
	for(x = 0; x < xsize; ++x) {
	  Uint8 *pixels = surf->pixels;

	  pixels[y * surf->w + x] = y;
	  pixels[(15 - y) * surf->w + x] = y;
	}
      }
      SDL_UnlockSurface(surf);
      rockiptr->copperbars[i] = SDL_DisplayFormat(surf);
      SDL_SetAlpha(rockiptr->copperbars[i], SDL_SRCALPHA|SDL_RLEACCEL, 141);
      SDL_FreeSurface(surf);
    }
  } else { // No memory...
    guru_alert(GM_FLAGS_DEADEND, GM_SS_BootStrap | GM_GE_NoMemory | GURU_SEC_intro, &init_intro);
  }
  return rockiptr;
}


void fld_logo(struct rockintro *intro, SDL_Surface *target, float movementrate) {
  int rastline;
  SDL_Surface *lsurf = intro->intro_image;
  SDL_Rect r = { (target->w - 640) / 2, 0, 0, 0 };
  SDL_Rect s = { 0, 0, lsurf->w, 1};

  if(intro->fld_amplitude <= 0) {
    s.h = lsurf->h;
    SDL_BlitSurface(intro->intro_image, NULL, target, &r);
  } else {
    float phi = intro->fld_logophase;
    float tline = intro->fld_logo_wagging_amplitude * sinf(phi);
    float camp = intro->fld_amplitude; // cosine amplitude

    intro->fld_last_topline = (int)tline;
    for(rastline = 0; rastline < lsurf->h; ++rastline) {
      s.y = rastline;
      r.y = tline;
      SDL_BlitSurface(intro->intro_image, &s, target, &r);
      tline += 1 + camp + cosf(phi) * camp;
      phi += 1.0 / 200.0 * M_PI;
    }
    intro->fld_logophase += movementrate / 11;
  }
}


void update_and_draw_intro(struct rockintro *intro, SDL_Surface *target, float movementrate) {
  int i, j;
  float z_positions[NUM_COPPERBARS];
  Sint16 y_positions[NUM_COPPERBARS];
  int minimum = 0;
  SDL_Rect r = { 0, 0, 0, 0 };

  for(i = 0; i < NUM_COPPERBARS; ++i) {
    y_positions[i] = sinf(intro->positions[i]) * 40 + 440;
    z_positions[i] = cosf(intro->positions[i]);
    intro->positions[i] += intro->speeds[i] * movementrate;
  }
  for(i = 0; i < NUM_COPPERBARS; ++i) {
    minimum = 0;
    for(j = 1; j < NUM_COPPERBARS; ++j) if(z_positions[j] < z_positions[minimum]) minimum = j;
    r.y = y_positions[minimum];
    SDL_BlitSurface(intro->copperbars[minimum], NULL, target, &r);
    z_positions[minimum] = 666;
  }
  fld_logo(intro, target, movementrate);
  update_and_draw_scroller(intro->scroller, target, movementrate * 1.412);
  draw_sparkles(target);
  update_sparkles(target);
}

void shutdown_intro(struct rockintro *intro) {
  int i;

  for(i = 0; i < NUM_COPPERBARS; ++i) {
    SDL_FreeSurface(i[intro->copperbars]);
  }
  destroy_scroller(intro->scroller);
  SDL_FreeSurface(intro->intro_image);
  reset_sparkles();
  free(intro);
}


void play_intro(int force, int oss_sound_flag, uiff_ctx_t iff) {
  SDL_Event event;
  int state;
  register int i;
  int last_state = -1;
  int running = 1;
  SDL_Surface *surf_screen = SDL_GetVideoSurface();
  short deltah, heading = 0;
  float *fp;

  if(force == 0) {
    switch(opt_intro) {
    case 3:
    case 2:
      //Do not bug user again...
      opt_intro = 0;
      save_setup();
    case 1:
      break;
    case 0:
      return;
    default:
      assert(0);
      return;
    }
  }

  struct rockintro *intro = init_intro(surf_screen, &iff);
  if(intro == NULL) {
    fprintf(stderr, "Can not initialise intro!\n");
    return;
  }
  if(oss_sound_flag) play_tune(3);

  while(running) {
    while(SDL_PollEvent(&event) != 0) {
      if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN) running = 0;
    }
    ticks_since_last = SDL_GetTicks() - last_ticks;
    last_ticks = SDL_GetTicks();
    if(ticks_since_last > 200) {
      movementrate = 0;
    } else {
      movementrate = ticks_since_last / 50.0;
    }
    SDL_FillRect(surf_screen, NULL, SDL_MapRGB(surf_screen->format, 0, 0, 0));
    update_and_draw_intro(intro, surf_screen, movementrate);
    state = (last_ticks / 0x1BC6) % 10;
    if(state != last_state) {
      switch(state) {
      case 6:
	shipdata.xship = -37;
	shipdata.yship = 225;
	break;
      case 5:
	shipdata.xship = -37;
	shipdata.yship = 370;
	break;
      case 1:
	shipdata.xship = -37;
	shipdata.yship = 80;
	break;
      case 3:
	while(activate_one_blubat());
	shipdata.xship = xsize / 2;
	shipdata.yship = ysize + 42;
	break;
      }
      last_state = state;
    }
    SDL_LockSurface(surf_screen);
    draw_engine_dots(surf_screen);
    SDL_UnlockSurface(surf_screen);
    switch(state) {
    case 5:
    case 1:
    case 6:
      draw_ship(surf_screen);
      create_engine_dots(220);
      shipdata.xship += movementrate * 6.08 * surf_screen->w / 800.0; //Ship should have the same speed, regardless of the screen size.
      if(shipdata.xship > xsize + 40) shipdata.xship = -40;
      break;
    case 2:
      draw_ghostly_rock_dodger(surf_t_rock, surf_t_dodger);
      fadetimer += movementrate / 1.92;
      break;
    case 3:
      display_blubats(surf_screen);
      move_all_blubats();
      break;
    case 4:
      draw_ghostly_rock_dodger(surf_t_rock, surf_t_dodger);
      fadetimer += movementrate / 1.41;
      break;
    case 7:
    case 8:
      deltah = (surf_screen->w - 640) / 2;
      i = intro->fld_last_topline;
      create_plumedots(7, 168 + deltah, i + 47, -heading, 4, 8);
      create_plumedots(7, surf_screen->w - 158 - deltah, i + 47, heading, 4, 8);
      heading += 2;
      break;
    case 9:
      fp = &intro->fld_logo_wagging_amplitude;
      if(*fp < 20) *fp += movementrate / 61.51;
      fp = &intro->fld_amplitude;
      if(*fp < 1.0 / 5.014847583) *fp += movementrate / 77.8111472142;
      break;
    }
    SDL_Flip(surf_screen);
  }
  init_engine_dots();
  deactivate_all_blubats();
  shutdown_intro(intro);
  fadetimer = 0; //Reset the fade timer as the title page will otherwise not begin correctly. See display_version_n_text().
}
