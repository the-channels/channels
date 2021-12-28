
#include <stdint.h>

#include "zxgui.h"
#include "system.h"
#include <text_ui.h>
#include <string.h>

static void _form_render(uint8_t x, uint8_t y, struct gui_form_t* this, struct gui_scene_t* scene)
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
                f = COLOR_FG_BLACK | COLOR_BRIGHT | COLOR_BG_CYAN;

                break;
            }
            case FORM_STYLE_DEFAULT:
            default:
            {
                c = GUI_FORM_LEFT_BOTTOM_CORNER;
                f = COLOR_FG_WHITE | COLOR_BG_BLACK;
                break;
            }
        }

        if (this->style != FORM_STYLE_EMPTY)
        {
            zxgui_rectangle(COLOR_FG_CYAN | COLOR_BRIGHT | COLOR_BG_BLACK, x, y, this->w, this->h, c);
        }

        zxgui_screen_color(f);
        zxgui_screen_clear(x + 1, y + 1, this->w - 1, 1);

        uint8_t ll = strlen(((char *) this->title)) / CHARACTERS_PER_CELL;
        int16_t title_offset = (this->w - ll) / 2;
        text_ui_color(f);
        text_ui_puts_at(x + title_offset, y + 1, this->title);
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

static uint8_t _form_event(enum gui_event_type event_type, void* event, struct gui_form_t* this, struct gui_scene_t* scene)
{
    struct gui_object_t* child = this->child;
    while (child)
    {
        if (child->event)
        {
            if (child->event(event_type, event, child, scene))
            {
                return 1;
            }
        }
        child = child->next;
    }
    return 0;
}

void zxgui_form_init(struct gui_form_t* form, xywh_t xywh,
    const char* title, uint8_t style) ZXGUI_CDECL
{
    zxgui_base_init(form, xywh, _form_render, _form_event);

    form->flags = GUI_FLAG_DIRTY;
    form->child = NULL;
    form->title = title;
    form->style = style;
}

void zxgui_form_add_child(struct gui_form_t* form, void* child) ZXGUI_CDECL
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