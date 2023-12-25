#include "prototypes_handler.hpp"

#include "engine/PLog.hpp"
#include "episode/episodeclass.hpp"
#include "exceptions.hpp"
#include "system.hpp"

void PrototypesHandler::clear(){
	this->unloadSpriteAssets();
    for(PrototypeClass*&prot:mPrototypes){
		if(prot!=nullptr){
			delete prot;
			prot = nullptr;
		}
	}
	mPrototypes.clear();
}


PrototypeClass* PrototypesHandler::loadPrototype(const std::string& filename_in, EpisodeClass*episode){
    bool legacy_spr = true; //try to load legacy .spr file
	std::string filename_clean; //filename without .spr

	if(filename_in.size()>4 && filename_in.substr(filename_in.size()-4,4)==".spr"){
		filename_clean = filename_in.substr(0, filename_in.size() -4);
	}
	else if(filename_in.size()>5 && filename_in.substr(filename_in.size()-5,5)==".spr2"){
		legacy_spr = false;
		filename_clean = filename_in.substr(0, filename_in.size() -5);
	}
	else if(filename_in.size()>4 && filename_in.substr(filename_in.size()-4,4)==".SPR"){
		filename_clean = filename_in.substr(0, filename_in.size() -4);
	}

	else if(filename_in.size()>0){
		legacy_spr = false;
		filename_clean = filename_in;
	}
	else{
		return nullptr;
	}

	/**
	 * @brief Verify if the sprite is already loaded.
	 * It's completely necessary because it prevents infinite recursive loop in some cases.
	 */
	for(PrototypeClass* prot:mPrototypes){
		if(prot->filename==filename_clean){
			return prot;
		}
	}

	PrototypeClass* protot = nullptr;

	try{
		std::string filename_j = filename_clean + ".spr2";
		PFile::Path path_j = Episode->Get_Dir(filename_j);
		if(FindAsset(&path_j,"sprites" PE_SEP)){
			protot = new PrototypeClass();
            /**
             * @brief 
             * TO DO Redesign it 
             */
            protot->LoadPrototypeJSON(path_j,
                [&](const std::string& filename_in){return this->loadPrototype(filename_in, episode);});
		}
		else if(legacy_spr){
			PFile::Path path = Episode->Get_Dir(filename_in);
			if (!FindAsset(&path, "sprites" PE_SEP)) {
				throw PExcept::FileNotFoundException(filename_clean, PExcept::MISSING_SPRITE_PROTOTYPE);
			}
			path_j = path;
			protot = new PrototypeClass();
			protot->LoadPrototypeLegacy(path);
		}
		else{
			throw PExcept::FileNotFoundException(filename_j, PExcept::MISSING_SPRITE_PROTOTYPE);
		}


		//protot->LoadAssets(path_j);
		protot->filename = filename_clean;

		mPrototypes.push_back(protot);

		if(!Episode->ignore_collectable){
			const std::string& collectable_name = Episode->collectable_name;
			if(protot->name.compare(0, collectable_name.size(), collectable_name)==0){
				protot->big_apple = true;
			}
		}

		//Check if ambient
		if(protot->how_destroyed==FX_DESTRUCT_EI_TUHOUDU && protot->damage==0){
			protot->ambient=true;
		}

		//Load transformation
		if(!protot->transformation_str.empty()){
			protot->transformation = this->loadPrototype(protot->transformation_str);
		}

		//Load bunus
		if(!protot->bonus_str.empty()){
			protot->bonus = this->loadPrototype(protot->bonus_str);
		}

		//Load ammo1
		if(!protot->ammo1_str.empty()){
			protot->ammo1 = this->loadPrototype(protot->ammo1_str);
		}

		//Load ammo2
		if(!protot->ammo2_str.empty()){
			protot->ammo2 = this->loadPrototype(protot->ammo2_str);
		}

		return protot;

	}
	catch(const std::exception& e){
		PLog::Write(PLog::ERR, "PK2 Sprites", e.what());
		if(protot!=nullptr){
			auto it = std::find(mPrototypes.begin(), mPrototypes.end(), protot);
			if(it!=mPrototypes.end()){
				mPrototypes.erase(it);
			}
			delete protot;
			protot = nullptr;
		}

		std::string s1 = "SpritePrototypes: ";
		s1 += e.what();

		throw PExcept::PException(s1);
	}

	return nullptr;
}

PrototypeClass* PrototypesHandler::get(int index){
    if(index<mPrototypes.size()){
		return mPrototypes[index];
	}
	return nullptr;
}

void PrototypesHandler::loadSpriteAssets(EpisodeClass* episode){
    for(PrototypeClass* prototype: this->mPrototypes){
		prototype->LoadAssets(episode);
	}
}
void PrototypesHandler::unloadSpriteAssets(){
	for(PrototypeClass* prototype: this->mPrototypes){
		prototype->UnloadAssets();
	}
}