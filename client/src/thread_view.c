#include <stdlib.h>
#include <font/fzx.h>
#include "zxgui.h"
#include "zxgui_internal.h"
#include "scenes.h"
#include "system.h"
#include "netlog.h"

#include "channels_proto.h"
#include "proto_asserts.h"
#include "channels.h"
#include "heap.h"

#define ENTRIES_PER_REQUEST (20)
#define SELECTED_THREAD_COLOR (BRIGHT | INK_BLACK | PAPER_YELLOW)
#define DESELECTED_THREAD_COLOR (BRIGHT | INK_WHITE | PAPER_BLACK)

static struct gui_scene_t thread_scene;
static struct gui_object_t* last_thread_static_object;

static struct gui_scene_t post_scene;
static struct gui_object_t* last_post_static_object;

static struct gui_scene_t scene_full_image;
static struct gui_button_t button_exit_full_image;
static struct gui_button_t button_grayscale_full_image;

static uint8_t first_object = 0;
static uint8_t grayscale = 0;
static uint8_t requests_locked = 0;
static uint8_t flush = 0;
static uint16_t entry_offset = 0;
static uint8_t process_screen_h = 0;
static uint8_t process_screen_num = 0;
static uint8_t current_screen = 0;
static uint8_t num_entries_total = 0;
static uint8_t entry_count_recv = 0;
static uint8_t* display_ptr = NULL;
static uint8_t display_row_num = 0;
static uint8_t recv_color_now = 0;
static uint8_t* display_row_ptr = NULL;
static uint8_t* display_color_ptr = NULL;
static uint8_t* display_color_row_ptr = NULL;
static uint16_t receive_w = 0;
static uint8_t auto_select_last = 0;
static char thread_title[64];
static char post_title[96];
static uint8_t post_mode = 0;

struct channel_entry_t
{
    char id[64];
    struct gui_label_t id_label;
    struct gui_dynamic_label_t comment_label;
    uint8_t has_attachment;
    uint16_t attachment_w;
    uint16_t attachment_h;
    uint8_t screen_y;
    uint8_t height;
    uint8_t screen_num;
    uint8_t offset;
    struct channel_entry_t* next;
    struct channel_entry_t* prev;
    uint8_t comment_blob_id;
};

static struct channel_entry_t entries[ENTRIES_PER_REQUEST];
static uint8_t last_allocated_entry = 0;
static struct channel_entry_t* selected_entry;
static struct channel_entry_t* first_display_entry;
static struct channel_entry_t* first_entry;
static struct channel_entry_t* last_entry;
static struct channel_entry_t* attachment_being_requested;
static struct channel_entry_t* stop_attachment_requests_at;

static void free_view()
{
    /*
     * destroy all subsequent object by doing this simple move
     * they're all allocated on heap (part of the channel_thread_t structure) and will be destroyed below anyway
     */
    last_thread_static_object->next = NULL;
    last_post_static_object->next = NULL;
    last_allocated_entry = 0;

    // destroy all allocations with this simple trick
    reset_heap();
    reset_heap_blobs();

    attachment_being_requested = NULL;
    first_display_entry = NULL;
    selected_entry = NULL;
    first_entry = NULL;
    last_entry = NULL;
}

static void process_attachment_image_color(uint8_t* d, uint16_t r)
{
    while (r)
    {
        uint8_t allow = r > receive_w ? receive_w : r;

        memcpy(display_color_ptr, d, allow);

        d += allow;
        r -= allow;
        display_color_ptr += allow;
        receive_w -= allow;

        if (receive_w == 0)
        {
            display_color_row_ptr += 32;
            display_color_ptr = display_color_row_ptr;
            receive_w = attachment_being_requested->attachment_w;
        }
    }
}

