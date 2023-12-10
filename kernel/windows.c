#include "windows.h"

#include "syscall.h"
#include "tasks.h"
#include "gui.h"
#include "sharedmem.h"
#include "util.h"

Window windows[MAX_WINDOWS];
int32_t currently_dragging_window = -1;
int32_t focused_window = -1;
int32_t window_under_cursor = -1;
bool window_under_cursor_inside_content = false;

int32_t window_z_order[MAX_WINDOWS]; // list of window indices, window_z_order[0] is frontmost window id
int32_t z_order_length = 0;

static int32_t find_window_slot();
int32_t get_framebuffer_shmem_id(int32_t window_id);
static int32_t swap_buffers(int32_t window_id);
static int32_t set_title(int32_t window_id, const char* title);
static void draw_window(int32_t id);
static int32_t z_order_find_index(int32_t window);
static void z_order_add_at(int32_t z_index, int32_t window);
static void z_order_remove_at(int32_t z_index);

void init_windows() {
    memset(windows, 0, sizeof(windows));

    for (int i = 0; i < MAX_WINDOWS; i++)
        window_z_order[i] = -1;
    
    register_syscall(SYSCALL_CREATE_WINDOW, create_window);
    register_syscall(SYSCALL_DESTROY_WINDOW, destroy_window);
    register_syscall(SYSCALL_GET_WINDOW_FB_SHMEM_ID, get_framebuffer_shmem_id);
    register_syscall(SYSCALL_WINDOW_SWAP_BUFFERS, swap_buffers);
    register_syscall(SYSCALL_SET_WINDOW_TITLE, set_title);
}

int32_t create_window(int32_t width, int32_t height, uint32_t flags) {
    int32_t index = find_window_slot();
    assert(windows[index].state == 0);

    memset(&windows[index], 0, sizeof(Window));

    windows[index].state = 1;
    windows[index].x = 30 + index * 40;
    windows[index].y = 50 + index * 35;
    windows[index].width = width;
    windows[index].height = height;
    windows[index].flags = flags;
    windows[index].actual_width = width + 2;
    windows[index].actual_height = height + WINDOW_CONTENT_YOFFSET + 1;
    uint32_t fb_bytes = width * height * 4;
    windows[index].framebuffer_size_bytes = fb_bytes;
    windows[index].fb_shmem_id = sharedmem_create(fb_bytes * (flags & WINDOW_FLAG_DOUBLE_BUFFERED ? 2 : 1), 0);
    windows[index].framebuffer = sharedmem_map(windows[index].fb_shmem_id, 0);
    windows[index].shown_buffer = 0;
    windows[index].owner_task_id = current_task->id;

    strncpy(windows[index].title, "Cool Titles!", WINDOW_TITLE_MAX_LENGTH);

    window_z_order[z_order_length++] = index;

    move_window_to_front(index);
    focused_window = index;

    gui.needs_redraw = true;

    return index;
}

void destroy_window(int32_t window_id) {
    Window* w = &windows[window_id];
    sharedmem_destroy(w->fb_shmem_id);

    memset(&windows[window_id], 0, sizeof(Window));

    int index = z_order_find_index(window_id);
    if (index != -1) {
        z_order_remove_at(index);
    }

    gui.needs_redraw = true;
}

static int32_t find_window_slot() {
    for (int32_t i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].state == 0) {
            return i;
        }
    }
    assert(0);
}

int32_t get_framebuffer_shmem_id(int32_t window_id) {
    return windows[window_id].fb_shmem_id;
}

static int32_t swap_buffers(int32_t window_id) {
    gui.needs_redraw = true;

    Window* w = &windows[window_id];
    if (w->flags & WINDOW_FLAG_DOUBLE_BUFFERED) {
        w->shown_buffer ^= 1;

        // put task in wait state and yield
        assert(current_task->state == TASK_STATE_READY);
        current_task->state = TASK_STATE_WAIT_FOR_REDRAW;
        task_schedule();
        assert(current_task->state != TASK_STATE_WAIT_FOR_REDRAW);
    }
    
    return w->shown_buffer;
}

static int32_t set_title(int32_t window_id, const char* title) {
    Window* w = get_window(window_id);
    assert(w);

    memset(w->title, 0, sizeof(w->title));
    strncpy(w->title, title, sizeof(w->title) - 1);
}

