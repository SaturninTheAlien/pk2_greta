#include "prototypes_handler.hpp"

#include <fstream>
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

PFile::Path PrototypesHandler::mGetDir(const std::string& filename)const{
	if(this->mEpisode!=nullptr){
		return this->mEpisode->Get_Dir(filename);
	}
	else if(this->mSearchingDirectory.empty()){
		return PFile::Path(filename);
	}
	else{
		std::string path(this->mSearchingDirectory + PE_SEP + filename);
		return PFile::Path(path);
	} 
}

bool PrototypesHandler::mFindSprite(PFile::Path& path)const{
	return FindAsset(&path, "sprites" PE_SEP);
}

PrototypeClass* PrototypesHandler::loadPrototype(const std::string& filename_in){

	if(this->mAssetsLoaded){
		throw PExcept::PException("Cannot load a sprite prototype after loading sprite assets!");
	}


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
	

	if(!this->mJsonPriority){
		if(legacy_spr){
			filename_clean+=".spr";
		}
		else{
			filename_clean+=".spr2";
		}
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

		if(this->mJsonPriority){
			std::string filename_j = filename_clean + ".spr2";
			PFile::Path path_j = this->mGetDir(filename_j);
			if(this->mFindSprite(path_j)){
				protot = new PrototypeClass();
				/**
				 * @brief 
				 * TO DO Redesign it 
				 */
				protot->LoadPrototypeJSON(path_j,
					[&](const std::string& filename_in){return this->loadPrototype(filename_in);});
			}
			else if(legacy_spr){
				PFile::Path path = this->mGetDir(filename_in);
				if (!this->mFindSprite(path)) {
					throw PExcept::FileNotFoundException(filename_clean, PExcept::MISSING_SPRITE_PROTOTYPE);
				}
				path_j = path;
				protot = new PrototypeClass();
				protot->LoadPrototypeLegacy(path);
			}
			else{
				throw PExcept::FileNotFoundException(filename_j, PExcept::MISSING_SPRITE_PROTOTYPE);
			}
		}
		else if(legacy_spr){
			PFile::Path path = this->mGetDir(filename_clean);
			if(this->mFindSprite(path)){
				protot = new PrototypeClass();
				protot->LoadPrototypeLegacy(path);
			}
			else{
				throw PExcept::FileNotFoundException(filename_clean, PExcept::MISSING_SPRITE_PROTOTYPE);
			}
		}
		else{
			PFile::Path path = this->mGetDir(filename_clean);
			if(this->mFindSprite(path)){
				protot = new PrototypeClass();
				/**
				 * @brief 
				 * TO DO Redesign it 
				 */
				protot->LoadPrototypeJSON(path,
					[&](const std::string& filename_in){return this->loadPrototype(filename_in);});
			}
			else{
				throw PExcept::FileNotFoundException(filename_clean, PExcept::MISSING_SPRITE_PROTOTYPE);
			}
		}

		protot->filename = filename_clean;

		mPrototypes.push_back(protot);

		if(this->mEpisode!=nullptr && !this->mEpisode->ignore_collectable){
			const std::string& collectable_name = this->mEpisode->collectable_name;
			if(protot->name.compare(0, collectable_name.size(), collectable_name)==0){
				protot->big_apple = true;
			}
		}

		/**
		 * @brief 
		 * Load dependecies
		 */

		if(this->mShouldLoadDependencies){
			//Check if ambient
			if(protot->indestructible && protot->damage==0 && protot->type!=TYPE_BONUS){
				protot->ambient=true;
			}

			//Load transformation
			if(!protot->transformation_str.empty()){
				protot->transformation = this->loadPrototype(protot->transformation_str);

				/**
				 * @brief 
				 * Fix red bonus stone not hurting enemies on fall
				 */
				if(protot->ambient && protot->transformation->type==TYPE_BONUS && !protot->enemy){
					protot->ambient = false;
				}
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

void PrototypesHandler::savePrototype(PrototypeClass*prototype, const std::string& filename)const{

	std::string filename2;
	if(filename.size()>5 && filename.substr(filename.size()-5,5)==".spr2"){
		filename2 = filename;
	}
	else{
		filename2 = filename+".spr2";
	}
	nlohmann::json j = *prototype;
	std::ofstream out(filename2.c_str());
	if(!out.good()){	
		throw std::runtime_error("Cannot save the file!");
	}
	out<<j.dump(4);
	out.close();
}


PrototypeClass* PrototypesHandler::get(std::size_t index)const{
    if(index<mPrototypes.size()){
		return mPrototypes[index];
	}
	return nullptr;
}

void PrototypesHandler::loadSpriteAssets(){
	if(this->mEpisode==nullptr){
		throw PExcept::PException("Cannot load sprite assets if the episode pointer is null");
	}

    for(PrototypeClass* prototype: this->mPrototypes){
		prototype->LoadAssets(this->mEpisode);
	}

	this->mAssetsLoaded = true;
}
void PrototypesHandler::unloadSpriteAssets(){
	for(PrototypeClass* prototype: this->mPrototypes){
		prototype->UnloadAssets();
	}

	this->mAssetsLoaded = false;
}