static void process_attachment_image(ChannelObject* object)
{
    if (first_object)
    {
        attachment_being_requested->attachment_w = get_uint16_property(object, 'w', 0) / 8;
        attachment_being_requested->attachment_h = get_uint16_property(object, 'h', 0) / 8;

        receive_w = attachment_being_requested->attachment_w;
        first_object = 0;
        return;
    }

    ChannelObjectProperty* prop = find_property(object, OBJ_PROPERTY_PAYLOAD);

    if (prop)
    {
        uint16_t r = prop->value_size;
        uint8_t* d = (uint8_t*)prop->value;

        if (recv_color_now)
        {
            process_attachment_image_color(d, r);
            return;
        }

        while (r)
        {
            uint8_t allow = r > receive_w ? receive_w : r;

            memcpy(display_row_ptr, d, allow);
            d += allow;
            r -= allow;
            display_row_ptr += allow;
            receive_w -= allow;

            if (receive_w == 0)
            {
                display_row_num++;
                display_ptr = zx_saddrpdown(display_ptr);
                display_row_ptr = display_ptr;
                receive_w = attachment_being_requested->attachment_w;

                if (display_row_num == attachment_being_requested->attachment_h * 8)
                {
                    recv_color_now = 1;
                    process_attachment_image_color(d, r);
                    break;
                }
            }
        }
    }
}

static void highlight_thread(struct channel_entry_t* th)
{
    if (selected_entry)
    {
        zxgui_screen_color(DESELECTED_THREAD_COLOR);
        zxgui_screen_recolor(0, selected_entry->screen_y, 32, 1);
    }

    selected_entry = th;

    zxgui_screen_color(SELECTED_THREAD_COLOR);
    zxgui_screen_recolor(0, selected_entry->screen_y, 32, 1);
}

static void get_entries_error(const char* error)
{
    free_view();

    netlog("%s\n", error);
    switch_alert(error, switch_connect_to_proxy);
}

static void switch_back_to_boards(struct gui_button_t* this)
{
    if (requests_locked)
        return;

    free_view();
    entry_offset = 0;
    switch_select_board();
}

static void switch_back_to_threads(struct gui_button_t* this)
{
    if (requests_locked)
        return;

    switch_thread_view();
}

static void redraw_screen();

static void exit_fullscreen_image(struct gui_button_t* this)
{
    redraw_screen();
}

static void get_fullscreen_image_response(struct proto_process_t* proto)
{
    zxgui_screen_color(INK_WHITE | BRIGHT | PAPER_BLACK);
    zxgui_screen_clear(0, 23, 32, 1);

    object_invalidate(&button_exit_full_image, GUI_FLAG_DIRTY);
    object_invalidate(&button_grayscale_full_image, GUI_FLAG_DIRTY);
}

static void process_fullscreen_image(ChannelObject* object)
{
    if (first_object)
    {
        first_object = 0;
        return;
    }

    ChannelObjectProperty* prop = find_property(object, OBJ_PROPERTY_PAYLOAD);

    if (prop)
    {
        memcpy(display_ptr, prop->value, prop->value_size);
        display_ptr += prop->value_size;
    }
}

static void get_fullscreen_image_error(const char* error)
{
    netlog("%s\n", error);
    switch_alert(error, redraw_screen);
}

static void open_full_picture(struct gui_button_t* this)
{
    if (requests_locked)
        return;

    zxgui_scene_set(&scene_full_image);

    if (this == &button_grayscale_full_image)
    {
        grayscale = 1 - grayscale;
    }

    first_object = 1;
    display_ptr = (uint8_t*)0x4000;
    uint16_t target_w = 256;
    uint16_t target_h = 192;

    if (grayscale)
    {
        zxgui_screen_color(INK_WHITE | PAPER_BLACK);
        zxgui_screen_recolor(0, 0, 32, 24);

        button_grayscale_full_image.title = "COLOR";
    }
    else
    {
        button_grayscale_full_image.title = "GRAYSCALE";
    }

    const char *tid;

    if (post_mode)
    {
        tid = channels_get_thread();
    }
    else
    {
        tid = selected_entry->id;
    }

    declare_str_property_on_stack(key, OBJ_PROPERTY_ID, "image", NULL);
    declare_str_property_on_stack(channel, 'c', channels_get_channel(), &key);
    declare_str_property_on_stack(board_, 'b', channels_get_board(), &channel);
    declare_str_property_on_stack(thread_, 't', tid, &board_);
    declare_str_property_on_stack(post_, 'p', selected_entry->id, &thread_);
    declare_arg_property_on_stack(target_w_, 'w', target_w, &post_);
    declare_arg_property_on_stack(target_h_, 'h', target_h, &target_w_);
    declare_str_property_on_stack(encoding_, 'e', (grayscale ? "grayscale_zx" : "color_zx"), &target_h_);

    declare_object_on_stack(request, 128, &encoding_);

    if (channels_send_request(request, process_fullscreen_image, get_fullscreen_image_response, get_fullscreen_image_error))
    {
        switch_alert("Disconnected from Proxy", switch_connect_to_proxy);
        return;
    }
}

