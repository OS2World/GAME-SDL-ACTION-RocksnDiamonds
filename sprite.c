#include "sprite.h"
#ifdef DEBUG
#include <stdio.h>
#endif

int draw_sprite(struct sprite *sprite, Sint16 x, Sint16 y,
		SDL_Surface * target, Uint32 now) {
  static SDL_Rect dest;
  static SDL_Surface *source;

  source = get_current_sprite_surface(sprite, now);
  dest.x = x;
  dest.y = y;
  dest.w = source->w;
  dest.h = source->h;
  return SDL_BlitSurface(source, NULL, target, &dest);
}

SDL_Surface *get_current_sprite_surface(struct sprite * sprite, Uint32 now) {
  return sprite->
    surface_ptr[((now -
		  sprite->start_tick) / sprite->speed_divisor) %
		sprite->surface_count];
}

void init_sprite(struct sprite *sprite, SDL_Surface ** surfaces,
		 Uint16 surface_count, Uint16 delay, Uint32 now) {
  sprite->start_tick = now;
  sprite->surface_ptr = surfaces;
  sprite->surface_count = surface_count;
  sprite->speed_divisor = delay;
#ifdef DEBUG
  printf("init sprite=%p start=$%08X divisor=$%04hx count=$%04hx\n",
	 sprite, sprite->start_tick, sprite->speed_divisor, surface_count);
#endif
}

void init_sprite2(struct sprite *sprite, sdl_surfaces_t *surfaces, Uint16 delay, Uint32 now) {
  init_sprite(sprite, surfaces->surfaces, surfaces->num_surfaces, delay, now);
}
