#ifndef BT_COMMON_H
#define BT_COMMON_H

typedef enum {
    BT_CMD_NOOP = 0x00,
    BT_CONNECT,
    BT_DISCONNECT,
    AVRCP_PLAY,
    AVRCP_PAUSE,
    AVRCP_STOP,
    AVRCP_FF_START,
    AVRCP_FF_STOP,
    AVRCP_RWD_START,
    AVRCP_RWD_STOP,
    AVRCP_NEXT,
    AVRCP_PREV,
    AVRCP_GET_INFO
} bt_cmd_type_t;

// making compatible with avrcp.c
// avrcp_play_status_name
typedef enum {
    MEDIA_STOPPED,
    MEDIA_PLAYING,
    MEDIA_PAUSED,
    MEDIA_SEEK_FWD,
    MEDIA_SEEK_RWD,
    DEV_DISCONNECTED,
    DEV_CONNECTING,
    DEV_CONNECTED
} bt_avrcp_state_t;

typedef struct {
    char album_name[128];
    char track_title[128];
    char artist_name[64];
    char state_detail[32];

    uint32_t track_len_ms;
    uint8_t cur_track;
    uint8_t total_tracks;
    bt_avrcp_state_t bt_state;
} bt_now_playing_info_t;

#endif