static void fetch_next_thread_attachment();

static const char* comment_obtain_data(struct gui_dynamic_label_t* this)
{
    struct channel_entry_t* entry = (struct channel_entry_t*)this->user;
    return (const char*)open_heap_blob(entry->comment_blob_id);
}

static void comment_release_data(struct gui_dynamic_label_t* this)
{
    close_heap_blob();
}

static void redraw_screen()
{
    current_screen = first_display_entry->screen_num;

    if (post_mode)
    {
        sprintf(post_title, "%s %s %s %d/%d", channels_get_channel(), channels_get_board(),
            channels_get_thread(), first_display_entry->offset + 1, num_entries_total);
    }
    else
    {
        sprintf(thread_title, "%s %s %d/%d", channels_get_channel(), channels_get_board(),
            first_display_entry->offset + 1, num_entries_total);
    }

    last_thread_static_object->next = NULL;
    last_post_static_object->next = NULL;

    struct gui_scene_t* scene = post_mode ? &post_scene : &thread_scene;
    zxgui_scene_set(scene);

    struct channel_entry_t* th = first_display_entry;
    attachment_being_requested = first_display_entry;

    while (th)
    {
        if (th->screen_num != current_screen)
        {
            break;
        }

        uint8_t c;
        if (selected_entry == th)
        {
            c = SELECTED_THREAD_COLOR;
        }
        else
        {
            c = DESELECTED_THREAD_COLOR;
        }

        uint8_t h = th->screen_y;
        zxgui_label_init(&th->id_label, 0, h, 8, 1, th->id, c, 0);
        zxgui_scene_add(scene, &th->id_label);

        h++;

        uint8_t x_, w_;
        if (th->has_attachment)
        {
            x_ = 10;
            w_ = 22;
        }
        else
        {
            x_ = 0;
            w_ = 32;
        }

        zxgui_dynamic_label_init(&th->comment_label, x_, h, w_, th->height, INK_WHITE | PAPER_BLACK,
            GUI_FLAG_MULTILINE, comment_obtain_data, comment_release_data, th);

        zxgui_scene_add(scene, &th->comment_label);

        th = th->next;
    }

    stop_attachment_requests_at = th ? th->next : NULL;

    zxgui_screen_color(SELECTED_THREAD_COLOR);
    zxgui_screen_recolor(0, selected_entry->screen_y, 32, 1);

    fetch_next_thread_attachment();
}

static uint8_t request();

static void select_next_entry(struct gui_button_t* this)
{
    if (selected_entry->next)
    {
        if (selected_entry->next->screen_num != current_screen)
        {
            if (requests_locked)
                return;

            first_display_entry = selected_entry->next;
            selected_entry = selected_entry->next;
            redraw_screen();
        }
        else
        {
            highlight_thread(selected_entry->next);
        }
    }
    else
    {
        if (entry_offset + ENTRIES_PER_REQUEST < num_entries_total)
        {
            if (requests_locked)
                return;

            entry_offset += ENTRIES_PER_REQUEST;
            auto_select_last = 0;
            request();
        }
    }
}

static void select_prev_entry(struct gui_button_t* this)
{
    if (selected_entry->prev)
    {
        if (selected_entry->prev->screen_num != current_screen)
        {
            if (requests_locked)
                return;

            selected_entry = selected_entry->prev;
            uint8_t rewind_to = selected_entry->screen_num;

            first_display_entry = first_entry;
            while (first_display_entry)
            {
                if (first_display_entry->screen_num == rewind_to)
                {
                    break;
                }

                first_display_entry = first_display_entry->next;
            }

            redraw_screen();
        }
        else
        {
            highlight_thread(selected_entry->prev);
        }
    }
    else
    {
        if (entry_offset)
        {
            if (requests_locked)
                return;

            entry_offset -= ENTRIES_PER_REQUEST;
            auto_select_last = 1;
            request();
        }
    }
}

static void open_thread(struct gui_button_t* this)
{
    if (requests_locked)
        return;

    channels_set_thread(selected_entry->id);

    netlog("Opening thread %s/%s/%s\n", channels_get_channel(), channels_get_board(), channels_get_thread());

    post_mode = 1;
    entry_offset = 0;

    if (request() == 0)
    {
        switch_progress("Opening Thread", NULL);
    }
}

