#include "zxgui.h"
#include "system.h"

struct gui_scene_t* current_scene = NULL;

struct gui_object_t* zxgui_scene_get_last_object(struct gui_scene_t* scene)
{
    struct gui_object_t* object = scene->child;
    if (object == NULL)
    {
        return NULL;
    }
    while (object->next)
    {
        object = object->next;
    }
    return object;
}

void zxgui_scene_init(struct gui_scene_t* scene, gui_scene_update_f update)
{
    scene->child = NULL;
    scene->focus = NULL;
    scene->update = update;
    scene->key_pressed = NULL;
}

void zxgui_scene_add(struct gui_scene_t* scene, void* object) __z88dk_callee
{
    struct gui_object_t* last = zxgui_scene_get_last_object(scene);
    struct gui_object_t* o = (struct gui_object_t*)object;
    o->next = NULL;
    if (last == NULL)
    {
        scene->child = o;
    }
    else
    {
        last->next = o;
    }
}

static void scene_render()
{
    struct gui_object_t* child = current_scene->child;
    while (child)
    {
        child->render(0, 0, child, current_scene);
        child = child->next;
    }
}

void zxgui_scene_set(struct gui_scene_t* scene)
{
    clear_screen_with(COLOR_BRIGHT | COLOR_FG_GREEN | COLOR_BG_BLACK);
    current_scene = scene;

    struct gui_object_t* child = current_scene->child;
    while (child)
    {
        object_invalidate(child, GUI_FLAG_DIRTY);
        child = child->next;
    }

    scene_render();
}

void zxgui_scene_set_focus(struct gui_scene_t* scene, void* object) __z88dk_callee
{
    scene->focus = object;
}

uint8_t zxgui_scene_dispatch_event(struct gui_scene_t* scene, enum gui_event_type event_type, void* event) __z88dk_callee
{
    struct gui_object_t* child = current_scene->child;
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

extern void update_keyboard();

void zxgui_scene_iteration(void)
{
    scene_render();

    if (current_scene->update)
    {
        current_scene->update(current_scene);
    }

    update_keyboard();
}