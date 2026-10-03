//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "engine/PRender.hpp"

#include "engine/PLog.hpp"

#include "PSdl.hpp"
#include <stdexcept>

void PSdl::load_ui_texture(void* surface) {

    ui_surface = (SDL_Surface*)surface;
    ui_texture = SDL_CreateTextureFromSurface(renderer, (SDL_Surface*)ui_surface);
    if (ui_texture == NULL) {

        PLog::Write(PLog::ERR, "PSdl", "Couldn't load texture!");

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
    PRender::get_window_size(&w, &h);

    screen_dest.x = screen_dst.x * w;
    screen_dest.y = screen_dst.y * h;
    screen_dest.w = screen_dst.w * w;
    screen_dest.h = screen_dst.h * h;

}

int PSdl::set_shader(int mode) {
    if (mode == PRender::SHADER_NEAREST) {
        this->scale_mode = SDL_SCALEMODE_NEAREST;
        return 0;
    }
    if (mode == PRender::SHADER_LINEAR) {
        this->scale_mode = SDL_SCALEMODE_LINEAR;
        return 0;
    }

    if(this->ui_texture){
        SDL_SetTextureScaleMode(this->ui_texture, this->scale_mode);
    }


    return 1;
}

int PSdl::set_vsync(bool set) {
    if (renderer != nullptr &&
        !SDL_SetRenderVSync(renderer, set ? 1 : 0)) {

        PLog::Write(PLog::ERR, "PSDL",
            "Couldn't set vsync: %s", SDL_GetError());

        return 1;
    }

    return 0;
}


void PSdl::update(void* _buffer8) {

    SDL_Surface* buffer8 = (SDL_Surface*)_buffer8;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, buffer8);

    if (!texture) {
        throw std::runtime_error(std::string("Couldn't create texture: ") + SDL_GetError());
    }

    SDL_SetTextureScaleMode(texture, scale_mode);

    SDL_RenderClear(renderer);

    SDL_FRect dst = {
        static_cast<float>(screen_dest.x),
        static_cast<float>(screen_dest.y),
        static_cast<float>(screen_dest.w),
        static_cast<float>(screen_dest.h)
    };

    SDL_RenderTexture(renderer, texture, nullptr, &dst);
    
    int w, h;
    SDL_GetCurrentRenderOutputSize(renderer, &w, &h);
    float prop_x = (float)w;
    float prop_y = (float)h;
    
    if (ui_surface) {

        if (!ui_texture){
            ui_texture = SDL_CreateTextureFromSurface(renderer, (SDL_Surface*)ui_surface);
            SDL_SetTextureScaleMode(this->ui_texture, this->scale_mode);
        }
        
        for (auto opt : render_list) {
            
            u8 mod = opt.alpha * 256;
            if (mod == 0)
                continue;
            
            SDL_SetTextureAlphaMod(ui_texture, mod);
            
            SDL_FRect dst;
            dst.x = static_cast<float>(opt.dst.x * prop_x);
            dst.y = static_cast<float>(opt.dst.y * prop_y);
            dst.w = static_cast<float>(opt.dst.w * prop_x);
            dst.h = static_cast<float>(opt.dst.h * prop_y);

            SDL_FRect src;
            src.x = static_cast<float>(opt.src.x * 1024);
            src.y = static_cast<float>(opt.src.y * 1024);
            src.w = static_cast<float>(opt.src.w * 1024);
            src.h = static_cast<float>(opt.src.h * 1024);

            SDL_RenderTexture(renderer, ui_texture, &src, &dst);
            
        }

        render_list.clear();

    }

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);

}

PSdl::PSdl(int width, int height, void* window) {

    curr_window = (SDL_Window*)window;

    renderer = SDL_CreateRenderer(curr_window, nullptr);
    if (!renderer) {
        PLog::Write(PLog::FATAL, "PSdl",
                    "Couldn't create renderer: %s", SDL_GetError());
        throw std::runtime_error("Cannot create SDL renderer!");
    }

    if (!SDL_SetRenderVSync(renderer, 1)) {
        PLog::Write(PLog::WARN, "PSdl",
                    "Couldn't enable vsync: %s", SDL_GetError());
    }

    SDL_RenderClear(renderer);

}

PSdl::~PSdl() {

    SDL_DestroyRenderer(renderer);
    SDL_DestroySurface(ui_surface);

    PLog::Write(PLog::DEBUG, "PSdl", "Terminated");

}
