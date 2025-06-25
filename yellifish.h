#ifndef __YELLIFISH_H_2014__
#define __YELLIFISH_H_2014__
#include <SDL/SDL.h>
#include "u-iff.h"
#include "sprite.h"

#define YELLIFISH_TENTACLESEGMENTS 16
#define YELLIFISH_TENTACLENO 4
#define YELLIFISH_MAXNUM 3 //!< maximum number of yellifish

typedef struct Yellifish {
  float posx, posy;
  float velx, vely;
  float offset[YELLIFISH_TENTACLENO]; //!< offset in pixel for the tentacle animation
  unsigned char tentacleparts[YELLIFISH_TENTACLENO][YELLIFISH_TENTACLESEGMENTS]; //!< which tentaclepart
  float tspeed[YELLIFISH_TENTACLENO]; //!< speed for each tentacle
  float length; //!< length is the total length of the long tentacle
  short tentacle_active; //!< is the tentacle active? 0=no, 1=tentacle moving down, -1=tentacle moving up
  unsigned short partoffset[YELLIFISH_TENTACLENO]; //!< offset in the tentacleparts table
  short active; //!< this is > 0 if the yellifish is active, < 0 the yellifish is dying
  struct sprite ysprite; //!< yellifish sprite
} yellifish_t;


typedef struct Yellifishsubsystem {
  struct Yellifish yellifishs[YELLIFISH_MAXNUM];
  int minimum_level; //!< minimum level before yellifish appear
  int level_modulo; //!< for checking level % level_modulo == 0 (only then yellifish do appear)
  float minimum_length; //!< minimum tentacle length
  float maximum_length; //!< maximum tentacle length
  float maximum_velocity; //!< maximum velovity component of yellifish
  float attack_distance; //!< if X-Distance smaller then attack prey
  float delta_velocity; //!< delta velocity per update
  float yelliprobability; //!< probability that a yellifish appears
  short headoffsetx, headoffsety; //!< offset between head image and tentacle images
} yellifishsubsystem_t;


/*! \brief initialise yellifish subsystem
 *
 * This loads the animation graphics, etc. After calling this function
 * init_yellifish() may be called.
 *
 * \return 0 on OK, != 0 on error
 */
yellifishsubsystem_t *init_yellifish_subsystem(uiff_ctx_t iff);

/*! \brief Shutdown yellifish subsystem
 *
 * Shutdown subsystem and free system resources.
 *
 * \param yellifishs pointer to subsystem structure
 */
void shutdown_yellifish_subsystem(yellifishsubsystem_t *yellifishs);

/*! \brief initialise a single yellifish
 *
 * Yellifish is initialised with some default values. Uses the random
 * number generator.
 *
 * \param ysys pointer to yellifish subsystem structure
 * \param yelli pointer to a yellifish structure
 * \param posx starting X-position
 * \param posy starting Y-position
 */
void init_yellifish(yellifishsubsystem_t *ysys, yellifish_t *yelli, int posx, int posy);


/*! \brief activate a yellifish if conditions are right
 *
 * This function will create a new yellifish if there is a free slot
 * available and if the level modulo the level module is zero and a
 * random number is below the random number threshold. Of course the
 * minimum level needed is also taken into account.
 * 
 * \param yelli pointer to yellifish subsystem
 * \param level current level as an int
 * \param x x-position of newly created yellfish
 * \param y y-position of newly created yellfish
 * \return NULL if no yellfish created or no more yellifish slots available, pointer to yellifish_t struct on success
 */
yellifish_t *maybe_activate_yellifish(yellifishsubsystem_t *yelli, int level, int x, int y);

yellifish_t *activate_yellifish(yellifishsubsystem_t *yelli, int x, int y);

/*! \brief yellifish display function
 *
 * This will also update the animation.
 *
 * \param ysys pointer to the yellifish subsystem struct
 * \param yelli pointer to a single yellifish
 * \param surf_screen display on which the yellifish is displayed
 */
void display_yellifish(yellifishsubsystem_t *ysys, yellifish_t *yelli, SDL_Surface *surf_screen);

void display_yellifishs(yellifishsubsystem_t *yelli, SDL_Surface *surf_screen);

/*! \brief update yellifish
 *
 * Update the yellifish positions. This means that the yellifish will follow its prey.
 * \param yelli Yellifish subsystem pointer
 * \param preyx x-position of prey
 * \param preyy y-position of prey
 */
void update_yellifishs(yellifishsubsystem_t *yelli, float preyx, float preyy);

yellifish_t *check_yellifish_hit(yellifishsubsystem_t *yelli, float minlaserx, float maxlaserx, float lasery);

void kill_yellifish(yellifish_t *yptr);

void deactivate_all_yellifish(yellifishsubsystem_t *yelli);

#endif
