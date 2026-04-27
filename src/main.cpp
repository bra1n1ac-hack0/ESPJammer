// ============================================================
//  ESPJAMMER v1.0
//  ESP32 Pentesting Tool Firmware
//  UI Framework with OLED menu, WiFi & BLE modules
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "esp_wifi.h"

// ── Firmware Info ────────────────────────────────────────────
#define FW_NAME     "ESPJAMMER"
#define FW_VERSION  "v1.0"
#define FW_AUTHOR   "@bra1n1ac_hack"

// ── Display ──────────────────────────────────────────────────
#define SCREEN_W   128
#define SCREEN_H    64
#define OLED_RESET  -1

// ── Buttons ──────────────────────────────────────────────────
#define BTN_UP    12
#define BTN_DOWN  13
#define BTN_SEL   14
#define BTN_BACK  27

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RESET);

// ── Menu System ──────────────────────────────────────────────
struct MenuItem {
    const char* label;
    void (*action)();
};

// Forward declarations
void menuWifi();
void menuBluetooth();
void menuBLE();
void menuSettings();
void wifiScan();
void wifiSniff();
void wifiBeacon();
void wifiBack();
void bleScan();
void bleSpam();
void bleBack();

// Main menu
MenuItem mainMenu[] = {
    { "[ WiFi ]",       menuWifi      },
    { "[ Bluetooth ]",  menuBluetooth },
    { "[ BLE ]",        menuBLE       },
    { "[ Settings ]",   menuSettings  },
};
const int mainMenuLen = 4;

// WiFi submenu
MenuItem wifiMenu[] = {
    { "  Scan APs",    wifiScan   },
    { "  Sniff Pkts",  wifiSniff  },
    { "  Beacon Spam", wifiBeacon },
    { "  < Back",      wifiBack   },
};
const int wifiMenuLen = 4;

// BLE submenu
MenuItem bleMenu[] = {
    { "  Scan Devices", bleScan  },
    { "  Apple Spam",   bleSpam  },
    { "  < Back",       bleBack  },
};
const int bleMenuLen = 3;

// ── State ────────────────────────────────────────────────────
MenuItem* currentMenu    = mainMenu;
int       currentMenuLen = mainMenuLen;
int       selectedIndex  = 0;
int       scrollOffset   = 0;

String    logLine1   = "";
String    logLine2   = "";
String    statusMode = "IDLE";

// ── Display Helpers ──────────────────────────────────────────
void drawStatusBar() {
    display.fillRect(0, 0, SCREEN_W, 10, WHITE);
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(2, 1);
    display.print(FW_NAME);
    display.setCursor(72, 1);
    display.print(statusMode);
    display.setTextColor(WHITE);
}

void drawMenu() {
    const int VISIBLE = 3;
    display.setTextSize(1);

    for (int i = 0; i < VISIBLE; i++) {
        int idx = scrollOffset + i;
        if (idx >= currentMenuLen) break;

        int y = 13 + (i * 11);

        if (idx == selectedIndex) {
            display.fillRect(0, y - 1, SCREEN_W, 10, WHITE);
            display.setTextColor(BLACK);
        } else {
            display.setTextColor(WHITE);
        }

        display.setCursor(4, y);
        display.print(currentMenu[idx].label);
    }
    display.setTextColor(WHITE);

    // Scroll indicator bar
    if (currentMenuLen > VISIBLE) {
        int barH = (VISIBLE * 33) / currentMenuLen;
        int barY = 12 + (scrollOffset * 33) / currentMenuLen;
        display.fillRect(126, barY, 2, barH, WHITE);
    }
}

