#ifndef _SWITCH_WIFICONFIGWEB_H_
#define _SWITCH_WIFICONFIGWEB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "softAP.h"

char* _doGenWiFiConfigPage() {
    char* strWebPage = malloc(3072);
    char* strHtmlTemplate = "\
<html> \
<head> \
    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/> \
    <title>WiFi Config Page For SmartOnOff</title> \
    <style> \
        html { \
            font-family: Arial; \
            display: inline-block; \
            margin: 0px auto; \
            text-align: center; \
        } \
    </style> \
</head> \
\
<body> \
    <br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/> \
    <div style=\"text-align: center; margin-top: 60px;\"> \
        <h1 style=\"font-size:39px\">WiFi Config Page :></h1> \
        <div class=\"text\"> \
            <input type=\"text\" placeholder=\"SSID ( 支持中文; 区分大小写; 最多 30 个字符 )\" id=\"ssid\" required maxlength=30 style=\"width: 626px; height: 80px; font-size: 21pt; margin-top: 20px;\"> \
        </div> \
        <div class=\"text\"> \
            <input type=\"text\" placeholder=\"Password ( Case sensitive; Max 30 characters )\" id=\"pwd\" maxlength=30 style=\"width: 626px; height: 80px; font-size: 21pt; margin-top: 20px;\"> \
        </div> \
        <button onclick=\"save()\" style=\"margin-top: 50px; width: 200px; height: 80px; font-size: 25pt;\">Save</button> \
    </div> \
    <script type=\"text/javascript\"> \
        function save() { \
            const ssid = document.getElementById(\"ssid\").value.trim(); \
            const password = document.getElementById(\"pwd\").value.trim(); \
            console.log(`ssid = ${ssid}, pwd = ${password}`); \
            if (ssid === \"\") { \
                alert(\"Please input WiFi SSID ...\"); \
            } else { \
                fetch(\"http://%s.%s/wifi_config\", { \
                    method: \"POST\", \
                    headers: { \
                        \"Accept\": \"application/json\", \
                        \"Content-Type\": \"application/json\" \
                    }, \
                    body: JSON.stringify({ ssid, pwd: password }) \
                }); \
                alert(\"Save OK.\"); \
                setTimeout(() => { window.location.href = \"http://%s.%s/\"; }, 10000); \
            } \
        } \
    </script> \
</body> \
</html> \
";
    char* softApName = generate_ap_name();
    sprintf(strWebPage, strHtmlTemplate, softApName, "local", softApName, "local");
    free(softApName);
    return strWebPage;
}

char* getWiFiConfigPage() {
    return _doGenWiFiConfigPage();
}

#ifdef __cplusplus
}
#endif

#endif /* _SWITCH_WIFICONFIGWEB_H_ */
