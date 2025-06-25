/*
    Space Rocks! Avoid the rocks as long as you can!
    Copyright (C) 2001  Paul Holt <pcholt@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

// includes
#include <SDL/SDL_image.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "config.h"
#include "globals.h"
#include "SFont.h"
#include "sound.h"
#include "random_gen.h"
#include "sparkles.h"
#include "blubats.h"
#include "display_subsystem.h"
#include "engine_exhaust.h"
#include "game_state.h"
#include "greeblies.h"
#include "guru_meditation.h"
#include "highscore_io.h"
#include "input_functions.h"
#include "intro.h"
#include "laser.h"
#include "mood_item.h"
#include "powerup.h"
#include "rocks.h"
#include "ship.h"
#include "datafun.h"
#include "signal_handling.h"
#include "sekrit_lab.h"
#include "yellifish.h"
#include "infoscreen.h"

// constants
#define M 255
#define BIG_FONT_FILE "20P_Betadance.png"
#define ROCKIFF "iff.configdata.rock" //!< rockdodger iff file

// macros
#define COND_ERROR(a) if ((a)) {initerror=strdup(SDL_GetError());return 1;}
#define NULL_ERROR(a) COND_ERROR((a)==NULL)
#define NULL_GURU(a, b, c) if((a)==NULL){initerror=strdup(SDL_GetError()); guru_meditation(GM_FLAGS_DEADEND, (b), (void*)(c)); return 1;}

typedef struct Game_Over_Structure {
  SDL_Surface *surf_t_game;	// Title element "game"
  SDL_Surface *surf_t_over;	// Title element "over"
} GameOver_t;

// SDL_Surface global variables
SDL_Surface *surf_t_paused;	// Title element "paused"
SDL_Surface *surf_font_big;	// The big font
SDL_Surface *surf_gauge_shield[2];	// The shield gauge 0=on,1=off
SDL_Surface *surf_gauge_laser[2];	// The laser gauge 0=on,1=off

Scroller_t *highscore_scroller = NULL; //!< Scroller for highscore

yellifishsubsystem_t *yellifishs; //!< array of yellifishs and more
yellifish_t yellifish_for_infoscreen;
SDL_Surface *mood_icon; //!< mood item icon for the infoscreen

// Other global variables
char topline[128];
char *initerror = "";
char *rockiff_name = NULL; //!< filename of rockdodger binary (iff) file with parameters/manipulation/configuration contents

int laser = 0;
int oss_sound_flag = 0;
float faderate;

struct min_max {
  int min, max;
} rockfree_space_mm[121];
int rockfree_space_num = 0;
int rockfree_space_max = 0;

static const char *sequence[] = {
  "Rock Dodger",
  "Press SPACE to start!",
  "http://bitbucket.org/rpkrawczyk/rockdodger/",
  "Arrow keys move the ship.",
  "Press S for shield and D for laser.",
  "If you need a pause press P.",
  "Pressing F1 gets you to the setup screen.",
  "You can bounce off the sides of the screen.",
  "Caution, do not try this in real life. Space has no sides.",
  "",
  "If you have a <500Mhz machine -- buy a new one!",
  "",
  "The little green guys are called Greeblies.",
  "Later in the game you will meet their friends...",
  "...the very dangerous Blubats!",
  "Watch out! Blubats may leave some droppings!",
  "Blubat droppings should not be inhaled!",
  "Further on, you will meet the Yellifish,",
  "devious creatures they are... Beware!",
   "Cunning predators hunting you down!",
   "",
  "Code: RPK, Paul Holt",
  "GfX: RPK, Paul Holt",
  "Music: Jack Beatmaster, zake, Strobo, Roz",
  "",
  "Small icons floating leisurely over the screen",
  "are powerups. Get them!",
  "",
  "The engine on the back of the ship provides no thrust.",
  "It's a sparkler, there for effect.",
  ""
};
const int nsequence = sizeof(sequence) / sizeof(char *);

extern char *optarg;
extern int optind, opterr, optopt;

unsigned short screenshot_number = 0; /*!< Count the number of screen
                                         shots. This makes is possible
                                         to produce multiple
                                         screenshots per run.
				       */

// ************************************* FUNCS

/*! \brief Reinitialise all game parameters for a fresh start.
 *
 * \return 0 = OK, 1 = ERROR
 */
int restart_game() {
  static const char *txts[] = {
    "Congratulations! You have a high score.",
    "We have a winner!",
    "Wot a hero!",
    "A place in the hall of fame has been reserved for you!",
    "Amazing performance! You earned a high score.",
    "Unbelievable, do you do anything besides playing this game?",
    "Oh, come on... Admit it... You cheated, did you not?",
    /* Do you remember Uridium? */
    "You have amassed a great score!"
  };
  const int num_of_txts = sizeof(txts) / sizeof(const char *);
  char buf[160];

  state = TITLE_PAGE;
  cleanup_globals();
  destroy_space_dots(current_spacedot_engine);
  NULL_ERROR(current_spacedot_engine = init_space_dots_engine(surf_screen, opt_cicada_spacedots, *iff_ctx));
  if(surf_green_block) SDL_FreeSurface(surf_green_block);
  surf_green_block = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA, surf_screen->w, 22, 32, surf_screen->format->Rmask, surf_screen->format->Gmask, surf_screen->format->Bmask, surf_screen->format->Amask);
  NULL_ERROR(surf_green_block);
  SDL_FillRect(surf_green_block, NULL, SDL_MapRGB(surf_screen->format, 0, 0xa8, 0x1c));
  if(highscore_scroller) destroy_scroller(highscore_scroller);
  strcat(strcpy(buf, txts[SDL_GetTicks() % num_of_txts]), " Please enter your name using the keyboard and press enter!");
  assert(strlen(buf) < sizeof(buf) - 1);
  NULL_ERROR(highscore_scroller = init_scroller(buf,
						14 * font_height + 110,
						7.28,
						surf_screen->w));

  deactivate_greeblies();
  return 0;
}


