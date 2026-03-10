#ifdef __linux__
#include "globals.h"
#include "input.h"
#include "render.h"
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/XF86keysym.h>

#include <X11/Xlib-xcb.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <X11/keysymdef.h>
#include <X11/XKBlib.h>

#include <GL/glx.h>

#include <stdbool.h>


typedef struct platform_state_s {
    Display *display;
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    GLXWindow glx_window;
    GLXContext context;
    xcb_window_t window;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_win;
    xcb_window_t server_last_focus_window;
    uint8_t server_revert_to;
} platform_state_t;

xcb_get_image_reply_t *image_reply;
static platform_state_t platform_state;

b8 platform_screenshot(u8 **raw, u8 *comp, frame_t *size) {
    if (!raw || !comp || !size) {
        fprintf(stderr, "Invalid arguments\n");
        return false;
    }

    xcb_screen_t *screen = platform_state.screen;
    xcb_get_image_cookie_t image_cookie = xcb_get_image(platform_state.connection,
                                                  XCB_IMAGE_FORMAT_Z_PIXMAP,
                                                  screen->root,
                                                  0,
                                                  0,
                                                  screen->width_in_pixels,
                                                  screen->height_in_pixels,
                                                  ~0u);
    image_reply = xcb_get_image_reply(platform_state.connection, image_cookie, NULL);
    if (!image_reply || 0 >= xcb_get_image_data_length(image_reply)) {
        fprintf(stderr,"get_image failed\n");
        return false;
    }

    *raw  = xcb_get_image_data(image_reply);
    *comp = image_reply->depth / 8 + 1;
    size->width  = screen->width_in_pixels;
    size->height  = screen->height_in_pixels;

    return true;
}

