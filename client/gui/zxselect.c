
#include <string.h>
#include "zxgui.h"
#include <fzx_ui.h>
#include <stdint.h>
#include <spectrum.h>
#include <ctype.h>

static void _select_render(uint8_t x, uint8_t y, struct gui_select_t* this, struct gui_scene_t* scene)
{
    x += this->x + 1;
    y += this->y + 1;

    uint8_t flags = is_object_invalidated(this);
    if (!flags)
        return;

    this->obtain_data(this);

    uint8_t focus = this == scene->focus;
    uint8_t color = focus ? INK_YELLOW | BRIGHT | PAPER_BLACK : INK_WHITE | PAPER_BLACK;
    uint8_t color_inv = focus ? INK_BLACK | BRIGHT | PAPER_YELLOW : INK_BLACK | PAPER_WHITE;

    if (flags & GUI_FLAG_DIRTY)
    {
        zxgui_rectangle(color, x - 1, y - 1, this->w, this->h, GUI_SELECT_LEFT_BOTTOM_CORNER);
    }

    uint8_t offset = 0;
    uint8_t selection_offset = 0xFF;

    struct gui_select_option_t* selection = this->selection;
    struct gui_select_option_t* last_selection = this->last_selection;

    uint8_t last_offset = last_selection == NULL ? 0 : 0xFF;

    uint8_t hh = this->h - 1;
    struct gui_select_option_t* o = this->first;
    struct gui_select_option_t* page;
    while (o)
    {
        if ((selection_offset == 0xFF) && (offset % hh == 0))
        {
            page = o;
        }

        if (o == selection)
        {
            selection_offset = offset;
        }

        if (o == last_selection)
        {
            last_offset = offset;
        }

        offset++;
        o = o->next;
    }

    if ((last_offset / hh) != (selection_offset / hh))
    {
        // different pages
        flags = GUI_FLAG_DIRTY;
    }

    if (flags & GUI_FLAG_DIRTY)
    {
        zxgui_screen_color(color);
        zxgui_screen_clear(x, y, this->w - 1, hh);
        fzx_ui_set_paper(x * 8, y * 8, (this->w - 1) * 8, (this->h - 1) * 8);

        offset = 0;

        o = page;
        while (o)
        {
            if (o == selection)
            {
                fzx_ui_color(color_inv);
                fzx_ui_switch_xor();

                zxgui_screen_color(color_inv);
                zxgui_screen_clear(x, y + offset, this->w - 1, 1);
            }
            else
            {
                zxgui_screen_color(color);
                fzx_ui_color(color);
                fzx_ui_switch_or();
            }

            if (offset == 0 && o->prev)
            {
                zxgui_screen_put(x + this->w - 2, y + offset, GUI_ICON_LESS_TO_FOLLOW);
            }
            else if (offset == hh - 1 && o->next)
            {
                zxgui_screen_put(x + this->w - 2, y + offset, GUI_ICON_MORE_TO_FOLLOW);
            }

            fzx_ui_puts_at(9, offset * 8 + 1, o->value);

            offset++;

            if (offset >= hh)
            {
                break;
            }
            o = o->next;
        }

        fzx_ui_set_paper(0, 0, 256, 192);
    }

    if (flags & GUI_FLAG_DIRTY_INTERNAL)
    {
        zxgui_screen_color(color_inv);
        zxgui_screen_recolor(x, y + (selection_offset % hh), this->w - 1, 1);
        if (last_offset != 0xFF)
        {
            zxgui_screen_color(color);
            zxgui_screen_recolor(x, y + (last_offset % hh), this->w - 1, 1);
        }
    }
}

static void select_option(struct gui_select_t* this, struct gui_select_option_t* option)
{
    if (this->selection == option)
    {
        return;
    }
    this->last_selection = this->selection;
    this->selection = option;
    object_invalidate(this, GUI_FLAG_DIRTY_INTERNAL);
    this->selected(this, option);
}

static uint8_t _select_event(enum gui_event_type event_type, void* event, struct gui_select_t* this, struct gui_scene_t* scene)
{
    if (scene->focus != (void*)this)
        return 0;

    this->obtain_data(this);

    switch (event_type)
    {
        case GUI_EVENT_KEY_PRESSED:
        {
            struct gui_event_key_pressed* ev = event;

            switch (ev->key)
            {
                case 11:
                {
                    if (this->selection->prev)
                    {
                        select_option(this, this->selection->prev);
                    }
                    break;
                }
                case 10:
                {
                    if (this->selection->next)
                    {
                        select_option(this, this->selection->next);
                    }
                    break;
                }
            }

            if (isalnum(ev->key))
            {
                struct gui_select_option_t* f = this->first;

                if (this->selection && strchr(this->selection->value, ev->key) == this->selection->value)
                {
                    f = this->selection->next;
                    if (f == NULL || strchr(f->value, ev->key) != f->value)
                    {
                        f = this->first;
                    }
                }

                for (struct gui_select_option_t* it = f; it; it = it->next)
                {
                    if (strchr(it->value, ev->key) == it->value)
                    {
                        select_option(this, it);
                        break;
                    }
                }
            }

            break;
        }
    }

    return 0;
}

void zxgui_select_init(struct gui_select_t* select, xywh_t xywh,
    gui_select_obtain_data_f obtain_data, void* user,
    gui_select_selected selected) ZXGUI_CDECL
{
    zxgui_base_init(select, xywh, _select_render, _select_event);

    select->obtain_data = obtain_data;
    select->user = user;
    select->buffer_offset = 0;
    select->flags = GUI_FLAG_DIRTY;
    select->first = NULL;
    select->last = NULL;
    select->selected = selected;
    select->selection = NULL;
    select->last_selection = NULL;
}

uint8_t* zxgui_select_add_option(struct gui_select_t* select, const char* option,
    uint8_t option_len, uint16_t user_data_amount) ZXGUI_CDECL
{
    uint8_t* buffer = select->obtain_data(select);
    uint16_t needed = sizeof(struct gui_select_option_t) + option_len + 1 + user_data_amount;

    struct gui_select_option_t* o = (struct gui_select_option_t*)(buffer + select->buffer_offset);
    select->buffer_offset += needed;

    memcpy(o->value, option, option_len);
    o->value[option_len] = 0;
    o->user = user_data_amount ? o->value + option_len + 1 : NULL;
    o->next = NULL;

    if (select->last)
    {
        select->last->next = o;
    }
    else
    {
        select->first = o;
        select->selection = o;
    }

    o->prev = select->last;
    select->last = o;
    return o->user;
}