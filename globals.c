#include "globals.h"
#include <assert.h>
#include <math.h>
#include "display_subsystem.h"
#include "game_state.h"
#include "highscore_io.h"
#include "input_functions.h"
#include "SFont.h"
#include "scroller.h"
#include "ship.h"
#include "sound.h"

struct Setup_Data {
  Scroller_t *scroller;
  short current;
  const char *entry_name;
  const char *description; //!< will be in the scroller on the bottom of the screen
  char *values[12];
};

char *edition_name;
int xsize = 800;
int ysize = 600;
SDL_Surface *surf_screen = NULL;
SDL_Surface *surf_green_block = NULL;
SDL_Surface *surf_t_rock = NULL;	// Title element "rock"
SDL_Surface *surf_t_dodger = NULL;	// Title element "dodgers"
spacedots_t *current_spacedot_engine = NULL;
unsigned short opt_fullscreen = 1;
unsigned short opt_cicada_spacedots = 0;
unsigned short opt_intro = 2;
float movementrate;
float level = 0;
float shieldlevel, laserlevel, shieldpulse = 0;
int initialshield, gameover = 0;

RD_VIDEO_TYPE heatcolor[W * 3];

enum states state = TITLE_PAGE;	//!< current game state
float state_timeout = 600.0;	//!< timeout before next game state switch
float fadetimer = 0;
Uint32 initticks = 0;
Uint32 last_ticks = 0;
Uint32 ticks_since_last = 0;
Uint32 ign_k_utl_ticks = 0;
int music_volume = 128; //See sound.c...
int nships;
int last_levelup_snd;
uiff_ctx_t *iff_ctx;

static const char resolution_format[] = "resolution = ( %d, %d )\n";
static const char spacedots_format[] = "spacedots = %31s\n";
static const char fullscreen_format[] = "fullscreen = %31s\n";
static const char intro_format[] = "intro = %31s\n";
static const char version_format[] = "version = %13s\n";
static const char music_volume_format[] = "musvol = %13s\n";
static char configuration_file_version[14];

struct Setup_Data setup[] = {
  { NULL, 0, "", "Use cursor keys to select and change options, press enter or F1 to quit the screen.", { "", NULL } },
  { NULL, 0, "resolution", "Select the screen resolution!", { "", "640x480", "800x480", "800x600", "1024x768", "1152x864", "1280x1024", "1440x900", "1600x900", NULL } },
  { NULL, 0, "fullscreen", "Use the full-screen display or windowed mode for rockdodger?", { "fullscreen", "windowed", NULL } },
  { NULL, 0, "spacedots", "How are the spacedots (the little stars in the background) drawn? The classic version draws them directly into the surface, the cicada version uses surfaces (may be faster).", { "classic", "cicada", NULL } },
  { NULL, 8, "music volume", "Set the volume for the background music.", { "0", "16", "32", "48", "64", "80", "96", "112", "128", "144", "160" } },
  { NULL, 0, "intro", "Should the intro be displayed? The intro is usually only displayed for the first time after a version change.", { "no", "yes", NULL } },
  { NULL, 0, "", "Rockdoger V"VERSION" compiled on "COMPILEDATE".", { "", NULL } },
  { NULL, 0, "activate config?", "Set this to yes and press ENTER to activate the current configuration. Valid options are: 'no' keep old configuration, 'yes' activate configuration, 'yes and save' activate and save the configuration.", { "no", "yes", "yes and save", NULL } },
  { NULL, 0, NULL }
};
#define SETUP_SIZE (sizeof(setup) / sizeof(struct Setup_Data))

void update_movement_rate() {
  ticks_since_last = SDL_GetTicks() - last_ticks;
  last_ticks = SDL_GetTicks();
  if(ticks_since_last > 200) {
    movementrate = 0;
  } else {
    movementrate = ticks_since_last / 50.0;
  }
}


void cleanup_globals() {
  struct Setup_Data *setupptr;

  initticks = SDL_GetTicks();
  for(setupptr = setup; setupptr->entry_name; ++setupptr) {
    if(setupptr->scroller) {
      destroy_scroller(setupptr->scroller);
      setupptr->scroller = NULL;
    }
  }
}


struct Setup_Data *find_setup_entry(const char *name) {
  struct Setup_Data *setupptr;
  
  for(setupptr = setup; setupptr->entry_name; ++setupptr) {
    if(strcmp(name, setupptr->entry_name) == 0) return setupptr;
  }
  return NULL;
}

