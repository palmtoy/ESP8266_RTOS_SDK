#ifndef _SWITCH_WIFISTATION_H_
#define _SWITCH_WIFISTATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/netif.h"
#include "freertos/event_groups.h"
#include "httpSrv.h"

#define ROUTER_MAXIMUM_RETRY  CONFIG_ROUTER_MAXIMUM_RETRY

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the Router with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *STATION_TAG = "WIFI-STATION";
static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ROUTER_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(STATION_TAG, "retry to connect to the router");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(STATION_TAG,"connect to the router fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(STATION_TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (httpSrvObj == NULL) {
        ESP_LOGI(STATION_TAG, "Starting http webserver");
        start_webserver();
    }
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    stopHttpSrv();
}

bool wifi_init_sta(char* wifiSSID, char* wifiPwd, const char* domainName) {
    bool bConnected = false;
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, NULL));

    ESP_LOGI(STATION_TAG, "wifi_init_station: wifiSSID = %s, wifiPwd = %s", wifiSSID, wifiPwd);
    wifi_config_t wifi_config = {
        .sta = {}
    };
    strlcpy((char *) wifi_config.sta.ssid, wifiSSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char *) wifi_config.sta.password, wifiPwd, sizeof(wifi_config.sta.password));

    /* Setting a password implies station will connect to all security modes including WEP/WPA.
        * However these modes are deprecated and not advisable to be used. Incase your Access point
        * doesn't support WPA2, these mode can be enabled by commenting below line */
    if (strlen((char *)wifi_config.sta.password)) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    struct netif * netif = NULL;
    void * nif = NULL;
    esp_err_t err = tcpip_adapter_get_netif(TCPIP_ADAPTER_IF_STA, &nif);
    if (err) {
        ESP_LOGE(STATION_TAG, "Get netif Failed.");
        return bConnected;
    }
    netif = (struct netif *)nif;
    netif_set_hostname(netif, domainName);

    ESP_LOGI(STATION_TAG, "wifi_init_station finished.");
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(STATION_TAG, "Connected to router SSID:%s password:%s", wifiSSID, wifiPwd);
        bConnected = true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(STATION_TAG, "Failed to connect to SSID:%s, password:%s", wifiSSID, wifiPwd);
    } else {
        ESP_LOGE(STATION_TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
    return bConnected;
}

#ifdef __cplusplus
}
#endif

#endif /* _SWITCH_WIFISTATION_H_ */
