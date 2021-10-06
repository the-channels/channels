#include <stdlib.h>
#include "zxgui.h"
#include <text_ui.h>
#include "scenes.h"
#include "system.h"
#include "netlog.h"

#include "channels_proto.h"
#include "proto_asserts.h"
#include "channels.h"
#include "heap.h"

/* 28 pages of 4k each are available to us, each page fits 2 posts (1k post 1k post image) */
#define ENTRIES_PER_REQUEST (16)
#define MAX_ENTRIES_PER_SCREEN (8)
#define SELECTED_THREAD_COLOR (INK_BLACK | PAPER_YELLOW )
#define DESELECTED_THREAD_COLOR (INK_YELLOW | PAPER_BLACK)

struct channel_ui_entry_t
{
    struct gui_dynamic_label_t comment_label;
    struct gui_dynamic_image_t attachment_image;
};

struct scene_objects_t
{
    struct gui_scene_t scene;

    struct gui_scene_t scene_full_image;
    struct gui_button_t button_exit_full_image;
    struct gui_animated_icon_t loading_icon;
    struct gui_button_t button_grayscale_full_image;

    struct gui_button_t button_cancel;
    struct gui_button_t button_help;
    struct gui_label_t post_label;
    struct gui_object_t* last_static_object;

    struct channel_ui_entry_t ui_entries[MAX_ENTRIES_PER_SCREEN];
};

struct help_scene_objects_t
{
    struct gui_scene_t help_scene;

    struct gui_button_t button_exit_help;
    struct gui_button_t button_thread;
    struct gui_button_t button_picture;
    struct gui_button_t next_thread;
    struct gui_button_t next_thread_6;
    struct gui_button_t prev_thread;
    struct gui_button_t prev_thread_7;
    struct gui_button_t button_reply;
    struct gui_button_t button_new_post;
};

struct posting_scene_objects_t
{
    struct gui_scene_t scene;
    struct gui_label_t title;
    struct gui_button_t button_exit;
    struct gui_edit_t post_body;
    char post_content[1024];
};

struct scene_objects_t* scene_objects = NULL;
struct help_scene_objects_t* help_scene_objects = NULL;
struct posting_scene_objects_t* posting_scene_objects = NULL;

static uint8_t first_object = 0;
static uint8_t grayscale = 0;
static uint8_t requests_locked = 0;
static uint8_t stop_fetching_attachments_and_leave = 0;
static uint8_t flush = 0;
static uint16_t entry_offset = 0;
static uint16_t reply_cached_offset = 0;
static uint16_t reply_cached_screen = 0;
static uint8_t process_screen_h = 0;
static uint8_t process_screen_num = 0;
static uint8_t process_entries_on_screen = 0;
static uint8_t current_screen = 0;
static uint8_t num_entries_total = 0;
static uint8_t entry_count_recv = 0;
static uint8_t* display_ptr = NULL;
static uint8_t auto_select_last = 0;
static uint8_t auto_select_screen = 0;

static char title[96];
static uint8_t post_mode = 0;

#define MAX_REPLY_STACK (8)
struct reply_stack_entry
{
    char reply_id[48];
};

#define ENTRY_FLAG_HAS_ATTACHMENT (0x01)
#define ENTRY_FLAG_ATTACHMENT_BEING_RECEIVED (0x02)
#define ENTRY_FLAG_ATTACHMENT_RECEIVED (0x04)
#define ENTRY_FLAG_ATTACHMENT_ERRORED (0x08)

struct channel_entry_t
{
    char id[24];
    char title[48];
    struct channel_ui_entry_t* ui;
    uint8_t flags;
    uint8_t attachment_w;
    uint8_t attachment_h;
    uint8_t screen_y;
    uint8_t height;
    uint8_t screen_num;
    uint8_t offset;
    struct channel_entry_t* next;
    struct channel_entry_t* prev;
    uint8_t comment_blob_id;
    uint8_t attachment_blob_id;
    uint16_t attachment_id;
    uint16_t attachment_recv_size;
    uint8_t replies;
};

static struct channel_entry_t* entries = NULL;
static uint8_t last_allocated_entry = 0;
static uint8_t replies_stack_pointer = 0;
static struct reply_stack_entry replies_stack[MAX_REPLY_STACK];
static struct channel_entry_t* selected_entry;
static struct channel_entry_t* first_display_entry;
static struct channel_entry_t* first_entry;
static struct channel_entry_t* last_entry;
static struct channel_entry_t* attachment_being_requested;

