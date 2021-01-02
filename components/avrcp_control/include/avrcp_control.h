#ifndef AVRCP_CONTROL_H
#define AVRCP_CONTROL_H

#include <inttypes.h>
#include <stdint.h>

/* Setup AVRCP service */
int avrcp_setup(char* announce_str);
int avrcp_setup_with_autoconnect(char* announce_str, char* cxn_address, TaskHandle_t bt_auto_task);

uint8_t avrcp_ctl_connect();

uint8_t avrcp_ctl_disconnect();

uint8_t avrcp_ctl_play();

uint8_t avrcp_ctl_pause();

uint8_t avrcp_ctl_stop();

uint8_t avrcp_ctl_next();

uint8_t avrcp_ctl_prev();

uint8_t avrcp_ctl_start_ff();

uint8_t avrcp_ctl_start_rwd();

uint8_t avrcp_ctl_end_long_press();

uint8_t avrcp_get_now_playing();

#endif // AVRCP_CONTROL_H
