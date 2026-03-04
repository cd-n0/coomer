#ifndef INPUT_H_
#define INPUT_H_

typedef enum mouse_button_e {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_SCROLL_UP,
    MOUSE_BUTTON_SCROLL_DOWN,
    MOUSE_BUTTON_SIDE_FORWARD,
    MOUSE_BUTTON_SIDE_BACKWARD,
    MOUSE_BUTTON_MAX_BUTTONS
} mouse_button_t;

typedef enum keyboard_key_e {
    KEYBOARD_KEY_Q,
    KEYBOARD_KEY_ESC,
    KEYBOARD_KEY_F,
    KEYBOARD_KEY_LCTRL
} keyboard_key_t;

void input_process_button(mouse_button_t button, b8 press);
void input_process_key(keyboard_key_t key, b8 press);
void input_process_mouse_move(i32 x, i32 y);

#endif /* INPUT_H_ */