static void enable_loading_icon()
{
    scene_objects->loading_icon.flags &= ~GUI_FLAG_HIDDEN;
    object_invalidate(&scene_objects->loading_icon, GUI_FLAG_DIRTY);
}

static void disable_loading_icon()
{
    object_invalidate(&scene_objects->loading_icon, GUI_FLAG_DIRTY | GUI_FLAG_HIDDEN);
}

static void free_view()
{
    /*
     * destroy all subsequent object by doing this simple move
     * they're all allocated on heap (part of the channel_thread_t structure) and will be destroyed below anyway
     */
    last_allocated_entry = 0;

    // destroy all allocations with this simple trick
    reset_heap();
    reset_heap_blobs();

    entries = NULL;
    scene_objects = NULL;
    help_scene_objects = NULL;
    attachment_being_requested = NULL;
    first_display_entry = NULL;
    selected_entry = NULL;
    first_entry = NULL;
    last_entry = NULL;
}

static void allocate_entries()
{
    entries = alloc_heap(ENTRIES_PER_REQUEST * sizeof(struct channel_entry_t));
    proto_assert_str(entries, "Cannot allocate");
}

static void process_attachment_image(ChannelObject* object)
{
    if (first_object)
    {
        attachment_being_requested->attachment_w = get_uint16_property(object, 'w', 0) / 8;
        attachment_being_requested->attachment_h = get_uint16_property(object, 'h', 0) / 8;
        uint16_t attachment_size = get_uint16_property(object, 's', 0);
        proto_assert_str(attachment_size <= SPECTRANET_BLOB_SIZE, "attachment size is higher than blob size");
        first_object = 0;
        return;
    }

    ChannelObjectProperty* prop = find_property(object, OBJ_PROPERTY_PAYLOAD);

    if (prop)
    {
        uint16_t r = prop->value_size;
        uint8_t* d = (uint8_t*)prop->value;

        uint8_t* ptr = open_heap_blob(attachment_being_requested->attachment_blob_id);
        memcpy(ptr + attachment_being_requested->attachment_recv_size, d, r);
        attachment_being_requested->attachment_recv_size += r;
    }
}

static void highlight_thread(struct channel_entry_t* th)
{
    if (selected_entry && (selected_entry != th))
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

    netlog_1(error);
    switch_alert(error, switch_connect_to_proxy);
}

static void switch_back_to_boards()
{
    if (requests_locked)
    {
        stop_fetching_attachments_and_leave = 1;
        return;
    }

    free_view();

    entry_offset = 0;
    switch_channel_view();
}

static uint8_t request();
static void switch_posting_mode();

static void switch_back_to_threads()
{
    if (requests_locked)
    {
        stop_fetching_attachments_and_leave = 1;
        return;
    }

    if (replies_stack_pointer)
    {
        replies_stack_pointer--;

        if (replies_stack_pointer == 0)
        {
            entry_offset = reply_cached_offset;
            auto_select_screen = reply_cached_screen;
            reply_cached_offset = 0;
            auto_select_last = 0;
        }

        if (request() == 0)
        {
            switch_progress(replies_stack_pointer ? "Opening Replies" : "Opening Original Thread", NULL);
        }
        return;
    }

    switch_thread_view();
}

static void switch_back(struct gui_button_t* this)
{
    if (post_mode)
    {
        switch_back_to_threads();
    }
    else
    {
        switch_back_to_boards();
    }
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

    object_invalidate(&scene_objects->button_exit_full_image, GUI_FLAG_DIRTY);
    object_invalidate(&scene_objects->button_grayscale_full_image, GUI_FLAG_DIRTY);
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
    netlog_1(error);
    switch_alert(error, redraw_screen);
}

