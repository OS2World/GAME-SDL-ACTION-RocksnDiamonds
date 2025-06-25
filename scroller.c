#include "scroller.h"
#include "SFont.h"
#include <assert.h>

#define SCROLLER_GAP 45.0

Scroller_t *init_scroller(const char *stext, Sint16 pos, float defspeed, Uint16 visiwidth) {
  int text_scroller_width;
  Scroller_t *scroller = calloc(1, sizeof(Scroller_t));
  SDL_Surface *surf = NULL;

  if(scroller) {
    scroller->position = -((float)visiwidth) - SCROLLER_GAP;
    scroller->speed = defspeed;
    scroller->y = pos;
    text_scroller_width = SFont_wide(stext);
#ifdef DEBUG
    printf("Text scroller surface has a width of %d pixels.\n", text_scroller_width);
#endif
    /*
     * Just create a surface, no special needs, etc. Therefore we use
     * a simple 2-bit surface, give it a colorkey for transparency and
     * convert it to the display format.
     */
    if((surf = SDL_CreateRGBSurface(SDL_SWSURFACE, text_scroller_width, 36, 2, 0, 0, 0, 0)) != NULL) {
      SDL_SetColorKey(surf, SDL_SRCCOLORKEY, 0); //Set black (default fill?) to transparent
      scroller->playfield = SDL_DisplayFormat(surf); //Convert to display format (no problems with rgbamask, etc.)
      if(scroller->playfield == NULL) goto on_error;
      SDL_FreeSurface(surf);
      surf = NULL;
      PutString(scroller->playfield, 0, 0, stext);
    } else goto on_error;
  }
  return scroller;
 on_error:
  if(surf != NULL) SDL_FreeSurface(surf);
  if(scroller != NULL) free(scroller);
  return NULL;
}

void destroy_scroller(Scroller_t *scroller) {
  SDL_FreeSurface(scroller->playfield);
  free(scroller);
}

void update_and_draw_scroller(Scroller_t *scroller, SDL_Surface *target, float speed) {
  SDL_Rect r = { 0, scroller->y, 0, 0 };

  assert(scroller != NULL);
  assert(target != NULL);
  scroller->position += scroller->speed * speed;
  if(scroller->position <= -target->w) {
  } else if(scroller->position <= 0) {
    r.w = target->w + scroller->position;
    r.x = -scroller->position;
    SDL_BlitSurface(scroller->playfield, NULL, target, &r);
  } else if(scroller->position > scroller->playfield->w){
    scroller->position = -target->w - SCROLLER_GAP;
  } else {
    SDL_Rect src = { scroller->position, 0, target->w, scroller->playfield->h };
    r.w = target->w;
    r.x = 0;
    SDL_BlitSurface(scroller->playfield, &src, target, &r);
  }
}