void drawLogArea() {
    display.drawFastHLine(0, 47, SCREEN_W, WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(2, 49);
    display.print(logLine1.substring(0, 21));
    display.setCursor(2, 57);
    display.print(logLine2.substring(0, 21));
}

void renderUI() {
    display.clearDisplay();
    drawStatusBar();
    drawMenu();
    drawLogArea();
    display.display();
}

void setLog(String l1, String l2 = "") {
    logLine1 = l1;
    logLine2 = l2;
}

// ── Button Handling ──────────────────────────────────────────
bool btnPressed(int pin) {
    static unsigned long lastDebounce[40] = {0};
    if (digitalRead(pin) == LOW && millis() - lastDebounce[pin] > 200) {
        lastDebounce[pin] = millis();
        return true;
    }
    return false;
}

void handleButtons() {
    if (btnPressed(BTN_UP)) {
        if (selectedIndex > 0) {
            selectedIndex--;
            if (selectedIndex < scrollOffset) scrollOffset--;
        }
    }
    if (btnPressed(BTN_DOWN)) {
        if (selectedIndex < currentMenuLen - 1) {
            selectedIndex++;
            if (selectedIndex >= scrollOffset + 3) scrollOffset++;
        }
    }
    if (btnPressed(BTN_SEL)) {
        if (currentMenu[selectedIndex].action)
            currentMenu[selectedIndex].action();
    }
    if (btnPressed(BTN_BACK)) {
        currentMenu    = mainMenu;
        currentMenuLen = mainMenuLen;
        selectedIndex  = 0;
        scrollOffset   = 0;
        statusMode     = "IDLE";
        setLog("Ready.", "Select a module");
    }
}

// ── Menu Navigation ──────────────────────────────────────────
void switchMenu(MenuItem* menu, int len) {
    currentMenu    = menu;
    currentMenuLen = len;
    selectedIndex  = 0;
    scrollOffset   = 0;
}

void menuWifi()      { switchMenu(wifiMenu, wifiMenuLen); }
void menuBluetooth() { setLog("BT: Coming soon"); }
void menuBLE()       { switchMenu(bleMenu, bleMenuLen); }
void menuSettings()  { setLog("Settings TBD"); }

void wifiBack() {
    switchMenu(mainMenu, mainMenuLen);
    statusMode = "IDLE";
    setLog("Ready.", "Select a module");
}

void bleBack() {
    switchMenu(mainMenu, mainMenuLen);
    statusMode = "IDLE";
    setLog("Ready.", "Select a module");
}

// ── WiFi Actions ─────────────────────────────────────────────
void wifi_sniffer_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    logLine1 = "RSSI: " + String(pkt->rx_ctrl.rssi);
    logLine2 = "Len:  " + String(pkt->rx_ctrl.sig_len);
}

void wifiScan() {
    statusMode = "SCAN";
    setLog("Scanning APs...", "Please wait...");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    uint16_t apCount = 0;
    esp_wifi_scan_start(NULL, true);
    esp_wifi_scan_get_ap_num(&apCount);

    setLog("Scan done!", "APs found: " + String(apCount));
    statusMode = "IDLE";
}

void wifiSniff() {
    statusMode = "SNIFF";
    setLog("Sniffing pkts", "Ch: 1");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_cb);
}

void wifiBeacon() {
    statusMode = "BEACON";
    setLog("Beacon spam ON", "SSIDs: 10");
}

// ── BLE Actions ──────────────────────────────────────────────
void bleScan() {
    statusMode = "BLE";
    setLog("BLE scanning...", "Devices: 0");
}

void bleSpam() {
    statusMode = "SPAM";
    setLog("Apple spam ON", "Pkts sent: 0");
}

// ── Splash Screen ────────────────────────────────────────────
void showSplash() {
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(8, 4);
    display.print("ESP");
    display.setCursor(8, 22);
    display.print("JAMMER");

    display.drawFastHLine(0, 42, SCREEN_W, WHITE);

    display.setTextSize(1);
    display.setCursor(4, 46);
    display.print(FW_VERSION);
    display.setCursor(36, 46);
    display.print("by ");
    display.print(FW_AUTHOR);

    display.setCursor(4, 56);
    display.print("[ loading...  ]");
    display.display();
    delay(800);

    for (int i = 0; i <= 120; i += 6) {
        display.fillRect(4, 56, i, 7, WHITE);
        display.display();
        delay(40);
    }
    delay(600);
}

// ── Setup & Loop ─────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    pinMode(BTN_UP,   INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SEL,  INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);

    Wire.begin(21, 22);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 not found — check wiring");
        while (true);
    }

    display.setFont(NULL);
    showSplash();

    setLog("Ready.", "Select a module");
    renderUI();
}

void loop() {
    handleButtons();
    renderUI();
    delay(50);
}