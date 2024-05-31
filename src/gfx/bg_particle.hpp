//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

enum{
    BGPARTICLE_NOTHING,
	BGPARTICLE_WATERDROP,
	BGPARTICLE_LEAF1,
	BGPARTICLE_LEAF2,
	BGPARTICLE_LEAF3,
	BGPARTICLE_LEAF4,
	BGPARTICLE_FLAKE1,
	BGPARTICLE_FLAKE2,
	BGPARTICLE_FLAKE3,
	BGPARTICLE_FLAKE4
};

class BgParticle{
public:
    BgParticle()=default;
    ~BgParticle()=default;   

    double x = 0;
    double y = 0;
    double a = 0; //x velocity
    double b = 0; //y velocity

    int getType()const{
        return this->mType;
    }
    void setType(int type);

    void checkScreenBorders(int cam_x, int cam_y);    

    void draw(int cam_x, int cam_y)const{
        this->mDraw(*this, cam_x, cam_y);
    }

    void update(){
        this->mUpdate(*this);
    }    
private:
    int mType = 0;
    void (*mDraw)(const BgParticle& p, int cam_x, int cam_y) = nullptr;
    void (*mUpdate)(BgParticle& p) = nullptr;
};