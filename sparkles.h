#ifndef __BANGDOTS_H__2010
#define __BANGDOTS_H__2010
#include "config.h"
#include "u-iff.h"

/*! \brief A list of colours
 *
 * Remember that the colour-list length is one less than the actual
 * number of elements. See the get_life_colour() function for further
 * information.
 */
typedef struct Sparkle_life_colours {
  SDL_Color *clist;	//!< colour list if multiple colours
  unsigned cl_len;	//!< length of colour list excluding the last duplicated element
} sparkle_life_colours_t;

/*! \brief sparkles for the "particle engine"
 *
 * Sparkles can have a single colour or may change colour. The colour
 * is chosen from a list.
 */
typedef struct Sparkle {
  float x;		//!< X-position
  float y;		//!< Y-position
  float dx;		//!< delta X
  float dy;		//!< delta Y
  float life;		//!< When reduced to 0, kill
  float decay;		//!< Amount by which to reduce life each time dot is drawn
  union {
    RD_VIDEO_TYPE col;	//!< colour if a single colour is used
    struct {
      sparkle_life_colours_t *fading;	//!< fading colours
      Uint32 oldfadecol;		//!< we save the last generated colour here for testing if pixel was overwritten
    };
  };
  unsigned short spty;	//!< sparcle type
} sparkle_t;


extern sparkle_life_colours_t hot_colours;
extern sparkle_life_colours_t cool_colours;

Uint32 get_life_colour(float life, sparkle_life_colours_t *fcols, SDL_Surface *surf);

/*! \brief initialise sparkles engine
 *
 * Initialises the engine, the information about parameters and colours
 * is read from the file with filename datafn.
 *
 * \param iff iff context (call by value!)
 * \return NULL on error
 */
sparkle_t *init_sparkles(uiff_ctx_t iff);

/*! \brief Create bangdots
 *
 * Bangdots are used for explosions etc.
 */
void makebangdots(int xbang, int ybang, int xvel, int yvel, SDL_Surface * s,
		  int power, int amtdots);

/*! \brief Draw the sparkles (bangdots (explosion), plume dots, etc.)
 *
 * \param surf Surface to draw on.
 */
void draw_sparkles(SDL_Surface *surf);

/*! \brief Move and age bangdots.
 *
 * This function moves all sparkles and cools them. If they are too
 * cool or have moved out of the screen they are set to inactive
 * status. Also all sparkles are inactivated which collide with
 * something or leave the screen.
 */
void update_sparkles(SDL_Surface *surf);

/*! \brief Reset all sparkles/dots.
 *
 * The sparkles/dots are reset so that no more dots are active.
 */
void reset_sparkles(void);

/* \brief Create the icy plumedots
 *
 * \param num maximum number of plumedots to create
 * \param x coordinate of center
 * \param y coordinate of center
 * \param heading in which direction do the plumes fly?
 * \param angle spreading angle of the plumes, 360 is the full circle
 * \param speed speed of the plumes
 */
void create_plumedots(int num, short x, short y, short heading, short angle, float speed);

#endif
