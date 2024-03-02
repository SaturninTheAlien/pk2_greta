#pragma once

#include "3rd_party/sol.hpp"


namespace PK2lua{

enum{
    LUA_EVENT_SKULL_BLOCKS_CHANGED,
    LUA_EVENT_1,
    LUA_EVENT_2
};

void ExposeGameAPI(sol::state& lua);
void ClearEventListeners();
void TriggerEventListeners(int event_type);

}