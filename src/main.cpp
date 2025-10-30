#include "hit.hpp"
#include "wireless.hpp"
#include <Arduino.h>
#include <OneButton.h>

OneButton button(9, true); // GPIO9, 内部上拉
bool g_click = false;
bool g_long_click = false;

void handleClick() {
    Serial.println("短按");
    g_click = true;
}

void handleDoubleClick() {
    Serial.println("双按");
}

void handleLongPressStart() {
    Serial.println("长按开始");
    g_long_click = true;
}

void handleLongPressStop() {
    Serial.println("长按结束");
}

void loop2(void *pvParameters);

void setup() {
    Serial.begin(115200);

    // 绑定回调函数
    button.attachClick(handleClick);
    button.attachDoubleClick(handleDoubleClick);
    button.attachLongPressStart(handleLongPressStart);
    button.attachLongPressStop(handleLongPressStop);
    button.setClickMs(100); // 短按时间（毫秒）
    button.setPressMs(800); // 长按时间（毫秒）

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
    if (Wireless::peer_packet.is_red != peer_packet_last.is_red) {
        Hit::color = Wireless::peer_packet.is_red ? Hit::RED : Hit::BLUE;
    }
    // 击打次数变化
    if (Wireless::peer_packet.hit_cnt != peer_packet_last.hit_cnt) {
        Hit::last_hit_time =  millis() / 1000.0f;
    }
    peer_packet_last = Wireless::peer_packet;

    // 发送自己的数据包
    Wireless::my_packet.hit_cnt = Hit::hit_cnt;
    Wireless::my_packet.is_red = (Hit::color == Hit::RED) ? 1 : 0;
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
            } else { // 正常运行业务逻辑
                app_onLoop();
            }
        } else {           // 在配对中
            if (g_click) { // 短按保存配对
                Wireless::savePairing();
                Hit::color = Hit::RED;
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
