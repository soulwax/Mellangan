#pragma once

#include <SDL3/SDL.h>

#include <array>

class Key {
public:
    bool down = false, clicked = false;

    void toggle(bool pressed) {
        if (pressed && !down) presses++;
        down = pressed;
    }
    void tick() {
        clicked = presses > 0;
        presses = 0;
    }
    void release() { down = false, presses = 0; }

private:
    int presses = 0;
};

class Input {
public:
    Key up, down, left, right, confirm, escape, pause, one, two, three, restart, quit, fps, fullscreen;
    float mouseX = 0, mouseY = 0;
    bool mouseClicked = false, mouseMoved = false;

    void handle(const SDL_Event &e, float density) {
        switch (e.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if (Key *k = bind(e.key.scancode)) k->toggle(e.type == SDL_EVENT_KEY_DOWN);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            mouseX = e.motion.x * density, mouseY = e.motion.y * density, moves++;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (e.button.button == SDL_BUTTON_LEFT) mouseX = e.button.x * density, mouseY = e.button.y * density, clicks++;
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            for (Key *k : keys()) k->release();
            break;
        default:
            break;
        }
    }

    void tick() {
        for (Key *k : keys()) k->tick();
        mouseClicked = clicks > 0, mouseMoved = moves > 0;
        clicks = moves = 0;
    }

private:
    int clicks = 0, moves = 0;

    std::array<Key *, 14> keys() {
        return {&up, &down, &left, &right, &confirm, &escape, &pause, &one, &two, &three, &restart, &quit, &fps, &fullscreen};
    }

    Key *bind(SDL_Scancode sc) {
        switch (sc) {
        case SDL_SCANCODE_W: case SDL_SCANCODE_UP: return &up;
        case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN: return &down;
        case SDL_SCANCODE_A: case SDL_SCANCODE_LEFT: return &left;
        case SDL_SCANCODE_D: case SDL_SCANCODE_RIGHT: return &right;
        case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: case SDL_SCANCODE_SPACE: return &confirm;
        case SDL_SCANCODE_ESCAPE: return &escape;
        case SDL_SCANCODE_P: return &pause;
        case SDL_SCANCODE_1: return &one;
        case SDL_SCANCODE_2: return &two;
        case SDL_SCANCODE_3: return &three;
        case SDL_SCANCODE_R: return &restart;
        case SDL_SCANCODE_Q: return &quit;
        case SDL_SCANCODE_F3: return &fps;
        case SDL_SCANCODE_F11: return &fullscreen;
        default: return nullptr;
        }
    }
};