static void draw_window(int32_t id) {
    Window* w = &windows[id];

    uint32_t border_color = focused_window == id ? 0x303030 : 0x101010;

    // copy the contents of the framebuffer to the screen
    uint32_t* source = ((uint32_t) w->framebuffer) + (w->shown_buffer == 0 ? 0 : w->framebuffer_size_bytes);
    graphics_copy_rect(w->x + WINDOW_CONTENT_XOFFSET, w->y + WINDOW_CONTENT_YOFFSET, w->width, w->height, 0, 0, source);

    // window title bar
    graphics_fill_rect(w->x + 1, w->y + 1, w->width, WINDOW_CONTENT_YOFFSET - 1, border_color);

    // window outline
    graphics_draw_hline(w->x, w->y, w->width + WINDOW_CONTENT_XOFFSET * 2, border_color);
    graphics_draw_hline(w->x, w->y + WINDOW_CONTENT_YOFFSET + w->height, w->width + WINDOW_CONTENT_XOFFSET * 2, border_color);
    graphics_draw_vline(w->x, w->y, w->height + WINDOW_CONTENT_YOFFSET, border_color);
    graphics_draw_vline(w->x + w->width + 1, w->y, w->height + WINDOW_CONTENT_YOFFSET, border_color);

    // window title
    graphics_draw_string(w->title, w->x + 5, w->y + 5, 0xFFFFFF);

    // close button
    int32_t close_button_x = w->x + w->width + 2 - CLOSE_BUTTON_WIDTH - 1;
    int32_t close_button_y = w->y + 1;
    graphics_fill_rect(close_button_x, close_button_y, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT, 0xFFFFFF);
    graphics_draw_string("x", close_button_x + 5, close_button_y + 3, 0);

    // signal task
    push_cli();
    Task* task = get_task(w->owner_task_id);
    if (task->state == TASK_STATE_WAIT_FOR_REDRAW) {
        task->state = TASK_STATE_READY;
    }
    pop_cli();
}

bool check_window_close(int32_t window, int32_t x, int32_t y) {
    Window* w = get_window(window);

    int32_t close_button_x = w->x + w->width + 2 - CLOSE_BUTTON_WIDTH - 1;
    int32_t close_button_y = w->y + 1;

    if (x < close_button_x) return false;
    if (y < close_button_y) return false;
    if (x >= close_button_x + CLOSE_BUTTON_WIDTH) return false;
    if (y >= close_button_y + CLOSE_BUTTON_HEIGHT) return false;

    return true;
}

int32_t find_window_from_pos(int32_t x, int32_t y, bool* inside_content) {
    *inside_content = false;
    for (int i = 0; i < z_order_length; i++) {
        int32_t id = window_z_order[i];
        Window* w = get_window(id);
        if (w->state == 0)
            continue;

        if (w->x > x) continue;
        if (w->y > y) continue;
        if (w->x + w->actual_width < x) continue;
        if (w->y + w->actual_height < y) continue;

        if (y - w->y > WINDOW_CONTENT_YOFFSET)
            *inside_content = true;
        return id;
    }
    return -1;
}

Window* get_focused_window() {
    if (focused_window == -1)
        return NULL;
    
    return &windows[focused_window];
}

void destroy_all_windows_belonging_to(int32_t task_id) {
    // todo
}

void draw_windows() {
    for (int i = z_order_length - 1; i >= 0; i--) {
        int32_t id = window_z_order[i];

        Window* w = &windows[id];
        if (!w->state)
            continue;

        draw_window(id);
    }
}

Window* get_window(int32_t id) {
    if (id < 0 || id >= MAX_WINDOWS)
        return NULL;
    return &windows[id];
}

static int32_t z_order_find_index(int32_t window) {
    for (int32_t i = 0; i < MAX_WINDOWS; i++) {
        if (window_z_order[i] == window) {
            return i;
        }
    }
    return -1;
}

static void z_order_add_at(int32_t z_index, int32_t window) {
    // todo
}

static void z_order_remove_at(int32_t z_index) {
    // shift windows in the range [index, z_order_length] to the left once
    for (int32_t i = z_index; i < z_order_length; i++) {
        window_z_order[i] = window_z_order[i + 1];
    }
    z_order_length--;
}

void move_window_to_front(int32_t window) {
    if (window_z_order[0] == window)
        return;
    assert(window >= 0 && window < MAX_WINDOWS);

    int32_t index = z_order_find_index(window);
    assert(index != -1);

    // shift windows in the range [0, index - 1] to the right once
    for (int i = index - 1; i >= 0; i--) {
        window_z_order[i + 1] = window_z_order[i];
    }

    window_z_order[0] = window;
}
