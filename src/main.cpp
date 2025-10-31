#include "hit.hpp"
#include "wireless.hpp"
#include <Arduino.h>
#include <OneButton.h>
#include <Preferences.h>

static constexpr auto TIMEOUT_MS = 100;

Preferences preferences;
OneButton button(9, true); // GPIO9, 内部上拉
bool g_click = false;
bool g_long_click = false;
uint32_t g_last_cdc_receive_ms = 0;

void handleClick() {
    g_click = true;
}

void handleLongPressStart() {
    g_long_click = true;
}

void serialEvent() {
    uint8_t data[10];
    uint32_t i = 0;
    while (Serial.available()) {
        if (i < 10) {
            data[i++] = Serial.read();
        } else {
            i++;
            Serial.read();
        }
    }
    if (i == 10) {
        memcpy(Wireless::my_packet.dbus, data, 10);
        g_last_cdc_receive_ms = millis();
    }
}

void loop2(void *pvParameters);

void setup() {
    preferences.begin("armor", false);

    Serial.begin(115200);
    Serial0.begin(115200, SERIAL_8N1, 20, 21);

    // 绑定回调函数
    button.attachClick(handleClick);
    button.attachLongPressStart(handleLongPressStart);
    button.setClickMs(50);  // 短按时间（毫秒）
    button.setPressMs(500); // 长按时间（毫秒）

    Hit::begin();
    Wireless::begin();

    xTaskCreate(loop2, "loop2", 8192, NULL, 1, NULL);
}

void loop() {
    Hit::onLoop();
}

void app_onLoop() {
    static Wireless::packet_t peer_packet_last;

    // 颜色变化
    if (Wireless::peer_packet.color != peer_packet_last.color && Wireless::peer_packet.color != 0) {
        Hit::color = Wireless::peer_packet.color;
        preferences.putUInt("color", Hit::color);
    }
    // 击打次数变化
    if (Wireless::peer_packet.hit_cnt != peer_packet_last.hit_cnt) {
        Hit::last_hit_ms = millis();
    }
    peer_packet_last = Wireless::peer_packet;

    // 串口发送DBUS数据
    if (millis() - Wireless::last_receive_ms > TIMEOUT_MS) { // 超时数据清零
        memset(Wireless::peer_packet.dbus, 0, sizeof(Wireless::peer_packet.dbus));
    }
    Serial0.write(Wireless::peer_packet.dbus, sizeof(Wireless::peer_packet.dbus));
    Serial0.flush();

    // CDC发送颜色、击打次数、最后一条消息延迟
    uint32_t latency_ms = millis() - Wireless::last_receive_ms;
    Serial.printf("%d,%d,%d\n", Wireless::peer_packet.color, Wireless::peer_packet.hit_cnt, latency_ms);
    Serial.flush();

    // 发送自己的数据包
    Wireless::my_packet.color = Hit::color;
    Wireless::my_packet.hit_cnt = Hit::hit_cnt;
    if (millis() - g_last_cdc_receive_ms > TIMEOUT_MS) { // 超时数据清零
        memset(Wireless::my_packet.dbus, 0, sizeof(Wireless::my_packet.dbus));
    }
}

void loop2(void *pvParameters) {
    while (1) {
        button.tick();
        Wireless::onLoop();

        if (!Wireless::is_pairing) { // 不在配对中
            if (g_long_click) {      // 长按配对
                Wireless::entryPairing();
                Hit::detect_hit = false;
            } else if (g_click) { // 短按切换颜色
                if (Hit::color == Hit::RED) {
                    Hit::color = Hit::BLUE;
                } else {
                    Hit::color = Hit::RED;
                }
                preferences.putUInt("color", Hit::color);
            } else { // 正常运行业务逻辑
                app_onLoop();
            }
        } else {           // 在配对中
            if (g_click) { // 短按保存配对
                Wireless::savePairing();
                Hit::color = Hit::RED;
                preferences.putUInt("color", Hit::color);
                Hit::detect_hit = true;
            } else {
                if (Wireless::pairing_found) {
                    Hit::color = Hit::GREEN;
                } else {
                    Hit::color = Hit::YELLOW;
                }
            }
        }

        g_click = false;
        g_long_click = false;

        delay(10);
    }
}
