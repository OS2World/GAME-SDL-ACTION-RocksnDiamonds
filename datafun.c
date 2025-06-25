#include "datafun.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SDL/SDL_image.h>

char *data_dir;

char *get_data_dir(void) {
  char buf[4096];

  buf[sizeof(buf) - 1] = 0;
  strcpy(buf, "./data");
  if(missing(buf)) {
    char *env;

    env = getenv("ROCKDODGER_DATADIR");
    if(env != NULL) {
      strncpy(buf, env, sizeof(buf) - 1);
      if(missing(buf)) {
	fprintf(stderr, "Cannot find data directory $ROCKDODGER_DATADIR\n");
	exit(-1);
      }
    } else {
      snprintf(buf, sizeof(buf) - 1, "%s/%s", DATADIR, PACKAGENAME);
      if(missing(buf)) {
	fprintf(stderr, "Cannot find data in %s\n", buf);
	exit(-2);
      }
    }
  }
  return strdup(buf);
}

char *load_file(const char *s) {
  static char retval[1024];

  snprintf(retval, sizeof(retval), "%s/%s", data_dir, s);
  retval[sizeof(retval) - 1] = '\0';
  return retval;
}

const char *load_file_dir(const char *dir, const char *name) {
  static char retval[4096];

  snprintf(retval, sizeof(retval), "%s/%s/%s", data_dir, dir, name);
  retval[sizeof(retval) - 1] = '\0';
  return retval;
}


SDL_Surface *load_image_no_convert(const char *fname, short kr, short kg, short kb) {
  SDL_Surface *surf = IMG_Load(load_file_dir("images", fname));

  if(surf != NULL) {
    if(kr >= 0 && kg >= 0 && kb >= 0) {
      RD_VIDEO_TYPE key =
	(RD_VIDEO_TYPE) SDL_MapRGB(surf->format, kr, kg, kb);
      SDL_SetColorKey(surf, SDL_SRCCOLORKEY | SDL_RLEACCEL, key);
    }
  } else {
#ifdef DEBUG
    fprintf(stderr, "IMG_Load(): %s\n", SDL_GetError());
#endif
  }
  return surf;
}


SDL_Surface *load_image(const char *fname, short kr, short kg, short kb) {
  SDL_Surface *surf = NULL;
  SDL_Surface *temp = load_image_no_convert(fname, kr, kg, kb);

  if(temp != NULL) {
    surf = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
  }
  return surf;
}


int missing(const char *dirname) {
  struct stat buf;

  if(lstat(dirname, &buf) == 0) {
    return (!S_ISDIR(buf.st_mode));
  }
#ifdef DEBUG
  perror("missing()");
#endif
  return 1;
}


sdl_surfaces_t *load_images_no_convert(const char *fname, short kr, short kg, short kb) {
  unsigned short int i, j;
  char name[256];
  SDL_Surface *tmpsurf[1<<16]; //Maximum!
  sdl_surfaces_t *surfaces;

  for(i = 0; i < (1<<16) - 1; i++) {
    snprintf(name, sizeof(name), fname, i);
    if((tmpsurf[i] = load_image_no_convert(name, kr, kg, kb)) == NULL) {
      if(i == 0) {
	return NULL;
      } /* maybe later: fill up to n with first image... see greeeblies */
      break;
    } else {
      printf("\tImage '%s' was loaded.\n", name);
    }
  }
  if(i == (1<<16) - 1) return NULL; //That many images???
  surfaces = malloc(sizeof(struct sdl_surfaces));
  if(surfaces != NULL) {
    surfaces->num_surfaces = i;
    if( (surfaces->surfaces = calloc(i, sizeof(SDL_Surface*))) == NULL)
      return NULL;
    surfaces->surfaces_end = surfaces->surfaces + i;
    for(j = 0; j < i; ++j)
      *(surfaces->surfaces + j) = tmpsurf[j];
  }
  return surfaces;
}


sdl_surfaces_t *load_images_ck(const char *fname, short kr, short kg, short kb) {
  unsigned short int i, j;
  char name[256];
  SDL_Surface *tmpsurf[1<<16]; //Maximum!
  sdl_surfaces_t *surfaces;

  for(i = 0; i < (1<<16) - 1; i++) {
    snprintf(name, sizeof(name), fname, i);
    if((tmpsurf[i] = load_image(name, kr, kg, kb)) == NULL) {
      if(i == 0) {
	return NULL;
      } /* maybe later: fill up to n with first image... see greeeblies */
      break;
    } else {
      printf("\tImage '%s' was loaded.\n", name);
    }
  }
  if(i == (1<<16) - 1) return NULL; //That many images???
  surfaces = malloc(sizeof(struct sdl_surfaces));
  if(surfaces != NULL) {
    surfaces->num_surfaces = i;
    if( (surfaces->surfaces = calloc(i, sizeof(SDL_Surface*))) == NULL)
      return NULL;
    surfaces->surfaces_end = surfaces->surfaces + i;
    for(j = 0; j < i; ++j)
      *(surfaces->surfaces + j) = tmpsurf[j];
  }
  return surfaces;
}

void destroy_sdl_surfaces(sdl_surfaces_t *surfaces) {
  SDL_Surface **ptr;

  for(ptr = surfaces->surfaces; ptr < surfaces->surfaces_end; ++ptr) {
    SDL_FreeSurface(*ptr);
  }
  //Free array of surface pointers.
  free(surfaces->surfaces);
  memset(surfaces, 0, sizeof(*surfaces));
  //Free the surfaces
  free(surfaces);
}
