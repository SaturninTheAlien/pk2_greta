#
# ndk-build module for the game, for use from an SDL3 Android project
# (SDL3's android-project template, with this file included from app/jni/Android.mk).
#
# SDL3 and SDL3_image come from their official Android packages (SDL3-devel-*-android.zip and
# SDL3_image-devel-*-android.zip): put the .aar files in app/libs, and in app/build.gradle set
# buildFeatures { prefab true } and add them as dependencies. They are imported at the bottom.
#
# The project must also provide these modules, each exporting its include directories:
#   openmpt   libopenmpt's build/android_ndk/Android.mk
#   libzip
#   lua
#
# Application.mk needs APP_STL := c++_shared, since the libraries share the C++ runtime.
#

LOCAL_PATH := $(call my-dir)
SRC_DIR := $(LOCAL_PATH)/src

include $(CLEAR_VARS)

LOCAL_MODULE := PK2

# Relative to LOCAL_PATH, as ndk-build expects
LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%,%, \
	$(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*.cpp))

LOCAL_C_INCLUDES := $(SRC_DIR)

LOCAL_CPP_FEATURES += exceptions rtti
LOCAL_SHARED_LIBRARIES := SDL3-shared SDL3_image-shared openmpt libzip lua
LOCAL_STATIC_LIBRARIES := SDL3-Headers
LOCAL_CPPFLAGS += -std=c++17 -DPK2_VERSION=\"1.5.4\"
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

# https://google.github.io/prefab/build-systems.html
$(call import-module,prefab/SDL3)
$(call import-module,prefab/SDL3_image)
