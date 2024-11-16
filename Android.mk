SRC_DIR := $(call my-dir)/src

include $(CLEAR_VARS)
LOCAL_MODULE := PK2

LOCAL_SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp)
LOCAL_SRC_FILES += $(SRC_DIR)/engine/render/PSdl.cpp


# TODO
# Make these paths portable

LOCAL_C_INCLUDES := $(SRC_DIR) \
       /home/saturnin/android/projects/SDL2-Android/build/SDL2-2.30.9/include \
       /home/saturnin/android/projects/SDL2-Android/build/SDL2_image-2.8.2/include \
       /home/saturnin/android/projects/SDL2-Android/build/SDL2_mixer-2.8.0/include \
       /home/saturnin/android/projects/lib/lib/$(APP_ABI)/include \
       $(PROJECT_ROOT)/lua/src

LOCAL_CPP_FEATURES += exceptions
LOCAL_SHARED_LIBRARIES := LIBCPP SDL2 SDL2_image SDL2_mixer libzip lua
LOCAL_STL := c++_shared
LOCAL_CPPFLAGS += -std=c++17 -DPK2_USE_ZIP -DPK2_USE_LUA -DPK2_VERSION=\"v0.201\"
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)