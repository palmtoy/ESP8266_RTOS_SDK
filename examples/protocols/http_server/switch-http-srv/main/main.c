// Switch HTTP Server
#include "genweb.h"

static const char *TAG = "SWITCH-HTTP-SRV";

#define SWITCH_GPIO_POWER 0
#define LED_GPIO_BLUE 2
#define GPIO_LV_HIGH 0
#define GPIO_LV_LOW 1

static void initialise_mdns(void)
{
    char* hostname = "esp01s";
    //initialize mDNS
    ESP_ERROR_CHECK( mdns_init() );
    //set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK( mdns_hostname_set(hostname) );
    ESP_LOGI(TAG, "mDNS hostname set to: [%s.local]", hostname);
}

static void initLEDs(void) {
    gpio_set_direction(SWITCH_GPIO_POWER, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_BLUE, GPIO_MODE_OUTPUT);
    gpio_set_level(SWITCH_GPIO_POWER, GPIO_LV_LOW);
    gpio_set_level(LED_GPIO_BLUE, GPIO_LV_LOW);
}

static void switchLEDs(bool bSwitchOn)
{
    if (bSwitchOn) {
        gpio_set_level(SWITCH_GPIO_POWER, GPIO_LV_HIGH);
        gpio_set_level(LED_GPIO_BLUE, GPIO_LV_HIGH);
        ESP_LOGI(TAG, "Turn on the power LED");
    } else {
        gpio_set_level(SWITCH_GPIO_POWER, GPIO_LV_LOW);
        gpio_set_level(LED_GPIO_BLUE, GPIO_LV_LOW);
        ESP_LOGI(TAG, "Turn off the power LED");
    }
}

/* An HTTP GET handler */
esp_err_t root_get_handler(httpd_req_t *req)
{
    bool bStatusChange = false;
    bool switchStatus = false;
    char*  buf;
    size_t buf_len;
    // Read URL query string length and allocate memory for length + 1, extra byte for null termination
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            if (strcmp(buf, "switch_on") == 0) {
                bStatusChange = true;
                switchStatus = true;
                switchLEDs(switchStatus);
            } else if (strcmp(buf, "switch_off") == 0) {
                bStatusChange = true;
                switchStatus = false;
                switchLEDs(switchStatus);
            }
        }
        free(buf);
    }
    // Send response as the string passed in user context
    char* resp_str = getWebPage(bStatusChange, switchStatus);
    httpd_resp_send(req, (const char*)resp_str, strlen(resp_str));
    free(resp_str);
    return ESP_OK;
}

httpd_uri_t rootUri = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = root_get_handler,
    // pass response string in user context to demonstrate it's usage
    .user_ctx = "<html><body><h1 style=\"color:black; font-size:30px\">ESP01S Relay Controller :> </h1><p><a href=\"?switch_on\"><button>Switch ON</button></a></p><p><a href=\"?switch_off\"><button>Switch OFF</button></a></p></body></html>"
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handler /");
        httpd_register_uri_handler(server, &rootUri);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static httpd_handle_t server = NULL;

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    initialise_mdns();
    initLEDs();
    server = start_webserver();
}