void drawlaser() {
  int i, xc, hitrock, hitgreeb;
  yellifish_t *yptr;

  if(laserlevel < 0)
    return;
  laserlevel -= movementrate;
  if(laserlevel < 0) {
    // If the laser runs out completely, there will be a delay before it can be brought back up
    laserlevel = -W / 2;
    return;
  }
  hitrock = -1;
  hitgreeb = -1;
  xc = xsize;
  // let xc = x coordinate of the collision between the laser and a space rock
  // 1. Calculate xc and determine the asteroid that was hit
  for(i = 0; i < MAX_ROCKS; i++) {
    if(rock[i].active) {
      SDL_Surface *rocksurf = get_rock_surface(i, 0);
      if(shipdata.yship + 12 > rock[i].y && shipdata.yship + 12 < rock[i].y + rocksurf->h
	 && shipdata.xship + 32 < rock[i].x + (rocksurf->w / 2)
	 && rock[i].x + (rocksurf->w / 2) < xc) {
	xc = rock[i].x + (rocksurf->w / 2);
	hitrock = i;
      }
    }
  }
  for(i = 0; i < MAX_GREEBLES; i++) {
    int greebheight = 16;
    int greebwidth = 16;
    struct greeble *g;
    g = the_greeblies + i;

    if(g->active) {
      int greebx = (int) g->x;
      int greeby = (int) g->y;
      if(g->landed) {
	greebx += rock[g->target_rock_number].x;
	greeby += rock[g->target_rock_number].y;
      }
      if(shipdata.yship + 12 > greeby && shipdata.yship + 12 < greeby + greebheight &&
	 shipdata.xship + 32 < greebx + (greebwidth / 2) && greebx + (greebwidth / 2) <
	 xc) {
	xc = greebx + (greebwidth / 2);
	hitgreeb = i;
      }
    }
  }

  if(hitrock >= 0)
    heat_rock_up(hitrock, movementrate);
  if(hitgreeb >= 0)
    kill_greeb(hitgreeb);
  if((yptr = check_yellifish_hit(yellifishs, shipdata.xship + 32, xc, shipdata.yship + 12))) {
    inc_score(shipdata.xship + 32, shipdata.yship + 12, YELLIFISH_KILL_POINTS);
    kill_yellifish(yptr);
  }
  if((hitgreeb = check_blubat_hit((int) shipdata.yship + 12, xc)) >= 0) {
    kill_blubat(hitgreeb);
  }
  // Plot a number of random dots between shipdata.xship and xsize (or xc as in x-collision)
  draw_random_dots(surf_screen, shipdata.yship + 12, shipdata.xship + 32, xc);
}

/*! \brief Set the SDL video mode
 *
 * This function sets the video mode and surf_screen. It will use the
 * fullscreen flag to decide if it is a fullscreen surface or not.
 *
 * \return the new surf_screen
 */
SDL_Surface *set_video_mode() {
  Uint32 flag;

  // Attempt to get the required video size
  flag = SDL_DOUBLEBUF | SDL_HWSURFACE;
  if(opt_fullscreen)
    flag |= SDL_FULLSCREEN;
  surf_screen = SDL_SetVideoMode(xsize, ysize, RD_VIDEO_BPP, flag);
  return surf_screen;
}


