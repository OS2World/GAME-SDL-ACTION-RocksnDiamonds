#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <SDL/SDL.h>

//Always make sure that the next two variables are always in sync otherwise you will get trouble
#define RD_VIDEO_BPP 16 //!< rockdodger video display bits per pixel
#define RD_VIDEO_TYPE Uint16 //!< rockdodger video type for accessing the pixel
#define GAMESDIR "/usr/local/var/games"
#define DATADIR "/usr/local/share"
#define PACKAGENAME "rockdodger" //Current name of the package

// Local Variables:
// mode: c
// End:

#endif
