//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include <sstream>
#include <algorithm>
#include <filesystem>
#include "engine/PFile.hpp"

#include "engine/PLog.hpp"
#include "engine/PUtils.hpp"
#include "engine/platform.hpp"

#include <cstring>
#include <sys/stat.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

#include <SDL.h>

#ifdef PK2_USE_ZIP
#include <zip.h>
#endif

#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif
namespace PFile {

#ifdef PK2_USE_ZIP
struct Zip {
	std::string name;
	zip_t *zip;
	zip_source* src;
	

};
void CloseZip(Zip* zp) {
	if (zp) {
	
		//zip_close(zp->zip);
		zip_source_close(zp->src);
		zip_discard(zp->zip);	
		delete zp;
		
	}
}

Zip* OpenZip(std::string path) {
	
	SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "r");
	if (rw == NULL) {

        PLog::Write(PLog::ERR, "PFile", "Can't open %s", path.c_str());
		return nullptr;

    }

	int size = SDL_RWsize(rw);

	void* buffer = malloc(size);
	SDL_RWread(rw, buffer, size, 1);
	SDL_RWclose(rw);

	zip_error err;
	zip_source_t* src = zip_source_buffer_create(buffer, size, 1, &err);
    
    zip_t* zip = zip_open_from_source(src, ZIP_RDONLY, &err);
	

	std::string name = path.substr(path.find_last_of(PE_SEP) + 1);

	Zip* ret = new Zip;
	ret->name = name;
	
	ret->src = src;
	ret->zip = zip;

    return ret;

	return nullptr;
}

#endif

void Path::FixSep() {

	const char* nosep = PE_NOSEP;
	const char* sep = PE_SEP;
	std::replace(this->path.begin(), this->path.end(), nosep[0], sep[0]);

}

Path::Path(std::string path) {

	path = path.substr(0, path.find_last_not_of(" ") + 1);
    this->path = path;
	
	this->zip_file = nullptr;

	this->FixSep();

}

Path::Path(Zip* zip_file, std::string path) {

	if(zip_file == nullptr) {

		PLog::Write(PLog::ERR, "PFile", "No zip file for %s\n", path.c_str());

	}

	path = path.substr(0, path.find_last_not_of(" ") + 1);
    this->path = path;
	
    this->zip_file = zip_file;

	this->FixSep();

}

Path::Path(Path path, std::string file) {

	*this = path;
	
	file = file.substr(0, file.find_last_not_of(" ") + 1);
	this->path += file;
	
	this->FixSep();

}

Path::~Path() {

}

bool Path::operator==(Path path) { //needed?

	bool a = this->zip_file == path.zip_file; //compare with zip_file name?
	bool b = this->path == path.path;

	return a && b;

}

const char* Get_Extension(const char* string) {

	int len = strlen(string);
	const char* end = string + len;
	
	for( int i = 0; i < len; i++ ) {

		if (*(end - i) == '.' 
			|| *(end - i) == '/'
			|| *(end - i) == '\\') {

			return end - i;

		}

	}

	return string;

}

bool is_type(const char* file, const char* type) {

    if(type[0] == '\0') {

        return true;

    } else if(type[0] == '/' && strstr(file, ".") == NULL) {

        return true;

    } else {

        const char* ext = Get_Extension(file);
        if(strcmp(ext, type) == 0) {

            return true;

        }
    }

    return false;

}

#ifdef PK2_USE_ZIP

static int zip_get_index(zip_t* z, const char* filename, int* size) {

	struct zip_stat st;
    zip_stat_init(&st);

	int sz = zip_get_num_entries(z, 0);
    for (int i = 0; i < sz; i++) {

		zip_stat_index(z, i, 0, &st);

		if ( PUtils::NoCaseCompare(st.name, filename) ) {
			
			*size = st.size;
			return i;

		}

	}

	return -1;

}

static bool pathcomp(const char* path, const char* entry) {

	while(*path != '\0') {

		char a = *path;
		char b = *entry;

		if (a == '\\') a = '/';
		if (b == '\\') b = '/';

		if ( (a | ' ') != (b | ' ') )
			return false;
		
		path++;
		entry++;

	}

	return true;

}

std::vector<std::string> scan_zip(Zip* zip_file, const char* path, const char* type) {

    std::vector<std::string> result;

    int path_size = strlen(path);

    struct zip_stat st;
    zip_stat_init(&st);
    
    int sz = zip_get_num_entries(zip_file->zip, 0);
    for (int i = 0; i < sz; i++) {
        
        zip_stat_index(zip_file->zip, i, 0, &st);

		if( pathcomp(path, st.name) ) {

			std::string filename(st.name + path_size + 1);
			filename = filename.substr(0, filename.find("/")); //PE_SEP?

			if(filename.size() == 0)
				continue;
			
            if(is_type(filename.c_str(), type)) {

				//needed?
				bool repeated = false;
				for (std::string st : result)
					if (st == filename) {
						repeated = true;
						break;
					}
				
				if (!repeated)
                	result.push_back(filename);

            }

        }

    }

    PLog::Write(PLog::DEBUG, "PFile", "Scanned zip \"%s\" on \"%s\" for \"%s\". Found %i matches", zip_file->name.c_str(), path, type, (int)result.size());
    return result;

}

