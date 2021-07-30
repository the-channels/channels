
#include <stdint.h>

#include "zxgui.h"
#include "zxgui_internal.h"
#include <font/fzx.h>
#include "system.h"

static void form_render(uint8_t x, uint8_t y, struct gui_form_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (is_object_invalidated(this))
    {
        {
            struct gui_object_t* child = this->child;
            while (child)
            {
                object_invalidate(child, GUI_FLAG_DIRTY);
                child = child->next;
            }
        }

        uint8_t c;
        uint8_t f;

        switch(this->style)
        {
            case FORM_STYLE_FRAME:
            {
                c = GUI_SELECT_LEFT_BOTTOM_CORNER;
                f = INK_BLACK | BRIGHT | PAPER_CYAN;

                break;
            }
            case FORM_STYLE_DEFAULT:
            default:
            {
                c = GUI_FORM_LEFT_BOTTOM_CORNER;
                f = INK_WHITE | PAPER_BLACK;
                break;
            }
        }

        if (this->style != FORM_STYLE_EMPTY)
        {
            zxgui_rectangle(INK_CYAN | BRIGHT | PAPER_BLACK, x, y, this->w, this->h, c);
        }

        zxgui_screen_color(f);
        zxgui_screen_clear(x + 1, y + 1, this->w - 1, 1);
        font_state.fgnd_attr = f;

        int16_t title_offset = ((this->w + 1) * 8 - fzx_string_extent(font_state.font, (char*)this->title)) / 2;

        fzx_at(&font_state, x * 8 + title_offset, (y + 1) * 8 + 1);
        fzx_puts(&font_state, (char*)this->title);
    }

    x += 1;
    y += 2;

    struct gui_object_t* child = this->child;
    while (child)
    {
        child->render(x, y, child, scene);
        child = child->next;
    }
}

static uint8_t form_event(enum gui_event_type event_type, void* event, struct gui_form_t* this, struct gui_scene_t* scene)
{
    struct gui_object_t* child = this->child;
    while (child)
    {
        if (child->event)
        {
            uint8_t caught = child->event(event_type, event, child, scene);
            if (caught)
            {
                return caught;
            }
        }
        child = child->next;
    }
    return 0;
}

void zxgui_form_init(struct gui_form_t* form, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* title, uint8_t style)
{
    form->render = (gui_render_f)form_render;
    form->event = (gui_event_f)form_event;
    form->flags = GUI_FLAG_DIRTY;
    form->child = NULL;
    form->next = NULL;
    form->title = title;
    form->style = style;
    form->x = x;
    form->y = y;
    form->w = w;
    form->h = h;
}

void zxgui_form_add_child(struct gui_form_t* form, void* child)
{
    if (form->child == NULL)
    {
        form->child = child;
    }
    else
    {
        struct gui_object_t* last = zxgui_get_last_object((struct gui_object_t*)form->child);
        last->next = child;
    }
}