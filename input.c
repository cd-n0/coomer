#include "globals.h"
#include "input.h"
#include <stdbool.h>

#ifndef ZOOM_AMOUNT
#define ZOOM_AMOUNT 0.05f
#endif

void input_process_button(mouse_button_t button, b8 press) {
    switch(button) {
        case MOUSE_BUTTON_LEFT: {
            camera.panning = press ? true : false;
        } break;
        /* TODO: Clamp values */
        case MOUSE_BUTTON_SCROLL_UP: {
            if(flashlight.is_enabled && flashlight.is_resizing && press) {
                flashlight.radius += ZOOM_AMOUNT * flashlight.radius;
            }
            else if (press) {
                camera.scale += ZOOM_AMOUNT * camera.scale;
            }
        } break;
        case MOUSE_BUTTON_SCROLL_DOWN: {
            if(flashlight.is_enabled && flashlight.is_resizing && press) {
                flashlight.radius -= ZOOM_AMOUNT * flashlight.radius;
            }
            else if (press) {
                camera.scale -= ZOOM_AMOUNT * camera.scale;
            }
        } break;
        default: break;
    }
}

void input_process_mouse_move(i32 x, i32 y) {
    if (camera.panning) {
        i32 dx = x - mouse_position.x;
        i32 dy = y - mouse_position.y;
        /* TODO: Clamp values */
        camera.position.x -= (float)dx / camera.scale;
        camera.position.y -= (float)dy / camera.scale;
    }
    mouse_position.x = x;
    mouse_position.y = y;
}


void input_process_key(keyboard_key_t key, b8 press) {
    switch(key) {
        case KEYBOARD_KEY_Q:
        case KEYBOARD_KEY_ESC: {
            running = false;
        } break;
        case KEYBOARD_KEY_F: {
            if (press)
            flashlight.is_enabled = !flashlight.is_enabled;
        } break;
        case KEYBOARD_KEY_LCTRL: {
            flashlight.is_resizing = press ? true : false;
        } break;

        default: break;
    }
}
