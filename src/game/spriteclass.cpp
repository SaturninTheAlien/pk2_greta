//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "game/spriteclass.hpp"

#include "system.hpp"
#include "gfx/effect.hpp"

#include "engine/PDraw.hpp"
#include "engine/PSound.hpp"
#include "engine/PLog.hpp"
#include "engine/platform.hpp"
#include "episode/episodeclass.hpp"
#include "exceptions.hpp"

#include <cstring>
#include <algorithm>
#include <sstream>

class UnsupportedSpriteVersionException: public std::exception{
public:
	UnsupportedSpriteVersionException(const std::string& version);
	const char* what() const noexcept{
        return message.c_str();
    }
private:
	std::string message;
};

UnsupportedSpriteVersionException::UnsupportedSpriteVersionException(const std::string& version){
	std::ostringstream os;
	os<<"Unsupported sprite version: "<<version;
	this->message = os.str();
}

std::vector<PrototypeClass*>mPrototypes;
void Prototype_ClearAll(){
	for(PrototypeClass*&prot:mPrototypes){
		if(prot!=nullptr){
			delete prot;
			prot = nullptr;
		}
	}
	mPrototypes.clear();
}

PrototypeClass* Get_Prototype_Debug(std::size_t index){
	if(index<mPrototypes.size()){
		return mPrototypes[index];
	}
	return nullptr;
}