static void open_full_picture(struct gui_button_t* this)
{
    if (requests_locked)
        return;

    if (selected_entry == NULL || ((selected_entry->flags & ENTRY_FLAG_HAS_ATTACHMENT) == 0))
    {
        return;
    }

    zxgui_scene_set(&scene_objects->scene_full_image);

    if (this == &scene_objects->button_grayscale_full_image)
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

        scene_objects->button_grayscale_full_image.title = "COLOR";
    }
    else
    {
        scene_objects->button_grayscale_full_image.title = "GRAYSCALE";
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
    declare_arg_property_on_stack(url_, 'i', selected_entry->attachment_id, &channel);
    declare_arg_property_on_stack(target_w_, 'w', target_w, &url_);
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

static const uint8_t* attachment_obtain_data(struct gui_dynamic_image_t* this, uint16_t* data_size)
{
    struct channel_entry_t* entry = (struct channel_entry_t*)this->user;
    if ((entry->flags & ENTRY_FLAG_ATTACHMENT_RECEIVED) == 0)
    {
        return NULL;
    }
    *data_size = entry->attachment_recv_size;
    return open_heap_blob(entry->attachment_blob_id);
}

static void attachment_errored(struct channel_entry_t* entry);

static void redraw_screen()
{
    current_screen = first_display_entry->screen_num;
    uint8_t next_ui_entry = 0;

    if (post_mode)
    {
        if (replies_stack_pointer)
        {
            strcpy(title, "replies level ");
            itoa(replies_stack_pointer, title + 14, 10);
        }
        else
        {
            title[0] = 0;
            strcat(title, channels_get_channel());
            strcat(title, "/");
            strcat(title, channels_get_board());
            strcat(title, "/");
            strcat(title, channels_get_thread());
            strcat(title, ", ");
            uint8_t into = strlen(title);
            itoa(first_display_entry->offset + 1, title + into, 10);
            strcat(title, " out of ");
            into = strlen(title);
            itoa(num_entries_total, title + into, 10);
        }
    }
    else
    {
        title[0] = 0;
        strcat(title, channels_get_channel());
        strcat(title, "/");
        strcat(title, channels_get_board());
        strcat(title, ", ");
        uint8_t into = strlen(title);
        itoa(first_display_entry->offset + 1, title + into, 10);
        strcat(title, " out of ");
        into = strlen(title);
        itoa(num_entries_total, title + into, 10);
    }

    scene_objects->last_static_object->next = NULL;
    zxgui_scene_set(&scene_objects->scene);

    struct channel_entry_t* th = first_display_entry;

    while (th)
    {
        if (th->screen_num != current_screen)
        {
            break;
        }

        struct channel_ui_entry_t* ui_entry = &scene_objects->ui_entries[next_ui_entry++];

        th->ui = ui_entry;
        uint8_t h = th->screen_y;

        uint8_t cc = selected_entry == th ? SELECTED_THREAD_COLOR : DESELECTED_THREAD_COLOR;
        zxgui_screen_color(cc);

        zxgui_screen_put(0, h, GUI_ICON_REPLY);
        zxgui_screen_clear(1, h, 31, 1);
        uint8_t has_title = strlen(th->title);

        if (has_title)
        {
            uint8_t ll = strlen(th->title);
            has_title = (ll >> 4) + 1;
            zxgui_screen_clear(1, h, has_title, 1);
            text_ui_puts_at(1, h, th->title);
        }

        zxgui_screen_recolor(0, h, 32, 1);
        zxgui_screen_put(28, h, GUI_ICON_REPLIES);

        text_ui_color(cc);
        char header[8];
        itoa(th->replies, header, 10);
        text_ui_puts_at(29, h, header);

        h++;

        uint8_t x_, w_;
        if (th->flags & ENTRY_FLAG_HAS_ATTACHMENT)
        {
            x_ = 10;
            w_ = 22;
        }
        else
        {
            x_ = 0;
            w_ = 32;
        }

        if (th->flags & ENTRY_FLAG_HAS_ATTACHMENT)
        {
            zxgui_dynamic_image_init(&ui_entry->attachment_image, get_xywh(0,
                th->screen_y + 1, th->attachment_w, th->attachment_h),
                attachment_obtain_data, th);

            zxgui_scene_add(&scene_objects->scene, &ui_entry->attachment_image);

            if (th->flags & ENTRY_FLAG_ATTACHMENT_ERRORED)
            {
                attachment_errored(th);
            }
            else
            if ((th->flags & ENTRY_FLAG_ATTACHMENT_RECEIVED) == 0)
            {
                static uint8_t frames[] = {
                    GUI_ICON_LOADING_A_1,
                    GUI_ICON_LOADING_A_2,
                    GUI_ICON_LOADING_A_3,
                    GUI_ICON_LOADING_A_4
                };

                zxgui_icon(INK_WHITE | PAPER_BLACK, 4,
                    th->screen_y +
                    th->attachment_h / 2, 2, 2, frames);
            }
        }

        zxgui_dynamic_label_init(&ui_entry->comment_label, get_xywh(x_, h, w_, th->height), INK_WHITE | PAPER_BLACK,
            GUI_FLAG_MULTILINE, comment_obtain_data, th);

        th = th->next;
    }

    /*
     * Add all comments to be latest because they take longest to render
     */
    th = first_display_entry;
    while (th)
    {
        if (th->screen_num != current_screen)
        {
            break;
        }

        zxgui_scene_add(&scene_objects->scene, &th->ui->comment_label);

        th = th->next;
    }

    highlight_thread(selected_entry);
}

static uint8_t request();

static void attachment_errored(struct channel_entry_t* entry)
{
    static uint8_t frames[] = {
        QUESTION_MARK_1,
        QUESTION_MARK_2,
        QUESTION_MARK_3,
        QUESTION_MARK_4
    };

    zxgui_icon(INK_RED | PAPER_BLACK,
        4, entry->screen_y + entry->attachment_h / 2, 2, 2, frames);
}

static void do_stop_fetching_attachments_and_leave()
{
    stop_fetching_attachments_and_leave = 0;
    attachment_being_requested = NULL;
    disable_loading_icon();

    if (post_mode)
    {
        switch_back_to_threads();
    }
    else
    {
        switch_back_to_boards();
    }
}

static void get_attachment_error(const char* error)
{
    requests_locked = 0;

    if (attachment_being_requested->screen_num == current_screen)
    {
        attachment_errored(attachment_being_requested);
    }

    netlog_2("Error fetching attachment: ", error);

    attachment_being_requested->flags |= ENTRY_FLAG_ATTACHMENT_ERRORED;
    attachment_being_requested = attachment_being_requested->next;

    if (stop_fetching_attachments_and_leave)
    {
        do_stop_fetching_attachments_and_leave();
        return;
    }

    fetch_next_thread_attachment();
}

static void get_attachment_response(struct proto_process_t* proto)
{
    requests_locked = 0;

    attachment_being_requested->flags &= ~ENTRY_FLAG_ATTACHMENT_BEING_RECEIVED;
    attachment_being_requested->flags |= ENTRY_FLAG_ATTACHMENT_RECEIVED;

    if (attachment_being_requested->screen_num == current_screen)
    {
        object_invalidate(&attachment_being_requested->ui->attachment_image, GUI_FLAG_DIRTY);
    }
    attachment_being_requested = attachment_being_requested->next;

    if (stop_fetching_attachments_and_leave)
    {
        do_stop_fetching_attachments_and_leave();
        return;
    }

    fetch_next_thread_attachment();
}

static void fetch_next_thread_attachment()
{
    if (attachment_being_requested == NULL)
    {
        disable_loading_icon();
        return;
    }

    while (((attachment_being_requested->flags & ENTRY_FLAG_HAS_ATTACHMENT) == 0) ||
            (attachment_being_requested->flags & ENTRY_FLAG_ATTACHMENT_RECEIVED))
    {
        attachment_being_requested = attachment_being_requested->next;

        if (attachment_being_requested == NULL)
        {
            disable_loading_icon();
            return;
        }
    }

    first_object = 1;
    attachment_being_requested->attachment_recv_size = 0;
    attachment_being_requested->flags |= ENTRY_FLAG_ATTACHMENT_BEING_RECEIVED;

    uint16_t target_w = (uint16_t)attachment_being_requested->attachment_w * 8;
    uint16_t target_h = (uint16_t)attachment_being_requested->attachment_h * 8;

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
    declare_arg_property_on_stack(url_, 'i', attachment_being_requested->attachment_id, &channel);
    declare_str_property_on_stack(encoding_, 'e', "color_zx", &url_);
    declare_arg_property_on_stack(target_w_, 'w', target_w, &encoding_);
    declare_arg_property_on_stack(target_h_, 'h', target_h, &target_w_);

    declare_object_on_stack(request, 160, &target_h_);

    if (channels_send_request(request, process_attachment_image, get_attachment_response, get_attachment_error))
    {
        return;
    }

    requests_locked = 1;
}

static void key_pressed(int key)
{
    switch (key)
    {
        case 13:
        {
            if (requests_locked)
                return;

            if (post_mode)
            {
                if (selected_entry->replies == 0)
                    return;

                if (replies_stack_pointer >= MAX_REPLY_STACK)
                    return;

                if (replies_stack_pointer == 0)
                {
                    reply_cached_offset = entry_offset;
                    reply_cached_screen = current_screen;
                }

                strcpy(replies_stack[replies_stack_pointer].reply_id, selected_entry->id);

                replies_stack_pointer++;
                entry_offset = 0;
                flush = 0;

                if (request() == 0)
                {
                    switch_progress("Opening Replies", NULL);
                }
            }
            else
            {
                channels_set_thread(selected_entry->id);

                post_mode = 1;
                entry_offset = 0;

                if (request() == 0)
                {
                    switch_progress("Opening Thread", NULL);
                }
            }

            break;
        }
        case 'p':
        {
            open_full_picture(NULL);
            break;
        }
        case 'n':
        {
            switch_posting_mode();
            break;
        }
        // select next entry
        case 10:
        case '6':
        {
            if (selected_entry->next)
            {
                if (selected_entry->next->screen_num != current_screen)
                {
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
            break;
        }
        // select prev entry
        case 11:
        case '7':
        {
            if (selected_entry->prev)
            {
                if (selected_entry->prev->screen_num != current_screen)
                {
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

                    if (entry_offset > ENTRIES_PER_REQUEST)
                    {
                        entry_offset -= ENTRIES_PER_REQUEST;
                    }
                    else
                    {
                        entry_offset = 0;
                    }
                    auto_select_last = 1;
                    request();
                }
            }
            break;
        }
    }
}

static void exit_help(struct gui_button_t* this)
{
    switch_back_to_threads();
}

static void exit_posting(struct gui_button_t* this)
{
    switch_posting_mode();
}

static void switch_posting_mode()
{
    if (requests_locked)
        return;

    free_view();

    posting_scene_objects = alloc_heap(sizeof(struct posting_scene_objects_t));
    proto_assert_str(posting_scene_objects, "Cannot allocate");

    zxgui_scene_init(&posting_scene_objects->scene, NULL);

    {
        zxgui_label_init(&posting_scene_objects->title, XYWH(0, 0, 32, 1), "NEW POST", INK_BLACK | PAPER_WHITE, 0);
        zxgui_scene_add(&posting_scene_objects->scene, &posting_scene_objects->title);
    }

    {
        strcpy(posting_scene_objects->post_content, "");
        zxgui_edit_init(&posting_scene_objects->post_body, XYWH(0, 1, 31, 21), posting_scene_objects->post_content, 64);
        zxgui_scene_add(&posting_scene_objects->scene, &posting_scene_objects->post_body);
    }

    {
        zxgui_button_init(&posting_scene_objects->button_exit, XYWH(0, 23, 0, 1), 32, GUI_ICON_SPACE, "BACK", exit_posting);
        zxgui_scene_add(&posting_scene_objects->scene, &posting_scene_objects->button_exit);
    }

    zxgui_scene_set_focus(&posting_scene_objects->scene, &posting_scene_objects->post_body);
    zxgui_scene_set(&posting_scene_objects->scene);
}

static void switch_help(struct gui_button_t* this)
{
    if (requests_locked)
        return;

    free_view();

    help_scene_objects = alloc_heap(sizeof(struct help_scene_objects_t));
    proto_assert_str(help_scene_objects, "Cannot allocate");

    zxgui_scene_init(&help_scene_objects->help_scene, NULL);

    {
        zxgui_button_init(&help_scene_objects->button_exit_help, XYWH(0, 23, 0, 1), 32, GUI_ICON_SPACE, "BACK", exit_help);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->button_exit_help);
    }

    {
        zxgui_button_init(&help_scene_objects->button_thread, XYWH(1, 1, 0, 1), 13, GUI_ICON_RETURN, "Open THREAD or POST or REPLIES", NULL);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->button_thread);
    }

    {
        zxgui_button_init(&help_scene_objects->button_picture, XYWH(1, 3, 0, 1), 'p', GUI_ICON_P, "Open PICTURE to a THREAD or a POST", NULL);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->button_picture);
    }

    {
        zxgui_button_init(&help_scene_objects->next_thread, XYWH(1, 5, 1, 1), 10, GUI_ICON_MORE_TO_FOLLOW, NULL, NULL);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->next_thread);
    }

    {
        zxgui_button_init(&help_scene_objects->next_thread_6, XYWH(2, 5, 4, 1), '6', GUI_ICON_6, "SELECT NEXT ENTRY", NULL);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->next_thread_6);
    }

    {
        zxgui_button_init(&help_scene_objects->prev_thread, XYWH(1, 7, 1, 1), 11, GUI_ICON_LESS_TO_FOLLOW, NULL, NULL);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->prev_thread);
    }

    {
        zxgui_button_init(&help_scene_objects->prev_thread_7, XYWH(2, 7, 4, 1), '7', GUI_ICON_7, "SELECT PREV ENTRY", NULL);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->prev_thread_7);
    }

    {
        zxgui_button_init(&help_scene_objects->button_reply, XYWH(1, 9, 0, 1), 'r', GUI_ICON_R, "Reply to a POST", NULL);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->button_reply);
    }

    {
        zxgui_button_init(&help_scene_objects->button_new_post, XYWH(1, 11, 0, 1), 'n', GUI_ICON_N, "New POST to a THREAD", NULL);
        zxgui_scene_add(&help_scene_objects->help_scene, &help_scene_objects->button_new_post);
    }

    zxgui_scene_set(&help_scene_objects->help_scene);
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
        uint16_t a = get_uint16_property(object, 'a', 0);
        uint16_t label_w;
        if (a)
        {
            th->flags |= ENTRY_FLAG_HAS_ATTACHMENT;
            th->attachment_id = a;
            th->attachment_w = 10;
            th->attachment_h = 10;

            /*
             * 10x10 is maximum we can fit in one 1024b blob
             */
            if (th->attachment_h > 10)
            {
                th->attachment_h = 10;
            }

            th->height = th->attachment_h;
            label_w = 21;
        }
        else
        {
            label_w = 31;
        }

        th->comment_blob_id = allocate_heap_blob();

        uint8_t* comment_blob_data = open_heap_blob(th->comment_blob_id);
        memcpy(comment_blob_data, comment->value, comment->value_size);
        comment_blob_data[comment->value_size] = 0;

        uint8_t max_height = post_mode ? 22 : 10;

        uint8_t min_height = zxgui_label_text_height(label_w, (char*)comment_blob_data,
            comment->value_size, max_height);

        if (th->height < min_height)
        {
            th->height = min_height;
        } else if (th->height < 2)
        {
            th->height = 2;
        }

        ChannelObjectProperty* ttl = find_property(object, OBJ_PROPERTY_TITLE);

        {
            th->title[0] = 0;

            if (ttl)
            {
                if (ttl->value_size >= sizeof(th->title))
                {
                    memcpy(th->title, ttl->value, sizeof(th->title) - 1);
                    th->title[sizeof(th->title) - 1] = 0;
                }
                else
                {
                    memcpy(th->title, ttl->value, ttl->value_size);
                    th->title[ttl->value_size] = 0;
                }
            }
        }

        if ((process_entries_on_screen >= MAX_ENTRIES_PER_SCREEN) || (process_screen_h + th->height + 1 >= 23))
        {
            process_screen_num++;
            process_screen_h = 0;
            process_entries_on_screen = 0;
            th->screen_y = 0;
        }
        else
        {
            th->screen_y = process_screen_h;
        }

        th->offset = entry_count_recv++;
        process_entries_on_screen++;
        process_screen_h += th->height + 1;
        th->screen_num = process_screen_num;

        memcpy(th->id, thead_id->value, thead_id->value_size);

        th->replies = get_uint16_property(object, 'r', 0);

        if (th->flags & ENTRY_FLAG_HAS_ATTACHMENT)
        {
            th->attachment_blob_id = allocate_heap_blob();
        }
        else
        {
            th->attachment_blob_id = 0xFF;
        }

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

