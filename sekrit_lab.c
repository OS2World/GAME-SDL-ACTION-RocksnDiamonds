#include <SDL/SDL.h>
#include <assert.h>
#include <math.h>
#include "sekrit_lab.h"
#include "guru_meditation.h"
#include "SFont.h"
#include "datafun.h"
#include "laser.h"
#include "random_gen.h"
#include "globals.h"
#include "sparkles.h"
#include "yellifish.h"


/*! \brief Perform one of the tests.
 *
 * For debugging only!
 *
 * \return 0 = no valid test found, otherwise 1
 */
int perform_a_sekrit_test(char *arg) {
  SDL_Event event;
  int running = 1;

  printf("Looking for test '%s'...\n", arg);
  movementrate = 1.0;
  if(strcmp(arg, "guru") == 0) {
    SDL_FillRect(surf_screen, NULL, SDL_MapRGB(surf_screen->format, 0, 0x55, 0xAA));
    printf("guru_display() = %d\n", guru_display_gp(surf_screen, GM_FLAGS_AUTOTIMEOUT | 2, 3, (void*)0xDEADBEA1, "Greeblies at work..."));
    printf("guru_display() = %d\n", guru_display_gp(surf_screen, GM_FLAGS_AUTOTIMEOUT| 1 | GM_FLAGS_CHOICE, 3, (void*)0xDEADBEA1, "Alert! This needs still a lot of work!"));
    printf("guru_display() = %d\n", guru_display_gp(surf_screen, GM_FLAGS_AUTOTIMEOUT | 0, 4, (void*)0xAA55AA55, "Normal..."));
    printf("guru_meditation() = %d\n", guru_meditation(GM_FLAGS_DEADEND | GM_FLAGS_EXIT_VIA_TERM, 0x48454c50, (void*)0x68656c70));
    return 1;
  } else if(strcmp(arg, "yellifish") == 0) {
    yellifish_t yelli1, yelli2, yelli3;
    yellifishsubsystem_t *ysys = init_yellifish_subsystem(*iff_ctx);

    init_yellifish(ysys, &yelli1, 100, 80);
    init_yellifish(ysys, &yelli2, 150, 70);
    init_yellifish(ysys, &yelli3, 250, 80);
    while(running) {
      while(SDL_PollEvent(&event)) {
	if(event.type == SDL_MOUSEBUTTONDOWN) running = 0;
      }
      SDL_FillRect(surf_screen, NULL, SDL_MapRGB(surf_screen->format, 0, 0, 0));
      display_yellifish(ysys, &yelli1, surf_screen);
      display_yellifish(ysys, &yelli2, surf_screen);
      display_yellifish(ysys, &yelli3, surf_screen);
      SDL_Flip(surf_screen);
      SDL_Delay(60);
    }

    SDL_Flip(surf_screen);
    SDL_Delay(2600);
    return 1;
  } else if(strcmp(arg, "laserpointer") == 0) {
    float positions[10];
    float speeds[10];
    int i;
    int ysize2 = ysize / 2 - 3;

    for(i = 0; i < 10; ++i) {
      speeds[i] = rnd() / 100;
      positions[i] = 0;
    }
    
    while(running) {
      if(SDL_PollEvent(&event) != 0 && event.type == SDL_MOUSEBUTTONDOWN) running = 0;
      SDL_FillRect(surf_screen, NULL, SDL_MapRGB(surf_screen->format, 0, 0, 0x13));
      for(i = 0; i < 10; ++i) {
	draw_random_dots(surf_screen, sinf(positions[i]) * ysize2 + ysize2 + 1, 0, xsize);
	positions[i] += speeds[i];
      }
      SDL_Flip(surf_screen);
      SDL_Delay(60);
    }
    return 1;
  } else if(strcmp(arg, "sparkles") ==0) {
    int i, j;
    float x;
    RD_VIDEO_TYPE *rawpixel;

    while(running) {
      if(SDL_PollEvent(&event) != 0 && event.type == SDL_MOUSEBUTTONDOWN) running = 0;
      SDL_FillRect(surf_screen, NULL, SDL_MapRGB(surf_screen->format, 0, 0, 0x13));
      SDL_LockSurface(surf_screen);
      rawpixel = (RD_VIDEO_TYPE *) surf_screen->pixels;
      for(i = 0; i < xsize; ++i) {
	x = i / (xsize - 1.0);
	for(j = 10; j < 50; ++j) {
	  rawpixel[surf_screen->pitch / sizeof(RD_VIDEO_TYPE) * j + i] = (RD_VIDEO_TYPE)get_life_colour(x, &hot_colours, surf_screen);
	}
	for(j = 110; j < 150; ++j) {
	  rawpixel[surf_screen->pitch / sizeof(RD_VIDEO_TYPE) * j + i] = (RD_VIDEO_TYPE)get_life_colour(x, &cool_colours, surf_screen);
	}
      }
      SDL_UnlockSurface(surf_screen);
      SDL_Flip(surf_screen);
      SDL_Delay(60);
    }
    return 1;
  } else if(strcmp(arg, "key_lab") ==0) {
    unsigned int event_nr = 0, k;
    unsigned int row = 0;
    char buf[30][128];
    SDL_Rect rect = { 0, 0, surf_screen->w, 20 };

    restart_game();
    assert(surf_screen != NULL);
    assert(surf_green_block != NULL);
    memset(buf, 0, sizeof(buf));
    while(running) {
      while(SDL_PollEvent(&event) == 1) {
	row = event_nr % 30;
	switch(event.type) {
	case SDL_QUIT:
	  running = 0;
	  break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	  sprintf(buf[row], "button=%d state=%d x=$%04hX y=$%04hX", event.button.button, event.button.state, event.button.x, event.button.y);
	  break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	  sprintf(buf[row], "state=%d sym=$%04X", event.key.state, event.key.keysym.sym);
	  break;
	case SDL_MOUSEMOTION:
	  //Ignore this one...
	  --event_nr;
	  break;
	default:
	  sprintf(buf[row], "event_nr = $%x, event.type = $%x", event_nr, (unsigned int)event.type);
	}
	++event_nr;
      }
      SDL_FillRect(surf_screen, NULL, SDL_MapRGB(surf_screen->format, 0xf, 0, 0xf));
      SDL_SetAlpha(surf_green_block, SDL_SRCALPHA, (int) (129 + 66 * sinf(fadetimer)));
      rect.y = row * 20;
      SDL_BlitSurface(surf_green_block, NULL, surf_screen, &rect);
      for(k = 0; k < 30; ++k) {
	PutString(surf_screen, 12, k * 20, buf[k]);
      }
      SDL_Flip(surf_screen);
      SDL_Delay(50);
      fadetimer += .192;
    }
    return 1;
  }
  puts("Not found!");
  return 0;
}

