# Pekka Kana 2 "Greta engine"

![pekka](res/gfx/manual/pekka.png?raw=true)

Copyright (c) 2003 Janne Kivilahti
https://github.com/poppijanne

Ported to SDL by Samuli Tuomola (2010)
https://github.com/stt/pk2

Ported to SDL2 by Danilo Lemos (2016-2023)
https://github.com/danilolc/pk2

Hard fork by SaturninTheAlien (2023-present)

"Pekka Kana 2 (Pekka the Rooster 2) is a jump 'n run game made in the spirit of classic platformers such as Super Mario, SuperTux, Sonic the Hedgehog, Jazz Jackrabbit, Super Frog and so on."
http://kiwi.mbnet.fi/pistegamez/pk2/

This repository contains the source code and resource files for the game Pekka Kana 2 created by Janne Kivilahti in 2003.
The source code was released by the original author on a forum
[thread](http://pistegamez.proboards.com/thread/543/level-editor-source-codes-available).

Since then a lot of things changed, including an initial SDL port by Samuli Tuomola in 2010 and then the SDL2 port, bug fixes,
code redesign by Danilo Lemos (2016-2020),
code redesign, bug fixes and new features by SaturninTheAlien (2023 - present)

The codename "Greta" refers to the stenograms in some levels from "Iv4n island 2".

This game was tested on Windows, GNU/Linux, Mac OS (Apple Silicon).

# Binary distibution
Compiled binaries for MS Windows available here (64-bit):\
https://drive.google.com/drive/folders/1tGwfXsvdTsnTQ29HcRmqSFKb__RW4mIW?usp=sharing 

They were compiled using MS Visual Studio Community 2022 on MS Windows 10,
so if there is any missing DLL file you perhaps need to install the latest Microsoft Visual C++ Redistributable:\
https://aka.ms/vs/17/release/vc_redist.x64.exe

# Compiling

### Ubuntu / Debian
To compile **Pekka Kana 2** into Debian GNU/Linux based distributions,
you need the **SDL2**, **SDL2_image**, **SDL2_mixer** and **Zip** libraries installed on
the system. To get everything you need, just run the following command on the terminal:

    $ apt install build-essential libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libzip-dev

Then run the "**make**" command inside the "**Pekka Kana 2**" directory.

    $ make -j4 pk2

The PK2 binary will be in "**bin/**" directory.

    $ ./bin/pekka-kana-2

To make the removal, within the compiled directory, execute this command:

    $ make clean

### macOS
Use [Homebrew](https://brew.sh/) to install SDL2 packages:
```
brew install sdl2 sdl2_gfx sdl2_image sdl2_mixer libzip
```
Now just clone the code and run `make` on pk2 directory and run `/bin/pekka-kana-2`, just like on Ubuntu/Debian.

# Our community
* Discord: https://discord.gg/kqDJfYX
* Old forum: http://pistegamez.proboards.com/
* Pistegamez site: https://www.pistegamez.net/

# User made episodes:
https://sites.google.com/view/makyuni/makyu-home/pekka-kana/makyunis-mapstore?authuser=0
To add a new episode, just download the zip, etract and put it inside "res/" on local data.

The game can be started with the "--dev" argument to enable the
cheats and "--test" following by the episode and level to
open directly on the level.

This example starts the level13.map (the robot boss fight) on dev mode:
```
./pekka-kana-2 --dev --test "rooster island 2/level13.map"
```
# Plans:
* A new sprite format based on JSON, still supporting legacy .spr format for the compatibility reason.
* Translate all the Finnish variable names and comments to English.
* Phase out obsolete c-style arrays and replace them respectively with std::string, std::vector and std::array. c-style arrays are intended to remain only where they are necessary like supporting the legacy .spr / .map format.
* Rewrite PDraw to use SDL_Texture (hardware optimised) instead of SDL_Surface, maybe adding PNG support.
* New map format, bigger maps and more tiles.

# Known bugs
* Killing the evil one results music fading after reaching the exit tile.
* (Only the last Windows Snapshot) It seems that there are too many sprites sounds and some of them perhaps should be skipped intentionally.
It doesn't depend on the used sprite format.
Maybe because of my temporary solution. I'm putting -1 as the channel number in "Mix_PlayChannel".
I do this because the channels don't work on Windows properly. If I don't do this, there are only a few sprites sounds at the beginning of the game and then the utter silence.

# Used libraries
* JSON for modern c++\
https://github.com/nlohmann/json
* SDL2, SDL2_image, SDL2_mixer\
https://www.libsdl.org/
* libzip\
https://libzip.org/