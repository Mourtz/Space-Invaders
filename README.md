# Space-Invaders
> Unfortunately, I don't think you can run this on your Atari 2600. :sob:

[![Project Status: WIP - The project has not reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/0.1.0/wip.svg)](http://www.repostatus.org/#wip)
[![DUB](https://img.shields.io/dub/l/vibe-d.svg)](https://github.com/Mourtz/Space-Invaders/blob/master/LICENSE)

[![Build Status](https://travis-ci.org/Mourtz/Space-Invaders.svg?branch=master)](https://travis-ci.org/Mourtz/Space-Invaders)

---

<h3> ToDo </h3>

* ~~somehow adjust terminal resolution to make game more visually stunning.~~
* ~~implement enemies.~~
* ~~fix segmentation faults when something "goes" out of bounds.~~
* ~~finalize game mechanics.~~
* [ OPTIONAL ]
  * create user interface.
  * add sound effects/music.
* make a bare metal version of the game.

> currently game runs on 3 threads using POSIX threading library. Which means, before porting the game to bare metal a functional sceduler must be implemented. Otherwise, the game could also be downgraded in order to work on 1 thread. 

* make it bootable.

<h3> Dependencies </h3>
```
gcc nasm libncurses5-dev libpthread
```
