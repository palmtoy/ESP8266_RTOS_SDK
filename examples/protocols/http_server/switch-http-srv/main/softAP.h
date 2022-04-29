#ifndef _SWITCH_SOFTAP_H_
#define _SWITCH_SOFTAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#define AP_NAME_PREFIX  CONFIG_ESP_WIFI_SSID
#define AP_WIFI_PASS    CONFIG_ESP_WIFI_PASSWORD
#define AP_MAX_CONN     CONFIG_ESP_MAX_STA_CONN

static const char *SOFT_AP_TAG = "SOFT-AP";

static char* generate_ap_name()
{
    uint8_t mac[6];
    char* apName;
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    if (-1 == asprintf(&apName, "%s-%02x%02x", AP_NAME_PREFIX, mac[4], mac[5])) {
        abort();
    }
    return apName;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(SOFT_AP_TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(SOFT_AP_TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap()
{
    tcpip_adapter_init();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    char* tmpSoftApName = generate_ap_name();
    char* softApName;
    if (-1 == asprintf(&softApName, "%s.local", tmpSoftApName)) {
        abort();
    }
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(softApName),
            .max_connection = AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    strlcpy((char *) wifi_config.ap.ssid, softApName, sizeof(wifi_config.ap.ssid));
    strlcpy((char *) wifi_config.ap.password, AP_WIFI_PASS, sizeof(wifi_config.ap.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(SOFT_AP_TAG, "WIFI_INIT_SOFTAP finished. SSID:%s password:%s", softApName, AP_WIFI_PASS);
    free(tmpSoftApName);
    free(softApName);
}

#ifdef __cplusplus
}
#endif

#endif /* _SWITCH_SOFTAP_H_ */
