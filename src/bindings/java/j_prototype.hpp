#pragma once

#ifdef PK2_USE_JAVA

#include <vector>
#include "engine/PDLL.hpp"
#include "game/prototype.hpp"
#include <jni.h>

extern std::vector<PrototypeClass*> jSpritePrototypes;


extern "C"{

PK2_EXPORT jint Java_pk2_Prototype_getType
  (JNIEnv *, jobject);
PK2_EXPORT jstring Java_pk2_Prototype_getFilename
  (JNIEnv *, jobject);

PK2_EXPORT jstring Java_pk2_Prototype_getName
  (JNIEnv *, jobject);

PK2_EXPORT jint Java_pk2_Prototype_getWidth
  (JNIEnv *, jobject);

PK2_EXPORT jint Java_pk2_Prototype_getHeight
  (JNIEnv *, jobject);

PK2_EXPORT jstring Java_pk2_Prototype_getTextureName
  (JNIEnv *, jobject);

PK2_EXPORT jint Java_pk2_Prototype_getColor
  (JNIEnv *, jobject);

PK2_EXPORT jint Java_pk2_Prototype_getFrameX
  (JNIEnv *, jobject);

PK2_EXPORT jint Java_pk2_Prototype_getFrameY
  (JNIEnv *, jobject);

PK2_EXPORT jint Java_pk2_Prototype_getFrameWidth
  (JNIEnv *, jobject);

PK2_EXPORT jint Java_pk2_Prototype_getFrameHeight
  (JNIEnv *, jobject);

}

#endif