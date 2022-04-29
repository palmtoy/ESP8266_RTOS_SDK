#ifndef _SWITCH_GENWEB_H_
#define _SWITCH_GENWEB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/param.h>
#include "freertos/FreeRTOS.h"

struct
{
    char on[3];
    char off[4];
} G_SWITCH_DICT = {"ON", "OFF"};

struct S_SWITCH_ON
{
    char color[7];
    char disabled[9];
} G_SWITCH_ON_1 = { "gray;", "disabled" }, G_SWITCH_ON_0 = { "green;", "" };

struct S_SWITCH_OFF
{
    char color[6];
    char disabled[9];
} G_SWITCH_OFF_1 = { "red;", "" }, G_SWITCH_OFF_0 = { "gray;", "disabled" };

static bool G_SWITCH_STATUE = false;

struct S_SWITCH_ON* getSwitchOnStatus() {
    if (G_SWITCH_STATUE) {
        return &G_SWITCH_ON_1;
    } else {
        return &G_SWITCH_ON_0;
    }
}

struct S_SWITCH_OFF* getSwitchOffStatus() {
    if (G_SWITCH_STATUE) {
        return &G_SWITCH_OFF_1;
    } else {
        return &G_SWITCH_OFF_0;
    }
}

char* getSwitchStatusColor() {
    if (G_SWITCH_STATUE) {
        return "green";
    } else {
        return "red";
    }
}

char* getSwitchStatusText() {
    if (G_SWITCH_STATUE) {
        return G_SWITCH_DICT.on;
    } else {
        return G_SWITCH_DICT.off;
    }
}

char* _doGenWebPage() {
    char* strWebPage = malloc(2048);
    char* strHtmlTemplate = "\
<html> \
\
<head> \
    <title>Smart Relay Controller</title>\
    <style> \
        html { \
            font-family: Arial; \
            display: inline-block; \
            margin: 0px auto; \
            text-align: center; \
        } \
        .buttonOn { \
            background-color: %s \
            color: white; \
            padding: 80px 200px; \
            text-align: center; \
            display: inline-block; \
            font-size: 80px; \
            margin: 8px 2px; \
            cursor: pointer; \
        } \
        .buttonOff { \
            background-color: %s \
            color: white; \
            padding: 80px 180px; \
            text-align: center; \
            display: inline-block; \
            font-size: 80px; \
            margin: 12px 2px; \
            cursor: pointer; \
        } \
    </style> \
</head> \
\
<body> \
    <br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/> \
    <h1 style=\"font-size:39px\">Smart Relay Controller :></h1> \
    <h2 style=\"font-size:36px;color: %s\">Switch Status: <strong>%s</strong></h2> \
    <p> \
        <a href=\"?switch_on\"><button %s class=\"buttonOn\">Switch ON</button></a> \
    </p> \
    <p> \
        <a href=\"?switch_off\"><button %s class=\"buttonOff\">Switch OFF</button></a> \
    </p> \
    <br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/> \
    <a href=\"?wifi_config_page\" style=\"font-size:23px\">WiFi Config </a> \
</body> \
</html> \
";
    sprintf(strWebPage, strHtmlTemplate,
        getSwitchOnStatus()->color, getSwitchOffStatus()->color,
        getSwitchStatusColor(), getSwitchStatusText(),
        getSwitchOnStatus()->disabled, getSwitchOffStatus()->disabled);
    return strWebPage;
}

char* getWebPage(bool bStatusChange, bool switchStatus) {
    if (bStatusChange) {
        G_SWITCH_STATUE = switchStatus;
    }
    return _doGenWebPage();
}

#ifdef __cplusplus
}
#endif

#endif /* _SWITCH_GENWEB_H_ */