int init(GameOver_t *game_over_data) {
  int i;

  // Make sure that the name_input_buf is null terminated.
  *name_input_buf = '\0';
  read_high_score_table();
  data_dir = get_data_dir();
  if(oss_sound_flag) {
    // Initialise SDL with audio and video
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
      oss_sound_flag = 0;
      printf("Can't open sound, starting without it\n");
      atexit(SDL_Quit);
    } else {
      atexit(SDL_Quit);
      atexit(SDL_CloseAudio);
      oss_sound_flag = init_sound();
    }
  } else {
    // Initialise with video only
    COND_ERROR(SDL_Init(SDL_INIT_VIDEO) != 0);
    atexit(SDL_Quit);
  }

  // Attempt to get the required video size
  set_video_mode();
  NULL_ERROR(surf_screen);
  NULL_GURU(iff_ctx = get_iff_rock(rockiff_name != NULL ? rockiff_name : load_file(ROCKIFF)), GM_FLAGS_DEADEND | GM_FLAGS_ABORTIFY | GM_FLAGS_CHOICE | GM_SS_BootStrap | GM_GE_OpenLib | GURU_SEC_main, &get_iff_rock);
  edition_name = load_edition_name(*iff_ctx);
  //TODO: Add a splash screen...
  SDL_FillRect(surf_screen, NULL, SDL_MapRGB(surf_screen->format, 0x2e, 0x2e, 0x2e));
  SDL_Flip(surf_screen);

  // Attempt to initialize a joystick
  if(joystick_flag) {
    if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
      joystick_flag = 0;
      printf("Can't initialize joystick subsystem, starting without it.\n");
    } else {
      int njoys;
      njoys = SDL_NumJoysticks();
      printf("%d joystick(s) detected\n", njoys);
      if(njoys == 0) {
	printf
	  ("That's not enough joysticks to start with joystick support\n");
	joystick_flag = 0;
      } else {
	joysticks[0] = SDL_JoystickOpen(0);
	if(joysticks[0] == NULL) {
	  printf("Couldn't open joystick %d\n", 0);
	  joystick_flag = 0;
	}
      }
    }
  }

  set_signalhandling_up();
  // Set the title bar text
  SDL_WM_SetCaption("Rock Dodgers", "rockdodgers");

  // Set the heat color from the range 0 (cold) to 300 (blue-white)
  for(i = 0; i < W * 3; i++) {
    heatcolor[i] = SDL_MapRGB(surf_screen->format,
			      (i < W) ? (i * M / W) : (M),
			      (i < W) ? 0 : (i <
					     2 * W) ? ((i - W) * M / W) : M,
			      (i < 2 * W) ? 0 : ((i - W) * M / W)
			      // Got that?
			      );
#ifdef DEBUG
  printf("heatcolor[%04x] = { %02x, %02x, %02x }\t", i,
			      (i < W) ? (i * M / W) : (M),
			      (i < W) ? 0 : (i <
					     2 * W) ? ((i - W) * M / W) : M,
			      (i < 2 * W) ? 0 : ((i - W) * M / W)
	 ); //No, not really...
  if(i == W * 3 - 1) putchar('\n');
#endif
  }
  // Load global font
  if(!(surf_font_big = IMG_Load(load_file(BIG_FONT_FILE)))) {
    guru_meditation(GM_FLAGS_DEADEND, 0x30060000, (void*)0x464f4e54);
    return 1;
  }
  InitFont(surf_font_big);

  // Load the Title elements
  // GM_SS_Graphics | GM_GE_IOError
  NULL_GURU(surf_t_rock = load_image("rock.png", 255, 0, 0), GM_SS_Graphics | GM_GE_IOError, 0x524f434b);
  NULL_GURU(surf_t_dodger = load_image("dodgers.png", 255, 0, 0), GM_SS_Graphics | GM_GE_IOError, 0x444f4447);
  NULL_GURU(game_over_data->surf_t_game = load_image("game.png", 255, 0, 0), GM_SS_Graphics | GM_GE_IOError, 0x47414d45);
  NULL_GURU(game_over_data->surf_t_over = load_image("over.png", 255, 0, 0), GM_SS_Graphics | GM_GE_IOError, 0x4f564552);
  NULL_GURU(surf_t_paused = IMG_Load(load_file("paused.png")), GM_SS_Graphics | GM_GE_IOError, 0x50415553);
  NULL_GURU(surf_gauge_laser[0] = IMG_Load(load_file("laser0.png")), GM_SS_Graphics | GM_GE_IOError, 0x4c415330);
  NULL_GURU(surf_gauge_laser[1] = IMG_Load(load_file("laser1.png")), GM_SS_Graphics | GM_GE_IOError, 0x4c415331);
  NULL_GURU(surf_gauge_shield[0] = IMG_Load(load_file("shield0.png")), GM_SS_Graphics | GM_GE_IOError, 0x53484930);
  NULL_GURU(surf_gauge_shield[1] = IMG_Load(load_file("shield1.png")), GM_SS_Graphics | GM_GE_IOError, 0x53484931);

  // Load the little spaceship surface from the spaceship file
  NULL_GURU(surf_small_ship = load_image("ship_small.png", 0, 255, 0), GM_SS_Graphics | GM_GE_IOError, 0x5348534d);

  NULL_GURU(init_ship(), 0x40000000 | GM_GE_ProcCreate, 0x53484950);
  init_engine_dots();
  NULL_GURU(init_infoscreen(*iff_ctx), GM_SS_Misc | GM_GE_OpenDev | GURU_SEC_infoscreen, &init_infoscreen);
  NULL_ERROR(init_blubats(*iff_ctx));
  NULL_ERROR(init_powerup());
  NULL_ERROR(mood_icon = init_mood_item(*iff_ctx));
  // Load all our lovely rocks
  NULL_ERROR(init_rocks());
  NULL_ERROR(init_greeblies());
  NULL_ERROR((yellifishs = init_yellifish_subsystem(*iff_ctx)));
  init_yellifish(yellifishs, &yellifish_for_infoscreen, 0, 0);
  NULL_GURU(init_sparkles(*iff_ctx), GM_SS_Graphics | GM_GE_OpenLib | GURU_SEC_sparkles, &init_sparkles);
  // Remove the mouse cursor
#ifdef SDL_DISABLE
  SDL_ShowCursor(SDL_DISABLE);
#endif

  return 0;
}


void showgauge(int x, SDL_Surface * surf[2], float fraction) {
  static int endx, w;
  static SDL_Rect src, dest;
  src.y = 0;
  if(fraction > 0) {
    if(fraction > 1)
      fraction = 1.0;
    src.x = 0;
    w = src.w = surf[0]->w * fraction;
    src.h = surf[0]->h;
    endx = src.w;
    dest.w = src.w;
    dest.h = src.h;
    dest.x = x;
    dest.y = ysize - src.h - 10;
    SDL_BlitSurface(surf[0], &src, surf_screen, &dest);
  } else {
    endx = 0;
    w = 0;
  }

  src.x = endx;
  src.w = surf[1]->w - w;
  src.h = surf[1]->h;
  dest.w = src.w;
  dest.h = src.h;
  dest.x = x + endx;
  dest.y = ysize - src.h - 10;
  SDL_BlitSurface(surf[1], &src, surf_screen, &dest);
}


