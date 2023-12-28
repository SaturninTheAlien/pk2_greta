# Pekka Kana 2 "Greta engine"

![pekka](res/gfx/manual/pekka.png?raw=true)

Copyright (c) 2003 Janne Kivilahti
https://github.com/poppijanne
https://github.com/pistegamez

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
code redesign by Danilo Lemos (2016-2023),
code redesign, bug fixes and new features by SaturninTheAlien (2023 - present)

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
https://sites.google.com/view/makyuni/makyu-home/pekka-kana/makyunis-mapstore?authuser=0\
To add an episode just download a zip file and put it into "/res/data/mapstore" ("/data/mapstore" on Windows).

The game can be started with the "--dev" argument to enable the
cheats and "--test" followed by "episode_name/level_filename" to play/test a particular level. 

This example starts the level13.map (the robot boss fight) on dev mode:
```
./pekka-kana-2 --dev --test "rooster island 2/level13.map"
```

# New features
* A new sprite format .spr2 based on JSON. The legacy .spr format is still supported for compatibility reasons. All the obsolete c-style arrays in PrototypeClass have been replaced by std::vector, std::string and so there are no limits like max number of AIs and so on. There is still 12 character length limit of the sprite filename length due to the map format. Currently sprite filenames are stored as "name.spr". During the level loading, firstly "name.spr2" is searched for, then "name.spr". If the sprite name is stored as "name", only "name.spr2" is searched for.

* Support for custom SFX sounds (such as jump, splash, map sounds and so on) in zip episodes.

* New Fields in SPR2:
    * "always_active" - if true, the sprite won't deactivate when off-screen.
    * "dead_weight" - weight of the sprite corpse, if not defined the default legacy behaviour,
    * "commands" - description below.

* New AIs:
    * AI_ATTACK_1_IF_PLAYER_ABOVE (31)
    * AI_ATTACK_2_IF_PLAYER_ABOVE (32)
    * AI_TRANSFORM_IF_PLAYER_BELOW (33)
    * AI_TRANSFORM_IF_PLAYER_ABOVE (34)
    * AI_FOLLOW_COMMANDS (40)
    * AI_MOVE_X_COS_FREE (68)
    * AI_MOVE_Y_COS_FREE (69)
    * AI_TRANSFORM_IF_SKULL_BLOCKS_CHANGED (82)
    * AI_DIE_IF_SKULL_BLOCKS_CHANGED (83)

    * AI_RETURN_TO_ORIG_X_CONSTANT (84) - sprite returns to it's starting x position with constant speed and in comparison to the legacy AI_RETURN_TO_ORIG_X (28) there are no oscillations at the target x position,
    * AI_RETURN_TO_ORIG_Y_CONSTANT (85) - sprite returns to it's starting y position with constant speed and in comparison to the legacy AI_RETURN_TO_ORIG_Y (29) there are no oscillations at the target y position,

    * AI_EMIT_EVENT1_IF_DEAD (86) - a way to interact between sprites, maybe changing another set of skull blocks in the future with bigger tilesets,
    * AI_EMIT_EVENT1_IF_DAMAGED (87),
    * AI_TRANSFORM_IF_EVENT1 (88),
    * AI_DIE_IF_EVENT1 (89), 

    * AI_EMIT_EVENT2_IF_DEAD (90) - similar to event1 but without shaking the screen,
    * AI_EMIT_EVENT2_IF_DAMAGED (91),   
    * AI_TRANSFORM_IF_EVENT2 (92)
    * AI_DIE_IF_EVENT2 (93)

    * AI_TRANSFORM_IF_DAMAGED (129) - this AI was implemented, but not working due to a bug before,

    * AI_INFINITE_ENERGY (141) - sprite has infinite energy, it can "receive" damage but it never dies. Only DAMAGE_TYPE_ALL (like falling into the void or die command can really kill it)

* Sprite inheritance / "parent" field.

* AI_FOLLOW_COMMANDS (40) and "commands" field in the new sprite format:
Currently available commands:
> waypoint_x, \<tile_x\>\
> waypoint_y, \<tile_y\>\
> waypoint_xy, \<tile_x\>, \<tile_y\>\
> waypoint_orig_xy, \
> waypoint_seen_player, \
> make_sound \<sound_type\>, \
> die, \
> transform, \
> wait, \<ticks\>, \
> wait_random, \<ticks_min\>, \<ticks_max\>, \
> thunder

Commands / waypoints are executed by the sprite in a loop unless there is "die" or "transform" command.

* Foreground sprites (type 6).
Currently they don't have any AIs supported or parallax types.

# Fixed bugs
* Fixed a potential memory leak in AI 39 (AI_DIE_WITH_MOTHER_SPPRITE)
* Removed initial splash effect if the sprite spawns in the water

* Fixed the bug causing dropped bonuses with weight 0 to levitate (now definitely!),
* Wall sprites are no longer walls after their death.

* Ambient sprites (indestructible and harmless) such as chains and white butterflies no longer block attacks.

* Fixed the bug causing indestructible sprites taking damage after transformation.
* Fixed the bug making it possible to win a level by the dead Pekka.
Player corpse reaching the exit sign no longer makes the level completed.
* Killing the evil one no longer results "hiscore" music fading after reaching the exit tile.

* Fixed the dead player teleporter bug. Player corpse is no longer teleported by the teleporters.

* Fixed broken pixels in some backgrounds caused by converting the color 255 to alpha (for example backround in "Iv4n island 2/level13.map"). Currently the color-to-alpha feature is disabled while loading the background image.

* Fixed incorrectly cropped Pekka's head on the map screen.
* Fixed the "frog corpse" bug.
* Fixed the visible wind bug.
* Friendly sprites from gift no longer kill the enemy player ("Iv4n island 2/spyrooster"),
but not vice versa (the hedgehog gift trap still works).
* Enemy player transformed into friendly prototype by potion is no longer enemy ("Iv4n island 2/spyrooster", robohead transformed into rooster is no longer immune to the enemies),
but not vice versa (for example, rooster player is still friendly after using a pig potion).

* If there is a missing tileset or sprite prototype there will be a dialog window and the game will quit without “Segmentation fault”.

* Fixed the glitch that some dropped bonuses like oranges dropped by hedgehogs used to nearly always move left.
Also the hedgehog from the box used to move nearly always left at the beginning due to the same bug.

# Plans:
* Translate all the Finnish variable names and comments to English.
* Phase out obsolete c-style arrays and replace them respectively with std::string, std::vector and std::array. c-style arrays are intended to remain only where they are necessary like supporting the legacy .spr / .map format.
* Rewrite PDraw to use SDL_Texture (hardware optimised) instead of SDL_Surface, maybe adding PNG support.
* New map format, bigger maps and more tiles.

# Known bugs
* Sprites sometimes don't detect edges near BLOCK_BARRIER_DOWN (legacy bug)


# Used libraries
* JSON for modern c++\
https://github.com/nlohmann/json
* SDL2, SDL2_image, SDL2_mixer\
https://www.libsdl.org/
* libzip\
https://libzip.org/