static void get_attachment_error(const char* error)
{
    requests_locked = 0;

    static uint8_t frames[] = {
        QUESTION_MARK_1,
        QUESTION_MARK_2,
        QUESTION_MARK_3,
        QUESTION_MARK_4
    };

    zxgui_icon(INK_RED | PAPER_BLACK,
        4, attachment_being_requested->screen_y +
        attachment_being_requested->attachment_h / 2, 2, 2, frames);

    netlog("Error fetching attachment: %s\n", error);

    attachment_being_requested = attachment_being_requested->next;

    fetch_next_thread_attachment();
}

static void get_attachment_response(struct proto_process_t* proto)
{
    requests_locked = 0;
    attachment_being_requested = attachment_being_requested->next;
    fetch_next_thread_attachment();
}

static void fetch_next_thread_attachment()
{
    if (attachment_being_requested == NULL || attachment_being_requested == stop_attachment_requests_at)
    {
        return;
    }

    while (!attachment_being_requested->has_attachment)
    {
        attachment_being_requested = attachment_being_requested->next;

        if (attachment_being_requested == NULL || attachment_being_requested == stop_attachment_requests_at)
        {
            return;
        }
    }

    if (attachment_being_requested->screen_num != current_screen)
    {
        return;
    }

    first_object = 1;
    recv_color_now = 0;
    display_row_num = 0;
    display_ptr = zx_cxy2saddr(0, attachment_being_requested->screen_y + 1);
    display_color_ptr = zx_cxy2aaddr(0, attachment_being_requested->screen_y + 1);
    display_row_ptr = display_ptr;
    display_color_row_ptr = display_color_ptr;

    static uint8_t frames[] = {
        GUI_ICON_LOADING_A_1,
        GUI_ICON_LOADING_A_2,
        GUI_ICON_LOADING_A_3,
        GUI_ICON_LOADING_A_4
    };

    zxgui_icon(INK_WHITE | PAPER_BLACK, 4,
        attachment_being_requested->screen_y +
        attachment_being_requested->attachment_h / 2, 2, 2, frames);

    uint16_t target_w = attachment_being_requested->attachment_w * 8;
    uint16_t target_h = attachment_being_requested->attachment_h * 8;

    const char* thid;

    if (post_mode)
    {
        thid = channels_get_thread();
    }
    else
    {
        thid = attachment_being_requested->id;
    }

    declare_str_property_on_stack(key, OBJ_PROPERTY_ID, "image", NULL);
    declare_str_property_on_stack(channel, 'c', channels_get_channel(), &key);
    declare_str_property_on_stack(board_, 'b', channels_get_board(), &channel);
    declare_str_property_on_stack(thread_, 't', thid, &board_);
    declare_str_property_on_stack(post_, 'p', attachment_being_requested->id, &thread_);
    declare_str_property_on_stack(encoding_, 'e', "color_zx", &post_);
    declare_arg_property_on_stack(target_w_, 'w', target_w, &encoding_);
    declare_arg_property_on_stack(target_h_, 'h', target_h, &target_w_);

    declare_object_on_stack(request, 160, &target_h_);

    if (channels_send_request(request, process_attachment_image, get_attachment_response, get_attachment_error))
    {
        return;
    }

    requests_locked = 1;
}