void display_version_n_text() {
  char buf[120];
  const char *text;

  assert(edition_name);
  sprintf(buf, "Version %s - The %s edition  ( %s )", VERSION, edition_name, COMPILEDATE);
  int x = (xsize - SFont_wide(buf)) / 2 + sinf(fadetimer / 4.5) * 10;
  PutString(surf_screen, x, ysize - 50 + sinf(fadetimer / 2) * 5, buf);

  text = sequence[(int) (fadetimer / 40) % nsequence];
  x = (xsize - SFont_wide(text)) / 2 + cosf(fadetimer / 4.5) * 10;
  PutString(surf_screen, x, ysize - 100 + cosf(fadetimer / 3) * 5, text);
  fadetimer += movementrate / 2.0;
}


/*! \brief Display de list o high scores mon.
 */
void display_highscores() {
  unsigned int i;
  char s[128];

  PutString(surf_screen, 180, 50, "High scores");
  for(i = 0; i < MAXIMUM_HIGH_SCORE_ENTRIES; ++i) {
    sprintf(s, "#%1d", i + 1);
    PutString(surf_screen, 150, 50 + (i + 2) * font_height, s);
    sprintf(s, "%4ld", high[i].score);
    PutString(surf_screen, 200, 50 + (i + 2) * font_height, s);
    sprintf(s, "%3s", high[i].name);
    PutString(surf_screen, 330, 50 + (i + 2) * font_height, s);
  }
}


/*! \brief Enter the highscore
 *
 * Called once per frame from draw() if we are in highscore enter
 * mode.
 */
void enter_highscore(void) {
  int i;

  assert(state == HIGH_SCORE_ENTRY);
  for(i = 0; i < MAXIMUM_HIGH_SCORE_ENTRIES - 1; ++i) {
    if(high[i].score < high[i + 1].score) {
      i = guru_meditation(GM_FLAGS_ABORTIFY | GM_FLAGS_DEADEND, GM_SS_GamePort | GM_GE_BadParm | GURU_SEC_main, &enter_highscore);
      printf("guru_meditation() = %d\n", i);
      return;
    }
  }
  if(scorerank >= 0) {
    play_tune(2);
    if(SFont_Input
       (surf_screen, 330, 50 + (scorerank + 2) * font_height, 300, name_input_buf)) {
      // Insert new high score and name into high score table
      high[scorerank].score = score;
      high[scorerank].name = strdup(name_input_buf);
      high[scorerank].allocated = 1;
      scorerank = -1;		//we are unworthy again

      // Set the global name string to "", ready for the next winner
      name_input_buf[0] = 0;

      // Change state to briefly show high scores page
      state = HIGH_SCORE_DISPLAY;
      state_timeout = 200;

      // Write the high score table to the file
      write_high_score_table();

      // Play the title page tune
      play_tune(0);
    }
  } else {
    state = HIGH_SCORE_DISPLAY;
    state_timeout = 400;
    play_tune(0);
  }
}


void draw_titlepage(void) {
  draw_ghostly_rock_dodger(surf_t_rock, surf_t_dodger);
  display_version_n_text();
}

/*! \brief Draw the fading game over.
 *
 * This function draws the gostly GAME OVER once the last ship was
 * lost. Currently these are also global variables but this is goint
 * to change.
 *
 * \param fadetimer timer variable
 * \param gost game over structure pointer
 * \param surf_screen target-screen surface
 */
void draw_gameover(float fadetimer, const GameOver_t *gost, SDL_Surface *surf_screen) {
  SDL_Rect src, dest;
  float fadegame, fadeover;

  if(fadetimer < 3.0 / faderate) {
    fadegame = fadetimer / (3.0 / faderate);
  } else {
    fadegame = 1.0;
  }
  if(fadetimer < 3.0 / faderate) {
    fadeover = 0.0;
  } else if(fadetimer < 6.0 / faderate) {
    fadeover = ((3.0 / faderate) - fadetimer) / (6.0 / faderate);
  } else {
    fadeover = 1.0;
  }
  src.w = gost->surf_t_game->w;
  src.h = gost->surf_t_game->h;
  src.x = 0;
  src.y = 0;
  dest.w = src.w;
  dest.h = src.h;
  dest.x = (xsize - src.w) / 2;
  dest.y = (ysize - src.h) / 2 - 40;
  SDL_SetAlpha(gost->surf_t_game, SDL_SRCALPHA,
	       (int) (fadegame *
		      (200 + 55 * cosf(fadetimer))));
  SDL_BlitSurface(gost->surf_t_game, &src, surf_screen, &dest);
  src.w = gost->surf_t_over->w;
  src.h = gost->surf_t_over->h;
  dest.w = src.w;
  dest.h = src.h;
  dest.x = (xsize - src.w) / 2;
  dest.y = (ysize - src.h) / 2 + 40;
  SDL_SetAlpha(gost->surf_t_over, SDL_SRCALPHA,
	       (int) (fadeover * (200 + 55 * sinf(fadetimer))));
  SDL_BlitSurface(gost->surf_t_over, &src, surf_screen, &dest);
}


