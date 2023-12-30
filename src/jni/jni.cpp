#ifdef PK2_USE_JAVA
#include <iostream>
#include "jni.hpp"
#include "pk2_main.hpp"

void Java_pk2_PekkaKana2_setAssetsPath(JNIEnv *env, jclass cls, jstring jpath){

    std::string path = env->GetStringUTFChars(jpath, nullptr);
    std::cout<<"Setting the assets path to "<<path<<std::endl;

    pk2_setAssetsPath(path);   
}

jboolean Java_pk2_PekkaKana2_testLevel
    (JNIEnv *env, jclass pk2jclass, jstring j_levelName, jboolean dev_mode){
    std::string levelName =env->GetStringUTFChars(j_levelName, nullptr);

    pk2_main(dev_mode, false,  true, levelName);

    return true;
}

#endif