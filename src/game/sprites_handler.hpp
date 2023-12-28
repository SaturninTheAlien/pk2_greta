#pragma once
#include "prototypes_handler.hpp"
#include "spriteclass.hpp"

#include <list>

class MapClass;

class SpritesHandler{
public:
    SpritesHandler();
    ~SpritesHandler(){
        clearAll();
    };
    void clearAll();
    PrototypeClass* getLevelPrototype(int index);
    void loadAllLevelPrototypes(const MapClass& map);

    void add_bg(SpriteClass* sprite) {
        bgSprites_List.push_back(sprite);
    }

    void add_fg(SpriteClass* sprite){
        fgSprites_List.push_back(sprite);
    }

    void sortBg();
    void onGameStart();
    void onSkullBlocksChanged();
    void onEvent1();
    void onEvent2();

    int onTickUpdate();

    void addSprite(PrototypeClass* protot, int is_Player_Sprite, double x, double y, SpriteClass* parent, bool isbonus);
    void addSpriteAmmo(PrototypeClass* protot, double x, double y, SpriteClass* emo);

    PrototypesHandler prototypesHandler;
    
    std::list<SpriteClass*> Sprites_List;
    std::list<SpriteClass*> bgSprites_List;
    std::list<SpriteClass*> fgSprites_List;

    SpriteClass* Player_Sprite;
private:
    bool spriteDestructed (SpriteClass* sprite);
    void loadLevelPrototype(const std::string& name, int id);
    PrototypeClass* Level_Prototypes_List[MAX_PROTOTYYPPEJA] = {nullptr};
};