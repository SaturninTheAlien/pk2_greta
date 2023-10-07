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
				/*PLog::Write(PLog::ERR, "PK2 sprites", "Couldn't find %s", filename_in.c_str());
				return nullptr;*/
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
		PLog::Write(PLog::ERR, "PK2 sprites", e.what());
		if(protot!=nullptr){
			delete protot;
			protot = nullptr;
		}

		std::string s1 = "SpritePrototypes: ";
		s1 += e.what();

		throw PExcept::PException(s1);

		//PLog::Write(PLog::ERR, "PK2 sprites", "Couldn't find %s", filename_in.c_str());
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
	makes_sounds		= proto.makes_sounds;
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
	makes_sounds		= proto.makes_sounds;
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

const std::map<std::string, int> jsonAnimationsMap = {
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

const std::map<std::string, int> jsonSoundsMap ={
    {"damage", SOUND_DAMAGE},
    {"destruction", SOUND_DESTRUCTION},
    {"attack1", SOUND_ATTACK1},
    {"attack2", SOUND_ATTACK2},
    {"random", SOUND_RANDOM},
    {"special1", SOUND_SPECIAL1},
    {"special2", SOUND_SPECIAL2}
};

const std::map<std::string ,u8> jsonColorsMap={
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
		for(std::pair<std::string, int> p: jsonAnimationsMap){
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

	jsonReadEnumU8(j, "color", this->color, jsonColorsMap);

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

	jsonReadBool(j, "makes_sounds", this->makes_sounds);

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
		for(std::pair<std::string, int> p: jsonSoundsMap){
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
	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {	
		throw PExcept::FileNotFoundException(path.c_str(), PExcept::MISSING_SPRITE_PROTOTYPE);
	}

	char versio[4];
	file->read(versio, 4);

	if (strcmp(versio,"1.0") == 0){
		PrototypeClass10 proto;
		file->read(&proto, sizeof(proto));
		this->SetProto10(proto);
	}
	else if (strcmp(versio,"1.1") == 0){
		PrototypeClass11 proto;
		file->read(&proto, sizeof(proto));
		this->SetProto11(proto);
	}
	else if (strcmp(versio,"1.2") == 0){
		PrototypeClass12 proto;
		file->read(&proto, sizeof(proto));
		this->SetProto12(proto);
	}
	else if (strcmp(versio,"1.3") == 0){
		PrototypeClass13 proto;
		file->read(&proto, sizeof(proto));
		this->SetProto13(proto);
	}
	else {
		file->close();
		throw UnsupportedSpriteVersionException(versio);
	}
	this->version = versio;
	file->close();
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

		/*PLog::Write(PLog::ERR, "PK2", "Couldn't find sprite image %s", this->picture_filename.c_str());
		return -1;*/

	}

	int bufferi = PDraw::image_load(image, false);
	if (bufferi == -1) {
		throw PExcept::FileNotFoundException(this->picture_filename, PExcept::MISSING_SPRITE_TEXTURE);
		/*PLog::Write(PLog::ERR, "PK2", "Couldn't load sprite image %s", this->picture_filename.c_str());
		return -1;*/

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

		this->frames[frame_i] = PDraw::image_cut(bufferi,frame_x,frame_y,picture_frame_width,picture_frame_height); //frames
		this->frames_mirror[frame_i] = PDraw::image_cut(bufferi,frame_x,frame_y,picture_frame_width,picture_frame_height); //flipped frames
		PDraw::image_fliphori(this->frames_mirror[frame_i]);

		frame_x += this->picture_frame_width + 3;
	
	}

	PDraw::image_delete(bufferi);

	//Load sounds
	for (int i = 0; i < SPRITE_SOUNDS_NUMBER; i++) {

		if(!this->sound_files[i].empty()){
			PFile::Path sound = path;
			sound.SetFile(this->sound_files[i]);

			if (FindAsset(&sound, "sprites" PE_SEP)) {

				this->sounds[i] = PSound::load_sfx(sound);

			} else {

				PLog::Write(PLog::ERR, "PK2", "Can't find sound %s", this->sound_files[i].c_str());
				//return -1;

			}
		}
	}
	//return 0;
}

int PrototypeClass::Draw(int x, int y, int frame){
	PDraw::image_clip(this->frames[frame], x, y);
	return 0;
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

void SpriteClass::AI_Basic(){
	if (x < 10)
	{
		x = 10;
		vasemmalle = false;
	}

	if (x > 8192)
	{
		x = 8192;
		oikealle = false;
	}

	if (y > 9920)
	{
		y = 9920;
	}

	if (y < -32)
	{
		y = -32;
	}

	if (a < 0)
		flip_x = true;

	if (a > 0)
		flip_x = false;

	action_timer++;

	if (action_timer > 31320) // divisible by 360
		action_timer = 0;

}
void SpriteClass::AI_Turning_Horizontally(){
	if (energy > 0)
	{
		if (!oikealle)
		{
			a = this->prototype->max_speed / -3.5;
		}

		if (!vasemmalle)
		{
			a = this->prototype->max_speed / 3.5;
		}
	}
}
void SpriteClass::AI_Turning_Vertically(){
	if (energy > 0)
	{
		if (!alas)
		{
			b = this->prototype->max_speed / -3.5;
		}

		if (!ylos)
		{
			b = this->prototype->max_speed / 3.5;
		}
	}
}
void SpriteClass::AI_Climber(){
	if (energy > 0)
	{
		if (!alas && vasemmalle)
		{
			b = 0;
			a = this->prototype->max_speed / -3.5;
			//return 1;
		}

		if (!ylos && oikealle)
		{
			b = 0;
			a = this->prototype->max_speed / 3.5;
			//b = this->prototype->max_speed / 3.5;
			//return 1;
		}

		if (!oikealle && alas)
		{
			a = 0;
			b = this->prototype->max_speed / 3.5;
			//return 1;
		}

		if (!vasemmalle && ylos)
		{
			a = 0;
			b = this->prototype->max_speed / -3.5;
			//return 1;
		}
	}
}
void SpriteClass::AI_Climber2(){
	if (energy > 0){
		if (vasemmalle && oikealle && ylos && alas) {

			if (a < 0) {
				b = this->prototype->max_speed / 3.5;
				//a = 0;
			}
			else if (a > 0) {
				b = this->prototype->max_speed / -3.5;
				//a = 0;
			}
			else if (b < 0) {
				a = this->prototype->max_speed / -3.5;
				//b = 0;
			}
			else if (b > 0) {
				a = this->prototype->max_speed / 3.5;
				//b = 0;
			}
			if (b != 0)
				a = 0;
		}
	}
}
void SpriteClass::AI_Look_For_Cliffs(){
	double max = prototype->max_speed / 3.5;

	if (energy > 0)
	{

		if (this->reuna_oikealla && this->a > -max)
		{
			this->a -= 0.13;
		}

		if (this->reuna_vasemmalla && this->a < max)
		{
			this->a += 0.13;
		}

		/*
		if (this->reuna_oikealla && this->a > 0)
		{
			this->a = this->a * -1;
			flip_x = true;
		}

		if (this->reuna_vasemmalla && this->a < 0)
		{
			this->a = this->a * -1;
			flip_x = false;
		}
	*/
	}
}
void SpriteClass::AI_Random_Jump(){
	if (energy > 0)
	{
		if (rand()%150 == 10 && b == 0 && jump_timer == 0 && ylos)
		{
			jump_timer = 1;
		}
	}
}
void SpriteClass::AI_BlueFrog(){
	if (energy > 0)
	{
		if (action_timer%100 == 0 && jump_timer == 0 && ylos)
		{
			jump_timer = 1;
		}
	}
}
void SpriteClass::AI_RedFrog(){
	if (energy > 0)
	{
		if (action_timer%100 == 0 && ylos)
		{
			jump_timer = 1;

		}

		if (jump_timer > 0)
		{
			if (!flip_x)
				a = this->prototype->max_speed / 3.5;
			else
				a = this->prototype->max_speed / -3.5;
		}
	}
}
void SpriteClass::AI_Random_Change_Dir_H(){
	if (energy > 0)
	{
		if (rand()%150 == 1)
		{
			int max = (int)prototype->max_speed / 4;

			while (a == 0 && max > 0)
				a = rand()%max+1 - rand()%max+1;
		}
	}
}
void SpriteClass::AI_Random_Turning(){
	if (energy > 0)
	{
		if (action_timer%400 == 1 && a == 0)
		{
			flip_x = !flip_x;
		}
	}
}
void SpriteClass::AI_Turn_Back_If_Demaged() {

	if (damage_timer == 1 && energy > 0) {
		
		if (a != 0) a = -a;

		flip_x = !flip_x;
	
	}
}
void SpriteClass::AI_Random_Move_Vert_Hori(){
	if (energy > 0)
	{
		if (rand()%150 == 1 || action_timer == 1)
		if ((int)a == 0 || (int)b == 0)
		{
			int max = (int)prototype->max_speed;

			if (max != 0)
			{
				while (a == 0)
					a = rand()%(max+1) - rand()%(max+1);

				while (b == 0)
					b = rand()%(max+1) - rand()%(max+1);

				//a /= 3.0;
				//b /= 3.0;
			}

		}
	}
}
void SpriteClass::AI_Follow_Player(SpriteClass &player){
	if (energy > 0 && player.energy > 0)
	{
		double max = prototype->max_speed / 3.5;

		if (a > -max && x > player.x)
		{
			a -= 0.1;
		}

		if (a < max && x < player.x)
		{
			a += 0.1;
		}

		seen_player_x = (int)(player.x+player.a);
		seen_player_y = (int)(player.y+player.b);

		if (prototype->max_speed == 0)
		{
			if (player.x < x)
				flip_x = true;
			else
				flip_x = false;
		}
	}
}
void SpriteClass::AI_Run_Away_From_Player(SpriteClass &player){
	if (energy > 0 && player.energy > 0)
	{
		if ((player.x < x && flip_x && !player.flip_x) || (player.x > x && !flip_x && player.flip_x))
			if ((player.x - x < 300 && player.x - x > -300) &&
				(player.y - y < prototype->height && player.y - y > -prototype->height))
			{
				double max = prototype->max_speed / 2.5;

				if (x > player.x) {
					a = max;
					flip_x = false;
				}

				if (x < player.x) {
					a = max * -1;
					flip_x = true;
				}
			}
	}
}


void SpriteClass::AI_Follow_Player_If_Seen(SpriteClass &player){
	if (energy > 0  && player.energy > 0){
		double max = prototype->max_speed / 3.5;

		if (seen_player_x != -1){
			if (a > -max && x > seen_player_x)
				a -= 0.1;

			if (a < max && x < seen_player_x)
				a += 0.1;
		}

		if ((player.x < x && flip_x) || (player.x > x && ! flip_x)){
			if ((player.x - x < 300 && player.x - x > -300) &&
				(player.y - y < prototype->height && player.y - y > -prototype->height)){
				seen_player_x = (int)(player.x+player.a);
				seen_player_y = (int)(player.y+player.b);
			}
			else{
				seen_player_x = -1;
				seen_player_y = -1;
			}
		}
	}
}
/*
int SpriteClass::AI_Jahtaa_Pelaajaa(SpriteClass &player){
	if (energy > 0 && player.energy > 0)
	{
		double max = prototype->max_speed / 3.5;

		if (seen_player_x != -1)
		{
			if (a > -max && x > seen_player_x)
				a -= 0.1;

			if (a < max && x < seen_player_x)
				a += 0.1;

			if (x - seen_player_x > -8 && x - seen_player_x < 8) // onko sprite saavuttanut pelaajan viime sijainnin
				seen_player_x = -1;
		}

		if (seen_player_y != -1 && weight == 0)
		{
			if (b > -max && y > seen_player_y)
				b -= 0.1;

			if (b < max && y < seen_player_y)
				b += 0.1;

			if (y - seen_player_y > -8 && y - seen_player_y < 8) // onko sprite saavuttanut pelaajan viime sijainnin
				seen_player_y = -1;
		}

		if (((player.x < x && !flip_x) || (player.x > x && flip_x)) &&
			player.attack2_timer != player.prototype->charge_time)
			return 0;

		if ((player.x - x < 300 && player.x - x > -300) &&
			(player.y - y < prototype->height && player.y - y > -prototype->height))
		{
			seen_player_x = (int)(player.x+player.a);
			seen_player_y = (int)(player.y+player.b);
		}
	}


	return 0;
}*/
void SpriteClass::AI_Follow_Player_Vert_Hori(SpriteClass &player){
	if (energy > 0 && player.energy > 0)
	{
		double max = prototype->max_speed / 3.5;

		if (a > -max && x > player.x)
		{
			a -= 0.1;
		}

		if (a < max && x < player.x)
		{
			a += 0.1;
		}

		if (b > -max && y > player.y)
		{
			b -= 0.4;
		}

		if (b < max && y < player.y)
		{
			b += 0.4;
		}

		seen_player_x = (int)(player.x+player.a);
		seen_player_y = (int)(player.y+player.b);

		if (prototype->max_speed == 0)
		{
			if (player.x < x)
				flip_x = true;
			else
				flip_x = false;
		}
	}
}
void SpriteClass::AI_Follow_Player_If_Seen_Vert_Hori(SpriteClass &player){
	if (energy > 0  && player.energy > 0){
		double max = prototype->max_speed / 3.5;

		if (seen_player_x != -1){
			if (a > -max && x > seen_player_x)
				a -= 0.1;

			if (a < max && x < seen_player_x)
				a += 0.1;

			if (b > -max && y > seen_player_y)
				b -= 0.4;

			if (b < max && y < seen_player_y)
				b += 0.4;
		}

		if ((player.x < x && flip_x) || (player.x > x && ! flip_x)){
			if ((player.x - x < 300 && player.x - x > -300) &&
				(player.y - y < 80 && player.y - y > -80)){
				seen_player_x = (int)(player.x+player.a);
				seen_player_y = (int)(player.y+player.b);
			}
			else{
				seen_player_x = -1;
				seen_player_y = -1;
			}
		}
	}
}
void SpriteClass::AI_Transform_When_Energy_Under_2(){

	PrototypeClass* transformation = this->prototype->transformation;

	if (transformation!=nullptr&& energy < 2 && transformation != prototype) {
		prototype = transformation;
		initial_weight = prototype->weight;
	}
}
void SpriteClass::AI_Transform_When_Energy_Over_1(){

	PrototypeClass* transformation = this->prototype->transformation;
	if (transformation!=nullptr && energy > 1 && transformation != prototype) {
		prototype = transformation;
		initial_weight = prototype->weight;
		Effect_Destruction(FX_DESTRUCT_SAVU_HARMAA, (u32)this->x, (u32)this->y);
		//return true;
	}

	//return false;
}
void SpriteClass::AI_Self_Transformation(){
	PrototypeClass* transformation = this->prototype->transformation;
	if (transformation!=nullptr && energy > 0 && transformation != prototype)
	{
		if (mutation_timer/*charging_timer*/ == 0)
			mutation_timer/*charging_timer*/ = prototype->charge_time;

		if (mutation_timer/*charging_timer*/ == 1)
		{
			prototype = transformation;
			initial_weight = prototype->weight;

			ammo1 = prototype->ammo1;
			ammo2 = prototype->ammo2;

			animation_index = -1;

			Animaatio(ANIMATION_IDLE,true);
		}
	}
}
void SpriteClass::AI_Transform_If_Demaged(){
	PrototypeClass* transformation = this->prototype->transformation;
	if (transformation!=nullptr && energy > 0 && transformation != prototype)
	{
		if (saatu_vahinko > 0)
		{
			prototype = transformation;
			initial_weight = prototype->weight;

			ammo1 = prototype->ammo1;
			ammo2 = prototype->ammo2;

			animation_index = -1;

			Animaatio(ANIMATION_IDLE,true);
		}
	}
}
void SpriteClass::AI_Die_If_Parent_Nullptr(){
	if (parent_sprite != nullptr)
	{
		if (parent_sprite->energy < 1 && energy > 0)
		{
			saatu_vahinko = energy;
			saatu_vahinko_tyyppi = DAMAGE_ALL;
		}
	}
}
void SpriteClass::AI_Attack_1_If_Demaged(){
	if (saatu_vahinko > 0 && energy > 0)
	{
		this->attack1_timer = this->prototype->attack1_time;
		this->charging_timer = 0;
	}
}
void SpriteClass::AI_Attack_2_If_Demaged(){
	if (saatu_vahinko > 0 && energy > 0)
	{
		this->attack2_timer = this->prototype->attack2_time;
		this->charging_timer = 0;
	}
}
void SpriteClass::AI_Attack_1_Nonstop(){
	if (this->charging_timer == 0 && energy > 0)
	{
		this->attack1_timer = this->prototype->attack1_time;
	}
}
void SpriteClass::AI_Attack_2_Nonstop(){
	if (this->charging_timer == 0 && energy > 0)
	{
		this->attack2_timer = this->prototype->attack2_time;
	}
}
void SpriteClass::AI_Attack_1_if_Player_in_Front(SpriteClass &player){
	if (energy > 0 && damage_timer == 0 && player.energy > 0)
	{
		if ((player.x - x < 200 && player.x - x > -200) &&
			(player.y - y < prototype->height && player.y - y > -prototype->height))
		{
			if ((player.x < x && flip_x) || (player.x > x && !flip_x))
			{
				this->attack1_timer = this->prototype->attack1_time;
			}
		}
	}
}
void SpriteClass::AI_Attack_2_if_Player_in_Front(SpriteClass &player){
	if (energy > 0 && damage_timer == 0 && player.energy > 0)
	{
		if ((player.x - x < 200 && player.x - x > -200) &&
			(player.y - y < prototype->height && player.y - y > -prototype->height))
		{
			if ((player.x < x && flip_x) || (player.x > x && !flip_x))
			{
				this->attack2_timer = this->prototype->attack2_time;
			}
		}
	}
}
void SpriteClass::AI_Attack_1_if_Player_Bellow(SpriteClass &player){
	if (energy > 0 && damage_timer == 0 && player.energy > 0)
	{
		if ((player.x - x < prototype->width && player.x - x > -prototype->width) &&
			(player.y > y && player.y - y < 350))
		{
			this->attack1_timer = this->prototype->attack2_time;
		}
	}
}
/**
 * @brief 
 * 
 * "Greta Engine", new AI
 */
void SpriteClass::AI_Attack_1_If_Player_Above(SpriteClass& player){
	if (energy > 0 && damage_timer == 0 && player.energy > 0)
	{
		if ((player.x - x < prototype->width && player.x - x > -prototype->width) &&
			(player.y < y && y - player.y < 350))
		{
			this->attack1_timer = this->prototype->attack1_time;
		}
	}
}

/**
 * @brief 
 * 
 * "Greta Engine", new AI
 */
void SpriteClass::AI_Attack_2_If_Player_Above(SpriteClass& player){
	if (energy > 0 && damage_timer == 0 && player.energy > 0)
	{
		if ((player.x - x < prototype->width && player.x - x > -prototype->width) &&
			(player.y < y && y - player.y < 350))
		{
			this->attack2_timer = this->prototype->attack2_time;
		}
	}
}

/**
 * @brief 
 * 
 * "Greta Engine", new AI
 */
void SpriteClass::AI_Transform_If_Player_Above(SpriteClass& player){
	if(energy > 0 && player.energy>0){
		if ((player.x - x < prototype->width && player.x - x > -prototype->width) &&
			(player.y < y && y - player.y < 350))
		{
			this->AI_Self_Transformation();
		}
	}
}

/**
 * @brief 
 * 
 * "Greta Engine", new AI
 */
void SpriteClass::AI_Transform_If_Player_Bellow(SpriteClass& player){
	if(energy > 0 && player.energy>0){
		if ((player.x - x < prototype->width && player.x - x > -prototype->width) &&
			(player.y > y && player.y - y < 350))
		{
			this->AI_Self_Transformation();
		}
	}
}
///////


void SpriteClass::AI_Jump_If_Player_Above(SpriteClass &player){
	if (energy > 0 && jump_timer == 0 && player.energy > 0)
	{
		if ((player.x - x < prototype->width && player.x - x > -prototype->width) &&
			(player.y < y && y - player.y < 350))
		{
			jump_timer = 1;
		}
	}
}
void SpriteClass::AI_NonStop(){
	if (energy > 0)
	{

		double max = prototype->max_speed / 3.5;

		if (flip_x)
		{
			if (a > -max)
				a -= 0.1;
		}
		else
		{
			if (a < max)
				a += 0.1;
		}
	}
}
void SpriteClass::AI_Friction_Effect(){

	if (energy > 0)
	{
		if (!alas)
			a /= 1.07;
		else
			a /= 1.02;
	}
}
void SpriteClass::AI_Hiding(){

	if (energy > 0 && hidden)
	{
		a /= 1.02;
		crouched = true;
	}
}
void SpriteClass::AI_Return_To_Orig_X(){

	if (energy < 1 || seen_player_x !=  -1)
		return;

	double max = prototype->max_speed / 3.5;

	if (x < orig_x-16 && a < max)
		a += 0.05;

	if (x > orig_x+16 && a > -max)
		a -= 0.05;
}
void SpriteClass::AI_Return_To_Orig_Y(){

	if (energy > 0 && seen_player_x == -1)
	{
		double max = prototype->max_speed / 3.5;

		if (y < orig_y-16 && b < max)
			b += 0.04;

		if (y > orig_y+16 && b > -max)
			b -= 0.04;
	}
}
void SpriteClass::AI_Fall_When_Shaken(int tarina){

	if (energy > 0 && tarina > 0)
	{
		initial_weight = 0.5;
	}
}
void SpriteClass::AI_Damaged_by_Water(){
	if (energy > 0)
		if (this->in_water)
			saatu_vahinko++;
}
void SpriteClass::AI_Kill_Everyone(){
	if (energy > 0)
		this->enemy = !this->enemy;
}
void SpriteClass::AI_Jumper(){
	if (x < 10)
	{
		x = 10;
	}

	if (y > 9920)
	{
		y = 9920;
	}

	if (energy > 0)
	{
		if (!alas && b==0 && jump_timer == 0)
		{
			jump_timer = 1;
		}
	}

	if (a < 0)
		flip_x = true;

	if (a > 0)
		flip_x = false;

}
void SpriteClass::AI_Move_X(double liike){
	if (energy > 0)
		this->x = this->orig_x + liike;
}
void SpriteClass::AI_Move_Y(double liike){
	if (energy > 0)
		this->y = this->orig_y + liike;
}
void SpriteClass::AI_Tippuu_If_Switch_Pressed(int kytkin){
	if (kytkin > 0)
	{
		initial_weight = 1.5;
	}
}
void SpriteClass::AI_Move_If_Switch_Pressed(int kytkin, int ak, int bk){
	if (kytkin > 0)
	{
		if (a == 0 && ak != 0)
		{
			a = prototype->max_speed / 3.5 * ak; // ak = -1 / 1
		}

		if (b == 0 && bk != 0)
			b = prototype->max_speed / 3.5 * bk; // bk = -1 / 1
	}

	flip_x = false;
}
bool SpriteClass::AI_Info(SpriteClass &player){
	if ((player.x - x < 10 && player.x - x > -10) &&
		(player.y - y < prototype->height && player.y - y > -prototype->height))
	{
		return true;
	}

	return false;
}
void SpriteClass::AI_Rooster(){
	if (x < 10)
	{
		x = 10;
	}

	if (y > 9920)
	{
		y = 9920;
	}

	if (energy > 0)
	{

		if (rand()%50 == 10 && a != 0)
			a /= 1.1;

		if (rand()%150 == 10 && b == 0 && jump_timer == 0 && ylos)
		{
			jump_timer = 1;
			while (a == 0)
				a = rand()%2 - rand()%2;
		}

		if (rand()%20 == 1 && b == 0 && jump_timer == 0 && !oikealle && !flip_x)
		{
			jump_timer = 1;
			while (a == 0)
				a = rand()%2;
		}

		if (rand()%20 == 1 && b == 0 && jump_timer == 0 && !vasemmalle && flip_x)
		{
			jump_timer = 1;
			while (a == 0)
				a = rand()%2 * -1;
		}

		if (rand()%200 == 10)
			a = rand()%2 - rand()%2;

		if (jump_timer == prototype->max_jump && a == 0)
		{
			while (a == 0)
				a = rand()%2 - rand()%2;
		}
		/* // reunatesti
		if (rand()%100 == 2)
			a = rand()%2-rand()%2;

		if (reuna_vasemmalla && a < 0)
			a = 0;

		if (reuna_oikealla && a > 0)
			a = 0;*/

		if (a < 0)
			flip_x = true;

		if (a > 0)
			flip_x = false;

	}
}
void SpriteClass::AI_Bonus(){
	if (x < 10)
	{
		x = 10;
	}

	if (y > 9920)
	{
		y = 9920;
	}
}
void SpriteClass::AI_Egg(){
	if (x < 10)
	{
		x = 10;
	}

	if (y > 9920)
	{
		y = 9920;
	}

	if (!alas)
		energy = 0;

	//a /= 1.01;

	if (energy == 0 && charging_timer == 0)
		charging_timer = prototype->charge_time;

	if (this->charging_timer == 1)
		this->removed = true;

}
void SpriteClass::AI_Egg2(){
	if (x < 10)
	{
		x = 10;
	}

	if (y > 9920)
	{
		y = 9920;
	}

	if (!alas)
		saatu_vahinko = prototype->energy;

	//a /= 1.01;

	if (energy == 0 && charging_timer == 0)
		charging_timer = prototype->charge_time;

	if (charging_timer == 1)
		removed = true;

}
void SpriteClass::AI_Projectile(){
	if (x < 10)
	{
		x = 10;
	}

	if (y > 9920)
	{
		y = 9920;
	}

	if (a < 0)
		flip_x = true;

	if (a > 0)
		flip_x = false;

	if (this->charging_timer == 0)
		charging_timer = this->prototype->charge_time;

	if (this->charging_timer == 1)
	{
		this->saatu_vahinko = this->prototype->energy;
		this->saatu_vahinko_tyyppi = DAMAGE_ALL;
	}

	if (energy < 1)
		removed = true;


}
void SpriteClass::AI_SelfDestruction(){
	if (this->charging_timer == 0)
		charging_timer = this->prototype->charge_time;

	if (this->charging_timer == 1)
	{
		this->saatu_vahinko = this->energy;
		this->saatu_vahinko_tyyppi = DAMAGE_ALL;
	}
}
bool SpriteClass::AI_Teleport(const std::list<SpriteClass*>& spritet, SpriteClass &player){
	bool siirto = false;

	if (energy > 0 && charging_timer == 0 && attack1_timer == 0)
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

void SpriteClass::AI_Destructed_Next_To_Player(SpriteClass &player) {

	double dx = this->x - player.x;
	double dy = this->y - player.y;

	int dist = this->prototype->energy * 32;

	if (this->energy > 0 && dx*dx + dy*dy < dist*dist) {

		this->saatu_vahinko = this->prototype->energy;
		this->saatu_vahinko_tyyppi = DAMAGE_ALL;
		
	}
}

void SpriteClass::Animation_Basic(){

	int uusi_animaatio = -1;
	bool alusta = false;

	if (energy < 1 && !alas)
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

		if ((jump_timer > prototype->max_jump || b > 1.5) && alas)
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

	if (energy < 1 && !alas) {
	
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

		if ((jump_timer > 90+10/*prototype->max_jump || b > 1.5*/) && alas) {
		
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

void SpriteClass::AI_Follow_Commands() {
	if(this->energy<=0 || this->prototype->commands.size()==0)return;

	if(this->current_command >= this->prototype->commands.size()){
		this->current_command = 0;
	}

	SpriteCommands::Command* c = this->prototype->commands[this->current_command];
	if(c->execute(this)){
		// next command
		this->current_command++;
	}
}