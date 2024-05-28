//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "background.hpp"
#include "engine/PDraw.hpp"
#include "system.hpp"
#include "exceptions.hpp"

void Background::load(PFile::Path path){
    if (!FindAsset(&path, "gfx" PE_SEP "scenery" PE_SEP)){
        throw PExcept::FileNotFoundException(path.c_str(), PExcept::MISSING_BACKGROUND);
    }
	
	PDraw::image_load(this->picture, path, true, false);
	if (this->picture == -1)
		throw PExcept::PException("Cannot load the background!");
}

void Background::clear(){
    PDraw::image_delete(this->picture);
}


void Background::draw(int camera_x, int camera_y){
    int pallarx = ( camera_x % (640*3) ) / 3;
	int pallary = ( camera_y % (480*3) ) / 3;

	if (this->scrolling == BACKGROUND_STATIC){
	
		PDraw::image_clip(this->picture,0,0);
		PDraw::image_clip(this->picture,640,0);
	
	} else if (this->scrolling == BACKGROUND_PARALLAX_HORI){
	
		PDraw::image_clip(this->picture,0   - pallarx,0);
		PDraw::image_clip(this->picture,640 - pallarx,0);

		if (screen_width > 640)
			PDraw::image_clip(this->picture,640*2 - pallarx,0);
	
	} else if (this->scrolling == BACKGROUND_PARALLAX_VERT){
	
		PDraw::image_clip(this->picture,0,0   - pallary);
		PDraw::image_clip(this->picture,0,480 - pallary);

		if (screen_width > 640){
			PDraw::image_clip(this->picture,640,0   - pallary);
			PDraw::image_clip(this->picture,640,480 - pallary);
		}
	
	} else if (this->scrolling == BACKGROUND_PARALLAX_VERT_AND_HORI){
	
		PDraw::image_clip(this->picture,0   - pallarx, 0-pallary);
		PDraw::image_clip(this->picture,640 - pallarx, 0-pallary);
		PDraw::image_clip(this->picture,0   - pallarx, 480-pallary);
		PDraw::image_clip(this->picture,640 - pallarx, 480-pallary);

		if (screen_width > 640){
			PDraw::image_clip(this->picture,640*2 - pallarx,0-pallary);
			PDraw::image_clip(this->picture,640*2 - pallarx,480-pallary);
		}
	}
};