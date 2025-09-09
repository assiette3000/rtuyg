#include <iostream>
#include <fstream>
#include <cmath>
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"

class Level {
    public:
    int width, height;
    int *data = nullptr;
    Level(std::string name) {
        std::ifstream ifs("../res/lvl/" + name + ".txt");
        ifs >> width;
        ifs >> height;
        std::cout << "level size: " << width << ", " << height << std::endl;
        data = new int[width*height];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                ifs >> data[x+y*width];
            }
        }
    }
    ~Level() {
        delete data;
    }
};

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture_character;
    SDL_Texture *texture_box;

    double character_x = 48., character_y = 64.;
    double character_vx = 0., character_vy = 0.;

    double camera_x = 0., camera_y = 0.;

    bool hook_shooting = false;
    bool hook_attached = false;
    double hook_length = 0.;
    double hook_angle = -M_PI/4.;
    double hook_end_x = 0., hook_end_y = 0.;

    SDL_Event event;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cout << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("rtuyg", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        std::cout << "Couldn't create window and renderer: " << SDL_GetError() << std::endl;
        return 1;
    }

    texture_character = IMG_LoadTexture(renderer, "../res/img/character.png");
    if (!texture_character) {
        std::cout << "couldn't load character texture." << std::endl;
        return 1;
    }
    texture_box = IMG_LoadTexture(renderer, "../res/img/box.png");

    Level level("lvl1");
    SDL_FRect character_rect = {0,0,32,32};

    bool quit = false;
    while (!quit) {
        SDL_PollEvent(&event);
        switch(event.type) {
            case SDL_EVENT_KEY_DOWN:
                switch (event.key.scancode) {
                    case SDL_SCANCODE_LEFT:
                        character_vx = -4.;
                        break;
                    case SDL_SCANCODE_RIGHT:
                        character_vx = 4.;
                        break;
                    case SDL_SCANCODE_Z:
                        character_vy = -8.;
                        break;
                    case SDL_SCANCODE_X:
                        if (!hook_attached) {
                            hook_shooting = true;
                            hook_length = 0.;
                            hook_angle = -M_PI/4;
                        }
                        hook_attached = false;
                        break;
                    default:
                        break;
                }
                break;

            case SDL_EVENT_KEY_UP:
                switch (event.key.scancode) {
                    case SDL_SCANCODE_LEFT:
                        if (character_vx <= 0.) {
                            character_vx = 0.;
                        }
                        break;
                    case SDL_SCANCODE_RIGHT:
                        if (character_vx >= 0.) {
                            character_vx = 0.;
                        }
                        break;
                    default:
                        break;
                }
                break;

            case SDL_EVENT_QUIT:
                quit = true;
                break;
        }

        if (hook_shooting) {
            hook_length += 8.;
            if (hook_length >= 8.*32.) {
                hook_shooting = false;
            }
        }

        if (hook_attached) {
            if (hook_length >= 4.*32.) {
                hook_length -= 8.;
            }
            else {
                hook_angle -= .1;
                if (hook_angle <= -(M_PI*3./4.)) {
                    hook_attached = false;
                }
            }

            double old_x = character_x;
            double old_y = character_y;
            character_x = hook_end_x - hook_length * std::cos(hook_angle) - 32.;
            character_y = hook_end_y - hook_length * std::sin(hook_angle);
            character_vx = character_x - old_x;
            character_vy = character_y - old_y;
        }
        else {
            character_vy += 9.81/32.;
            character_x += character_vx;
            character_y += character_vy;
        }


        if (character_vx > 0 && level.data[((int)(character_x/32.)+1)+(int)(character_y/32.+.5)*level.width]) {
            character_x = (((int)(character_x) / 32)) * 32;
        }
        if (character_vx < 0 && level.data[((int)(character_x/32.))+(int)(character_y/32.+.5)*level.width]) {
            character_x = (((int)(character_x) / 32)+1) * 32;
        }


        if (character_vy > 0 && level.data[(int)(character_x/32.+.5)+((int)(character_y/32.)+1)*level.width]) {
            character_vy = 0.;
            character_y = (((int)(character_y) / 32)) * 32;
        }
        if (character_vy < 0 && level.data[(int)(character_x/32.+.5)+(int)(character_y/32.)*level.width]) {
            character_vy = 0.;
            character_y = (((int)(character_y) / 32)+1) * 32;
        }

        if (character_y >= 32.*12.) {
            character_vy = 0.;
            character_y = 32.*12.;
        }

        if (hook_shooting) {
            hook_end_x = character_x + 32. + hook_length * std::cos(hook_angle);
            hook_end_y = character_y + hook_length * std::sin(hook_angle);
        }
        if (hook_shooting && level.data[(int)(hook_end_x/32.)+(int)(hook_end_y/32.)*level.width]) {
            hook_shooting = false;
            hook_attached = true;
        }


        // render

        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_RenderClear(renderer);

        camera_x = character_x + 16. - 320.;
        camera_y = character_y + 16. - 240.;

        SDL_FRect box_rect = {0, 0, 32, 32};
        for (int y = 0; y < level.height; y++) {
            for (int x = 0; x < level.width; x++) {
                if (level.data[x+y*level.width]) {
                    box_rect.x = x*32. - camera_x;
                    box_rect.y = y*32. - camera_y;
                    SDL_RenderTexture(renderer, texture_box, nullptr, &box_rect);
                }
            }
        }

        character_rect.x = character_x - camera_x;
        character_rect.y = character_y - camera_y;

        SDL_RenderTexture(renderer, texture_character, nullptr, &character_rect);

        if (hook_shooting || hook_attached) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderLine(renderer,
                character_x + 32. - camera_x,
                character_y - camera_y,
                hook_end_x - camera_x,
                hook_end_y - camera_y
            );
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16.);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

