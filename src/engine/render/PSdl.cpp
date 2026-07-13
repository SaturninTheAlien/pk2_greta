//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "engine/PRender.hpp"

#include "engine/PLog.hpp"

#include "PSdl.hpp"

#include <SDL.h>

#include <stdexcept>

bool PSdl::ensure_screen_resources(SDL_Surface* source) {

    if (!source)
        return false;

    if (screen_texture && screen_surface &&
        screen_surface->w == source->w && screen_surface->h == source->h)
        return true;

    destroy_screen_resources();

    screen_surface = SDL_CreateRGBSurfaceWithFormat(
        0, source->w, source->h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!screen_surface) {
        PLog::Write(PLog::ERR, "PSdl", "Couldn't create conversion surface: %s", SDL_GetError());
        return false;
    }

    screen_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        source->w,
        source->h);
    if (!screen_texture) {
        PLog::Write(PLog::ERR, "PSdl", "Couldn't create screen texture: %s", SDL_GetError());
        SDL_FreeSurface(screen_surface);
        screen_surface = NULL;
        return false;
    }

    SDL_SetTextureBlendMode(screen_texture, SDL_BLENDMODE_NONE);
    apply_texture_filter(screen_texture);
    return true;

}

void PSdl::apply_texture_filter(SDL_Texture* texture) {

    if (!texture)
        return;

#if SDL_VERSION_ATLEAST(2, 0, 12)
    SDL_ScaleMode scale_mode = shader_mode == PRender::SHADER_NEAREST
        ? SDL_ScaleModeNearest
        : SDL_ScaleModeLinear;

    if (SDL_SetTextureScaleMode(texture, scale_mode) != 0)
        PLog::Write(PLog::WARN, "PSdl", "Couldn't set texture filtering: %s", SDL_GetError());
#endif

}

void PSdl::destroy_screen_resources() {

    SDL_DestroyTexture(screen_texture);
    screen_texture = NULL;

    SDL_FreeSurface(screen_surface);
    screen_surface = NULL;

}

void PSdl::load_ui_texture(void* surface) {

    SDL_DestroyTexture(ui_texture);
    ui_texture = NULL;

    if (ui_surface && ui_surface != surface)
        SDL_FreeSurface(ui_surface);

    ui_surface = (SDL_Surface*)surface;
    if (!ui_surface) {
        PLog::Write(PLog::ERR, "PSdl", "Couldn't load UI surface!");
        return;
    }

    ui_texture = SDL_CreateTextureFromSurface(renderer, ui_surface);
    if (ui_texture == NULL) {

        PLog::Write(PLog::ERR, "PSdl", "Couldn't load texture!");

    } else {

        apply_texture_filter(ui_texture);

    }

}
void PSdl::render_ui(PRender::FRECT src, PRender::FRECT dst, float alpha) {

    RenderOptions opt;

    opt.src = src;
    opt.dst = dst;
    opt.alpha = alpha;

    render_list.push_back(opt);

}

void PSdl::clear_screen() {

    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

}

void PSdl::set_screen(PRender::FRECT screen_dst) {

    int w, h;
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
        PRender::get_window_size(&w, &h);

    screen_dest.x = screen_dst.x * w;
    screen_dest.y = screen_dst.y * h;
    screen_dest.w = screen_dst.w * w;
    screen_dest.h = screen_dst.h * h;

}

int PSdl::set_shader(int mode) {

    const char* quality;

    if (mode == PRender::SHADER_NEAREST)
        quality = "0";
    else if (mode == PRender::SHADER_LINEAR)
        quality = "1";
    else
        return 1;

    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, quality) != SDL_TRUE)
        return 1;

    shader_mode = mode;

#if SDL_VERSION_ATLEAST(2, 0, 12)
    apply_texture_filter(screen_texture);
    apply_texture_filter(ui_texture);
#else
    // Older SDL versions only read the filtering hint when creating a texture.
    SDL_DestroyTexture(screen_texture);
    screen_texture = NULL;
    SDL_DestroyTexture(ui_texture);
    ui_texture = NULL;
#endif

    return 0;
}

int PSdl::set_vsync(bool set) {

    if(renderer!=nullptr && SDL_RenderSetVSync(renderer, set)!=0){
        PLog::Write(PLog::ERR, "PSDL", "Couldn't set vsync %s", SDL_GetError());
    }

    /*Uint32 sync = set? SDL_RENDERER_PRESENTVSYNC : 0;

    if (renderer) {

        ui_texture = NULL;
        SDL_DestroyRenderer(renderer);

    }

    Uint32 sync = set? SDL_RENDERER_PRESENTVSYNC : 0;

    renderer = SDL_CreateRenderer(curr_window, -1, SDL_RENDERER_ACCELERATED | sync);
    if (!renderer) {

        PLog::Write(PLog::FATAL, "PSdl", "Couldn't create renderer!");
        return -1;

    }

    SDL_RenderClear(renderer);*/

    return 0;

}

void PSdl::update(void* _buffer8) {

    SDL_Surface* buffer8 = (SDL_Surface*)_buffer8;

    if (!ensure_screen_resources(buffer8))
        return;

    if (SDL_BlitSurface(buffer8, NULL, screen_surface, NULL) != 0) {
        PLog::Write(PLog::ERR, "PSdl", "Couldn't convert screen surface: %s", SDL_GetError());
        return;
    }

    if (SDL_UpdateTexture(screen_texture, NULL, screen_surface->pixels, screen_surface->pitch) != 0) {
        PLog::Write(PLog::ERR, "PSdl", "Couldn't update screen texture: %s", SDL_GetError());
        return;
    }

    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, screen_texture, NULL, &screen_dest);
    
    int w, h;
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
        PRender::get_window_size(&w, &h);
    float prop_x = (float)w;
    float prop_y = (float)h;
    
    if (ui_surface) {

        if (!ui_texture) {
            ui_texture = SDL_CreateTextureFromSurface(renderer, (SDL_Surface*)ui_surface);
            apply_texture_filter(ui_texture);
        }

        for (auto opt : render_list) {
            
            u8 mod = opt.alpha * 256;
            if (mod == 0)
                continue;
            
            SDL_SetTextureAlphaMod(ui_texture, mod);
            
            SDL_Rect dst;
            dst.x = opt.dst.x * prop_x;
            dst.y = opt.dst.y * prop_y;
            dst.w = opt.dst.w * prop_x;
            dst.h = opt.dst.h * prop_y;

            SDL_Rect src;
            src.x = opt.src.x * 1024;
            src.y = opt.src.y * 1024;
            src.w = opt.src.w * 1024;
            src.h = opt.src.h * 1024;

            SDL_RenderCopy(renderer, ui_texture, &src, &dst);
            
        }

        render_list.clear();

    }

    SDL_RenderPresent(renderer);

}

PSdl::PSdl(int, int, void* window) {

    curr_window = (SDL_Window*)window;

    renderer = SDL_CreateRenderer(curr_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {

        PLog::Write(PLog::FATAL, "PSdl", "Couldn't create renderer!");
		throw std::runtime_error("Cannot create SDL renderer!");
	}

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_RenderClear(renderer);

}

PSdl::~PSdl() {

    destroy_screen_resources();

    SDL_DestroyTexture(ui_texture);
    ui_texture = NULL;

    SDL_FreeSurface(ui_surface);
    ui_surface = NULL;

    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    PLog::Write(PLog::DEBUG, "PSdl", "Terminated");

}
