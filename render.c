#include <GL/gl.h>
#include <GLES3/gl3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "globals.h"
#include "platform.h"
#include "render.h"
#include "cd_clock.h"

static cd_clock render_clock;

#define MULTILINE_STR(...) #__VA_ARGS__
const char* fragment_shader_source = MULTILINE_STR(
\x23version 130\n
out mediump vec4 color;
in mediump vec2 texcoord;
uniform sampler2D tex;
uniform vec2 cursorPos;
uniform vec2 windowSize;
uniform float flShadow;
uniform float flRadius;
uniform float cameraScale;

void main()
{
    vec4 cursor = vec4(cursorPos.x, windowSize.y - cursorPos.y, 0.0, 1.0);
    color = mix(
        texture(tex, texcoord), vec4(0.0, 0.0, 0.0, 0.0),
        length(cursor - gl_FragCoord) < (flRadius * cameraScale) ? 0.0 : flShadow);
}
);

const char *vertex_shader_source = MULTILINE_STR(
\x23version 130\n
in vec3 aPos;
in vec2 aTexCoord;
out vec2 texcoord;

uniform vec2 cameraPos;
uniform float cameraScale;
uniform vec2 windowSize;
uniform vec2 screenshotSize;
uniform vec2 cursorPos;

vec3 to_world(vec3 v) {
    vec2 ratio = vec2(
        windowSize.x / screenshotSize.x / cameraScale,
        windowSize.y / screenshotSize.y / cameraScale);
    return vec3((v.x / screenshotSize.x * 2.0 - 1.0) / ratio.x,
                (v.y / screenshotSize.y * 2.0 - 1.0) / ratio.y,
                v.z);
}

void main()
{
	gl_Position = vec4(to_world((aPos - vec3(cameraPos * vec2(1.0, -1.0), 0.0))), 1.0);
	texcoord = aTexCoord;
}
);

static f32 lerp(f32 a, f32 b, f32 decay, f32 dt) {
    return b + (a - b) * exp(-decay * dt);
}

static GLuint newShader(const char *shader, GLenum kind) {
  GLuint result = glCreateShader(kind);

  const char *vertex_src[] = {shader};
  glShaderSource(result, 1, vertex_src, NULL);
  glCompileShader(result);

  GLint success;
  glGetShaderiv(result, GL_COMPILE_STATUS, &success);

  if (!success) {
    char info[512];
    glGetShaderInfoLog(result, 512, NULL, info);
    fprintf(stderr, "Fail to compile shader\n%s\n", info);
    exit(1);
  }

  return result;
}

static GLuint newShaderProgram(void) {
  GLuint result = glCreateProgram();
  GLuint vertex_shader = newShader(vertex_shader_source, GL_VERTEX_SHADER);
  GLuint fragment_shader = newShader(fragment_shader_source, GL_FRAGMENT_SHADER);

  glAttachShader(result, vertex_shader);
  glAttachShader(result, fragment_shader);

  glLinkProgram(result);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  GLint success;
  glGetProgramiv(result, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(result, 512, NULL, info);
    fprintf(stderr, "Failed to create shader program:\n%s\n", info);
    exit(1);
  }

  return result;
}

b8 render_initialize(void) {
    uint8_t *raw, comp;
    frame_t image_dimensions;
    if (!platform_screenshot(&raw, &comp, &image_dimensions)) {
        fprintf(stderr, "No image data\n");
        return false;
    }

    shader = newShaderProgram();
    glUseProgram(shader);

    GLuint vao, vbo, ebo;

    if (!platform_get_display_size(&image)) {
        fprintf(stderr, "Failed to get display size\n");
        return false;
    };
    GLfloat w = image.width;
    GLfloat h = image.height;

    GLfloat vertices[][5] = {
        {w, 0, 0.0, 1.0, 1.0}, // Top right
        {w, h, 0.0, 1.0, 0.0}, // Bottom right
        {0, h, 0.0, 0.0, 0.0}, // Bottom left
        {0, 0, 0.0, 0.0, 1.0}  // Top left
    };
    GLuint indices[] = {0, 1, 3, 1, 2, 3};

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices,
                 GL_STATIC_DRAW);

    GLsizei stride = 5 * sizeof(GLfloat);

    glVertexAttribPointer(0, 3, GL_FLOAT, 0, stride, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, 0, stride,
                          (const void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,
                 h, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                 raw);
    glGenerateMipmap(GL_TEXTURE_2D);

    glUniform1i(glGetUniformLocation(shader, "tex"), 0);

    glEnable(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    cd_clock_start(&render_clock);

    return true;
}

void render_deinitialize(void) {
}

#ifndef LERP_DECAY
#define LERP_DECAY 10.f
#endif

void render_draw(void) {
    static camera_t camera_state = {.scale = 1.f};
    static f64 last_elapsed = 0;
    cd_clock_update(&render_clock);
    f64 dt = render_clock.elapsed - last_elapsed;
    last_elapsed = render_clock.elapsed;
    /* printf("dt:%lf, FPS:%lf\n", dt, 1/dt); */
    camera_state.position.x = lerp(camera_state.position.x, camera.position.x, LERP_DECAY, dt);
    camera_state.position.y = lerp(camera_state.position.y, camera.position.y, LERP_DECAY, dt);
    camera_state.scale = lerp(camera_state.scale, camera.scale, LERP_DECAY, dt);

    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform2f(glGetUniformLocation(shader, "cameraPos"), camera_state.position.x,
                camera_state.position.y);
    glUniform1f(glGetUniformLocation(shader, "cameraScale"), camera_state.scale);
    glUniform2f(glGetUniformLocation(shader, "screenshotSize"), image.width,
                image.height);
    glUniform2f(glGetUniformLocation(shader, "windowSize"), window_size.width,
                window_size.height);
    glUniform2f(glGetUniformLocation(shader, "cursorPos"), mouse_position.x,
                mouse_position.y);
    //glUniform1f(glGetUniformLocation(shader, "flShadow"), flash_light.shadow);
    //glUniform1f(glGetUniformLocation(shader, "flRadius"), flash_light.radius);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    platform_gl_swap_buffers();
}

void render_process_resize(u32 x, u32 y) {
    glViewport(0, 0, x, y);
    window_size.width = x;
    window_size.height =y;
}