void yellifish_displayer(SDL_Surface *target, SDL_Rect *dest) {
  yellifish_for_infoscreen.posx = dest->x + yellifishs->headoffsetx;
  yellifish_for_infoscreen.posy = dest->y + yellifishs->headoffsety;
  display_yellifish(yellifishs, &yellifish_for_infoscreen, target);
}


void display_infoscreen() {
  struct Entry infotxt[] = {
#ifndef NDEBUG
    { "DEBUG", 640, 400, {INFOICON, {surf_small_ship}}, -1 },
#endif
    { "The good", 0, 0, {INFONONE}, 0 },
    { "", 0, 0, {INFONONE}, 0 },
    { "Laser powerup", 0, 0, {INFOICON, {surf_powerups[POWERUP_LASER]->surfaces[0]}}, 0 },
    { "shield powerup", 0, 0, {INFOICON, {surf_powerups[POWERUP_SHIELD]->surfaces[0]}}, 0 },
    { "Extra life", 0, 0, {INFOICON, {surf_powerups[POWERUP_LIFE]->surfaces[0]}}, 0 },
    /* ------------------------------------------------------------ */
    { "The bad", 0, 0, {INFONONE}, 1 },
    { "", 0, 0, {INFONONE}, 1 },
    { "Rocks", 0, 0, {INFOICON, {get_rock_surface(0, 0)}}, 1 },
    { "Greeblies (100 pts)", 0, 0, {INFOICON, {surf_greeblies->surfaces[0]}}, 1 },
    { "Blubats (125 pts)", 0, 0, {INFOICON, {surf_blubats->surfaces[0]}}, 1 },
    { "Yellifish (175 pts)", 0, 0, {INFOFUNP, {.icondisplayfun = &yellifish_displayer}}, 1},
    /* ------------------------------------------------------------ */
    { "The other", 0, 0, {INFONONE}, 2 },
    { "", 0, 0, {INFONONE}, 2 },
    { "Your ship", 0, 0, {INFOICON, {surf_small_ship}}, 2 },
    { "Mood items (harmless)", 0, 0, {INFOICON, {mood_icon}}, 2 },
    { NULL }
  };

  display_infotext(infotxt, surf_screen);
}


/*! \brief Handle powerups
 *
 * This function checks if there is a powerup displayed, if the ship
 * has "hit" it, and than handles the powerup procedure.
 *
 * \param shieldsup is the shield up? 
 */
void handle_powerups(int shieldsup) {
  // If the game displays the laserpowerup. The check is done
  // explicitly.
  if(get_current_powerup() != POWERUP_NONE) {
    display_powerup(surf_screen);
    //Yes, shields protect against *everything*!
    if(!((initialshield > 0) || (shieldsup && shieldlevel > 0))) {
      int bangpowerup = ship_check_collision(surf_screen);
      if(bangpowerup) {
	switch(get_current_powerup()) {
	case POWERUP_LASER:
	  laserlevel += 16;
	  if(laserlevel > 3 * W) {
	    laserlevel = 3 * W;
	  }
	  break;
	case POWERUP_SHIELD:
	  shieldlevel += 13;
	  if(shieldlevel > 3 * W) {
	    shieldlevel = 3 * W;
	  }
	  break;
	case POWERUP_LIFE:
	  nships += 1; //Increase number of ships.
	  break;
	  /* None will never happen as it is checked above. But better
	     to be save then sorry... */
	case POWERUP_NONE:
	case POWERUP_MAXPOWERUP:
	  puts("Illegal powerup!");
	  play_sound(4);
	  /*
	   * We use this goto so that only one sound is played. The
	   * powerup must always be deactivated.
	   */
	  goto illegal_powerup_encounterd;
	  break;
	}
	play_sound(5);
      illegal_powerup_encounterd:
	deactivate_powerup();
      }
    }
  }
}

static void draw_paused(SDL_Surface *screen) {
  SDL_Rect dest;
  dest.w = surf_t_paused->w;
  dest.h = surf_t_paused->h;
  dest.x = (xsize - dest.w) / 2;
  dest.y = (ysize - dest.h) / 2;
  SDL_BlitSurface(surf_t_paused, NULL, screen, &dest);
}


/*! \brief Draw all
 *
 * Draw all the objects on the standard surface.
 *
 * \param shieldsup is the shield up?
 * \param game_over_data pointer to game over structure
 * \return bang=0 if everything ok, bang=1 if ship was hit.
 */ 
