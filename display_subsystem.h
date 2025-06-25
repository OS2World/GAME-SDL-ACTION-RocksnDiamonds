#ifndef __DISPLAY_SUBSYSTEM_H_2013__
#define __DISPLAY_SUBSYSTEM_H_2013__

/*! \brief Draw the background items.
 *
 * Harmless items in the background without any interaction. These are
 * infinite black and space dots.
 */
void draw_background_objects();

/*! \brief draw the ghostly (fading) rock dodger title
 *
 * This function will use the fadetime to change the alpha of the
 * given surfaces (they are modified!). You can, of course, provide
 * other surfaces and this function should work euqally well...
 * 
 * \param surf_t_rock Surface to the "Rock" title string
 * \param surf_t_dodger Surface to the "Dodger" title string
 */
void draw_ghostly_rock_dodger(SDL_Surface *surf_t_rock, SDL_Surface *surf_t_dodger);


#endif
