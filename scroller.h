#ifndef __SCROLLER_H_20130126__
#define __SCROLLER_H_20130126__
#include <SDL/SDL_video.h>


typedef struct Scoller_t {
  SDL_Surface *playfield; //!< scroller surface with the whole text
  float position; //!< position of the scroller
  float speed; //!< default speed multiplier per frame
  Sint16 y; //!< at which position is the scroller?
} Scroller_t;

/*!\brief Intialise a scroller structure
 *
 * Frees memory on errors.
 *
 * \param stext text for scroller
 * \param pos y position of the scroller
 * \param defspeed default speed per frame
 * \param visiwidth visible width of the scroller
 * \return pointer to Scroller_t structure or NULL
 */
Scroller_t *init_scroller(const char *stext, Sint16 pos, float defspeed, Uint16 visiwidth);

/*! \brief Free memory of the scroller
 *
 */
void destroy_scroller(Scroller_t *scroller);


/*! \brief Update scroller and draw it on the target surface
 *
 * \param scroller Scroller_t structure pointer
 * \param target Surface to draw on
 * \param speed Speed fraction per frame, 1.0 = exact framerate was hit
 */
void update_and_draw_scroller(Scroller_t *scroller, SDL_Surface *target, float speed);

#endif
