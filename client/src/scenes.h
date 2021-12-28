#ifndef SCENES_H
#define SCENES_H

extern void set_connect_to_proxy_address(const char* address);
extern void switch_connect_to_proxy();
extern void init_progress();
extern void switch_progress(const char* progress_message);
extern void switch_select_channel();
extern void switch_channel_view();
extern void switch_thread_view();
extern void switch_channel_settings_view();
void alert_and_switch_to_connect(const char* message);

#endif