#endif

//Scans directory to find file based on case
bool Path::NoCaseFind() {

	std::string filename = this->GetFileName();
	this->SetFile("");

	std::vector<std::string> list = this->scandir("");

	int sz = list.size();
	for(int i = 0; i < sz; i++) {
		
		std::string name = list[i];
		
		if(PUtils::NoCaseCompare(name.c_str(), filename.c_str())) {

			this->SetFile(name);
			PLog::Write(PLog::DEBUG, "PFile", "Found on %s", this->path.c_str());

			return true;
		}

	}

	this->SetFile(filename);
	PLog::Write(PLog::INFO, "PFile", "%s not found", this->path.c_str());

	return false;

}

#ifdef __ANDROID__

std::vector<std::string> scan_apk(const char* dir, const char* type) {

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));
	jmethodID method_id = env->GetMethodID(clazz, "listDir", "(Ljava/lang/String;)[Ljava/lang/String;");

	jstring param = env->NewStringUTF(dir);
	jobjectArray array = (jobjectArray)env->CallObjectMethod(activity, method_id, param);

	jsize size = env->GetArrayLength(array);
	
	std::vector<std::string> result;

	for (int i = 0; i < size; i++) {

		jstring filename = (jstring)env->GetObjectArrayElement(array, i);
		jboolean isCopy;
		const char* file = env->GetStringUTFChars(filename, &isCopy);

		if( file[0] != '.' ) {

			if(type[0] == '/' && strstr(file, ".") == NULL) { //provisory way - consider file without '.' a directory

				result.push_back(file);
				continue;

			} else if(type[0] == '\0') {
			
				result.push_back(file);
				continue;
			
			} else {

				const char* ext = Get_Extension(file);
				if(strcmp(ext, type) == 0) {

					result.push_back(file);
					continue;

				}
			}
		}

		if (isCopy == JNI_TRUE) {
    		env->ReleaseStringUTFChars(filename, file);
		}
	}


	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

    PLog::Write(PLog::DEBUG, "PFile", "Scanned APK on \"%s\" for \"%s\". Found %i matches", dir, type, (int)result.size());

	return result;

}

#endif

//#define PK2_OLD_FILESYSTEM
/*
bool Path::Find() {

	// Scan dir on ZIP
	if (this->zip_file != nullptr)
		return this->NoCaseFind();
	
	#ifdef __ANDROID__

	// Scan dir on APK
	if (c_str()[0] != '/')
		return this->NoCaseFind();

	#endif

	const char* cstr = this->path.c_str();

	PLog::Write(PLog::DEBUG, "PFile", "Find %s", cstr);

	struct stat buffer;
	if(stat(cstr, &buffer) == 0) {

		PLog::Write(PLog::DEBUG, "PFile", "Found on %s", cstr);
		return true;

	}

	PLog::Write(PLog::INFO, "PFile", "%s not found, trying different cAsE", cstr);
	
	return this->NoCaseFind();
	
}*/




namespace fs = std::filesystem;
std::vector<std::string> scan_file(const char* dir, const char* type){
	
	std::vector<std::string> result;
	for (const auto & entry : fs::directory_iterator(dir)){
		std::string filename = entry.path().filename().string();

		//std::cout<<filename<<std::endl;
		if(filename[0]!='.'){

			if(type[0] == '/' && entry.is_directory()) {

				result.push_back(filename);
				continue;

			} else if(type[0] == '\0') {
			
				result.push_back(filename);
				continue;
			
			} else {

				const char* ext = Get_Extension(filename.c_str());
				if(strcmp(ext, type) == 0) {

					result.push_back(filename);
					continue;

				}

			}

		}
	}
	std::sort(result.begin(), result.end());
	PLog::Write(PLog::DEBUG, "PFile", "Scanned on \"%s\" for \"%s\". Found %i matches", dir, type, (int)result.size());

	return result;
}

