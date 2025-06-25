#ifndef __ROCKDODGER_GLOBALS_H__20101222
#define __ROCKDODGER_GLOBALS_H__20101222
#include <SDL/SDL.h>
#include "config.h"
#include "spacedots.h"
#include "u-iff.h"


//! Number of points per yellifish kills
#define YELLIFISH_KILL_POINTS 175

//! Number of points per blubat kills
#define BLUBAT_KILL_POINTS 125

//! Number of points per greeble kills
#define GREEBLE_KILL_POINTS 100

//! Number of points per screen update
#define SCORE_INCREMENT_FACTOR 1

//! Size of the bangdots array
#define MAX_BANG_DOTS 0x20400

//! Maximum number of stars in the background
#define MAX_SPACE_DOTS 0x1000
//! Number of playfields for cicada display
#define NUMBER_CICADA_PLAYFIELDS 4

//! Maximum number of engine exhaust dots
#define MAX_ENGINE_DOTS 0x2000

//! Maximum number of black points around the ship for collision detection.
#define MAX_BLACK_POINTS 150

#define POWERUPDELAY 511	//!< Skip so many frames before we get a new powerup
#define MAX_BLUBAT_IMAGES 8	//!< Max number of blubat image files
#define MAX_GREEBLES 70		//!< Max number of greeblies
#define MAX_ROCKS 120		//!< MaX Rocks visible at once
#define MAX_ROCK_IMAGESS 8	//!< Number of rock image files, not number of rocks visible
#define MAX_POWERUP_IMAGES 64	//!< Maximum number of powerup images for the animation

//! TODO: ???
#define W 100

/* Guru Specific Error Codes / Rockdodger sections */
#define GURU_SEC_bangdots           0x0001
#define GURU_SEC_blubats            0x0002
#define GURU_SEC_datafun            0x0003
#define GURU_SEC_display_subsystem  0x0004
#define GURU_SEC_engine_exhaust     0x0005
#define GURU_SEC_gas_plume          0x0006
#define GURU_SEC_greeblies          0x0007
#define GURU_SEC_guru_meditation    0x0008
#define GURU_SEC_highscore_io       0x0009
#define GURU_SEC_input_functions    0x000A
#define GURU_SEC_intro              0x000B
#define GURU_SEC_laser              0x000C
#define GURU_SEC_main               0x000D
#define GURU_SEC_mood_item          0x000E
#define GURU_SEC_powerup            0x000F
#define GURU_SEC_random_gen         0x0010
#define GURU_SEC_globals 0x0011
#define GURU_SEC_rocks              0x0012
#define GURU_SEC_scroller           0x0013
#define GURU_SEC_SFont              0x0014
#define GURU_SEC_ship               0x0015
#define GURU_SEC_signal_handling    0x0016
#define GURU_SEC_sound              0x0017
#define GURU_SEC_spacedots          0x0018
#define GURU_SEC_sprite             0x0019
#define GURU_SEC_sparkles	    0x001A
#define GURU_SEC_yellifish	    0x001B
#define GURU_SEC_infoscreen	    0x001C

/*! \brief Simple function for updating a surface
 */
typedef void (*Simple_surface_update_function)(SDL_Surface *);

extern char *edition_name;

//! Width of the screen
extern int xsize;

//! Height of the screen
extern int ysize;

//Surfaces
extern SDL_Surface *surf_green_block;	//!< Just a green block for highlighting
extern SDL_Surface *surf_t_rock;	//!< Title element "rock"
extern SDL_Surface *surf_t_dodger;	//!< Title element "dodgers"

/*! \brief screen display surface
 *
 * This actually is set in set_video_mode() and can also be get by a
 * call to SDL_GetVideoSurface(). I do not know how fast this function
 * call is and if the variable is actually useless... (RPK)
 */
extern SDL_Surface *surf_screen;

/*! \brief pointer to spacedots (background) engine
 */
extern spacedots_t *current_spacedot_engine;

//! Fullscreen?
extern unsigned short opt_fullscreen;
extern unsigned short opt_cicada_spacedots; //!< use cicada space dots or normal ones?
extern unsigned short opt_intro; //!< should the intro be displayed?

/*! \brief Speed of the game
 *
 * This is set to number of ticks since last frame divided by 50. A
 * value of one means we are running at 50 frames per second. If the
 * game runs slower or faster it can be scaled appropriately.
 *
 */
extern float movementrate;

extern float level; //!< The current game level. Take care! This is a float!
extern float shieldlevel, laserlevel, shieldpulse;
extern int initialshield;
extern RD_VIDEO_TYPE heatcolor[W * 3];


//! Available game states
enum states {
  TITLE_PAGE,
  GAME_PLAY,
  DEAD_PAUSE,
  GAME_OVER,
  GAME_PAUSED,
  HIGH_SCORE_ENTRY,
  HIGH_SCORE_DISPLAY,
  DEMO,
  SETUP,
  INFO_SCREEN,
  RESTART_GAME,
  QUIT_GAME
};

extern enum states state;	//!< current game state
extern float state_timeout;	//!< timeout before next game state switch
extern float fadetimer;		//!< timer for the on and off appearence of the ghostly rock dodger
extern Uint32 initticks;	//!< set at each start of the gameloop
extern Uint32 last_ticks;	//!< set at each *end* of the draw function
extern Uint32 ticks_since_last;	//!< at end of draw function: ticks_since_last = SDL_GetTicks()-last_ticks;
extern Uint32 ign_k_utl_ticks;	//!< ignore all keys until this tick count has been reached
extern int music_volume;	//!< the global volume used for playing the music, see sound.c.
extern int nships;		//!< Number of ships the player has.
extern int last_levelup_snd;	//!< When was the last sound on level change?
extern uiff_ctx_t *iff_ctx;	//!< global context for rockdodger iff file

/*! \brief clean up global variables
 *
 * This will clean up the global variables and bring them into a sane
 * state. This is necessary if we like to restart the game, etc.
 */
void cleanup_globals();


/*! \brief update movement rate and tick counters
 *
 * This will set the global variables ticks_since_last, last_ticks,
 * and movementrate.
 */
void update_movement_rate();


/*! \brief Reinitialise all game parameters for a fresh start.
 *
 * \return 0 = OK, 1 = ERROR
 */
int restart_game();


/*! \brief Setuploop during game
 *
 */
void setuploop();


void save_setup();


int load_setup();


/*! \brief Get rockdodger iff file context
 *
 * This will open the file fname and create an iff context if this a
 * rockdodger file. If not it will return NULL.
 *
 * \param fname file name to open
 * \return NULL or pointer to iff context
 */
uiff_ctx_t *get_iff_rock(const char *fname);

/*! \brief Load the edition name from the iff context
 *
 * This will load the edition from the parameter file. The context is
 * passed by value as this is the easiest way to handle changes to the
 * structure. Remember to free the string. If the edition can not be
 * loaded "#unknown#" is returned.
 *
 * \param iff iff context to load from
 * \return strdup of edition name
 */
char *load_edition_name(uiff_ctx_t iff);

#endif