int draw(int shieldsup, const GameOver_t *game_over_data) {
  int i;
  int bang;

  bang = 0;
  draw_background_objects();
  // If it's firing, draw the laser
  if(laser) {
    drawlaser();
  } else {
    if(laserlevel < 3 * W) {
      laserlevel += movementrate / 4;
    }
  }

  // Draw ship
  if(state == GAME_PLAY || state == GAME_PAUSED || state == DEMO) {
    draw_ship(surf_screen);
    // Create more engine dots comin out da back
    if(state == GAME_PAUSED) {
      create_engine_dots(87);
    } else {
      create_engine_dots(250);
    }
  }

  // Draw all the rocks, in all states. 
  display_rocks(surf_screen);
  // Draw all blubats
  display_blubats(surf_screen);
  // Draw the yellifish
  display_yellifishs(yellifishs, surf_screen);

  switch (state) {
  case GAME_OVER:
    // If it's game over, show the game over graphic in the dead centre
    draw_gameover(fadetimer, game_over_data, surf_screen);
    fadetimer += movementrate / 1.0;
    break;
  case RESTART_GAME:
  case TITLE_PAGE:
    draw_titlepage();
    break;
  case HIGH_SCORE_ENTRY:
    enter_highscore();
    update_and_draw_scroller(highscore_scroller, surf_screen, movementrate);
    display_highscores();
    break;
  case HIGH_SCORE_DISPLAY:
    display_highscores();
    display_version_n_text();
    break;
  case INFO_SCREEN:
    display_infoscreen();
    display_version_n_text();
    break;
  case GAME_PAUSED:
    draw_paused(surf_screen);
    break;
  case DEAD_PAUSE:
  case GAME_PLAY:
    // This many points per screen update.
    inc_score(0, 0, SCORE_INCREMENT_FACTOR);
    break;
  case DEMO:
#if defined(DEBUG)
    {
      int i;
      for(i = 0; i < rockfree_space_num; ++i) {
	SDL_Rect rect = { xsize - 10, rockfree_space_mm[i].min, 9, rockfree_space_mm[i].max - rockfree_space_mm[i].min};
	Uint32 col = i != rockfree_space_max ? SDL_MapRGB(surf_screen->format, 0xa8, 0x1, 0x22) : SDL_MapRGB(surf_screen->format, 0x8, 0xb1, 0x22);
	SDL_FillRect(surf_screen, &rect, col);
      }
    }
#endif
    display_version_n_text();
    break;
  case SETUP:
    //Not done here!
    assert(0);
  case QUIT_GAME:
    break;			// handled below
  }

  if(state == GAME_PLAY || state == DEMO) {
    // Show the freaky shields
    if((initialshield > 0) || (shieldsup && shieldlevel > 0)) {
      draw_ship_shield(surf_screen);
    } else {
      // When the shields are off, check that the black points 
      // on the ship are still black, and not covered up by rocks
      bang = ship_check_collision(surf_screen);
    }
    handle_powerups(shieldsup);
  }

  // Draw all the little ships
  if(state == GAME_PLAY || state == DEAD_PAUSE || state == GAME_OVER || state == DEMO) {
    draw_little_ships(nships, surf_screen);
    // Show the shield gauge
    showgauge(10, surf_gauge_shield, shieldlevel / (3 * W));
    // Show the laser gauge
    showgauge(200, surf_gauge_laser, laserlevel / (3 * W));
  }

#ifdef DEBUG
  //printf("last_ticks=%08X ticks_since_last=%02X movementrate=%10.4e\n", last_ticks, ticks_since_last, movementrate);
  char tempbuf[80];
  sprintf(tempbuf, "state_timeout=%g", state_timeout);
  PutString(surf_screen, 0, 0, tempbuf);
#endif

  // Always draw the last score
  i = xsize - 250;
  snprintf(topline, sizeof(topline), "Score: %ld", score);
  PutString(surf_screen, i, 30, topline);
  snprintf(topline, sizeof(topline), "Level: %d", (int) level);
  PutString(surf_screen, i, 50, topline);

  // Update the surface
  SDL_Flip(surf_screen);
  return bang;
}


/*! \brief called if ship was hit
 */
void crash_boom_bang() {
  if(oss_sound_flag) {
    // Play the explosion sound
    play_sound(0);
  }
  if(state != DEMO) {
    way_of_the_exploding_ship();
    if(--nships <= 0) {
      state = GAME_OVER;
      state_timeout = 200.0;
      fadetimer = 0.0;
      faderate = movementrate;
       /* Otherwise if level is >x.98 and smaller then x+1 the
	* function draw_infinite_black() will cause problems. It will
	* loop the sound and will not black the background.
	*/
      level = floorf(level);
      //Remove blubats
      deactivate_all_blubats();
      //Remove yellifish
      deactivate_all_yellifish(yellifishs);
    } else {
      state = DEAD_PAUSE;
      state_timeout = 120.0;
    }
  }
}


/*

  Subsystems [Loki Games, Programming Linux Games, No Starch Press, p 15, 2001]:
Keyboard
Mouse			Input Subsystem
Game Pad
			Network Subsystem
LAN or
Internet
			Update Subsystem
3D Hardware /
Framebuffer
			Display Subsystem
			Audio Subsystem
Sound Card
Figure 1–1: A typical game loop

 */

/*! \brief Prepare ship for start.
 *
 * This function is called before the game begins. It deactivates all
 * enemies, resets the level, and sets the game-play state.
 */
void ready_for_takeoff(void) {
  deactivate_greeblies();
  deactivate_all_blubats();
  deactivate_all_yellifish(yellifishs);
  reset_rocks();
  level = 1;
  last_levelup_snd = 0;
  state = GAME_PLAY;
  play_tune(1);
  sfx_enabled = 1; //Now we want all the SFXs!
  play_sound(4);
  reset_ship_state();
}

/*! \brief perform the demo feat
 *
 * The ship is moved into the biggest available space between the
 * rocks. In debug mode bars visualize the available spaces. Shields
 * are avtivated if the rocks are too close.
 *
 * \param inputstate pointer the the input state
 */