static void init_thread_scene()
{
    zxgui_scene_init(&thread_scene, NULL);

    {
        static struct gui_label_t thread_label;
        zxgui_label_init(&thread_label, 23, 23, 9, 1, thread_title, INK_GREEN | PAPER_BLACK, 0);
        zxgui_scene_add(&thread_scene, &thread_label);
    }

    {
        static struct gui_button_t button_cancel;
        zxgui_button_init(&button_cancel, 0, 23, 0, 1, 32, GUI_ICON_SPACE, "BACK", switch_back_to_boards);
        zxgui_scene_add(&thread_scene, &button_cancel);
    }

    {
        static struct gui_button_t button_thread;
        zxgui_button_init(&button_thread, 4, 23, 0, 1, 13, GUI_ICON_RETURN, "OPEN", open_thread);
        zxgui_scene_add(&thread_scene, &button_thread);
    }

    {
        static struct gui_button_t button_picture;
        zxgui_button_init(&button_picture, 8, 23, 0, 1, 'p', GUI_ICON_P, "PICTURE", open_full_picture);
        zxgui_scene_add(&thread_scene, &button_picture);
    }

    {
        static struct gui_button_t next_thread;
        zxgui_button_init(&next_thread, 13, 23, 1, 1, 10, GUI_ICON_MORE_TO_FOLLOW, NULL, select_next_entry);
        zxgui_scene_add(&thread_scene, &next_thread);
    }

    {
        static struct gui_button_t next_thread_6;
        zxgui_button_init(&next_thread_6, 14, 23, 4, 1, '6', GUI_ICON_6, "NEXT", select_next_entry);
        zxgui_scene_add(&thread_scene, &next_thread_6);
    }

    {
        static struct gui_button_t prev_thread;
        zxgui_button_init(&prev_thread, 18, 23, 1, 1, 11, GUI_ICON_LESS_TO_FOLLOW, NULL, select_prev_entry);
        zxgui_scene_add(&thread_scene, &prev_thread);
    }

    {
        static struct gui_button_t prev_thread_7;
        zxgui_button_init(&prev_thread_7, 19, 23, 4, 1, '7', GUI_ICON_7, "PREV", select_prev_entry);
        zxgui_scene_add(&thread_scene, &prev_thread_7);
    }

    last_thread_static_object = zxgui_scene_get_last_object(&thread_scene);
}

static void init_post_scene()
{
    zxgui_scene_init(&post_scene, NULL);

    {
        static struct gui_label_t post_label;
        zxgui_label_init(&post_label, 19, 23, 13, 1, post_title, INK_GREEN | PAPER_BLACK, 0);
        zxgui_scene_add(&post_scene, &post_label);
    }

    {
        static struct gui_button_t button_cancel;
        zxgui_button_init(&button_cancel, 0, 23, 0, 1, 32, GUI_ICON_SPACE, "BACK", switch_back_to_threads);
        zxgui_scene_add(&post_scene, &button_cancel);
    }

    {
        static struct gui_button_t button_picture;
        zxgui_button_init(&button_picture, 4, 23, 0, 1, 'p', GUI_ICON_P, "PICTURE", open_full_picture);
        zxgui_scene_add(&post_scene, &button_picture);
    }

    {
        static struct gui_button_t next_thread;
        zxgui_button_init(&next_thread, 9, 23, 1, 1, 10, GUI_ICON_MORE_TO_FOLLOW, NULL, select_next_entry);
        zxgui_scene_add(&post_scene, &next_thread);
    }

    {
        static struct gui_button_t next_thread_6;
        zxgui_button_init(&next_thread_6, 10, 23, 4, 1, '6', GUI_ICON_6, "NEXT", select_next_entry);
        zxgui_scene_add(&post_scene, &next_thread_6);
    }

    {
        static struct gui_button_t prev_thread;
        zxgui_button_init(&prev_thread, 14, 23, 1, 1, 11, GUI_ICON_LESS_TO_FOLLOW, NULL, select_prev_entry);
        zxgui_scene_add(&post_scene, &prev_thread);
    }

    {
        static struct gui_button_t prev_thread_7;
        zxgui_button_init(&prev_thread_7, 15, 23, 4, 1, '7', GUI_ICON_7, "PREV", select_prev_entry);
        zxgui_scene_add(&post_scene, &prev_thread_7);
    }

    last_post_static_object = zxgui_scene_get_last_object(&post_scene);
}

void init_thread_view()
{
    init_thread_scene();
    init_post_scene();

    {
        zxgui_button_init(&button_exit_full_image, 0, 23, 0, 1, 32, GUI_ICON_SPACE, "BACK", exit_fullscreen_image);
        zxgui_scene_add(&scene_full_image, &button_exit_full_image);
    }

    {
        zxgui_button_init(&button_grayscale_full_image, 8, 23, 0, 1, 'p', GUI_ICON_P, "GRAYSCALE", open_full_picture);
        zxgui_scene_add(&scene_full_image, &button_grayscale_full_image);
    }
}