b8 platform_initialize(const char *window_name, b8 windowed) {
    int default_screen;

    /* Open Xlib Display */ 
    platform_state.display = XOpenDisplay(0);
    if(!platform_state.display)
    {
        fprintf(stderr, "Can't open display\n");
        return false;
    }

    default_screen = DefaultScreen(platform_state.display);

    /* Get the XCB connection from the display */
    platform_state.connection = 
        XGetXCBConnection(platform_state.display);
    if(!platform_state.connection)
    {
        XCloseDisplay(platform_state.display);
        fprintf(stderr, "Can't get xcb connection from display\n");
        return false;
    }

    /* Acquire event queue ownership */
    XSetEventQueueOwner(platform_state.display, XCBOwnsEventQueue);

    /* Find XCB screen */
    xcb_screen_iterator_t screen_iter = 
        xcb_setup_roots_iterator(xcb_get_setup(platform_state.connection));
    for(int screen_num = default_screen;
        screen_iter.rem && screen_num > 0;
        --screen_num, xcb_screen_next(&screen_iter));
    platform_state.screen = screen_iter.data;

        int visualID = 0;

    int glx_maj, glx_min;
    if (!glXQueryVersion(platform_state.display, &glx_maj, &glx_min)
        || glx_maj < 1
        || (glx_maj == 1 && glx_min < 3)) {
        fprintf(stderr, "Unsupported GLX version\n");
        exit(EXIT_FAILURE);
    }


    /* Query framebuffer configurations that match visual_attribs */
    GLXFBConfig *fb_configs = 0;
    int num_fb_configs = 0;
    static int visual_attribs[] =
        {
            GLX_X_RENDERABLE, True,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DOUBLEBUFFER, True,
            /*GLX_SAMPLE_BUFFERS  , 1, */
            /*GLX_SAMPLES         , 4, */
            None
        };

    fb_configs = glXChooseFBConfig(platform_state.display, default_screen, visual_attribs, &num_fb_configs);
    if(!fb_configs || num_fb_configs == 0)
    {
        fprintf(stderr, "glXGetFBConfigs failed\n");
        return false;
    }

    //printf("Found %d matching FB configs\n", num_fb_configs);

    /* Select first framebuffer config and query visualID */
    GLXFBConfig fb_config = fb_configs[0];
    glXGetFBConfigAttrib(platform_state.display, fb_config, GLX_VISUAL_ID , &visualID);


    /* Create OpenGL context */
    platform_state.context = glXCreateNewContext(platform_state.display, fb_config, GLX_RGBA_TYPE, 0, True);
    if(!platform_state.context)
    {
        fprintf(stderr, "glXCreateNewContext failed\n");
        return false;
    }
    XFree(fb_configs);

    /* Create XID's for colormap and window */
    xcb_colormap_t colormap = xcb_generate_id(platform_state.connection);
    platform_state.window = xcb_generate_id(platform_state.connection);

    /* Create colormap */
    xcb_create_colormap(
        platform_state.connection,
        XCB_COLORMAP_ALLOC_NONE,
        colormap,
        platform_state.screen->root,
        visualID
    );

    xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(platform_state.connection , xcb_get_geometry(platform_state.connection, platform_state.screen->root) , NULL);
    /* after getting 'geometry' (check for NULL) */
    if (!geometry) { fprintf(stderr,"geometry failed\n"); return false; }
    int width = geometry->width;
    int height = geometry->height;
    image.width = width;
    image.height = height;
    free(geometry);

    /* Create window */
    uint32_t eventmask =
        XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_EXPOSURE       |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION    |
        XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE       |
        XCB_EVENT_MASK_BUTTON_PRESS;
    b8 override_redirect = windowed ? 0 : 1;
    b8 save_under = windowed ? 0 : 1;
    /* Value list should be given in the order of xcb_cw_t enum */
    uint32_t valuelist[] = { override_redirect, save_under, eventmask, colormap, XCB_COLORMAP_ALLOC_NONE};
    uint32_t valuemask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_SAVE_UNDER | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

    xcb_create_window(
        platform_state.connection,
        XCB_COPY_FROM_PARENT,
        platform_state.window,
        platform_state.screen->root,
        0, 0,
        width, height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        visualID,
        valuemask,
        valuelist
    );
    /* Change the title */
    xcb_change_property(
        platform_state.connection,
        XCB_PROP_MODE_REPLACE,
        platform_state.window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,  /* data should be viewed 8 bits at a time */
        strlen(window_name),
        window_name);

    /* Tell the server to notify when the window manager */
    /* attempts to destroy the window. */
    xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(
        platform_state.connection,
        0,
        strlen("WM_DELETE_WINDOW"),
        "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(
        platform_state.connection,
        0,
        strlen("WM_PROTOCOLS"),
        "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* wm_delete_reply = xcb_intern_atom_reply(
        platform_state.connection,
        wm_delete_cookie,
        NULL);
    xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(
        platform_state.connection,
        wm_protocols_cookie,
        NULL);
    platform_state.wm_delete_win = wm_delete_reply->atom;
    platform_state.wm_protocols = wm_protocols_reply->atom;

    xcb_change_property(
        platform_state.connection,
        XCB_PROP_MODE_REPLACE,
        platform_state.window,
        wm_protocols_reply->atom,
        4,
        32,
        1,
        &wm_delete_reply->atom);
    free(wm_protocols_reply);
    free(wm_delete_reply);

    /* NOTE: window must be mapped before glXMakeContextCurrent */
    xcb_map_window(platform_state.connection, platform_state.window); 

    /* Create GLX Window */
    GLXDrawable drawable = 0;

    platform_state.glx_window = 
        glXCreateWindow(
            platform_state.display,
            fb_config,
            platform_state.window,
            0
        );

    if(!platform_state.window)
    {
        xcb_destroy_window(platform_state.connection, platform_state.window);
        glXDestroyContext(platform_state.display, platform_state.context);

        fprintf(stderr, "glXDestroyContext failed\n");
        return false;
    }

    drawable = platform_state.glx_window;

    /* make OpenGL context current */
    if(!glXMakeContextCurrent(platform_state.display, drawable, drawable, platform_state.context))
    {
        xcb_destroy_window(platform_state.connection, platform_state.window);
        glXDestroyContext(platform_state.display, platform_state.context);

        fprintf(stderr, "glXMakeContextCurrent failed\n");
        return false;
    }

    xcb_get_input_focus_cookie_t input_focus_cookie = xcb_get_input_focus(platform_state.connection);
    xcb_get_input_focus_reply_t *input_focus_reply = xcb_get_input_focus_reply(platform_state.connection, input_focus_cookie, NULL);
    platform_state.server_last_focus_window = input_focus_reply->focus;
    platform_state.server_revert_to = input_focus_reply->revert_to;
    free(input_focus_reply);
    xcb_set_input_focus(platform_state.connection, platform_state.server_revert_to, platform_state.window, XCB_CURRENT_TIME);

    return true;
}