void perform_demonstration(inputstate_t *inputstate) {
  int i, j;
  
  rockfree_space_num = 1;
  rockfree_space_mm[0].min = 0;
  rockfree_space_mm[0].max = ysize;
  for(i = 0; i < MAX_ROCKS; ++i) {
    if(rock[i].active && rock[i].x + get_rock_surface(i, 0)->w > shipdata.xship) {
      int y = rock[i].y;
      int Y = rock[i].y + get_rock_surface(i, 0)->h;
      for(j = 0; j < rockfree_space_num; ++j) {
	if(y > rockfree_space_mm[j].min && Y < rockfree_space_mm[j].max) {
	  rockfree_space_mm[rockfree_space_num] = rockfree_space_mm[j];
	  rockfree_space_mm[j].max = y;
	  rockfree_space_mm[rockfree_space_num].min = Y;
	  ++rockfree_space_num;
	  break;
	} else if(y > rockfree_space_mm[j].min && y < rockfree_space_mm[j].max) {
	  rockfree_space_mm[j].max = y;
	  break;
	} else if(Y > rockfree_space_mm[j].min && Y < rockfree_space_mm[j].max) {
	  rockfree_space_mm[j].min = Y;
	}
	if((rockfree_space_mm[j].max - rockfree_space_mm[j].min) < 10 && rockfree_space_num > 0) {
	  rockfree_space_mm[j] = rockfree_space_mm[rockfree_space_num--];
	}
      }
    }
  }
  for(i = 0, j = 0; j < rockfree_space_num; ++j) {
    if(rockfree_space_mm[j].max - rockfree_space_mm[j].min >= rockfree_space_mm[i].max - rockfree_space_mm[i].min) i = j;
  }
  rockfree_space_max = i;
  inputstate->inputstate[SHIELD] = 1;
  for(j = 0; j < rockfree_space_num; ++j) {
    if(shipdata.yship >= rockfree_space_mm[j].min && shipdata.yship + surfaces_ship->surfaces[0]->h <= rockfree_space_mm[j].max) {
      inputstate->inputstate[SHIELD] = 0;
      break;
    }
  }
  inputstate->inputstate[LASER] = inputstate->inputstate[SHIELD];
  if(shipdata.yship < rockfree_space_mm[rockfree_space_max].min) {
    inputstate->inputstate[DOWN] = 1;
  }
  if(shipdata.yship > rockfree_space_mm[rockfree_space_max].max) {
    inputstate->inputstate[UP] = 1;
  }
}


/*! \brief Move the enemy game objects
 *
 * The enemy object are moved. This function is called once per
 * frame. Currently this includes the rocks, greeblies, blubats, and
 * yellifish.
 * 
 * \param yelli Yellifish subsystem pointer
 * \param preyx x-position of prey
 * \param preyy y-position of prey
 */
void update_enemy_objects(yellifishsubsystem_t *yelli, float preyx, float preyy) {
  assert(yelli != NULL);
  // Move all the rocks.
  update_rocks();
  // And now the greeblies.
  move_all_greeblies();
  // Update blubats.
  move_all_blubats();
  // Update yellifishs
  update_yellifishs(yelli, preyx, preyy);
}


/*! \brief Get/update the input state.
 *
 * This will get the newest input state, getting keyboard and joystick
 * input.
 *
 * \param inputstate pointer to inputstate structure to be filled
 * \return laserstatus 0=laser does not fire, 1=laser fires
 */
int get_input_state(inputstate_t *inputstate) {
  int laser = 0;

  if(state == GAME_PLAY || state == DEMO) {
    if(state != DEMO) get_joystick_input(inputstate);
    if(inputstate->inputstate[UP]) {
      shipdata.yvel -= 1.5 * movementrate;
      create_engine_dots2(5, 3); // Create engine dots out the side we're moving from
    }
    if(inputstate->inputstate[DOWN]) {
      shipdata.yvel += 1.5 * movementrate;
      create_engine_dots2(5, 1);
    }
    if(inputstate->inputstate[LEFT]) {
      shipdata.xvel -= 1.5 * movementrate;
      create_engine_dots2(5, 2);
    }
    if(inputstate->inputstate[RIGHT]) {
      shipdata.xvel += 1.5 * movementrate;
      create_engine_dots2(5, 0);
    }
    if(inputstate->inputstate[LASER]) {
      laser = 1;
    }
  }
  return laser;
}


/*! \brief game loop called once per frame
 *
 * This is the game loop where all the magic happens. This loop is
 * only left if state == QUIT_GAME or RESTART_GAME.
 *
 * \param game_over_data pointer to game-over structure 
 * \return 0 == quit the game, 1 == restart game
 */