static void process_entry(ChannelObject* object)
{
    if (first_object)
    {
        first_object = 0;
        ChannelObjectProperty* threads = find_property(object, 'c');
        if (threads)
        {
            char buf[16];
            buf[threads->value_size] = 0;
            memcpy(buf, threads->value, threads->value_size);

            num_entries_total = atoi(buf);
        }
    }

    ChannelObjectProperty* thead_id = find_property(object, OBJ_PROPERTY_ID);
    ChannelObjectProperty* comment = find_property(object, OBJ_PROPERTY_COMMENT);

    if (thead_id && comment)
    {
        proto_assert_str(last_allocated_entry < ENTRIES_PER_REQUEST, "Cannot allocate entry");
        struct channel_entry_t* th = &entries[last_allocated_entry++];
        memset((void*)th, 0, sizeof(struct channel_entry_t));

        uint16_t w = get_uint16_property(object, 'w', 0);
        uint16_t h = get_uint16_property(object, 'h', 0);

        uint16_t label_w;

        if (w && h)
        {
            th->has_attachment = 1;
            th->attachment_w = 10;
            th->attachment_h = (uint16_t)(((long)h * 10) / (long)w);

            if (th->attachment_h > 16)
            {
                th->attachment_h = 16;
            }

            th->height = th->attachment_h;
            label_w = 176;
        }
        else
        {
            label_w = 256;
        }

        uint8_t min_height = zxgui_label_text_height(label_w, comment->value, comment->value_size);
        if (post_mode)
        {
            if (min_height > 22)
            {
                min_height = 22;
            }
        }
        else
        {
            if (min_height > 10)
            {
                min_height = 10;
            }
        }


        if (th->height < min_height)
        {
            th->height = min_height;
        } else if (th->height < 2)
        {
            th->height = 2;
        }

        if (process_screen_h + th->height + 1 >= 23)
        {
            process_screen_num++;
            process_screen_h = 0;
            th->screen_y = 0;
        }
        else
        {
            th->screen_y = process_screen_h;
        }

        process_screen_h += th->height + 1;
        th->screen_num = process_screen_num;

        memcpy(th->id, thead_id->value, thead_id->value_size);

        th->comment_blob_id = allocate_heap_blob();
        uint8_t* comment_blob_data = open_heap_blob(th->comment_blob_id);
        memcpy(comment_blob_data, comment->value, comment->value_size);
        comment_blob_data[comment->value_size] = 0;
        close_heap_blob();

        th->offset = entry_offset + entry_count_recv++;

        if (last_entry)
        {
            last_entry->next = th;
            th->prev = last_entry;
        }
        else
        {
            first_entry = th;
            last_entry = th;
        }

        last_entry = th;
    }
}

static void get_entry_response(struct proto_process_t* proto)
{
    if (auto_select_last)
    {
        first_display_entry = first_entry;
        selected_entry = last_entry;

        uint8_t rewind_to = last_entry->screen_num;

        while (first_display_entry)
        {
            if (first_display_entry->screen_num == rewind_to)
            {
                break;
            }

            first_display_entry = first_display_entry->next;
        }
    }
    else
    {
        selected_entry = first_entry;
        first_display_entry = first_entry;
    }

    redraw_screen();
}

static uint8_t request()
{
    free_view();

    declare_str_property_on_stack(key_, OBJ_PROPERTY_ID, (post_mode ? "thread" : "threads"), NULL);
    declare_str_property_on_stack(channel_, 'c', channels_get_channel(), &key_);
    declare_str_property_on_stack(board_, 'b', channels_get_board(), &channel_);

    declare_str_property_on_stack(thread_, 't', channels_get_thread(), &board_);

    uint16_t limit = ENTRIES_PER_REQUEST;
    declare_arg_property_on_stack(limit_, 'l', limit, (post_mode ? &thread_ : &board_));
    declare_arg_property_on_stack(offset_, 'o', entry_offset, &limit_);
    declare_arg_property_on_stack(flush_, 'f', flush, &offset_);

    declare_object_on_stack(request, 128, &flush_);

    flush = 0;
    first_object = 1;
    entry_count_recv = 0;
    process_screen_num = 0;
    process_screen_h = 0;

    if (channels_send_request(request, process_entry, get_entry_response, get_entries_error))
    {
        switch_alert("Cannot fetch threads", switch_connect_to_proxy);
        return 1;
    }

    return 0;
}

void switch_thread_view()
{
    entry_offset = 0;
    requests_locked = 0;
    flush = 1;
    post_mode = 0;

    selected_entry = NULL;
    first_display_entry = NULL;
    first_entry = NULL;
    last_entry = NULL;
    attachment_being_requested = NULL;
    stop_attachment_requests_at = NULL;

    if (request() == 0)
    {
        switch_progress("Fetching Threads", NULL);
    }
}