#pragma once
#include "prototypes_handler.hpp"
#include "spriteclass.hpp"
#include "levelclass.hpp"

#include <list>

class LevelClass;

class SpritesHandler{
public:
    SpritesHandler();
    ~SpritesHandler(){
        clearAll();
    };
    void clearAll();
    PrototypeClass* getLevelPrototype(int index);
    void loadAllLevelPrototypes(const LevelClass& map);

    

    void sortBg();
    void onSkullBlocksChanged();
    void onEvent1();
    void onEvent2();

    int onTickUpdate();
    
    /**
     * @brief 
     * Spawning the player
     */
    void addPlayer(PrototypeClass*prototype, double x, double y);

    /**
     * @brief 
     * Spawn a sprite based on the level sprite tiles
     */
    void addLevelSprite(PrototypeClass*prototype, double x, double y);

    /**
     * @brief 
     * Spawn a bonus sprite dropped upon another sprite's death
     */
    void addDroppedBonusSprite(PrototypeClass*prototype, double x, double y);
    /**
     * @brief 
     * Spawn a sprite as a projectile
     */
    void addProjectileSprite(PrototypeClass* prototype, double x, double y, SpriteClass* shooter);
    /**
     * @brief 
     * Spawn a sprite from player's gift
     */
    void addGiftSprite(PrototypeClass* prototype);
    /**
     * @brief 
     * Spawn a sprite from lua
     */

    SpriteClass* addLuaSprite(PrototypeClass* prototype, double x, double y, SpriteClass*parent=nullptr){
        
        return this->mCreateSprite(prototype, false, x, y, parent);
    }

    PrototypesHandler prototypesHandler;
    
    std::list<SpriteClass*> Sprites_List;
    std::list<SpriteClass*> bgSprites_List;
    std::list<SpriteClass*> fgSprites_List;

    SpriteClass* Player_Sprite = nullptr;
private:
    SpriteClass * mCreateSprite(PrototypeClass* prototype, bool player, double x, double y, SpriteClass*parent_sprite=nullptr);

    void mAddBG(SpriteClass* sprite) {
        bgSprites_List.push_back(sprite);
    }

    void mAddFG(SpriteClass* sprite){
        fgSprites_List.push_back(sprite);
    }

    bool spriteDestructed (SpriteClass* sprite);
    void loadLevelPrototype(const std::string& name, int id);
    PrototypeClass* Level_Prototypes_List[MAX_PROTOTYYPPEJA] = {nullptr};
};