static void alloc_view_objects()
{
    scene_objects = alloc_heap(sizeof(struct scene_objects_t));
    proto_assert_str(scene_objects, "Cannot allocate");

    zxgui_scene_init(&scene_objects->scene, NULL);

    scene_objects->scene.key_pressed = key_pressed;

    {
        zxgui_button_init(&scene_objects->button_cancel, XYWH(0, 23, 0, 1), 32, GUI_ICON_SPACE, "BACK", switch_back);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->button_cancel);
    }

    {
        zxgui_button_init(&scene_objects->button_help, XYWH(4, 23, 0, 1), 'h', GUI_ICON_QUESTION, "HELP", switch_help);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->button_help);
    }

    {
        zxgui_label_init(&scene_objects->post_label, XYWH(8, 23, 23, 1), title, INK_GREEN | PAPER_BLACK, 0);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->post_label);
    }

    {
        static uint8_t frames[] = {
                GUI_ICON_LOADING_SMALL_1,
                GUI_ICON_LOADING_SMALL_2
        };
        zxgui_animated_icon_init(&scene_objects->loading_icon, XYWH(31, 23, 1, 1), 2, BRIGHT | INK_GREEN | PAPER_BLACK, frames, 32);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->loading_icon);
    }

    {
        zxgui_button_init(&scene_objects->button_exit_full_image, XYWH(0, 23, 0, 1), 32, GUI_ICON_SPACE, "BACK", exit_fullscreen_image);
        zxgui_scene_add(&scene_objects->scene_full_image, &scene_objects->button_exit_full_image);
    }

    {
        zxgui_button_init(&scene_objects->button_grayscale_full_image, XYWH(8, 23, 0, 1), 'p', GUI_ICON_P, "GRAYSCALE", open_full_picture);
        zxgui_scene_add(&scene_objects->scene_full_image, &scene_objects->button_grayscale_full_image);
    }

    scene_objects->last_static_object = zxgui_scene_get_last_object(&scene_objects->scene);
}

