#ifndef SCENES_H
#define SCENES_H

extern void init_connect_to_proxy();
extern void switch_connect_to_proxy();

extern void init_progress();
extern void switch_progress(const char* progress_message, void (*cancel)(void));

extern void init_alert();
extern void switch_alert(const char* message, void (*callback)());

extern void init_select_channel();
extern void switch_select_channel();

extern void init_select_board();
extern void switch_select_board();

extern void init_thread_view();
extern void switch_thread_view();


#endif