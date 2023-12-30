#pragma once

#ifdef PK2_USE_JAVA
#include "engine/PDLL.hpp"
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Class:     pk2_PekkaKana2
 * Method:    setAssetsPath
 * Signature: (Ljava/lang/String;)V
 */
PK2_EXPORT void Java_pk2_PekkaKana2_setAssetsPath
  (JNIEnv *, jclass, jstring);

/*
 * Class:     pk2_PekkaKana2
 * Method:    testLevel
 * Signature: (Ljava/lang/String;Z)Z
 */
PK2_EXPORT jboolean Java_pk2_PekkaKana2_testLevel
  (JNIEnv *, jclass, jstring, jboolean);

#ifdef __cplusplus
}
#endif

#endif