static void get_entries_response(struct proto_process_t* proto)
{
    if (first_entry == NULL)
    {
        switch_alert("Server returned empty response", switch_connect_to_proxy);
        return;
    }

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
        first_display_entry = first_entry;

        if (auto_select_screen)
        {
            while (first_display_entry)
            {
                if (first_display_entry->screen_num == auto_select_screen)
                {
                    break;
                }

                first_display_entry = first_display_entry->next;
            }

            auto_select_screen = 0;
        }

        selected_entry = first_display_entry;
    }

    attachment_being_requested = first_entry;
    fetch_next_thread_attachment();

    alloc_view_objects();
    redraw_screen();
}

static uint8_t request()
{
    free_view();
    allocate_entries();

    declare_str_property_on_stack(key_, OBJ_PROPERTY_ID, (post_mode ? "thread" : "threads"), NULL);
    declare_str_property_on_stack(channel_, 'c', channels_get_channel(), &key_);
    declare_str_property_on_stack(board_, 'b', channels_get_board(), &channel_);

    declare_str_property_on_stack(thread_, 't', channels_get_thread(), &board_);

    uint16_t limit = ENTRIES_PER_REQUEST;
    declare_arg_property_on_stack(limit_, 'l', limit, (post_mode ? &thread_ : &board_));
    declare_arg_property_on_stack(offset_, 'o', entry_offset, &limit_);
    declare_arg_property_on_stack(flush_, 'f', flush, &offset_);

    declare_str_property_on_stack(replies_to, 'r',
        (replies_stack_pointer ? (const char*)replies_stack[replies_stack_pointer - 1].reply_id : ""), &flush_);

    declare_object_on_stack(request, 128, &replies_to);

    flush = 0;
    stop_fetching_attachments_and_leave = 0;
    first_object = 1;
    entry_count_recv = entry_offset;
    process_screen_num = 0;
    process_screen_h = 0;
    process_entries_on_screen = 0;

    enable_loading_icon();

    if (channels_send_request(request, process_entry, get_entries_response, get_entries_error))
    {
        switch_alert("Cannot fetch threads", switch_connect_to_proxy);
        return 1;
    }

    switch_progress("Fetching Threads", NULL);

    return 0;
}

void switch_thread_view()
{
    entry_offset = 0;
    auto_select_last = 0;
    requests_locked = 0;
    flush = 1;
    post_mode = 0;

    selected_entry = NULL;
    first_display_entry = NULL;
    first_entry = NULL;
    last_entry = NULL;
    attachment_being_requested = NULL;

    request();
}