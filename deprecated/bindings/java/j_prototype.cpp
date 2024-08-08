#include "j_prototype.hpp"

#ifdef PK2_USE_JAVA
std::vector<PrototypeClass*> jSpritePrototypes;


PrototypeClass* jGetPrototypeById(JNIEnv *env, jobject o){
    jclass c = env->GetObjectClass(o);
    jfieldID fid = env->GetFieldID(c, "sprite_id", "I");
    int sprite_id = env->GetIntField(o, fid);
    
    if(sprite_id<0||sprite_id>=int(jSpritePrototypes.size())){
        jclass cls = env->FindClass("java/lang/NullPointerException");
        env->ThrowNew(cls, "Prototype not found!");
        return nullptr;
    }

    return jSpritePrototypes[sprite_id];
}



jint Java_pk2_sprite_Prototype_getType(JNIEnv * env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return p->type;
    }
    return 0;
}

jstring Java_pk2_sprite_Prototype_getFilename(JNIEnv * env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return env->NewStringUTF(p->filename.c_str());
    }
    return 0;
}

jstring Java_pk2_sprite_Prototype_getName(JNIEnv *env, jobject o){

    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return env->NewStringUTF(p->name.c_str());
    }
    return nullptr;
}

jint Java_pk2_sprite_Prototype_getWidth(JNIEnv *env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return p->width;
    }

    return 0;
}

jint Java_pk2_sprite_Prototype_getHeight(JNIEnv *env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return p->height;
    }

    return 0;
}

jstring Java_pk2_sprite_Prototype_getTextureName(JNIEnv *env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return env->NewStringUTF(p->picture_filename.c_str());
    }
    return nullptr;
}

jint Java_pk2_sprite_Prototype_getColor(JNIEnv *env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return p->color;
    }

    return 0;
}

jint Java_pk2_sprite_Prototype_getFrameX(JNIEnv *env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return p->picture_frame_x;
    }

    return 0;
}

jint Java_pk2_sprite_Prototype_getFrameY(JNIEnv *env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return p->picture_frame_y;
    }

    return 0;
}

jint Java_pk2_sprite_Prototype_getFrameWidth(JNIEnv *env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return p->picture_frame_width;
    }

    return 0;
}

jint Java_pk2_sprite_Prototype_getFrameHeight(JNIEnv *env, jobject o){
    PrototypeClass* p = jGetPrototypeById(env, o);
    if(p!=nullptr){
        return p->picture_frame_height;
    }

    return 0;
}

#endif