void platform_deinitialize(void) {
    xcb_set_input_focus(platform_state.connection, platform_state.server_revert_to, platform_state.server_last_focus_window, XCB_CURRENT_TIME);

    glXDestroyWindow(platform_state.display, platform_state.glx_window);
    xcb_destroy_window(platform_state.connection, platform_state.window);

    glXDestroyContext(platform_state.display, platform_state.context);
    free(image_reply);

    XCloseDisplay(platform_state.display);
}

void platform_pump_messages(void) {
    xcb_generic_event_t *event = NULL;
    while ((event = xcb_poll_for_event(platform_state.connection))) {
        switch (event->response_type & XCB_EVENT_RESPONSE_TYPE_MASK) {
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {
                xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;
                mouse_button_t button = MOUSE_BUTTON_MAX_BUTTONS;
                switch (bp->detail) {
                    case XCB_BUTTON_INDEX_1:
                        button = MOUSE_BUTTON_LEFT;
                        break;
                    case XCB_BUTTON_INDEX_2:
                        button = MOUSE_BUTTON_MIDDLE;
                        break;
                    case XCB_BUTTON_INDEX_3:
                        button = MOUSE_BUTTON_RIGHT;
                        break;
                    case XCB_BUTTON_INDEX_4:
                        button = MOUSE_BUTTON_SCROLL_UP;
                        break;
                    case XCB_BUTTON_INDEX_5:
                        button = MOUSE_BUTTON_SCROLL_DOWN;
                        break;
                    case 8:
                        button = MOUSE_BUTTON_SIDE_BACKWARD;
                        break;
                    case 9:
                        button = MOUSE_BUTTON_SIDE_FORWARD;
                        break;
                }
                if (button != MOUSE_BUTTON_MAX_BUTTONS) input_process_button(button, bp->response_type == XCB_BUTTON_PRESS);
            } break;
            case XCB_MOTION_NOTIFY: {
                xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)event;

                input_process_mouse_move(motion->event_x, motion->event_y);
            } break;
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {
                xcb_key_release_event_t *kr = (xcb_key_release_event_t *)event;
                KeySym key_sym = XkbKeycodeToKeysym(
                    platform_state.display,
                    (KeyCode)kr->detail,
                    0,
                    (kr->state & ShiftMask)
                );

                b8 press = kr->response_type == XCB_KEY_PRESS;

                switch(key_sym) {
                    case XK_q:
                    case XK_Q: {
                        input_process_key(KEYBOARD_KEY_Q, press);
                    } break;
                    case XK_Escape: {
                        input_process_key(KEYBOARD_KEY_ESC, press);
                    } break;
                    case XK_f:
                    case XK_F: {
                        input_process_key(KEYBOARD_KEY_F, press);
                    } break;
                    case XK_Control_L: {
                        input_process_key(KEYBOARD_KEY_LCTRL, press);
                    } break;
                    default: break;
                }
            } break;
            case XCB_GRAPHICS_EXPOSURE:
            case XCB_EXPOSE: {
                xcb_expose_event_t *expose = (xcb_expose_event_t *)event;
                render_process_resize(expose->width, expose->height);
                /* Normally we should swap buffers as well but we swap buffers every frame instead */
            } break;
            case XCB_CLIENT_MESSAGE: {
                xcb_client_message_event_t* cm = (xcb_client_message_event_t*)event;

                /* Window close */
                if (cm->data.data32[0] == platform_state.wm_delete_win) {
                    running = false;
                }
            } break;
            /* Unknown/unused event type, ignore it */
            default: break;
        }
        free(event);
    }
}

void platform_gl_swap_buffers(void) {
    return glXSwapBuffers(platform_state.display, platform_state.glx_window);
}

b8 platform_get_display_size(frame_t *display_size) {
    xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(platform_state.connection , xcb_get_geometry(platform_state.connection, platform_state.screen->root) , NULL);
    if (!geometry) {
        fprintf(stderr,"xcb_get_geometry failed\n");
        return false;
    }
    display_size->width = geometry->width;
    display_size->height = geometry->height;
    free(geometry);

    return true;
}
#endif /* __linux__ */