bool Path::Find() {

	// Scan dir on ZIP
	if (this->zip_file != nullptr)
		return this->NoCaseFind();
	
	#ifdef __ANDROID__

	if (this->path[0] != '/')
		return this->NoCaseFind();

	#endif

	const char* cstr = this->path.c_str();

	PLog::Write(PLog::DEBUG, "PFile", "Find %s", cstr);
	if(fs::exists(this->path)&&fs::is_regular_file(this->path)){
		PLog::Write(PLog::DEBUG, "PFile", "Found on %s", cstr);
		return true;
	}

	#ifdef _WIN32
		return false;
	#else
		PLog::Write(PLog::INFO, "PFile", "%s not found, trying different cAsE", cstr);
		return this->NoCaseFind();
	#endif
}


bool Path::Is_Zip() {

	return this->zip_file != nullptr;

}

int Path::SetFile(std::string file) {

	int dif = this->path.find_last_of(PE_SEP);

	file = file.substr(0, file.find_last_not_of(" ") + 1);

	this->path = this->path.substr(0, dif + 1) + file;

	return 0;

}

int Path::SetPath(std::string path) {

	int s = path.size();
	if (s > 0)
		if (path[s-1] != PE_SEP[0] && path[s-1] != PE_NOSEP[0])
			path += PE_SEP;

	this->path = path + this->GetFileName();
	this->FixSep();

	return 0;

}

std::string Path::GetDirectory() {

	int dif = this->path.find_last_of(PE_SEP);
	return this->path.substr(0, dif);

}

std::string Path::GetFileName() {

	int dif = this->path.find_last_of(PE_SEP);
	return this->path.substr(dif + 1);

}

#ifdef PK2_USE_ZIP

zip_file_t* mOpenZipFile(Zip* zip_file, const char*filename, int&size){
	int index = zip_get_index(zip_file->zip, filename, &size);
	if (index < 0 || size<=0) {

		std::ostringstream os;
		os<<"Can't get RW from zip \""<<zip_file->name<<
		"\", file \""<<filename<<"\"";

		throw PFileException(os.str());
	}

	zip_file_t* zfile = zip_fopen_index(zip_file->zip, index, 0);
	if (!zfile) {

		std::ostringstream os;
		os<<"RW from zip \""<<zip_file->name<<"\", file \""<<
		filename<<"\" is NULL";


		throw PFileException(os.str());

	}

	return zfile;
}

#endif

RW Path::GetRW2(const char* mode)const {

	SDL_RWops* ret;
	if (this->zip_file != nullptr) {

		#ifdef PK2_USE_ZIP
		
		int size = 0;
		zip_file_t* zfile = mOpenZipFile(this->zip_file, this->path.c_str(), size);
		void* buffer = SDL_malloc(size);
		zip_fread(zfile, buffer, size);
		zip_fclose(zfile);

		ret = SDL_RWFromConstMem(buffer, size);
		return RW(ret, buffer);

		#else

		throw PFileException("Zip is not supported in this PK2 version!");
		
		#endif
		
	}
	else{
		ret = SDL_RWFromFile(this->path.c_str(), mode);
		if (!ret) {

			std::ostringstream os;
			os<<"Can't get RW from file\""<<this->path<<"\"";
			std::string s = os.str();
			//PLog::Write(PLog::ERR, "PFile", s.c_str());
			throw PFileException(s);
		}

		return RW(ret, nullptr);
	}
}

nlohmann::json Path::GetJSON()const{
	
	if(this->zip_file!=nullptr){

		#ifdef PK2_USE_ZIP

		int size = 0;
		zip_file_t* zfile = mOpenZipFile(this->zip_file, this->path.c_str(), size);
		char* buffer = new char[size+1];
		buffer[size] = '\0';
		
		zip_fread(zfile, buffer, size);
		zip_fclose(zfile);

		nlohmann::json res = nlohmann::json::parse(buffer);
		delete[] buffer;
		return res;

		#else

		throw PFileException("Zip is not supported in this PK2 version!");
		
		#endif

	}else{
		std::ifstream in(this->path.c_str());
		nlohmann::json res = nlohmann::json::parse(in);
		return res;
	}
}

RW::RW(RW&& source){
	this->_rwops = source._rwops;
	this->_mem_buffer = source._mem_buffer;

	source._rwops = nullptr;
	source._mem_buffer = nullptr;
}

int RW::read(std::string& str) {

	str.clear();

	while(1) {
		u8 c = SDL_ReadU8((SDL_RWops*)(this->_rwops));

		if (c == '\0')
			return str.size();
		
		str += c;
	
	}

}

