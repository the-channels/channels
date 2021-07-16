#include "zxgui.h"
#include "zxgui_internal.h"
#include "system.h"

static struct gui_scene_t* current_scene = NULL;

enum input_state_t
{
    waiting,
    key_pressed
};

static enum input_state_t input_state = waiting;
static int debounce_counter = 0;

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
}

void zxgui_scene_add(struct gui_scene_t* scene, void* object)
{
    struct gui_object_t* last = zxgui_scene_get_last_object(scene);
    struct gui_object_t* o = (struct gui_object_t*)object;
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
    if (current_scene)
    {
        zxgui_scene_dispatch_event(current_scene, GUI_EVENT_CHANGED_SCENE, NULL);
    }

    zx_colour(BRIGHT | INK_GREEN | PAPER_BLACK);
    zx_cls();
    current_scene = scene;

    struct gui_object_t* child = current_scene->child;
    while (child)
    {
        object_invalidate(child, GUI_FLAG_DIRTY);
        child = child->next;
    }

    scene_render();
}

void zxgui_scene_set_focus(struct gui_scene_t* scene, void* object)
{
    scene->focus = object;
}

uint8_t zxgui_scene_dispatch_event(struct gui_scene_t* scene, enum gui_event_type event_type, void* event)
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

static void update_keyboard()
{
    switch (input_state)
    {
        case waiting:
        {
            int key = in_inkey();
            if (key)
            {
                //netlog("key: %d\n", key);

                struct gui_event_key_pressed e;
                e.key = key;
                zxgui_scene_dispatch_event(current_scene, GUI_EVENT_KEY_PRESSED, &e);

                input_state = key_pressed;
                debounce_counter = 10;
            }
            break;
        }
        case key_pressed:
        {
            if (debounce_counter)
            {
                debounce_counter--;

                if (in_inkey())
                {
                    debounce_counter = 10;
                }
            }
            else
            {
                if (in_inkey() == 0)
                {
                    input_state = waiting;
                }
            }
            break;
        }
    }
}

void zxgui_scene_iteration()
{
    scene_render();

    if (current_scene->update)
    {
        current_scene->update(current_scene);
    }

    update_keyboard();
}