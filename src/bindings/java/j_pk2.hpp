#pragma once

#ifdef PK2_USE_JAVA
#include "engine/PDLL.hpp"
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     pk2_PekkaKana2
 * Method:    mInit
 * Signature: (Ljava/lang/String;)V
 */
PK2_EXPORT void Java_pk2_PekkaKana2_mInit(JNIEnv *, jclass, jstring);

/*
 * Class:     pk2_PekkaKana2
 * Method:    quit
 * Signature: ()Z
 */
PK2_EXPORT void Java_pk2_PekkaKana2_quit(JNIEnv *, jclass);

/*
 * Class:     pk2_PekkaKana2
 * Method:    testLevel
 * Signature: (Ljava/lang/String;Z)Z
 */
PK2_EXPORT jboolean Java_pk2_PekkaKana2_testLevel(JNIEnv *, jclass, jstring, jboolean);



/*
 * Class:     pk2_PrototypesHandler
 * Method:    mCreate
 * Signature: ()V
 */
PK2_EXPORT void Java_pk2_PrototypesHandler_mCreate(JNIEnv *env, jobject o,
    jboolean shouldLoadDependencies, jboolean jsonPriority);

/*
 * Class:     pk2_PrototypesHandler
 * Method:    mLoadSprite
 * Signature: (Ljava/lang/String;)I
 */
PK2_EXPORT jint Java_pk2_PrototypesHandler_mLoadSprite
  (JNIEnv *, jobject, jstring);


#ifdef __cplusplus
}
#endif

#endif