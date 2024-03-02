#pragma once

#include "prototype.hpp"
#include "engine/PFile.hpp"

class EpisodeClass;

class PrototypesHandler{

public:
    PrototypesHandler(bool shouldLoadDependencies, bool jsonPriority, EpisodeClass* episode=nullptr):
    mShouldLoadDependencies(shouldLoadDependencies), mJsonPriority(jsonPriority), mEpisode(episode){}

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
    PrototypeClass* loadPrototype(const std::string& filename_in);
    /**
     * @brief 
     * Save prototype to file
     */
    void savePrototype(PrototypeClass*prototype, const std::string& filename)const;

    /**
     * @brief 
     * Get sprite prototype by index for the purpose of debugging.
     * If out of range, nullptr is returned.
     */
    PrototypeClass* get(std::size_t index)const;

    void setSearchingDir(const std::string& dir){
        this->mSearchingDirectory = dir;
    }

    void loadSpriteAssets();
    void unloadSpriteAssets();
private:
    std::string mSearchingDirectory = "";

    /**
     * @brief 
     * if should load dependecies sprites such as ammo, transformation and bonus
     */
    bool mShouldLoadDependencies = true;
    bool mJsonPriority = true;    
    bool mAssetsLoaded = false;

    /**
     * @brief 
     * Episode pointer
     */
    class EpisodeClass*mEpisode = nullptr;

    std::vector<PrototypeClass*>mPrototypes;

    PFile::Path mGetDir(const std::string& filename)const;
    bool mFindSprite(PFile::Path& path)const;
};