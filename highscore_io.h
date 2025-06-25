#ifndef __HIGHSCORE_IO_H_20101222__
#define __HIGHSCORE_IO_H_20101222__

#define MAXIMUM_HIGH_SCORE_ENTRIES 8

/*! \brief Highscore table structure
 * 
 * The highscore table consists of the score, a char pointer to the
 * name of the winner, the level reached and an integer called
 * "allocated" which is set to 1 if the memory for the name was
 * allocated, 0 otherwise.
 */
struct highscore {
  long score;
  char *name;
  int level;
  int allocated;
};

//! high is a an array of highscore structs.
extern struct highscore high[MAXIMUM_HIGH_SCORE_ENTRIES];

extern long score;		//!< The current score.
extern int scorerank;		//!< minus one means no scorerank, we are not worthy
extern char name_input_buf[];	//!< name input buffer

FILE *hs_fopen(const char *mode);
void read_high_score_table();
void write_high_score_table();

/*! \brief Increment the score
 *
 * Increment the score counter. Currently there is no dependency on
 * the coordinates.
 *
 * \param x x coordinate of the item
 * \param y y coordinate of the item
 * \param dscore points to add
 */
void inc_score(int x, int y, long dscore);

/*! \brief clear score
 *
 * Clear the score counter and return old score.
 *
 * \return old score
 */
long clear_score(void);

/*! \brief handle game over (get lowest highscore, etc.)
 *
 * This function read always the high scores from disk (why?) and
 * updates the scorerank. If there is a scorerank found then the
 * corresponding entry is freed and replaced by an empty name with the
 * appropriate score.
 */
void game_over();
#endif
