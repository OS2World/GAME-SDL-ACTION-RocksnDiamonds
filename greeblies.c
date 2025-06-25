#include "sparkles.h"
#include "datafun.h"
#include "greeblies.h"
#include "guru_meditation.h"
#include "highscore_io.h"
#include "random_gen.h"
#include "rocks.h"
#include "globals.h"
#include "ship.h"
#include "sound.h"
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>

#define GREEBLIE_DATA_SIZE (sizeof(struct greeble) * MAX_GREEBLES)

struct greeble *the_greeblies;	// THE GREEBLIES
sdl_surfaces_t *surf_greeblies; // The images

void deactivate_greeblies() {
  unsigned int i;

  for(i = 0; i < MAX_GREEBLES; i++)
    the_greeblies[i].active = 0;
  for(i = 0; i < MAX_ROCKS; i++)
    rock[i].greeb = 0;
}

void *init_greeblies() {
#ifdef DEBUG
  FILE *f;
  char debugname[] = "/tmp/rockdodger$greeblies.XXXXXX";
  if(mkstemp(debugname) == -1) {
    perror("No mkstemp");
    return NULL;
  }
  f = fopen(debugname, "w+");
  if(f == NULL) {
    perror("no tmpfile");
    return f;
  }
  if(ftruncate(fileno(f), GREEBLIE_DATA_SIZE) == -1) return NULL;
  the_greeblies = mmap(NULL, GREEBLIE_DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(f), 0);
  printf("init_greeblies(): the_greeblies=%p f=%p, sizeof=$%04lx\n", the_greeblies, f, (unsigned long)GREEBLIE_DATA_SIZE);
  fclose(f);
#else
  the_greeblies = malloc(GREEBLIE_DATA_SIZE);
#endif
  if(the_greeblies == NULL) {
    guru_meditation(GM_FLAGS_DEADEND, GM_SS_BootStrap | GM_GE_NoMemory | GURU_SEC_greeblies, &init_greeblies);
  } else {
    deactivate_greeblies();
    // Load all the available greeblies
    if((surf_greeblies = load_images_ck("greeblie%d.bmp", 0, 0, 255)) == NULL) return NULL;
    if(surf_greeblies->num_surfaces == 0) return NULL;
  }
  memset(the_greeblies, 0, GREEBLIE_DATA_SIZE);
  return the_greeblies;
}

void shutdown_greeblies() {
  assert(the_greeblies != NULL);
#ifdef DEBUG
  munmap(the_greeblies, GREEBLIE_DATA_SIZE);
#else
  free(the_greeblies);
#endif
  the_greeblies = NULL;
}

void activate_greeblie(struct greeble *g) {
  int j;

  if(g->active) guru_meditation(GM_FLAGS_RECOVERY, GM_GE_BadParm | GURU_SEC_greeblies, &activate_greeblie);
  /* Choose an active rock to become attracted to. If there are no
   * such available rocks in the first 14.3% of spots we search, then
   * give up without activating any greeblies at all.
   * 
   * As the rocks are quite fast usually we do not use up all the
   * greeblie slots. If MAX_ROCKS rocks would be on the screen we
   * really get a greeblie mania.
   */
  for(j = 0; j < (MAX_ROCKS / 7); j++) {
    int i = rnd() * MAX_ROCKS;
    if(rock[i].active && rock[i].greeb == 0 && 4 * rock[i].x > 3 * xsize) {
      // Create a new greeb, alive, flying, from the right side of the screen at a
      // random position.
      g->active = 1;
      g->landed = 0;
      g->x = xsize;
      g->y = rnd() * ysize;
      g->imgcnt = 0;
      // It's flying to this rock, and the rock knows it.
      g->target_rock_number = i;
      rock[i].greeb = g - the_greeblies;
      return;
    }
  }
}

void activate_one_greeblie() {
  assert(the_greeblies != NULL);
  int i;
  for(i = 0; i < MAX_GREEBLES; i++) {
    if(!the_greeblies[i].active) {
      activate_greeblie(&the_greeblies[i]);
      return;
    }
  }
}