int gameloop(const GameOver_t *game_over_data) {
  inputstate_t inputstate;
  Uint8 *keystate;

  while(state != QUIT_GAME && state != RESTART_GAME) {
    update_movement_rate();
    keystate = input_subsystem(&inputstate);
    handle_state_timeout(infoscreen_enabled);
    ingame_maybe_rock();

    switch(state) {
    case TITLE_PAGE:
      // Before the rocks are drawn, the greeblies are shown playing amongst them.
      activate_one_greeblie();
      // TODO: where is the prey?
      update_enemy_objects(yellifishs, 0, 0);
      // Move the powerup
      update_powerup();
      update_sparkles(surf_screen);
      if((last_ticks / 5391) % 7 == 6) {
	//Rockmania!
	create_rock();
      }
      break;
    case DEMO:
      perform_demonstration(&inputstate);
      //Fall through
    case DEAD_PAUSE:
    case GAME_PLAY:
      // Increase difficulty
      level += movementrate / 250;
      //Activate a blubat if necessary
      if(rnd() < (level - 5) / 797.141347 && ((int)level) % 5 != 0) { //Every 5th level no blubats
	activate_one_blubat();
      }
      maybe_activate_yellifish(yellifishs, (int)level, xsize + 34, -24);
      //Update greeblies
      if(rnd() < .02 * level * movementrate) {
	activate_one_greeblie();
      }
      update_enemy_objects(yellifishs, shipdata.xship, shipdata.yship - 87.8228228);
      // Move ship to new position
      ship_update_position();
      // Move the powerup
      update_powerup();
      update_sparkles(surf_screen);
      break;
    case GAME_OVER:
      if(keystate[SDLK_ESCAPE]) play_sound(SAMPLE_XBAD);
      //DANGER! Falls through!
    case INFO_SCREEN:
    case HIGH_SCORE_DISPLAY:
    case HIGH_SCORE_ENTRY:
      update_enemy_objects(yellifishs, xsize / 2, ysize);
      // Move ship to new position
      ship_update_position();
      // Move the powerup
      update_powerup();
      update_sparkles(surf_screen);
      break;
    case SETUP:
      /*
       * The ignore is necessary so that we do not switch back and
       * forth between the two. It can be removed later on(?).
       */
      temporary_disable_key_input();
      setuploop();
      temporary_disable_key_input();
      break;
    case RESTART_GAME:
    case GAME_PAUSED:
    case QUIT_GAME:
      break;
    }

    //Was the ship hit?
    if(draw(inputstate.inputstate[SHIELD], game_over_data) && (state == GAME_PLAY || state == DEMO)) {
      crash_boom_bang();
    }

    if(keystate[SDLK_SPACE]
       && (state == HIGH_SCORE_DISPLAY || state == TITLE_PAGE
	   || state == DEMO || state == INFO_SCREEN)) {
      ready_for_takeoff();
    }

    laser = get_input_state(&inputstate);
    //Check for screen shot.
    if(state != HIGH_SCORE_ENTRY && inputstate.inputstate[SCREENSHOT]) {
      int ret;
      char buf[21];
      
      sprintf(buf, "snapshot.%02hX.bmp", screenshot_number++);
      ret = SDL_SaveBMP(surf_screen, buf);
      fprintf(stderr, "Screenshot '%s' saved %s.\n", buf, ret == 0 ? "successfully" : "unsuccessfully(!)");
    }
  }
  return state == RESTART_GAME ? 1 : 0;
}


void shutdown_subsystems() {
  int i;

  shutdown_greeblies();
  shutdown_blubats();
  shutdown_powerups();
  shutdown_yellifish_subsystem(yellifishs);
  destroy_mood_item();
  i = uiff_close(iff_ctx);
  assert(printf("close IFF context returned $%08"PRIx32"\n", i));
  if(i) fprintf(stderr, "Error on closing IFF context!\n");
  //We can free this safely as rockiff_name is either a result of strdup or NULL.
  free(rockiff_name);
  free(edition_name);
}


int main(int argc, char **argv) {
  int i, x;
  int force_intro = 0;
  GameOver_t game_over_data;

  assert(RD_VIDEO_BPP == 8 * sizeof(RD_VIDEO_TYPE));
  oss_sound_flag = 1;
  load_setup();
  while((x = getopt(argc, argv, "P:Ifwskh?x:y:")) >= 0)
    switch (x) {
    case 'P':
      rockiff_name = strdup(optarg);
      break;
    case 'I':
      force_intro = 1;
      break;
    case 'x':
      xsize = atoi(optarg);
      printf("xsize %d\n", xsize);
      break;
    case 'y':
      ysize = atoi(optarg);
      printf("ysize %d\n", ysize);
      break;
    case 's':
      oss_sound_flag = 0;
      break;
    case 'k':
      joystick_flag = 0;
      break;
    case 'f':
      opt_fullscreen = 1;
      break;
    case 'w':
      opt_fullscreen = 0;
      break;
    case '?':
    case 'h':
      printf("Rock Dodgers\n"
	     "  -h This help message\n"
	     "  -f Full screen\n"
	     "  -w Window mode\n"
	     "  -x xsize\n"
	     "  -y ysize\n"
	     "  -k Keyboard only - disable joystick\n"
	     "  -s Silent mode (no sound)\n"
	     "  -P parameter file\n"
	     "  -I force intro\n"
	     );
      exit(0);
      break;
    }

  initrnd();
  if(init(&game_over_data)) {
    printf("Cannot start: '%s'\n", initerror);
    return 1;
  }

  if((optind < argc) && (perform_a_sekrit_test(argv[optind]) != 0)) {
    //A non option argument was given!
    printf("Testing for %lu ticks.\n", (unsigned long int)SDL_GetTicks());
  } else {
    play_intro(force_intro, oss_sound_flag, *iff_ctx);
    //Sometimes the game slips directly from intro to gameplay if space was pressed.
    last_ticks = SDL_GetTicks();
    temporary_disable_key_input();
    if(oss_sound_flag) play_tune(0);
    while(1) {
      if(restart_game() != 0) {
	break; //We are gone now...
      }
      i = gameloop(&game_over_data);
      if(i == 0) {
	break;
      } else if(i == 1) {
	if(!set_video_mode()) {
	  fprintf(stderr, "Setting video mode (%d, %d) failed!\n", xsize, ysize);
	  break;
	}
	if(restart_game() != 0) {
	  fprintf(stderr, "Cannot start: '%s'\n", initerror);
	  break;
	}
      }
      SDL_Delay(1000);
    }
  }
  shutdown_subsystems();
  return 0;
}
