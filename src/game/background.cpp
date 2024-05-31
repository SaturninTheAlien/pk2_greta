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

	std::pair<int, int> p = PDraw::image_load_with_palette(path, false);
	this->picture = p.first;
	this->palette = p.second;

	PDraw::image_getsize(this->picture, this->width, this->height);

	//PDraw::palette_set_if_not_set(this->palette);

	if (this->picture == -1)
		throw PExcept::PException("Cannot load the background!");
}

void Background::setPalette(){
	PDraw::palette_set(this->palette);
}

void Background::clear(){
    PDraw::image_delete(this->picture);
	PDraw::palette_delete(this->palette);
}


void Background::draw(int camera_x, int camera_y){
    int pallarx = ( camera_x % (this->width*3) ) / 3;
	int pallary = ( camera_y % (this->height*3) ) / 3;

	if (this->scrolling == BACKGROUND_STATIC){
	
		PDraw::image_clip(this->picture,0,0);
		PDraw::image_clip(this->picture,this->width,0);
	
	} else if (this->scrolling == BACKGROUND_PARALLAX_HORI){
	
		PDraw::image_clip(this->picture,0   - pallarx,0);
		PDraw::image_clip(this->picture,this->width - pallarx,0);

		if (screen_width > this->width)
			PDraw::image_clip(this->picture,this->width*2 - pallarx,0);
	
	} else if (this->scrolling == BACKGROUND_PARALLAX_VERT){
	
		PDraw::image_clip(this->picture,0,0   - pallary);
		PDraw::image_clip(this->picture,0,this->height - pallary);

		if (screen_width > this->width){
			PDraw::image_clip(this->picture,this->width,0   - pallary);
			PDraw::image_clip(this->picture,this->width,this->height - pallary);
		}
	
	} else if (this->scrolling == BACKGROUND_PARALLAX_VERT_AND_HORI){
	
		PDraw::image_clip(this->picture,0   - pallarx, 0-pallary);
		PDraw::image_clip(this->picture,this->width - pallarx, 0-pallary);
		PDraw::image_clip(this->picture,0   - pallarx, this->height-pallary);
		PDraw::image_clip(this->picture,this->width - pallarx, this->height-pallary);

		if (screen_width > this->width){
			PDraw::image_clip(this->picture,this->width*2 - pallarx,0-pallary);
			PDraw::image_clip(this->picture,this->width*2 - pallarx,this->height-pallary);
		}
	}
};