void move_all_greeblies() {
  struct greeble *g;
  int i;

  assert(the_greeblies != NULL);
  for(i = 0; i < MAX_GREEBLES; i++) {
    g = &the_greeblies[i];
    if(g->active) {
      if(g->landed) {
	g->boredom++;
	// Landed greebles may take it into their head to jump off their
	// rock and travel to a new one. Boredom can cause this, but a
	// ship getting close to their rock will do it too.
	//printf("%f %f %f\n",shipdata.yship,rock[g->target_rock_number].y,g->y);
	if(rnd() < (0.001 * (g->boredom - 100))
	   || (g->target_rock_number
	       && fabsf(shipdata.yship - (rock[g->target_rock_number].y + g->y)) < 10
	       && (shipdata.xship < rock[g->target_rock_number].x))) {
	  int j;
	  for(j = 0; j < (MAX_ROCKS / 10) && g->landed; j++) {	// Find a target
	    int i = rnd() * MAX_ROCKS;
	    int dx;
	    struct rock_struct *r = &rock[i];
	    if(r->active && r->greeb == 0
	       && (dx = (rock[g->target_rock_number].x - r->x)) > 0
	       && dx < xsize / 3) {
	      // Change the greeble coordinates to represent free
	      // space coordinates
	      g->x += rock[g->target_rock_number].x;
	      g->y += rock[g->target_rock_number].y;
	      rock[g->target_rock_number].greeb = 0;
	      g->target_rock_number = i;
	      r->greeb = g - the_greeblies;
	      g->active = 1;
	      g->landed = 0;
	    }
	  }
	  // It's no problem if we don't find a target, just don't
	  // bother leaving the current one.
	}
	// If the greeble leaves the left side of the screen, he's gone.
	if(rock[g->target_rock_number].x + g->x < 0) {
	  g->active = 0;
	  rock[g->target_rock_number].greeb = 0;
	}
      } else {
	float dx = g->x - rock[g->target_rock_number].x + 10;
	float dy = g->y - rock[g->target_rock_number].y + 10;
	float dist = sqrtf(dx * dx + dy * dy);
	// Greebles are attracted to rocks. If the greeble is within a
	// certain distance of the target rock, set it to 'landed' mode.
	if(dist < 10) {
	  g->landed = 1;
	  g->boredom = 0;
	  // Change the greeble coordinates to represent the
	  // offset from the host rock.
	  g->x = dx - 10;
	  g->y = dy - 10;
	} else {
	  g->x -= 20 * movementrate * dx / dist;
	  g->y -= 10 * movementrate * dy / dist;
	}
      }
    }
  }
}

void display_greeb(struct greeble *g, SDL_Surface * surf_screen) {
  static SDL_Rect src, dest;
  short int surf_greeblies_idx;
  const short int speed_divisor = 3;

  surf_greeblies_idx = movementrate;
  if(g->active) {
    if(g->landed) {
      dest.x = (int) (g->x + rock[g->target_rock_number].x);
      dest.y = (int) (g->y + rock[g->target_rock_number].y);
    } else {
      dest.x = (int) (g->x);
      dest.y = (int) (g->y);
    }

    src.x = 0;
    src.y = 0;
    if(g->imgcnt < 0) {
      surf_greeblies_idx = 0;
    } else {
      surf_greeblies_idx = g->imgcnt / speed_divisor;
    }
    if(surf_greeblies_idx < surf_greeblies->num_surfaces) {
      src.w = surf_greeblies->surfaces[surf_greeblies_idx]->w;
      src.h = surf_greeblies->surfaces[surf_greeblies_idx]->h;
      dest.w = src.w;
      dest.h = src.h;
      SDL_BlitSurface(surf_greeblies->surfaces[surf_greeblies_idx], &src, surf_screen,
		      &dest);
    }
  }
  g->imgcnt += movementrate;
  if(g->imgcnt / speed_divisor >= surf_greeblies->num_surfaces) {
    g->imgcnt = -24; //A pause before replaying animation.
  }
}

void kill_greeb(int hitgreeb) {
  //The unfortunate greeblie.
  struct greeble *unfog;
  int r;

  assert(printf("kill_greeb(hitgreeb=%d):\n", hitgreeb));
  assert(the_greeblies != NULL);
  assert(hitgreeb < MAX_GREEBLES);
  unfog = &the_greeblies[hitgreeb];
  unfog->active = 0;
  if((r = unfog->target_rock_number)) {
    rock[r].greeb = 0;
  }
  if(unfog->landed) {
    play_sound(1 + (int) (rnd() * 3));
    makebangdots((int) (unfog->x + rock[unfog->target_rock_number].x),
		 (int) (unfog->y + rock[unfog->target_rock_number].y),
		 0, 0, surf_greeblies->surfaces[0], 15, 1);
  } else {
    makebangdots((int) (unfog->x), (int) (unfog->y), 0, 0, surf_greeblies->surfaces[0], 15, 1);
    play_sound(1 + (int) (rnd() * 3));
  }
  assert(printf("\t unfog=%p target_rock=%d\n", unfog, r));
  inc_score(unfog->x, unfog->y, GREEBLE_KILL_POINTS);
}
