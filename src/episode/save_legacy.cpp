//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

//TODO
//Move it to Mapstore or EpisodeClass

#include "save_legacy.hpp"

#include "episode/mapstore.hpp"
#include "system.hpp"

#include "engine/PLog.hpp"
#include "engine/PFilesystem.hpp"
#include "exceptions.hpp"

#include <cstring>
#include <string>

#define VERSION "3"

namespace PK2save{

const int PE_PATH_SIZE = 128;
const char* SAVES_FILE = "saves.dat";

struct PK2SAVE_V1{
	s32   jakso;
	char  episodi[260]; //260, 256, 128?
	char  nimi[20];
	bool  kaytossa;
	bool  jakso_lapaisty[100]; //100, 50?
	bool  _[3]; // ?
	s32   pisteet;
};

PK2SAVE saves_slots[SAVE_SLOTS_NUMBER];

int Empty_Records() {

	memset(saves_slots, 0, sizeof(saves_slots));

	for (int i = 0; i < SAVE_SLOTS_NUMBER; i++)
		saves_slots[i].empty = true;

	return 0;
	
}

void Save_All_Records() {

	char versio[2] = VERSION;
	char count_c[8];

	memset(count_c, 0, sizeof(count_c));
	snprintf(count_c, sizeof(count_c), "%i", SAVE_SLOTS_NUMBER);

	PFile::Path path = PFilesystem::GetDataFileW(SAVES_FILE);

	try{
		PFile::RW file = path.GetRW2("w");
		file.write(versio, sizeof(versio)); // Write version "2"
		file.write(count_c, sizeof(count_c)); // Write count "11"

		// Write saves
		for (int i = 0; i < SAVE_SLOTS_NUMBER; i++) {

			file.write(saves_slots[i].empty);
			file.write(saves_slots[i].next_level);
			file.write(saves_slots[i].episode, PE_PATH_SIZE);
			file.write(saves_slots[i].name, 20);
			file.write(saves_slots[i].score);
			file.write(saves_slots[i].level_status, EPISODI_MAX_LEVELS);
		}
		
		file.close();
	}
	catch(const PFile::PFileException& e){
		PLog::Write(PLog::ERR, "PK2", e.what());
		throw PExcept::PException("Can't save records");
	}
}

void LoadSaveSlots() {

	char versio[2];
	char count_c[8];
	int count;

	Empty_Records();

	PFile::Path path = PFilesystem::GetDataFileW(SAVES_FILE);
	if(!path.exists())return;

	try
	{
		PFile::RW file = path.GetRW2("r");
		file.read(versio, sizeof(versio));

		// Version 3 is always little endian
		if (strncmp(versio, "3", 2) == 0) {

			PLog::Write(PLog::INFO, "PK2", "Loading save version 3");

			file.read(count_c, sizeof(count_c));
			count = atoi(count_c);
			if (count > SAVE_SLOTS_NUMBER)
				count = SAVE_SLOTS_NUMBER;

			for (int i = 0; i < count; i++) {
			
				file.read(saves_slots[i].empty);
				file.read(saves_slots[i].next_level);
				file.read(saves_slots[i].episode, PE_PATH_SIZE);
				file.read(saves_slots[i].name, 20);
				file.read(saves_slots[i].score);
				file.read(saves_slots[i].level_status, EPISODI_MAX_LEVELS);

			}
		
		} else if (strncmp(versio, "2", 2) == 0) {

			PLog::Write(PLog::INFO, "PK2", "Loading save version 2");

			file.read(count_c, sizeof(count_c));
			count = atoi(count_c);
			if (count > SAVE_SLOTS_NUMBER)
				count = SAVE_SLOTS_NUMBER;

			file.read(saves_slots, sizeof(PK2SAVE) * count);
			Save_All_Records(); // Change to the current version
		
		} else if (strncmp(versio, "1", 2) == 0) {

			PLog::Write(PLog::INFO, "PK2", "Loading save version 1");

			file.read(count_c, sizeof(count_c));
			count = atoi(count_c);
			if (count > SAVE_SLOTS_NUMBER)
				count = SAVE_SLOTS_NUMBER;

			PK2SAVE_V1 save_v1;
			int episodi_size = 0;
			int jakso_lapaisty_size = 0;
			int align_size = 0;

			int s = file.size();
			int save_size = (s - 10) / count;
			int char_field = save_size - 8;
			PLog::Write(PLog::INFO, "PK2", "Save char size = %i", char_field);

			switch(char_field) {

				case 384:
					episodi_size = 260;
					jakso_lapaisty_size = 100;
					align_size = 3;
					break;
				
				case 332:
					episodi_size = 260;
					jakso_lapaisty_size = 50;
					align_size = 1;
					break;

				case 380:
					episodi_size = 256;
					jakso_lapaisty_size = 100;
					align_size = 3;
					break;
				
				case 328:
					episodi_size = 256;
					jakso_lapaisty_size = 50;
					align_size = 1;
					break;

				case 252:
					episodi_size = 128;
					jakso_lapaisty_size = 100;
					align_size = 3;
					break;
				
				case 200:
					episodi_size = 128;
					jakso_lapaisty_size = 50;
					align_size = 1;
					break;

				default: 
					PLog::Write(PLog::ERR, "PK2", "Couldn't load save file");
					file.close();

			}

			for (int i = 0; i < count; i++) {

				file.read(&save_v1.jakso, 4);
				file.read(&save_v1.episodi, episodi_size);
				file.read(&save_v1.nimi, 20);
				file.read(&save_v1.kaytossa, 1);
				file.read(&save_v1.jakso_lapaisty, jakso_lapaisty_size);
				file.read(&save_v1._, align_size);
				file.read(&save_v1.pisteet, 4);

				saves_slots[i].empty = !save_v1.kaytossa;
				saves_slots[i].next_level = save_v1.jakso;
				strncpy(saves_slots[i].episode, save_v1.episodi, 128);
				strncpy(saves_slots[i].name, save_v1.nimi, 20);
				saves_slots[i].score = save_v1.pisteet;
				for (int j = 0; j < 99; j++)
					saves_slots[i].level_status[j] = save_v1.jakso_lapaisty[j+1]? LEVEL_PASSED : 0;

			}
			//Save_All_Records(); // Change to the current version

		} else {

			PLog::Write(PLog::ERR, "PK2", "Can't read this save version");
			file.close();
			return;

		}

		file.close();

		/* code */
	}
	catch(const PFile::PFileException& e)
	{
		PLog::Write(PLog::DEBUG, "PK2", e.what());
		PLog::Write(PLog::INFO, "PK2", "No save files found");
		Save_All_Records();
	}
}

int Save_Record(int i) {

	if (!Episode) return -1;

	//clean record
	memset(&saves_slots[i], 0, sizeof(PK2SAVE));

	saves_slots[i].empty = false;
	strncpy(saves_slots[i].episode, Episode->entry.name.c_str(), 128);
	saves_slots[i].episode[127] = '\0';

	strncpy(saves_slots[i].name, Episode->player_name.c_str(), 20);
	saves_slots[i].name[19] = '\0';

	if(Episode->isCompleted()){
		saves_slots[i].next_level = UINT32_MAX;
	}
	else{
		saves_slots[i].next_level = Episode->next_level;
	}
	saves_slots[i].score = Episode->player_score;

	for (int j = 0; j < EPISODI_MAX_LEVELS; j++){
		saves_slots[i].level_status[j] = Episode->getLevelStatus(j);
	}
		

	Save_All_Records();

	return 0;

}


}