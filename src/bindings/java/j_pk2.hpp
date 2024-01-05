#pragma once

#ifdef PK2_USE_JAVA
#include "engine/PDLL.hpp"
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

PK2_EXPORT void Java_pk2_PekkaKana2_mInit(JNIEnv *, jclass, jstring);

PK2_EXPORT void Java_pk2_PekkaKana2_quit(JNIEnv *, jclass);

PK2_EXPORT jboolean Java_pk2_PekkaKana2_testLevel(JNIEnv *, jclass, jstring, jboolean);

PK2_EXPORT jstring Java_pk2_PekkaKana2_findAsset(JNIEnv *, jclass, jstring, jstring);

PK2_EXPORT void Java_pk2_sprite_PrototypesHandler_mCreate(JNIEnv *env, jobject o,
jboolean shouldLoadDependencies, jboolean jsonPriority);

PK2_EXPORT jint Java_pk2_sprite_PrototypesHandler_mLoadSprite(JNIEnv *, jobject, jstring);

PK2_EXPORT void Java_pk2_sprite_PrototypesHandler_clear(JNIEnv *, jobject);

PK2_EXPORT void Java_pk2_sprite_PrototypesHandler_setSearchingDir(JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif

#endif