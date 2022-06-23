#ifndef app_wifi_h_
#define app_wifi_h_

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*on_connected_f)(void);
typedef void (*on_failed_f)(void);
typedef void (*on_timeset_f)(void);

typedef struct {
    on_connected_f on_connected;
    on_failed_f on_failed;
    on_timeset_f on_timeset;
} connect_wifi_params_t;

void appwifi_connect(connect_wifi_params_t);


#ifdef __cplusplus
}
#endif


#endif