void save_setup() {
  FILE *f;
  char buf[256];
  struct Setup_Data *setupptr;

  snprintf(buf, sizeof(buf) - 1, "%s/.rockdodger", getenv("HOME"));
  if((f = fopen(buf, "w")) == NULL) return;
#ifdef DEBUG
  fprintf(stderr, "Writing config to '%s'.\n", buf);
#endif
  fprintf(f, "#rockdodger configuration\n");
  fprintf(f, version_format, VERSION);
  fprintf(f, resolution_format, xsize, ysize);
  //
  setupptr = find_setup_entry("spacedots");
  assert(setupptr);
  fprintf(f, spacedots_format, setupptr->values[setupptr->current]);
  //
  setupptr = find_setup_entry("fullscreen");
  assert(setupptr);
  fprintf(f, fullscreen_format, setupptr->values[setupptr->current]);
  //
  setupptr = find_setup_entry("intro");
  assert(setupptr);
  fprintf(f, intro_format, setupptr->values[setupptr->current]);
  //
  setupptr = find_setup_entry("music volume");
  assert(setupptr);
  fprintf(f, music_volume_format, setupptr->values[setupptr->current]);
  fclose(f);
}


int load_setup() {
  FILE *f;
  char buf[256];
  char inpbuf[32];
  int x0, x1;

  snprintf(buf, sizeof(buf) - 1, "%s/.rockdodger", getenv("HOME"));
  if(!(f = fopen(buf, "r"))) return 0;
  while(fgets(buf, sizeof(buf), f) != NULL) {
    if(strlen(buf) == 0 || buf[0] == '#') continue;
    if(sscanf(buf, resolution_format, &x0, &x1) == 2) {
      if(x0 > 0 && x1 > 0) {
	xsize = x0;
	ysize = x1;
      }
    } else if(sscanf(buf, spacedots_format, inpbuf) == 1) {
      if(strcmp(inpbuf, "cicada") == 0)
	opt_cicada_spacedots = 1;
      else
	opt_cicada_spacedots = 0;
    } else if(sscanf(buf, fullscreen_format, inpbuf) == 1) {
      if(strcmp(inpbuf, "fullscreen") == 0)
	opt_fullscreen = 1;
      else
	opt_fullscreen = 0;	
    } else if(sscanf(buf, intro_format, inpbuf) == 1) {
      if(opt_intro != 3) opt_intro = (strcmp(inpbuf, "yes") == 0) ? 1 : 0;
    } else if(sscanf(buf, version_format, configuration_file_version) == 1) {
      if(strcmp(configuration_file_version, VERSION) != 0) opt_intro = 3;
    } else if(sscanf(buf, music_volume_format, inpbuf) == 1) {
      music_volume = atoi(inpbuf);
      //Check for illegal values. Oh boy, I am sooo lazy.
      if(music_volume < 0) music_volume = 0;
      else if(music_volume > 160) music_volume = 160;
    } else {
      printf("Unknown setup line '%s'.\n", buf);
    }
  }
  fclose(f);
  return 1;
}


