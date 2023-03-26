
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h> // must be included before egl.h

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "WaylandGLES2.h"

#define WINDOW_WIDTH                1024
#define WINDOW_HEIGHT               768

typedef struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_surface *surface;
    struct wl_egl_window *egl_window;
    struct wl_region *region;
    struct wl_shell *shell;
    struct wl_shell_surface *shell_surface;
} wayland_data;

typedef struct {
    EGLNativeDisplayType native_display;
    EGLNativeWindowType native_window;
    int width;
    int height;
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
} egl_data;

wayland_data wayland;
egl_data egl;

void my_registry_add_object(void *data, struct wl_registry *registry, uint32_t name,
        const char *interface, uint32_t version) {
    wayland_data *wayland = data;

    printf("Got a registry event for %s: %u\n", interface, name);

    if (!strcmp(interface, "wl_compositor")) {
        wayland->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    } else if (!strcmp(interface, "wl_shell")) {
        wayland->shell = wl_registry_bind(registry, name, &wl_shell_interface, 1);
    }
}

void my_registry_remove_object(void *data, struct wl_registry *registry, uint32_t name) {
    printf("Got a registry losing event for %u\n", name);
}

const struct wl_registry_listener my_registry_listeners = {
    my_registry_add_object,
    my_registry_remove_object
};

void my_shell_surface_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial) {
    wl_shell_surface_pong(shell_surface, serial);
}

void my_shell_surface_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges,
        int32_t width, int32_t height) {
    wayland_data *wayland = data;

    wl_egl_window_resize(wayland->egl_window, width, height, 0, 0);
}

void my_shell_surface_popup_done(void *data, struct wl_shell_surface *shell_surface) {
    //
}

const struct wl_shell_surface_listener my_shell_surface_listeners = {
    my_shell_surface_ping,
    my_shell_surface_configure,
    my_shell_surface_popup_done
};

int main(int argc, char *argv[]) {
    printf("%s starting ...\n", argv[0]);

    memset(&wayland, 0, sizeof(wayland));
    memset(&egl, 0, sizeof(egl));

    wayland.display = wl_display_connect(NULL);
    if (wayland.display == NULL) {
        printf("wl_display_connect error: -1\n");
        return -1;
    }

    wayland.registry = wl_display_get_registry(wayland.display);
    wl_registry_add_listener(wayland.registry, &my_registry_listeners, &wayland);

    // call my_global_registry_handler to get compositor and shell

    wl_display_dispatch(wayland.display);
    wl_display_roundtrip(wayland.display);
    if (!wayland.compositor || !wayland.shell) {
        printf("wl_registry_bind compositor/shell error: -2\n");
        return -2;
    }

    egl.native_display = wayland.display;

    wayland.surface = wl_compositor_create_surface(wayland.compositor);
    if (!wayland.surface) {
        printf("wl_compositor_create_surface error: -3\n");
        return -3;
    }

    wayland.shell_surface = wl_shell_get_shell_surface(wayland.shell, wayland.surface);
    wl_shell_surface_add_listener(wayland.shell_surface, &my_shell_surface_listeners, &wayland);
    wl_shell_surface_set_toplevel(wayland.shell_surface);

    // create window

    wayland.region = wl_compositor_create_region(wayland.compositor);
    wl_surface_set_opaque_region(wayland.surface, wayland.region);

    wayland.egl_window = wl_egl_window_create(wayland.surface, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (wayland.egl_window == EGL_NO_SURFACE) {
        printf("wl_egl_window_create error: -4\n");
        return -4;
    }

    egl.width = WINDOW_WIDTH;
    egl.height = WINDOW_HEIGHT;
    egl.native_window = wayland.egl_window;

    // create EGL context

    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLConfig config;
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

    egl.display = eglGetDisplay(egl.native_display);
    if (egl.display == EGL_NO_DISPLAY) {
        printf("eglGetDisplay error: -5\n");
        return -5;
    }

    if (!eglInitialize(egl.display, &majorVersion, &minorVersion)) {
        printf("eglInitialize error: -6\n");
        return -6;
    }

    if ((eglGetConfigs(egl.display, NULL, 0, &numConfigs) != EGL_TRUE) || (numConfigs == 0)) {
        printf("eglGetConfigs error: -7\n");
        return -7;
    }

    if ((eglChooseConfig(egl.display, configAttribs, &config, 1, &numConfigs) != EGL_TRUE) || (numConfigs != 1)) {
        printf("eglChooseConfig error: -8\n");
        return -8;
    }

    egl.surface = eglCreateWindowSurface(egl.display, config, egl.native_window, NULL);
    if (egl.surface == EGL_NO_SURFACE) {
        printf("eglCreateWindowSurface error: -9\n");
        return -9;
    }

    egl.context = eglCreateContext(egl.display, config, EGL_NO_CONTEXT, contextAttribs);
    if (egl.context == EGL_NO_CONTEXT) {
        printf("eglCreateContext error: -10\n");
        return -10;
    }

    if (!eglMakeCurrent(egl.display, egl.surface, egl.surface, egl.context)) {
        printf("eglMakeCurrent error: -11\n");
        return -11;
    }

    while (1) {
        // let other windows handle their events
        wl_display_dispatch_pending(wayland.display);

        // clear screen by green color using OpenGL ES 2 API
        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // you can add here your render code

        usleep(500000);

        eglSwapBuffers(egl.display, egl.surface);
    }

    wl_display_disconnect(wayland.display);

    printf("%s cleanup ...\n", argv[0]);
    printf("%s exiting ...\n", argv[0]);

    return 0;
}