int RW::read(void* val, size_t size) {

	return SDL_RWread((SDL_RWops*)(this->_rwops), val, 1, size);

}
int RW::read(bool& val) {

	u8 v = SDL_ReadU8((SDL_RWops*)(this->_rwops));
	
	if (v == 0) val = false;
	else val = true;

	return 1;

}
int RW::read(u8& val) {

	val = SDL_ReadU8((SDL_RWops*)(this->_rwops));
	return 1;

}
int RW::read(s8& val) {

	val = SDL_ReadU8((SDL_RWops*)(this->_rwops));
	return 1;

}
int RW::read(u16& val) {

	val = SDL_ReadLE16((SDL_RWops*)(this->_rwops));
	return 1;

}
int RW::read(s16& val) {

	val = SDL_ReadLE16((SDL_RWops*)(this->_rwops));
	return 1;

}
int RW::read(u32& val) {

	val = SDL_ReadLE32((SDL_RWops*)(this->_rwops));
	return 1;

}
int RW::read(s32& val) {

	val = SDL_ReadLE32((SDL_RWops*)(this->_rwops));
	return 1;

}
int RW::read(u64& val) {

	val = SDL_ReadLE64((SDL_RWops*)(this->_rwops));
	return 1;

}
int RW::read(s64& val) {

	val = SDL_ReadLE64((SDL_RWops*)(this->_rwops));
	return 1;

}

int RW::write(std::string& str) {

	return SDL_RWwrite((SDL_RWops*)(this->_rwops), str.c_str(), 1, str.size() + 1);

}

int RW::write(const void* val, size_t size) {

	return SDL_RWwrite((SDL_RWops*)(this->_rwops), val, size, 1);

}
int RW::write(bool val) {

	return SDL_WriteU8((SDL_RWops*)(this->_rwops), val);

}
int RW::write(u8 val) {

	return SDL_WriteU8((SDL_RWops*)(this->_rwops), val);

}
int RW::write(s8 val) {

	return SDL_WriteU8((SDL_RWops*)(this->_rwops), val);

}
int RW::write(u16 val) {

	return SDL_WriteLE16((SDL_RWops*)(this->_rwops), val);

}
int RW::write(s16 val) {

	return SDL_WriteLE16((SDL_RWops*)(this->_rwops), val);

}
int RW::write(u32 val) {

	return SDL_WriteLE32((SDL_RWops*)(this->_rwops), val);

}
int RW::write(s32 val) {

	return SDL_WriteLE32((SDL_RWops*)(this->_rwops), val);

}
int RW::write(u64 val) {

	return SDL_WriteLE64((SDL_RWops*)(this->_rwops), val);

}
int RW::write(s64 val) {

	return SDL_WriteLE64((SDL_RWops*)(this->_rwops), val);

}

size_t RW::size() {

	SDL_RWops* rwops = (SDL_RWops*)(this->_rwops);

	return SDL_RWsize(rwops);

}

size_t RW::to_buffer(void** buffer) {

	SDL_RWops* rwops = (SDL_RWops*)(this->_rwops);
	
	size_t size = SDL_RWsize(rwops);
	
	if (size == 0) {

		PLog::Write(PLog::ERR, "PFile", "RW size = 0");
		return size;

	}

	*buffer = SDL_malloc(size);

	if (*buffer) {

		this->read(*buffer, size);
		return size;

	} else {

		PLog::Write(PLog::ERR, "PFile", "Could not alloc memory");
		return 0;

	}

}

void RW::close() {

	if(this->_rwops!=nullptr){
		SDL_RWops* rwops = (SDL_RWops*)(this->_rwops);
	
		int ret = SDL_RWclose(rwops);
		if (ret != 0) {
		
			PLog::Write(PLog::ERR, "PFile", "Error freeing rw");
		}

		this->_rwops = nullptr;
	}

	if(this->_mem_buffer!=nullptr){
		SDL_free(this->_mem_buffer);
		this->_mem_buffer = nullptr;
	}

}

std::unordered_map<std::string, std::vector<std::string>> scan_cache;
//std::unordered_map<std::string, std::vector<std::string>> scan_cache_zip; //TODO

std::vector<std::string> Path::scandir(const char* type) {
    
	std::vector<std::string> ret;

	std::string dir = this->path.substr(0, this->path.find_last_of(PE_SEP));
	std::string cache_entry(dir + type);
	
	const char* cstr = dir.c_str();
	
	if (this->zip_file != nullptr) {

    	#ifdef PK2_USE_ZIP
        
		return scan_zip(this->zip_file, cstr, type);
		
		#else
		
		return ret;
		
		#endif

	}
	
	// Look the cache	
	auto it = scan_cache.find(cache_entry);
	if (it != scan_cache.end()) {

		PLog::Write(PLog::DEBUG, "PFile", "Got cache on \"%s\" for \"%s\"", cstr, type);
		return it->second;

	}

    #ifdef __ANDROID__

    if (cstr[0] != '/') {

		ret = scan_apk(cstr, type);
		scan_cache[cache_entry] = ret;
        return ret;

	}

    #endif
	
	ret = scan_file(cstr, type);
	scan_cache[cache_entry] = ret;
	return ret;

}

};