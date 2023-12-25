#pragma once

#include "prototype.hpp"

class EpisodeClass;

class PrototypesHandler{

public:
    PrototypesHandler(){};
    ~PrototypesHandler(){
        clear();
    }
    /**
     * @brief
     * Do not copy objects of this class!
     */
    PrototypesHandler(const PrototypesHandler&)=delete;
    PrototypesHandler& operator=(const PrototypesHandler&)=delete;

    void clear();

    /**
     * @brief 
     * Load sprite prototype by name
     */
    PrototypeClass* loadPrototype(const std::string& filename_in, EpisodeClass*episode=nullptr);
    /**
     * @brief 
     * Get sprite prototype by index for the purpose of debugging.
     * If out of range, nullptr is returned.
     */
    PrototypeClass* get(int index);
    

    void loadSpriteAssets(EpisodeClass* episode);
    void unloadSpriteAssets();
private:
    std::vector<PrototypeClass*>mPrototypes;
};