PrototypeClass* Prototype_Load(const std::string& filename_in){

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
			protot->LoadPrototypeJSON(path_j);
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


		protot->LoadAssets(path_j);
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
		if(!protot->transformation_sprite.empty()){
			protot->transformation = Prototype_Load(protot->transformation_sprite);
		}

		//Load bunus
		if(!protot->bonus_sprite.empty()){
			protot->bonus = Prototype_Load(protot->bonus_sprite);
		}

		//Load ammo1
		if(!protot->ammo1_sprite.empty()){
			protot->ammo1 = Prototype_Load(protot->ammo1_sprite);
		}

		//Load ammo2
		if(!protot->ammo2_sprite.empty()){
			protot->ammo2 = Prototype_Load(protot->ammo2_sprite);
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


void to_json(nlohmann::json& j, const SpriteAnimation& a){
	j["loop"] = a.loop;
	j["sequence"] = a.sequence; 
}

void from_json(const nlohmann::json&j, SpriteAnimation& a){
	a.loop = j["loop"].get<bool>();
	a.sequence = j["sequence"].get<std::vector<int>>();
}

/* -------- SpriteClass Prototyyppi ------------------------------------------------------------------ */

PrototypeClass::PrototypeClass(){}

PrototypeClass::~PrototypeClass(){
	for(int& frame_index: this->frames){
		if(frame_index>0){
			PDraw::image_delete(frame_index);
		}
	}

	for(int& frame_index: this->frames_mirror){
		if(frame_index>0){
			PDraw::image_delete(frame_index);
		}
	}

	for(const int& sound_index: this->sounds){
		if(sound_index!=-1){
			PSound::free_sfx(sound_index);
		}
	}
	for(SpriteCommands::Command* & command: this->commands){
		if(command!=nullptr){
			delete command;
			command = nullptr;
		}
	}
}

void PrototypeClass::SetProto10(PrototypeClass10 &proto){
	this->picture_filename = proto.picture;
	this->name = proto.name;

	this->transformation_sprite = proto.transformation_sprite;
	this->bonus_sprite = proto.bonus_sprite;
	this->ammo1_sprite = proto.ammo1_sprite;
	this->ammo2_sprite = proto.ammo2_sprite;

	for (int i=0;i<SPRITE_SOUNDS_NUMBER;i++) {
		this->sound_files[i] = proto.sound_files[i];
		this->sounds[i] = proto.sounds[i];
	}

	//animations_number		= proto.animations_number;
	can_open_locks				= proto.can_open_locks;
	energy				= proto.energy;
	is_wall				= proto.is_wall;
	frames_number				= proto.frames_number;
	frame_rate			= proto.frame_rate;
	picture_frame_x				= proto.picture_frame_x;
	picture_frame_y				= proto.picture_frame_y;
	picture_frame_width	= proto.picture_frame_width;
	picture_frame_height	= proto.picture_frame_height;
	height				= proto.height;
	charge_time			= proto.charge_time;
	width				= proto.width;
	max_jump			= proto.max_jump;
	max_speed			= proto.max_speed;
	weight				= proto.weight;
	score				= proto.score;
	immunity_type				= proto.immunity_type;
	how_destroyed		= proto.how_destroyed;
	type				= proto.sprite_type;
	damage				= proto.damage;
	color				= proto.color;
	enemy			= proto.enemy;


	for (int i=0;i<5;i++){
		if(proto.AI[i]!=0){
			this->AI_v.push_back(proto.AI[i]);
		}
	}

	for(int i=0;i<SPRITE_ANIMATIONS_NUMBER;++i){
		this->animations[i] = proto.animaatiot[i];
	}
}
void PrototypeClass::SetProto11(PrototypeClass11 &proto){
	this->picture_filename = proto.picture;
	this->name = proto.name;

	this->transformation_sprite = proto.transformation_sprite;
	this->bonus_sprite = proto.bonus_sprite;
	this->ammo1_sprite = proto.ammo1_sprite;
	this->ammo2_sprite = proto.ammo2_sprite;

	for (int i=0;i<SPRITE_SOUNDS_NUMBER;i++) {
		this->sound_files[i] = proto.sound_files[i];
		this->sounds[i] = proto.sounds[i];
	}

	//animations_number		= proto.animations_number;
	can_open_locks				= proto.can_open_locks;
	bonuses_number        = proto.bonuses_number;
	energy				= proto.energy;
	is_wall				= proto.is_wall;
	frames_number				= proto.frames_number;
	frame_rate			= proto.frame_rate;
	attack1_time		= proto.attack1_time;
	attack2_time		= proto.attack2_time;
	picture_frame_x				= proto.picture_frame_x;
	picture_frame_y				= proto.picture_frame_y;
	picture_frame_width	= proto.picture_frame_width;
	picture_frame_height	= proto.picture_frame_height;
	height				= proto.height;
	charge_time			= proto.charge_time;
	width				= proto.width;
	max_jump			= proto.max_jump;
	max_speed			= proto.max_speed;
	weight				= proto.weight;
	parallax_type		= proto.parallax_type;
	score				= proto.score;
	immunity_type				= proto.immunity_type;
	vibrates				= proto.vibrates;
	how_destroyed		= proto.how_destroyed;
	type				= proto.sprite_type;
	damage				= proto.damage;
	damage_type		= proto.damage_type;
	color				= proto.color;
	enemy			= proto.enemy;

	for (int i=0;i<5;i++){
		if(proto.AI[i]!=0){
			this->AI_v.push_back(proto.AI[i]);
		}
	}

	/*for (int i=0;i<SPRITE_MAX_ANIMAATIOITA;i++) {
		for (int j=0;j<ANIMATION_SEQUENCE_SIZE;j++)
			animaatiot[i].sekvenssi[j] = proto.animaatiot[i].sekvenssi[j];

		animaatiot[i].looppi  = proto.animaatiot[i].looppi;
		animaatiot[i].frames_number = proto.animaatiot[i].frames_number;
	}*/
	for(int i=0;i<SPRITE_ANIMATIONS_NUMBER;++i){
		this->animations[i] = proto.animaatiot[i];
	}
}
void PrototypeClass::SetProto12(PrototypeClass12 &proto){

	this->picture_filename = proto.picture;
	this->name = proto.name;

	this->transformation_sprite = proto.transformation_sprite;
	this->bonus_sprite = proto.bonus_sprite;
	this->ammo1_sprite = proto.ammo1_sprite;
	this->ammo2_sprite = proto.ammo2_sprite;

	for (int i=0;i<SPRITE_SOUNDS_NUMBER;i++) {
		//strncpy(sound_files[aani], proto.sound_files[aani], 13);
		this->sound_files[i] = proto.sound_files[i];
		this->sounds[i] = proto.sounds[i];
	}

	sound_frequency			= proto.sound_frequency;
	//animations_number		= proto.animations_number;
	can_open_locks				= proto.can_open_locks;
	bonuses_number        = proto.bonuses_number;
	energy				= proto.energy;
	is_wall				= proto.is_wall;
	is_wall_up			= proto.is_wall_up;
	is_wall_down			= proto.is_wall_down;
	is_wall_right		= proto.is_wall_right;
	is_wall_left		= proto.is_wall_left;
	frames_number				= proto.frames_number;
	frame_rate			= proto.frame_rate;
	attack1_time		= proto.attack1_time;
	attack2_time		= proto.attack2_time;
	picture_frame_x				= proto.picture_frame_x;
	picture_frame_y				= proto.picture_frame_y;
	picture_frame_width	= proto.picture_frame_width;
	picture_frame_height	= proto.picture_frame_height;
	height				= proto.height;
	charge_time			= proto.charge_time;
	width				= proto.width;
	max_jump			= proto.max_jump;
	max_speed			= proto.max_speed;
	weight				= proto.weight;
	parallax_type		= proto.parallax_type;
	score				= proto.score;
	random_sound_frequency			= proto.random_sound_frequency;
	immunity_type				= proto.immunity_type;
	vibrates				= proto.vibrates;
	check_tiles		= proto.check_tiles;
	how_destroyed		= proto.how_destroyed;
	type				= proto.sprite_type;
	damage				= proto.damage;
	damage_type		= proto.damage_type;
	color				= proto.color;
	enemy			= proto.enemy;

	for (int i=0;i<5;i++){
		if(proto.AI[i]!=0){
			this->AI_v.push_back(proto.AI[i]);
		}
	}

	/*for (int i=0;i<SPRITE_MAX_ANIMAATIOITA;i++) {
		for (int j=0;j<ANIMATION_SEQUENCE_SIZE;j++)
			animaatiot[i].sekvenssi[j] = proto.animaatiot[i].sekvenssi[j];

		animaatiot[i].looppi  = proto.animaatiot[i].looppi;
		animaatiot[i].frames_number = proto.animaatiot[i].frames_number;
	}*/

	for(int i=0;i<SPRITE_ANIMATIONS_NUMBER;++i){
		this->animations[i] = proto.animaatiot[i];
	}
}
void PrototypeClass::SetProto13(PrototypeClass13 &proto){
	this->picture_filename = proto.picture;
	this->name = proto.name;

	this->transformation_sprite = proto.transformation_sprite;
	this->bonus_sprite = proto.bonus_sprite;
	this->ammo1_sprite = proto.ammo1_sprite;
	this->ammo2_sprite = proto.ammo2_sprite;

	for (int i=0;i<SPRITE_SOUNDS_NUMBER;i++) {
		this->sound_files[i] = proto.sound_files[i];
		this->sounds[i] = proto.sounds[i];
	}

	sound_frequency			= proto.sound_frequency;
	//animations_number		= proto.animations_number;
	can_open_locks				= proto.can_open_locks;
	bonuses_number        = proto.bonuses_number;
	energy				= proto.energy;
	is_wall				= proto.is_wall;
	is_wall_up			= proto.is_wall_up;
	is_wall_down			= proto.is_wall_down;
	is_wall_right		= proto.is_wall_right;
	is_wall_left		= proto.is_wall_left;
	frames_number				= proto.frames_number;
	frame_rate			= proto.frame_rate;
	attack1_time		= proto.attack1_time;
	attack2_time		= proto.attack2_time;
	picture_frame_x				= proto.picture_frame_x;
	picture_frame_y				= proto.picture_frame_y;
	picture_frame_width	= proto.picture_frame_width;
	picture_frame_height	= proto.picture_frame_height;
	height				= proto.height;
	charge_time			= proto.charge_time;
	width				= proto.width;
	max_jump			= proto.max_jump;
	max_speed			= proto.max_speed;
	weight				= proto.weight;
	parallax_type		= proto.parallax_type;
	score				= proto.score;
	random_sound_frequency			= proto.random_sound_frequency;
	immunity_type				= proto.immunity_type;
	vibrates				= proto.vibrates;
	check_tiles		= proto.check_tiles;
	how_destroyed		= proto.how_destroyed;
	type				= proto.sprite_type;
	damage				= proto.damage;
	damage_type		= proto.damage_type;
	color				= proto.color;
	enemy			= proto.enemy;

	effect		= proto.effect;
	is_transparent				= proto.is_transparent;
	projectile_charge_time			= proto.projectile_charge_time;
	can_glide			= proto.can_glide;
	//boss				= proto.boss;
	bonus_always			= proto.bonus_always;
	can_swim			= proto.can_swim;

	for (int i=0;i<10;i++){
		if(proto.AI[i]!=0){
			this->AI_v.push_back(proto.AI[i]);
		}
	}

	/*for (int i=0;i<SPRITE_MAX_ANIMAATIOITA;i++) {
		for (int j=0;j<ANIMATION_SEQUENCE_SIZE;j++)
			animaatiot[i].sekvenssi[j] = proto.animaatiot[i].sekvenssi[j];

		animaatiot[i].looppi  = proto.animaatiot[i].looppi;
		animaatiot[i].frames_number = proto.animaatiot[i].frames_number;
	}*/

	for(int i=0;i<SPRITE_ANIMATIONS_NUMBER;++i){
		this->animations[i] = proto.animaatiot[i];
	}
}

const std::map<std::string,int> PrototypeClass::AnimationsDict = {
	{"idle", ANIMATION_IDLE},
    {"walking", ANIMATION_WALKING},
    {"jump_up", ANIMATION_JUMP_UP},
    {"jump_down", ANIMATION_JUMP_DOWN},
    {"squat", ANIMATION_SQUAT},
    {"damage", ANIMATION_DAMAGE},
    {"death", ANIMATION_DEATH},
    {"attack1", ANIMATION_ATTACK1},
    {"attack2", ANIMATION_ATTACK2}
};

const std::map<std::string,int> PrototypeClass::SoundTypesDict={
    {"damage", SOUND_DAMAGE},
    {"destruction", SOUND_DESTRUCTION},
    {"attack1", SOUND_ATTACK1},
    {"attack2", SOUND_ATTACK2},
    {"random", SOUND_RANDOM},
    {"special1", SOUND_SPECIAL1},
    {"special2", SOUND_SPECIAL2}
};

const std::map<std::string,u8> PrototypeClass::ColorsDict={
	{"gray", COLOR_GRAY},
	{"blue", COLOR_BLUE},
	{"red", COLOR_RED},
	{"green", COLOR_GREEN},
	{"orange", COLOR_ORANGE},
	{"violet", COLOR_VIOLET},
	{"turquoise", COLOR_TURQUOISE},
	{"normal", COLOR_NORMAL}
};

void PrototypeClass::SetProto20(const nlohmann::json& j){
	using namespace PJson;

	if(j.contains("ai")){
		this->AI_v.clear();
		this->AI_v = j["ai"].get<std::vector<int>>();
	}

	jsonReadString(j, "ammo1", this->ammo1_sprite);


	jsonReadString(j, "ammo2", this->ammo2_sprite);

	if(j.contains("animations")){
		const nlohmann::json& j_animations = j["animations"]; 
		for(std::pair<std::string, int> p: AnimationsDict){
			if(j_animations.contains(p.first)){
				this->animations[p.second] = j_animations[p.first].get<SpriteAnimation>();
			}
		}
	}

	jsonReadBool(j, "always_active", this->always_active);

	jsonReadInt(j, "attack1_time", this->attack1_time);

	jsonReadInt(j, "attack2_time", this->attack2_time);

	jsonReadBool(j, "bonus_always", this->bonus_always);

	jsonReadString(j, "bonus", this->bonus_sprite);

	jsonReadInt(j, "bonuses_number", this->bonuses_number);

	//jsonReadBool(j, "boss", this->boss);

	jsonReadBool(j, "can_glide", this->can_glide);

	jsonReadBool(j, "can_open_locks", this->can_open_locks);

	jsonReadBool(j, "can_swim", this->can_swim);

	jsonReadInt(j, "charge_time", this->charge_time);

	jsonReadEnumU8(j, "color", this->color, ColorsDict);

	jsonReadInt(j, "damage", this->damage);

	jsonReadInt(j, "damage_type", this->damage_type);

	jsonReadEnumU8(j, "effect", this->effect);

	jsonReadBool(j, "enemy", this->enemy);

	jsonReadInt(j, "energy", this->energy);

	if(j.contains("frame")){
		const nlohmann::json& j_frame = j["frame"];
		this->picture_frame_x = j_frame["pos_x"].get<int>();
		this->picture_frame_y = j_frame["pos_y"].get<int>();
		this->picture_frame_width  = j_frame["width"].get<int>();
		this->picture_frame_height = j_frame["height"].get<int>();
	}

	jsonReadInt(j, "frame_rate", this->frame_rate);

	jsonReadInt(j, "frames_number", this->frames_number);

	jsonReadInt(j, "how_destroyed", this->how_destroyed);

	jsonReadInt(j, "immunity_type", this->immunity_type);

	jsonReadBool(j, "is_transparent", this->is_transparent);

	jsonReadBool(j, "is_wall", this->is_wall);

	jsonReadBool(j, "is_wall_down", this->is_wall_down);

	jsonReadBool(j, "is_wall_left", this->is_wall_left);

	jsonReadBool(j, "is_wall_right", this->is_wall_right);

	jsonReadBool(j, "is_wall_up", this->is_wall_up);

	jsonReadBool(j, "check_tiles", this->check_tiles);

	jsonReadInt(j, "max_jump", this->max_jump);

	jsonReadDouble(j, "max_speed", this->max_speed);

	jsonReadString(j, "name", this->name);

	jsonReadInt(j, "parallax_type", this->parallax_type);

	jsonReadString(j, "picture", this->picture_filename);

	jsonReadInt(j, "projectile_charge_time", this->projectile_charge_time);

	jsonReadBool(j, "random_sound_frequency", this->random_sound_frequency);

	jsonReadInt(j, "score", this->score);

	if(j.contains("size")){
		const nlohmann::json& size = j["size"];
		this->width = size["width"].get<int>();
		this->height = size["height"].get<int>();
	}

	jsonReadInt(j, "sound_frequency", this->sound_frequency);

	if(j.contains("sounds")){
		const nlohmann::json& j_sounds = j["sounds"];
		for(std::pair<std::string, int> p: SoundTypesDict){
			this->sounds[p.second] = -1;
			jsonReadString(j_sounds, p.first, this->sound_files[p.second]);
		}
	}

	jsonReadString(j, "transformation", this->transformation_sprite);

	jsonReadInt(j, "type", this->type);

	jsonReadBool(j, "vibrates", this->vibrates);

	jsonReadDouble(j, "weight", this->weight);

	if(j.contains("commands")){
		SpriteCommands::Parse_Commands(j["commands"], this->commands, this->width, this->height);
	}

	if(j.contains("color_to_alpha")){
		const nlohmann::json& j2 = j["color_to_alpha"];
		if(j2.is_null()){
			this->change_color_to_alpha = false;
		}
		else if(j2.is_number_integer()){
			this->change_color_to_alpha = true;
			this->color_to_alpha = j2.get<unsigned int>();
		}
	}
}

void PrototypeClass::LoadPrototypeJSON(PFile::Path path){
	const nlohmann::json proto = path.GetJSON();
	if(!proto.contains("version") || !proto["version"].is_string()){
		throw PExcept::PException("Incorrect JSON, no string field \"version\"");
	}
	std::string version = proto["version"].get<std::string>();
	if(version=="2.0"){

		if(proto.contains("parent")&&proto["parent"].is_string()){
			PrototypeClass* parentPrototype = Prototype_Load(proto["parent"]);
			if(parentPrototype!=nullptr){
				if(!parentPrototype->commands.empty()){
					throw PExcept::PException("The parent prototype cannot contain commands!");
				}

				*this = *parentPrototype;
			}
		}

		this->SetProto20(proto);
	}
	else{
		throw UnsupportedSpriteVersionException(version);
	}
}

void PrototypeClass::LoadPrototypeLegacy(PFile::Path path){

	try{
		PFile::RW file = path.GetRW2("r");
		char versio[4];
		file.read(versio, 4);

		if (strcmp(versio,"1.0") == 0){
			PrototypeClass10 proto;
			file.read(&proto, sizeof(proto));
			this->SetProto10(proto);
		}
		else if (strcmp(versio,"1.1") == 0){
			PrototypeClass11 proto;
			file.read(&proto, sizeof(proto));
			this->SetProto11(proto);
		}
		else if (strcmp(versio,"1.2") == 0){
			PrototypeClass12 proto;
			file.read(&proto, sizeof(proto));
			this->SetProto12(proto);
		}
		else if (strcmp(versio,"1.3") == 0){
			PrototypeClass13 proto;
			file.read(&proto, sizeof(proto));
			this->SetProto13(proto);
		}
		else {
			file.close();
			throw UnsupportedSpriteVersionException(versio);
		}
		this->version = versio;
		file.close();


	}
	catch(const std::exception& e){
		PLog::Write(PLog::ERR, "PK2 Sprites", e.what());
		throw PExcept::FileNotFoundException(path.c_str(), PExcept::MISSING_SPRITE_PROTOTYPE);
	}	
}


void PrototypeClass::LoadAssets(PFile::Path path){
	if(this->frames_number>0){
		this->frames.resize(this->frames_number);
		this->frames_mirror.resize(this->frames_number);
	}

	PFile::Path image = path;
	image.SetFile(this->picture_filename);
	
	if (!FindAsset(&image, "sprites" PE_SEP)) {

		throw PExcept::FileNotFoundException(this->picture_filename, PExcept::MISSING_SPRITE_TEXTURE);

	}

	int bufferi = PDraw::image_load(image, false, this->change_color_to_alpha, this->color_to_alpha);
	if (bufferi == -1) {
		throw PExcept::FileNotFoundException(this->picture_filename, PExcept::MISSING_SPRITE_TEXTURE);
	}

	//Change sprite colors
	if (this->color != COLOR_NORMAL){ 

		int w, h;
		u8 color;
		PDraw::image_getsize(bufferi,w,h);

		u8 *buffer = NULL;
		u32 width;
		PDraw::drawimage_start(bufferi, buffer, width);

		for (int x = 0; x < w; x++)
			for (int y = 0; y < h; y++)
				if ((color = buffer[x+y*width]) != 255) {
					color %= 32;
					color += this->color;
					buffer[x+y*width] = color;
				}

		PDraw::drawimage_end(bufferi);
	}

	int frame_x = picture_frame_x;
	int frame_y = picture_frame_y;

	//Get each frame
	for (int frame_i = 0; frame_i < frames_number; frame_i++) {

		if (frame_x + picture_frame_width > 640) {
			frame_y += this->picture_frame_height + 3;
			frame_x = picture_frame_x;
		}

		this->frames[frame_i] = PDraw::image_cut(bufferi,frame_x,frame_y,picture_frame_width,picture_frame_height, this->change_color_to_alpha, this->color_to_alpha); //frames
		this->frames_mirror[frame_i] = PDraw::image_cut(bufferi,frame_x,frame_y,picture_frame_width,picture_frame_height, this->change_color_to_alpha, this->color_to_alpha); //flipped frames
		PDraw::image_fliphori(this->frames_mirror[frame_i]);

		frame_x += this->picture_frame_width + 3;
	
	}

	PDraw::image_delete(bufferi);

	for (int i = 0; i < SPRITE_SOUNDS_NUMBER; i++) {

		if(!this->sound_files[i].empty()){
			PFile::Path sound = path;
			sound.SetFile(this->sound_files[i]);

			if (FindAsset(&sound, "sprites" PE_SEP)) {

				this->sounds[i] = PSound::load_sfx(sound);

			} else {

				PLog::Write(PLog::ERR, "PK2 Sprites", "Can't find sound %s", this->sound_files[i].c_str());
			}
		}
	}

	SpriteAI::AI_Table::INSTANCE.InitSpriteAIs(this->AI_f, this->AI_v);
}

void PrototypeClass::Draw(int x, int y, int frame)const{
	PDraw::image_clip(this->frames[frame], x, y);
}
bool PrototypeClass::HasAI(int ai)const{
	return std::find(this->AI_v.begin(), this->AI_v.end(), ai) != this->AI_v.end();
}

/* -------- SpriteClass  ------------------------------------------------------------------ */

SpriteClass::SpriteClass(){}
SpriteClass::SpriteClass(PrototypeClass *prototype, int player, double x, double y){
	if (prototype) {

		this->prototype         = prototype;
		this->player        = player;

		this->active     = true;
		this->removed        = false;

		this->x              = x;
		this->y              = y;
		this->orig_x         = x;
		this->orig_y         = y;
		
		this->energy        = prototype->energy;
		this->initial_weight = prototype->weight;
		this->weight         = prototype->weight;
		
		this->enemy     = prototype->enemy;
		this->ammo1         = prototype->ammo1;
		this->ammo2         = prototype->ammo2;
	
	}
}

SpriteClass::~SpriteClass() {}

int SpriteClass::Animaatio(int anim_i, bool reset){
	if (anim_i != animation_index){
		if (reset)
			current_sequence = 0;

		animation_index = anim_i;
	}

	return 0;
}
int SpriteClass::Animoi(){

	switch (prototype->first_ai()) {
		case AI_ROOSTER:           Animation_Rooster();  break;
		case AI_LITTLE_CHICKEN: Animation_Rooster();  break;
		case AI_BONUS:          Animation_Bonus(); break;
		case AI_EGG:            Animation_Egg();   break;
		case AI_EGG2:           Animation_Egg();   break;
		case AI_PROJECTILE:          Animation_Projectile(); break;
		case AI_JUMPER:         Animation_Rooster();  break;
		case AI_BASIC:          Animation_Basic(); break;
		case AI_TELEPORT:       Animation_Basic(); break;
		default:                break;
	}

	const SpriteAnimation& anim = prototype->animations[animation_index];

	/*if (current_sequence >= animaatio.frames_number)
		current_sequence = 0;*/

	if(current_sequence >= anim.sequence.size()){
		current_sequence = 0;
		return 0;
	}

	int frame = anim.sequence[current_sequence] - 1;

	// Calculate how much of the currently valid frame is still displayed
	if (frame_timer < prototype->frame_rate)
		frame_timer++;
	// If the time has elapsed, the next frame of the current animation is changed
	else {
		frame_timer = 0;

		// Are there more frames in the animation?
		if (current_sequence + 1 < anim.sequence.size())
			current_sequence++;
		// If not and the animation is set to loop, the animation is started from the beginning.
		else if(anim.loop) {
			current_sequence = 0;
		}
	}

	if (frame < 0)
		frame = 0;
	
	if (frame >= prototype->frames_number)
		frame = prototype->frames_number - 1;

	return frame;
}
int SpriteClass::Draw(int kamera_x, int kamera_y){
	// Tehdaan apumuuttujia
	int	l = (int)prototype->picture_frame_width/2,//width
		h = (int)prototype->picture_frame_height/2,
		x = (int)this->x-(kamera_x),
		y = (int)this->y-(kamera_y);

	int frame = this->Animoi();

	if (prototype->vibrates){
		x += rand()%2 - rand()%2;
		y += rand()%2 - rand()%2;
	}
	
	if (flip_x) {
		
		if(this->invisible_timer)
			PDraw::image_cliptransparent(prototype->frames_mirror[frame], x-l-1, y-h, 40, COLOR_GRAY);
		else
			PDraw::image_clip(prototype->frames_mirror[frame], x-l-1, y-h);

	} else {

		if(this->invisible_timer)
			PDraw::image_cliptransparent(prototype->frames[frame], x-l-1, y-h, 40, COLOR_GRAY);
		else
			PDraw::image_clip(prototype->frames[frame], x-l-1, y-h);
	
	}

	return 0;
}

void SpriteClass::HandleEffects() {

	if (super_mode_timer || prototype->effect == EFFECT_STARS)
		Effect_Super(x, y, prototype->width, prototype->height);
		
	if (prototype->effect == EFFECT_SMOKE)
		Effect_Points(x - 5, y - 5, prototype->width, prototype->height, prototype->color);

	if (prototype->effect == EFFECT_THUNDER && prototype->charge_time > 0)
		if (rand() % prototype->charge_time == 0) Do_Thunder();

}

bool SpriteClass::CanDamageOnCollision(const SpriteClass* target)const{
	if(target->invisible_timer>0){
		int damage_type = this->prototype->damage_type;
		if(damage_type==DAMAGE_ALL || damage_type==DAMAGE_FIRE){
			return true;
		}
		else if(this->prototype->is_wall && (damage_type==DAMAGE_COMPRESSION||damage_type==DAMAGE_DROP)){
			return true;
		}		
		else{

			return false;
		}
	}

	return true;
}

void SpriteClass::AI_Move_X(double liike){
	if (energy > 0)
		this->x = this->orig_x + liike;
}
void SpriteClass::AI_Move_Y(double liike){
	if (energy > 0)
		this->y = this->orig_y + liike;
}

bool SpriteClass::AI_Teleport(const std::list<SpriteClass*>& spritet, SpriteClass &player){
	bool siirto = false;

	if (energy > 0 &&player.energy>0 && charging_timer == 0 && attack1_timer == 0)
	{
		if (player.x <= x + prototype->width /2 && player.x >= x - prototype->width /2 &&
			player.y <= y + prototype->height/2 && player.y >= y - prototype->height/2 )
		{

			std::vector<SpriteClass*> portit;

			// search for teleports of the same type
			for (SpriteClass* sprite : spritet)
				if (prototype == sprite->prototype && sprite != this)
						portit.push_back(sprite);

			// if it didn't find any, search for all teleports
			if (portit.size() == 0) {
				for (SpriteClass* sprite : spritet)
					if (sprite->prototype->type == TYPE_TELEPORT && sprite != this)
						portit.push_back(sprite);
			}

			// if you don't have any teleports (excluding the teleport itself), return
			if (portit.size() == 0)
				return false;

			// arvotaan kohdeportti
			SpriteClass* dst = portit[rand()%portit.size()];
			
			player.x = dst->x;
			player.y = dst->y;
			//charging_timer    = prototype->charge_time;
			//attack1_timer = prototype->attack1_time;
			//spritet[i].charging_timer    = spritet[i].prototype->charge_time;
			dst->attack1_timer = dst->prototype->attack1_time;
			dst->charging_timer = 0;
			this->charging_timer = 0;

			siirto = true;
		}
	}

	return siirto;
}


void SpriteClass::Animation_Basic(){

	int uusi_animaatio = -1;
	bool alusta = false;

	if (energy < 1 && !can_move_down)
	{
		uusi_animaatio = ANIMATION_DEATH;
		alusta = true;
	}
	else
	{

		if (a > -0.2 && a < 0.2 && b == 0 && jump_timer <= 0)
		{
			uusi_animaatio = ANIMATION_IDLE;
			alusta = true;
		}

		if ((a < -0.2 || a > 0.2) && jump_timer <= 0)
		{
			uusi_animaatio = ANIMATION_WALKING;
			alusta = false;
		}

		if (b < 0)//-0.3
		{
			uusi_animaatio = ANIMATION_JUMP_UP;
			alusta = false;
		}

		if ((jump_timer > prototype->max_jump || b > 1.5) && can_move_down)
		{
			uusi_animaatio = ANIMATION_JUMP_DOWN;
			alusta = false;
		}

		if (crouched)
		{
			uusi_animaatio = ANIMATION_SQUAT;
			alusta = true;
		}

		if (attack1_timer > 0)
		{
			uusi_animaatio = ANIMATION_ATTACK1;
			alusta = true;
		}

		if (attack2_timer > 0)
		{
			uusi_animaatio = ANIMATION_ATTACK2;
			alusta = true;
		}

		if (damage_timer > 0)
		{
			uusi_animaatio = ANIMATION_DAMAGE;
			alusta = false;
		}

	}

	if (uusi_animaatio != -1)
		Animaatio(uusi_animaatio,alusta);

}
void SpriteClass::Animation_Rooster(){

	int uusi_animaatio = -1;
	bool alusta = false;

	if (energy < 1 && !can_move_down) {
	
		uusi_animaatio = ANIMATION_DEATH;
		alusta = true;
	
	} else {

		if (a > -0.2 && a < 0.2 && b == 0 && jump_timer <= 0) {

			uusi_animaatio = ANIMATION_IDLE;
			alusta = true;
		
		}

		if ((a < -0.2 || a > 0.2) && jump_timer <= 0) {

			uusi_animaatio = ANIMATION_WALKING;
			alusta = false;
		
		}

		if (b < 0) {

			uusi_animaatio = ANIMATION_JUMP_UP;
			alusta = false;

		}

		if ((jump_timer > 90+10/*prototype->max_jump || b > 1.5*/) && can_move_down) {
		
			uusi_animaatio = ANIMATION_JUMP_DOWN;
			alusta = false;
		
		}

		if (attack1_timer > 0) {

			uusi_animaatio = ANIMATION_ATTACK1;
			alusta = true;

		}

		if (attack2_timer > 0) {

			uusi_animaatio = ANIMATION_ATTACK2;
			alusta = true;

		}

		if (crouched) {

			uusi_animaatio = ANIMATION_SQUAT;
			alusta = true;

		}

		if (damage_timer > 0) {

			uusi_animaatio = ANIMATION_DAMAGE;
			alusta = false;

		}

	}

	if (uusi_animaatio != -1)
		Animaatio(uusi_animaatio,alusta);
}

void SpriteClass::Animation_Bonus() {
	Animaatio(ANIMATION_IDLE, true);
}
void SpriteClass::Animation_Projectile() {
	Animaatio(ANIMATION_IDLE, true);
}

void SpriteClass::Animation_Egg() {

	int uusi_animaatio = ANIMATION_IDLE;
	bool alusta = true;

	if (energy < prototype->energy)
		uusi_animaatio = ANIMATION_DEATH;
	
	Animaatio(uusi_animaatio, alusta);
}