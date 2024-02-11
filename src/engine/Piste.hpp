//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/PDLL.hpp"
#include "engine/platform.hpp"

#include "engine/PDraw.hpp"
#include "engine/PRender.hpp"
#include "engine/PInput.hpp"
#include "engine/PSound.hpp"
#include "engine/PUtils.hpp"
#include "engine/PLang.hpp"
#include "engine/PLog.hpp"
#include "engine/PFile.hpp"
#include "engine/PGui.hpp"
#include <functional>

namespace Piste {

void PK2_EXPORT init(int width, int height, const char* name, const char* icon, int audio_buffer_size, bool audio_multi_thread);
void PK2_EXPORT terminate();

void loop(std::function<void()> GameLogic);
void stop();
int get_fps();

int set_fps(int fps);
void set_debug(bool set);
void ignore_frame();
bool is_ready();

}
