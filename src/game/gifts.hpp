//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "sprites_handler.hpp"

#define MAX_GIFTS 4

bool Gifts_Add(PrototypeClass* protot);
int Gifts_Use(SpritesHandler& spritesHandler);
void Gifts_Clean();
int Gifts_Count();

PrototypeClass* Gifts_Get(int i);
void Gifts_Remove(int i);
void Gifts_Draw(int i, int x, int y);
int Gifts_ChangeOrder();