#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#define SCALEX 2.0f
#define SCALEY 2.0f
#define OFFSETX 252
#define OFFSETY 42
#define POSSTEP 0.00101

/*! \brief positions for "spline" in doctrine
 */
struct doctrine_pos {
  float x[4];			//!< The X positions
  float y[4];			//!< The Y positions
};


static struct doctrine_pos doctrine0[] = {
  {{-0.2, 2, -1, 1.2},
   {-0.2, 1, 1, -0.2}},
  {{666}, {NAN}}
};

SDL_Surface *disp;

static const char *doctrine2str(struct doctrine_pos *d) {
  static char buf[128];
  float *x = d->x;
  float *y = d->y;

  sprintf(buf, "(%.6e, %.6e), (%.6e, %.6e), (%.6e, %.6e), (%.6e, %.6e)",
	  *x, *y, x[1], y[1], x[2], y[2], x[3], y[3]
    );
  return buf;
}

static float calc_doctrine(float t, const float pos[4]) {
  float t_ = 1 - t;
  float t_2 = t_ * t_;
  float t_3 = t_2 * t_;
  float t2 = t * t;
  float t3 = t2 * t;

  return pos[0] * t_3 + 3 * pos[1] * t_2 * t + 3 * pos[2] * t_ * t2 +
    pos[3] * t3;
}

static void calc_doctrine_xy(float t, const struct doctrine_pos *doctrine, float &x, float &y) {
  x = calc_doctrine(t, doctrine->x);
  y = calc_doctrine(t, doctrine->y);
}


void plot_doctrine_pts(const struct doctrine_pos *doctrine) {
  float x, y;

  for(int i = 0; i < 4; ++i) {
    x = doctrine->x[i];
    y = doctrine->y[i];

    x /= SCALEX / 320;
    y /= SCALEY / 200;
    x += OFFSETX;
    y += OFFSETY;
    circleColor(disp, x, y, 3, 0xdead33ff);
  }
}


void plot_doctrine(const struct doctrine_pos *doctrine) {
  int i;
  float t, x, y;
  int ix, iy;

  rectangleColor(disp, 0 + OFFSETX, 0 + OFFSETY, 1.0/SCALEX*320 + OFFSETX, 1.0/SCALEY*200 + OFFSETY, 0x001CffFF);
  for(i = 0; i < 2048; ++i) {
    t = i / 2047.0;
    
    calc_doctrine_xy(t, &doctrine0[0], x, y);
    ix = x / SCALEX * 320;
    iy = y / SCALEY * 200;
    //printf("t=%e: %e %e %5d %d\n", t, x, y, ix, iy);
    pixelColor(disp, ix + OFFSETX, iy + OFFSETY, 0x44fea380);
  }
  plot_doctrine_pts(&doctrine0[0]);
}

int main(int argc, char **argv) {
  SDL_Event event;
  bool running = true;
  int point = 0;

  SDL_Init(SDL_INIT_EVERYTHING);
  disp = SDL_SetVideoMode(800, 600, 32, SDL_DOUBLEBUF | SDL_HWSURFACE);
  if(!disp) return -1;
  atexit(SDL_Quit);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  plot_doctrine(&doctrine0[0]);
  while(running) {
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_MOUSEBUTTONDOWN) running = false;
      else if(event.type == SDL_KEYDOWN) {
	switch(event.key.keysym.sym) {
	case SDLK_ESCAPE:
	  running = false;
	  break;
	case SDLK_1:
	  point = 0;
	  break;
	case SDLK_2:
	  point = 1;
	  break;
	case SDLK_3:
	  point = 2;
	  break;
	case SDLK_4:
	  point = 3;
	  break;
	case SDLK_LEFT:
	  doctrine0[0].x[point] -= event.key.keysym.mod & KMOD_SHIFT ? POSSTEP : POSSTEP * 15;
	  break;
	case SDLK_RIGHT:
	  doctrine0[0].x[point] += event.key.keysym.mod & KMOD_SHIFT ? POSSTEP : POSSTEP * 15;
	  break;
	case SDLK_DOWN:
	  doctrine0[0].y[point] += event.key.keysym.mod & KMOD_SHIFT ? POSSTEP : POSSTEP * 15;
	  break;
	case SDLK_UP:
	  doctrine0[0].y[point] -= event.key.keysym.mod & KMOD_SHIFT ? POSSTEP : POSSTEP * 15;
	  break;
	default:
	  break;
	}
	SDL_FillRect(disp, NULL, 0);
	plot_doctrine(&doctrine0[0]);
      }
    }
    SDL_Flip(disp);
    SDL_Delay(50);
  }
  printf("%s\n", doctrine2str(&doctrine0[0]));
  return 0;
}


//g++ -Wall -O2 indoctrinator.cc -lSDL -lSDL_gfx
