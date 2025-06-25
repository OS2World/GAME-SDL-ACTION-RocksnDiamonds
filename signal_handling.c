#include <signal.h>
#include <SDL/SDL.h>
#include "guru_meditation.h"

/*! \brief end game prematurely
 *
 * \param code return code, exit is called with 128 + code
 */
static void prematurely_quit_game(int code) {
  SDL_Quit();
  exit(128 + code);
}

static struct sigaction old_sa_structs[32];

static void handlesignal(int signal, siginfo_t *si, void *unused) {
  SDL_Surface *scr;

#ifdef DEBUG
  fprintf(stderr, "signal=%d si=%p ctx=%p\n", signal, si, unused);
#endif
  if((scr = SDL_GetVideoSurface()) != NULL) {
    guru_meditation(GM_FLAGS_DEADEND | GM_FLAGS_ABORTIFY, signal, si->si_addr);
    switch(signal) {
    case SIGHUP:
    case SIGINT:
    case SIGILL:
    case SIGTRAP:
    case SIGBUS:
    case SIGSEGV:
    case SIGTERM:
    case SIGCHLD:
    case SIGURG:
    case SIGSYS:
      old_sa_structs[signal].sa_sigaction(signal, si, unused);
    default:
      abort();
    }
  }
  prematurely_quit_game(signal);
}

void set_signalhandling_up(void) {
  struct sigaction sa;
  int i;
  int signums[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGBUS, SIGFPE, SIGUSR1, SIGUSR2, SIGTERM, SIGCHLD, SIGURG, SIGSYS };

  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = handlesignal;
  for(i = 0; i < sizeof(signums)/sizeof(int); ++i) {
    if(sigaction(signums[i], &sa, old_sa_structs + signums[i]) == -1) abort(); //Nothing to be done here...
  }
#ifndef DEBUG
  //Just too useful: catchsegv, etc.
  i = SIGSEGV;
  if(sigaction(signums[i], &sa, old_sa_structs + signums[i]) == -1) abort(); //Nothing to be done here...
#endif
}

