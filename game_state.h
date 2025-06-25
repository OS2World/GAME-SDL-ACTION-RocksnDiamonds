#ifndef __GAME_STATE_H__20130511
#define __GAME_STATE_H__20130511

/*! \brief Perform the dead pause
 *
 * Called when the player is dead. This will refill the shields and
 * reset the ship position.
 */
void dead_pause();

/*! \brief handle state changes due to timeout
 *
 * \param infoscreen_enabled = 1 if info screen should be displayed, zero if not
 */
void handle_state_timeout(int infoscreen_enabled);

/* \brief toggle game pause status
 *
 * This switches the game state between GAME_PLAY and GAME_PAUSED. In
 * debug mode all other mode will cause an assertion error!
 */
void pausegame();

/*! \brief reset ship state
 *
 * Reset the ship information, like position, shields, etc. This is
 * usually called when the game begins.
 */
void reset_ship_state();

#endif
