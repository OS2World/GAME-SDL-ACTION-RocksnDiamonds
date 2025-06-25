# Rockdodger #

## Introduction ##

Fly through an asteroid field avoiding rocks and greeblies. The
homepage and official development repository is at the web page:
https://bitbucket.org/rpkrawczyk/rockdodger.

This game had once upon in time the 4th place in "NoStarch game contest"!

## Controls ##

  *  Up = go up
  *  Down = go down
  *  Left = go left
  *  Right = go right
  *  D = Fire laser!
  *  S = Shields (very useful!)
  *  P = Pause 


## Command line ##

Run the program with "rockdodger".

  * -h Help message
  * -? Help message
  * -f Full screen
  * -w Window mode
  * -x X-size
  * -y Y-size
  * -k Keyboard only - disable joystick
  * -s Silent mode (no sound)
  * -P Paramter file name
  * -I always play the intro before starting the game

## History ##

Paul started development on his old Challenger, porting the game later
on to Linux and SDL. In 2002 the development was moved to code and in
2015 moved again to bitbucket. 

## Links ##

  * This game even has a Blog at [http://rockdodgergame.blogspot.de/](http://rockdodgergame.blogspot.de/).

## Prerequisites ##

You MUST have SDL installed. Any version later than 1.2 should be fine.

Be aware that there are problems with SDL before about version 1.1.6
or so, especially with the font support, so you might want to download
the latest SDL version 1.2. We are developing using version 1.3, so
sometime in the future you might find that this game runs a bit funny
if you don't have the latest version.

You also need the following libraries:

  * SDL_image library to load the png images.
  * SDL_mixer library to load play the music.

## Install ##

Untar the file into a directory, type 

```
#!bash

make
```

, and type 

```
#!bash

./rockdodger 

```
to run it. To run in full-screen mode, type

```
#!bash

./rockdodger -f
```