#ifndef __RANDOM_GEN_H__20101222
#define __RANDOM_GEN_H__20101222

/*! Initialise the random number generator for the game
 */
void initrnd();

/*! Get the next random number
 * \return next random number
 */
float rnd();

/*! \brief Get next float from random number generator
 *
 * This function actually calls the OS random number generator and
 * returns its value. It does *not* use the random number array.
 *
 * \return random float number
 */
float rnd_next_float();


#endif