void setuploop() {
  int i;
  short old_config[SETUP_SIZE];
  inputstate_t inputstate;
  char txtbuf[181];
  int crsrpos = 0;
  struct Setup_Data *setupptr;
  char resolution_buf[10];
  short activate_config;
  SDL_Rect drect = { 16, 0, 0, 0 };
  SDL_Rect srect = { 0, 0, surf_screen->w - 32, 20 };

  play_tune(4);
  for(i = 0; i < SETUP_SIZE; ++i) old_config[i] = setup[i].current;
  state_timeout = 69105;
  setupptr = find_setup_entry("music volume");
  assert(setupptr);
  setupptr->current = music_volume >> 4;
  assert(setupptr->current < 11);
  setupptr = find_setup_entry("fullscreen");
  assert(setupptr);
  setupptr->current = opt_fullscreen == 0 ? 1 : 0;
  setupptr = find_setup_entry("intro");
  assert(setupptr);
  setupptr->current = opt_intro == 1 ? 1 : 0;
  setupptr = find_setup_entry("spacedots");
  assert(setupptr);
  setupptr->current = opt_cicada_spacedots;
  setupptr = find_setup_entry("resolution");
  assert(setupptr);
  sprintf(resolution_buf, "%dx%d", xsize, ysize);
  setupptr->values[0] = resolution_buf;
  while(state == SETUP) {
    update_movement_rate();
    (void) input_subsystem(&inputstate);
    /* TODO: really not possible in setup? */
    handle_state_timeout(0);
    if(inputstate.inputstate[UP]) {
      --crsrpos;
      temporary_disable_key_input();
    } else if(inputstate.inputstate[DOWN]) {
      ++crsrpos;
      temporary_disable_key_input();
    } else if(inputstate.inputstate[RIGHT]) {
      setupptr = setup + crsrpos;
      if(setupptr->values[++setupptr->current] == NULL) setupptr->current = 0;
      temporary_disable_key_input();
    } else if(inputstate.inputstate[LEFT]) {
      setupptr = setup + crsrpos;
      if(--setupptr->current < 0) {
	while(setupptr->values[++setupptr->current]); //This will find the null pointer.
	--setupptr->current; //Now last entry.
      }
      temporary_disable_key_input();
    } else if(inputstate.inputstate[INPUT_SELECT]) {
      state = TITLE_PAGE;
    }
    /*
     * If the cursor positon is less then zero set it to zero or if
     * the position would be pointing to NULL which is the end of
     * available options also ignore the keystroke. If the first
     * character is less then a space then ignore it, too. We will use
     * control characters for special use cases...
     */
    if(crsrpos < 0) crsrpos = 0;
    else if(setup[crsrpos].entry_name == NULL || (setup[crsrpos].entry_name[0] < ' ' && setup[crsrpos].entry_name[0] != 0)) --crsrpos;
    if(setup[crsrpos].scroller == NULL) {
      setup[crsrpos].scroller = init_scroller(setup[crsrpos].description, surf_screen->h - 30, 4.62384, surf_screen->w);
    }
    //display subsystem
    draw_background_objects();
    XCenteredString(surf_screen, 20, "SETUP");
    for(setupptr = setup, i = 0; setupptr->entry_name; ++setupptr, ++i) {
      sprintf(txtbuf, "%-70s %12s", setupptr->entry_name, setupptr->values[setupptr->current]);
      PutString(surf_screen, 20, 60 + i * 22, setupptr->entry_name);
      PutString(surf_screen, surf_screen->w - 20 - SFont_wide(setupptr->values[setupptr->current]), 60 + i * 22, setupptr->values[setupptr->current]);
      SDL_SetAlpha(surf_green_block, SDL_SRCALPHA, (int) (51.0 + 30.0 * sinf(fadetimer)));
      drect.y = 60 + crsrpos * 22;
      SDL_BlitSurface(surf_green_block, &srect, surf_screen, &drect);
    }
    assert(setup[crsrpos].scroller);
    if(setup[crsrpos].scroller) update_and_draw_scroller(setup[crsrpos].scroller, surf_screen, movementrate);
    // Update the surface
    SDL_Flip(surf_screen);
    fadetimer += movementrate / 8.2;
  }
  //We are leaving... check if activated
  setupptr = find_setup_entry("activate config?");
  assert(setupptr);
  activate_config = setupptr->current;
  assert(activate_config >= 0 && activate_config <= 2);
  if(activate_config > 0) {
    setupptr->current = 0;
    //Activate
    setupptr = find_setup_entry("resolution");
    assert(setupptr);
    if(setupptr->current > 0) {
      i = sscanf(setupptr->values[setupptr->current], "%dx%d", &xsize, &ysize);
      assert(i == 2);
    }
    setupptr = find_setup_entry("fullscreen");
    assert(setupptr);
    opt_fullscreen = setupptr->current == 0 ? 1 : 0; //0=fullscreen, 1=no fullscrenn, qed.
    setupptr = find_setup_entry("spacedots");
    assert(setupptr);
    opt_cicada_spacedots = setupptr->current;
    setupptr = find_setup_entry("music volume");
    assert(setupptr);
    music_volume = atoi(setupptr->values[setupptr->current]);
    state = RESTART_GAME;
  }
  if(activate_config == 2) {
    save_setup();
  }
  if(activate_config == 0)
    for(i = 0; i < SETUP_SIZE; ++i) setup[i].current = old_config[i];
  play_tune(0);
  state_timeout = 496.69105;
}

uiff_ctx_t *get_iff_rock(const char *fname) {
  FILE *f = NULL;
  uiff_ctx_t *ctx = NULL;

  printf("Rockdodger data file is '%s'.\n", fname);
  if(!(f = fopen(fname, "r"))) {
    perror("Can not open rockdodger data-file");
  } else if((ctx = uiff_from_file(f, FORM, MakeID('R', 'O', 'C', 'K'))) == NULL) {
    perror("No context");
    fclose(f); 
  }
  return ctx;
}

char *load_edition_name(uiff_ctx_t iff) {
  int32_t chunksize;
  char buf[33];
  size_t bread;

  uiff_rewind_group(&iff);
  chunksize = uiff_find_chunk_ctx(&iff, MakeID('E', 'D', 'T', 'N'));
  assert(printf("FORM.ROCK EDTN size = %d\n", (int)chunksize));
  if(chunksize < 0) {
    edition_name = strcpy(buf, "#UNKNOWN#");
  } else {
    bread = fread(buf, 1, chunksize < sizeof(buf) - 1 ? chunksize : sizeof(buf) - 1, iff.f);
    assert(bread < sizeof(buf) - 1);
    buf[bread] = 0;
  }
  return strdup(buf);
}
