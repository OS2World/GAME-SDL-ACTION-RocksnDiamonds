#ifndef __GURU_MEDITATION_h_20130422__
#define __GURU_MEDITATION_h_20130422__
#include <SDL/SDL_video.h>

/* Colours... just for the fun of it! */
#define GM_FLAGS_YELLOW		0x0000 //!< Colour choice... yellow.
#define GM_FLAGS_RECOVERY	0x0000 //!< Recovery alert, colour choice... yellow.
#define GM_FLAGS_DEADEND	0x0001 //!< Colour choice... red. Never return from this alert!
#define GM_FLAGS_GREEN		0x0002 //!< Colour choice... green.
#define GM_FLAGS_BLUE		0x0003 //!< Colour choice... blue.
#define GM_FLAGS_CHOICE		0x0800 //!< Press left to retry, right to abort... is being displayed.
#define GM_FLAGS_AUTOTIMEOUT	0x2000 //!< Flash a few times and then continue
#define GM_FLAGS_EXIT_VIA_TERM	0x4000 //!< Do not use the exit() function but raise(SIGTERM).
#define GM_FLAGS_ABORTIFY	0x8000 //!< Pressing the right mouse button will call abort().

/* ---------- 8< ---------- */
/* These are actually the classic errors found on the Amiga. Use them
   on your own risk... */

/* Classic subsystems */
#define GM_SS_Exec	 0x01000000
#define GM_SS_Graphics	 0x02000000
#define GM_SS_Layers	 0x03000000
#define GM_SS_Intuition	 0x04000000
#define GM_SS_Math	 0x05000000
#define GM_SS_Clist	 0x06000000
#define GM_SS_DOS	 0x07000000
#define GM_SS_RAM	 0x08000000
#define GM_SS_ICON	 0x09000000
#define GM_SS_Expansion	 0x0A000000
#define GM_SS_Audio	 0x10000000
#define GM_SS_Console	 0x11000000
#define GM_SS_GamePort	 0x12000000
#define GM_SS_KeyBoard	 0x13000000
#define GM_SS_TrackDisk	 0x14000000
#define GM_SS_Timer	 0x15000000
#define GM_SS_CIA	 0x20000000
#define GM_SS_Disk	 0x21000000
#define GM_SS_Misc	 0x22000000
#define GM_SS_BootStrap	 0x30000000
#define GM_SS_WorkBench	 0x31000000
#define GM_SS_DiskCopy	 0x32000000

/* Classic general errors */
#define GM_GE_NoMemory	 0x00010000
#define GM_GE_MakeLib	 0x00020000
#define GM_GE_OpenLib	 0x00030000
#define GM_GE_OpenDev	 0x00040000
#define GM_GE_OpenRes	 0x00050000
#define GM_GE_IOError	 0x00060000
#define GM_GE_NoSignal	 0x00070000
#define GM_GE_BadParm	 0x00080000
#define GM_GE_CloseLib	 0x00090000
#define GM_GE_CloseDev	 0x000A0000
#define GM_GE_ProcCreate 0x000B0000

/*! \brief General purpose guru meditation display
 *
 * This function displays a Guru Meditation with subsystem, address
 * and text.
 *
 * This is only a display function! It will always return and will not
 * honor abort or deadend alerts (only make the appropriate changes to
 * the display). If you use GM_FLAGS_DEADEND the colour of the border
 * will be *always* red!
 *
 * \param ts target surface to display guru meditation on
 * \param flags or'ed together GM_FLAGS_*
 * \param subsystem subsystem id, define your own or use the Amiga ones
 * \param address adress where something happened
 * \param atext a text to display
 * \return which mouse button was pressed
 */
int guru_display_gp(SDL_Surface *ts, unsigned short flags, Uint32 subsystem, void *address, const char *atext);

/*! \brief Display Guru Meditation Alert
 *
 * This function just displays a guru meditation alert. In contrast to
 * guru_meditation() this function will ignore the GM_FLAGS_DEADEND
 * flag. GM_FLAGS_ABORTIFY is honored, though.
 *
 * It uses the default display surface.
 * \param flags or'ed together GM_FLAGS_*
 * \param subsystem subsystem id, define your own or use the Amiga ones
 * \param address adress where something happened
 * \return which mouse button was pressed
 */
int guru_alert(unsigned short flags, Uint32 subsystem, void *address);

/*! \brief Guru Meditation
 *
 * This is a true guru meditation alert. Warning! Setting
 * GM_FLAGS_DEADEND will never return but call exit(EXIT_FAILURE),
 * while using GM_FLAGS_ABORTIFY 0x8000 will call abort() on pressing
 * the right mouse button.
 *
 * It uses the default display surface.
 * \param flags or'ed together GM_FLAGS_*
 * \param subsystem subsystem id, define your own or use the Amiga ones
 * \param address adress where something happened
 * \return which mouse button was pressed or terminate application if GM_FLAGS_DEADEND was given.
 */
int guru_meditation(unsigned short flags, Uint32 subsystem, void *address);
#endif
