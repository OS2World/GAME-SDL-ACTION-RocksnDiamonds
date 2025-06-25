#include "infoscreen.h"
#include <assert.h>
#include "SFont.h"

/*
 * This was written while traveling in France on my pandora. Any
 * errors are due to the roads, etc. And the nice red vine, of
 * course...
 */

static struct {
} data = {
};


short int infoscreen_is_initialised = 0;
int infoscreen_enabled = 1;
int infoscreen_linespacing = 24;
/*
 * FORM.ROCK IFSC is the chunk with information about the infoscreen.
 *
 * . IFSC info screen
 *	0000 UBYTE	active? 1=yes, 0=0
 *	0001 UBYTE	pad
 *	0002 UWORD	line spacing in px.
 */

void *init_infoscreen(uiff_ctx_t iff) {
  int32_t chunksize;
  uint16_t ui16;
  int i;

  chunksize = uiff_find_chunk_wflags(&iff, MakeID('I', 'F', 'S', 'C'), IFF_FIND_REWIND);
  assert(printf("FORM.ROCK IFSC size = $%08lX\n", (unsigned long int)chunksize));
  if(chunksize >= 4) {
    i = fgetc(iff.f);
    if(i == 1) infoscreen_enabled = 1; else infoscreen_enabled = 0;
    i = fgetc(iff.f); //Padding
    ui16 = read16(iff.f);
    assert(printf("\tline spacing is $%04hX\n", ui16));
    infoscreen_linespacing = ui16;
  }
  infoscreen_is_initialised = 0x7353;
  return &data;
}


void display_infotext(const struct Entry *entries, SDL_Surface *display) {
  SDL_Rect dest;
  int x, y, i;
  int columny[INFOSCRCREEN_MAX_COLUMNS]; 
  int columnx[INFOSCRCREEN_MAX_COLUMNS];

  assert(infoscreen_is_initialised);
  assert(entries);
  assert(display);
  for(i = 0; i < INFOSCRCREEN_MAX_COLUMNS; ++i) {
    columnx[i] = 21 + (display->w - 42) / 3 * i;
    columny[i] = 72;
  }
  for(; entries->text; ++entries) {
    x = entries->posx;
    y = entries->posy;
    if(entries->column >= 0) {
      assert(entries->column < INFOSCRCREEN_MAX_COLUMNS);
      x = columnx[entries->column];
      y = columny[entries->column] += infoscreen_linespacing;
    }
    PutString(display, x + 48, y, entries->text);
    switch(entries->icontype) {
    case INFONONE:
      break;
    case INFOICON:
      if(entries->icon) {
	dest.x = x;
	dest.y = y;
	SDL_BlitSurface(entries->icon, NULL, display, &dest);
      }
      break;
    case INFOFUNP:
      assert(entries->icondisplayfun);
      dest.x = x;
      dest.y = y;
      (entries->icondisplayfun)(display, &dest);
      break;
    }
  }
}
