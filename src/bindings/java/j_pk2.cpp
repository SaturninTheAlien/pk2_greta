#ifdef PK2_USE_JAVA

#include "j_pk2.hpp"
#include "j_prototype.hpp"

#include "engine/PLog.hpp"
#include "engine/PFile.hpp"

#include "game/prototypes_handler.hpp"

#include "pk2_main.hpp"
#include "system.hpp"
#include <vector>

std::vector<PrototypesHandler*> jPrototypeHandlers;

void Java_pk2_PekkaKana2_mInit(JNIEnv *env, jclass cls, jstring jpath){

    std::string path = env->GetStringUTFChars(jpath, nullptr);


    pk2_setAssetsPath(path);
    pk2_init();

    PLog::Write(PLog::INFO, "JPK2", "The assets path set to: %s", path.c_str());
}

void Java_pk2_PekkaKana2_quit(JNIEnv *, jclass){

    for(PrototypeClass* &p:jSpritePrototypes){
        /**
         * @brief 
         * NOT DELETE THEM!!
         * Only set pointers to null.
         * They are deleted by PrototypesHandler
         */
        p = nullptr;
    }
    for(PrototypesHandler *&h:jPrototypeHandlers){
        if(h!=nullptr){
            delete h;
            h = nullptr;
        }
    }
    jSpritePrototypes.clear();
    jPrototypeHandlers.clear();
}

jboolean Java_pk2_PekkaKana2_testLevel
    (JNIEnv *env, jclass pk2jclass, jstring j_levelName, jboolean dev_mode){
    std::string levelName =env->GetStringUTFChars(j_levelName, nullptr);

    pk2_main(dev_mode, false,  true, levelName);

    return true;
}



void Java_pk2_sprite_PrototypesHandler_mCreate(JNIEnv *env, jobject o,
    jboolean shouldLoadDependencies, jboolean jsonPriority){
    jclass c = env->GetObjectClass(o);
    jfieldID fid = env->GetFieldID(c, "id", "I");

    int id = jPrototypeHandlers.size();
    jPrototypeHandlers.push_back(new PrototypesHandler(shouldLoadDependencies, jsonPriority));
    env->SetIntField(o, fid, id);    
}

PrototypesHandler* getPrototypeHandlerByID(JNIEnv * env, jobject o){
    jclass c = env->GetObjectClass(o);
    jfieldID fid = env->GetFieldID(c, "id", "I");

    int handler_id = env->GetIntField(o, fid);

    if(handler_id<0 || handler_id>= int(jPrototypeHandlers.size())){

        jclass cls = env->FindClass("java/lang/NullPointerException");
        env->ThrowNew(cls, "PrototypesHandler not found!");

        return nullptr;
    }

    return jPrototypeHandlers[handler_id];
}

jstring Java_pk2_PekkaKana2_findAsset(JNIEnv *env, jclass, jstring j_episode_name,jstring j_default_dir, jstring j_name){
    std::string episode_name = env->GetStringUTFChars(j_episode_name, nullptr);
    std::string default_dir = env->GetStringUTFChars(j_default_dir, nullptr);
    std::string name = env->GetStringUTFChars(j_name, nullptr);

    std::string path_str("episodes" PE_SEP);
	path_str += episode_name + PE_SEP + name;

    PFile::Path path(path_str);

    if(FindAsset(&path, default_dir.c_str())){
        return env->NewStringUTF(path.c_str());
    }    

    return nullptr;
}


jint Java_pk2_sprite_PrototypesHandler_mLoadSprite(JNIEnv * env, jobject o, jstring j_name){
    
    PrototypesHandler * handler = getPrototypeHandlerByID(env, o);
    if(handler!=nullptr){
        std::string name = env->GetStringUTFChars(j_name, nullptr);

        try{
            PrototypeClass * prototype = handler->loadPrototype(name);
            int sprite_id = jSpritePrototypes.size();
            jSpritePrototypes.push_back(prototype);

            return sprite_id;

        }
        catch(const std::exception& e){
            PLog::Write(PLog::ERR, "JPK2", "%s", e.what());
        }
    }

    return -1;
}

void Java_pk2_sprite_PrototypesHandler_clear(JNIEnv *env, jobject o){
    PrototypesHandler * handler = getPrototypeHandlerByID(env, o);
    if(handler!=nullptr){
        handler->clear();
    }
}

void Java_pk2_sprite_PrototypesHandler_setSearchingDir(JNIEnv *env, jobject o, jstring jstr){
    PrototypesHandler * handler = getPrototypeHandlerByID(env, o);
    if(handler!=nullptr){
        std::string s = env->GetStringUTFChars(jstr, nullptr);
        handler->setSearchingDir(s);
    }
}


#endif