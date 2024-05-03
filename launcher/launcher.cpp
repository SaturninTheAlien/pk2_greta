/**
 * @file main.cpp
 * Since PK2 is a dynamic linking library,
 * launcher.cpp is needed to load it and run the game.
 */

#include <iostream>
#include "pk2_main.hpp"

int main(int argc, char **argv) {

	bool test_level = false;
	bool dev_mode = false;
	bool show_fps = false;
	bool converting_sprite = false;
	bool updating_sprites = false;

	std::string filename_in;
	std::string filename_out;
	std::string test_path;

	int state = 0;
	for(int i=1;i<argc;++i){
		std::string arg = argv[i];
		switch (state)
		{
		case 0:{
			if(arg=="--help" || arg=="-h"){
				printf("Pekka Kana 2 (Pekka the Rooster 2) is a jump 'n run game made "
				"in the spirit of classic platformers such as Super Mario, SuperTux, Sonic the Hedgehog,"
				" Jazz Jackrabbit, Super Frog and so on.\n"
				"Available command arguments are:\n"
				"-h / --help -> print help,\n"
				"-v / --version -> print version string,\n"
				"-t / --test \"episode/level\" -> test/play particular level\n"
				"(e.g ./pekka-kana-2 --test \"rooster island 2/level13.map\"),\n"
				"-d / --dev -> enable the cheats and the debug tools,\n"
				"--fps -> enable the FPS counter.\n"
				);
				return 0;
			}
			else if(arg=="--version" || arg=="-v"){
				printf("%s\n", pk2_get_version().c_str());
				return 0;
			}
			else if(arg=="--dev" || arg=="-d" || arg=="dev"){
				dev_mode = true;
				//Piste::set_debug(true);
			}
			else if(arg=="--test" || arg=="-t" || arg=="test"){
				state = 1;				
			}
			else if(arg=="--path" || arg=="-p"){
				state = 2;
			}
			else if(arg=="--fps"){
				show_fps= true;
			}
			/*else if (arg=="--mobile") {
				PUtils::Force_Mobile();
			}*/
			else if	(arg=="--convert"){
				printf("Converting sprite\n");
				filename_in = "";
				filename_out = "";
				state=3;
				converting_sprite = true;
			}
			else if (arg=="--update-sprites"){
				printf("Updating sprites\n");
				filename_in = ".";
				filename_out = "";
				state=5;
				updating_sprites = true;
			}
			else {
				printf("Invalid arg\n");
				return 1;
			}
		}
		break;
		case 1:{
			test_path = arg;
			test_level = true;
			state = 0;
		}
		break;
		case 2:{
			if(!pk2_setAssetsPath(arg)){
				return 1;
			}
			state = 0;
		}
		break;
		case 3:{
			filename_in = arg;
			state = 4;			
		}
		break;
		case 4:{
			filename_out = arg;
			state = 0;
		}
		break;
		case 5:{
			filename_in = arg;
			state = 0;
		}
		break;

		default:
			printf("Invalid arg\n");
			return 1;
		}
	}

	pk2_init();

	if(updating_sprites){
		pk2_updateSprites(filename_in);
	}
	else if(converting_sprite){
		pk2_convertToNewFormat(filename_in, filename_out);
	}
	else{
		pk2_main(dev_mode, show_fps, test_level, test_path);
	}
	return 0;
}