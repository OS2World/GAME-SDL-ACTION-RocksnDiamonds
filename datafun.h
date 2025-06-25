#ifndef __DATAFUN_H__
#define __DATAFUN_H__
#include <SDL/SDL.h>

/*! \brief structure to hold multiple surfaces
 *
 * Mostly used for load_images_ck... All fields are set by the load
 * functions and should be left alone. surfaces_end and num_surfaces
 * are redundant!
 */
typedef struct sdl_surfaces {
  SDL_Surface **surfaces; //!< pointer to an array of surfaces
  SDL_Surface **surfaces_end; //!< pointer to SDL_Surface *after* the last surface
  unsigned short num_surfaces; //!< number of surfaces
} sdl_surfaces_t;

/*! \brief global variable for the path to the data directory
 * This variable must be set before any other function is called
 */
extern char *data_dir;

/*! \brief Determine rockdodger data directory
 *
 * The following directories are tried:
 *  - default: ./data
 *  - second alternative: ROCKDODGER_DATADIR
 *  - final alternative: @datadir@/@PACKAGENAME@
 *
 * \return a freshly allocated string with the data-directory name
 */
char *get_data_dir(void);

/*! \brief get load file name
 *
 * Get the load file name of a file. Actually does only prepend the
 * file path. The returned value is a static array!
 *
 * \return pointer to static array
 */
char *load_file(const char *s);

/*! \brief get load file name in a directory
 *
 * Get the load file name of a file. Actually does only prepend the
 * data directory path and the dir path. The returned value is a
 * static array!
 *
 * \return pointer to a static array
 */
const char *load_file_dir(const char *dir, const char *name);

/*! \brief load an image with optional colour key
 *
 * This function loads an image in the standard image directory and
 * transforms it to the display format. Optionally a colour key for
 * transparency can be given. If any of the kr, kg, kb is < 0 then no
 * colour key transparency is done.
 *
 * \warning The image is transformed into the screen format, this
 * breaks transparency!
 *
 * \param fname file name
 * \param kr red component for colour key
 * \param kg green component for colour key
 * \param kb blue component for colour key
 * \return surface or NULL on any error
 */
SDL_Surface *load_image(const char *fname, short kr, short kg, short kb);

/*! \brief load images with optional colour key
 *
 * This function loads images in the standard image directory and
 * transforms it to the display format. Optionally a colour key for
 * transparency can be given. If any of the kr, kg, kb is < 0 then no
 * colour key transparency is done.
 *
 * \warning The image is transformed into the screen format, this
 * breaks transparency!
 *
 * \param fname file name pattern with format qualifier for a unsigned short, e.g. "image.%hx.lbm" (I know this could lead to a security hole...)
 * \param kr red component for colour key
 * \param kg green component for colour key
 * \param kb blue component for colour key
 * \return surfaces or NULL on any error
 */
sdl_surfaces_t *load_images_ck(const char *fname, short kr, short kg, short kb);


sdl_surfaces_t *load_images_no_convert(const char *fname, short kr, short kg, short kb);


/*! \brief Check is directory is missing
 *
 * \param dirname directory name
 * \return 0 = no directory
 */
int missing(const char *dirname);

/*! \brief Destroy multiple surfaces in sdl_surfaces_t
 *
 * This function deletes all the surfaces which can be found in the
 * structure. It will also clear the whole surfaces so that there will
 * be no more dangling pointers. This sets the number of surfaces also
 * to zero.
 *
 * \param surfaces pointer to a sdl_surfaces_t which will be destroyed
 */
void destroy_sdl_surfaces(sdl_surfaces_t *surfaces);

#endif
