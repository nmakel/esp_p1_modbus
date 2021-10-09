#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ModbusIP_ESP8266.h>
#include <settings.h>


ModbusIP mbServer;
WiFiClient wifiClient;

char telegram[P1_MAX_LEN];
unsigned int currentCRC = 0;
long LAST_RECONNECT_ATTEMPT = 0;
long LAST_UPDATE_SENT = 0;

const char* ssid = STA_SSID;
const char* password = STA_PSK;


void setup() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    Serial.begin(P1_BAUD_RATE, SERIAL_8N1, SERIAL_FULL);
    USC0(UART0) = USC0(UART0) | BIT(UCRXI);

    setup_ota();

    mbServer.server(MODBUS_TCP_PORT);
    setup_registers();
}

void loop() {
    ArduinoOTA.handle();
    long now = millis();

    mbServer.task();

    if (now - LAST_UPDATE_SENT > UPDATE_INTERVAL) {
        read_p1();
    }
}


void setup_registers() {
    mbServer.addHreg(addr_IMPORT_ENERGY_ACTIVE_LOW, 0, 2);
    mbServer.addHreg(addr_IMPORT_ENERGY_ACTIVE_HIGH, 0, 2);
    mbServer.addHreg(addr_EXPORT_ENERGY_ACTIVE_LOW, 0, 2);
    mbServer.addHreg(addr_EXPORT_ENERGY_ACTIVE_HIGH, 0, 2);
    mbServer.addHreg(addr_TARIFF, 0, 2);
    mbServer.addHreg(addr_IMPORT_POWER_ACTIVE, 0, 2);
    mbServer.addHreg(addr_EXPORT_POWER_ACTIVE, 0, 2);
    mbServer.addHreg(addr_TOTAL_POWER_FAILURES, 0, 2);
    mbServer.addHreg(addr_TOTAL_POWER_FAILURES_LONG, 0, 2);
    mbServer.addHreg(addr_L1_VOLTAGE_SAGS, 0, 2);
    mbServer.addHreg(addr_L2_VOLTAGE_SAGS, 0, 2);
    mbServer.addHreg(addr_L3_VOLTAGE_SAGS, 0, 2);
    mbServer.addHreg(addr_L1_VOLTAGE_SWELLS, 0, 2);
    mbServer.addHreg(addr_L2_VOLTAGE_SWELLS, 0, 2);
    mbServer.addHreg(addr_L3_VOLTAGE_SWELLS, 0, 2);
    mbServer.addHreg(addr_L1_VOLTAGE, 0, 2);
    mbServer.addHreg(addr_L2_VOLTAGE, 0, 2);
    mbServer.addHreg(addr_L3_VOLTAGE, 0, 2);
    mbServer.addHreg(addr_L1_CURRENT, 0, 2);
    mbServer.addHreg(addr_L2_CURRENT, 0, 2);
    mbServer.addHreg(addr_L3_CURRENT, 0, 2);
    mbServer.addHreg(addr_L1_IMPORT_POWER_ACTIVE, 0, 2);
    mbServer.addHreg(addr_L2_IMPORT_POWER_ACTIVE, 0, 2);
    mbServer.addHreg(addr_L3_IMPORT_POWER_ACTIVE, 0, 2);
    mbServer.addHreg(addr_L1_EXPORT_POWER_ACTIVE, 0, 2);
    mbServer.addHreg(addr_L2_EXPORT_POWER_ACTIVE, 0, 2);
    mbServer.addHreg(addr_L3_EXPORT_POWER_ACTIVE, 0, 2);
    mbServer.addHreg(addr_IMPORT_GAS_VOLUME, 0, 2);
}


void read_p1() {
    if (Serial.available()) {
        memset(telegram, 0, sizeof(telegram));

        while (Serial.available()) {
            ESP.wdtDisable();
            int len = Serial.readBytesUntil('\n', telegram, P1_MAX_LEN);
            ESP.wdtEnable(1);

            process_p1(len);
        }
    }
}

void process_p1(int len) {
    telegram[len] = '\n';
    telegram[len + 1] = 0;
    yield();

    bool result = decode_p1(len + 1);

    if (result) {
        LAST_UPDATE_SENT = millis();
    }
}

bool decode_p1(int len) {
    int startChar = find_char_in_array_rev(telegram, '/', len);
    int endChar = find_char_in_array_rev(telegram, '!', len);
    bool validCRCFound = false;

    if (startChar >= 0) {
        currentCRC = crc16(0x0000, (unsigned char *) telegram + startChar, len - startChar);
    } else if (endChar >= 0) {
        currentCRC = crc16(currentCRC, (unsigned char*) telegram + endChar, 1);

        char messageCRC[5];
        strncpy(messageCRC, telegram + endChar + 1, 4);

        messageCRC[4] = 0;
        validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);

        currentCRC = 0;
    } else {
        currentCRC = crc16(currentCRC, (unsigned char*) telegram, len);
    }

    if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0) {
        IMPORT_ENERGY_ACTIVE_LOW = get_value(telegram, len, '(', '*');
        update_register(addr_IMPORT_ENERGY_ACTIVE_LOW, IMPORT_ENERGY_ACTIVE_LOW);
    }

    if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0) {
        IMPORT_ENERGY_ACTIVE_HIGH = get_value(telegram, len, '(', '*');
        update_register(addr_IMPORT_ENERGY_ACTIVE_HIGH, IMPORT_ENERGY_ACTIVE_HIGH);
    }

    if (strncmp(telegram, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0) {
        EXPORT_ENERGY_ACTIVE_LOW = get_value(telegram, len, '(', '*');
        update_register(addr_EXPORT_ENERGY_ACTIVE_LOW, EXPORT_ENERGY_ACTIVE_LOW);
    }

    if (strncmp(telegram, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0) {
        EXPORT_ENERGY_ACTIVE_HIGH = get_value(telegram, len, '(', '*');
        update_register(addr_EXPORT_ENERGY_ACTIVE_HIGH, EXPORT_ENERGY_ACTIVE_HIGH);
    }

    if (strncmp(telegram, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0) {
        TARIFF = get_value(telegram, len, '(', ')');
        update_register(addr_TARIFF, TARIFF);
    }

    if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0) {
        IMPORT_POWER_ACTIVE = get_value(telegram, len, '(', '*');
        update_register(addr_IMPORT_POWER_ACTIVE, IMPORT_POWER_ACTIVE); 
    }

    if (strncmp(telegram, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0) {
        EXPORT_POWER_ACTIVE = get_value(telegram, len, '(', '*');
        update_register(addr_EXPORT_POWER_ACTIVE, EXPORT_POWER_ACTIVE);
    }

    if (strncmp(telegram, "0-0:96.7.21", strlen("0-0:96.7.21")) == 0) {
        TOTAL_POWER_FAILURES = get_value(telegram, len, '(', ')');
        update_register(addr_TOTAL_POWER_FAILURES, TOTAL_POWER_FAILURES);
    }

    if (strncmp(telegram, "0-0:96.7.9", strlen("0-0:96.7.9")) == 0) {
        TOTAL_POWER_FAILURES_LONG = get_value(telegram, len, '(', ')');
        update_register(addr_TOTAL_POWER_FAILURES_LONG, TOTAL_POWER_FAILURES_LONG);
    }

    if (strncmp(telegram, "1-0:32.32.0", strlen("1-0:32.32.0")) == 0) {
        L1_VOLTAGE_SAGS = get_value(telegram, len, '(', ')');
        update_register(addr_L1_VOLTAGE_SAGS, L1_VOLTAGE_SAGS);
    }

    if (strncmp(telegram, "1-0:52.32.0", strlen("1-0:52.32.0")) == 0) {
        L2_VOLTAGE_SAGS = get_value(telegram, len, '(', ')');
        update_register(addr_L2_VOLTAGE_SAGS, L2_VOLTAGE_SAGS);
    }

    if (strncmp(telegram, "1-0:72.32.0", strlen("1-0:72.32.0")) == 0) {
        L3_VOLTAGE_SAGS = get_value(telegram, len, '(', ')');
        update_register(addr_L3_VOLTAGE_SAGS, L3_VOLTAGE_SAGS);
    }

    if (strncmp(telegram, "1-0:32.36.0", strlen("1-0:32.36.0")) == 0) {
        L1_VOLTAGE_SWELLS = get_value(telegram, len, '(', ')');
        update_register(addr_L1_VOLTAGE_SWELLS, L1_VOLTAGE_SWELLS);
    }

    if (strncmp(telegram, "1-0:52.36.0", strlen("1-0:52.36.0")) == 0) {
        L2_VOLTAGE_SWELLS = get_value(telegram, len, '(', ')');
        update_register(addr_L2_VOLTAGE_SWELLS, L2_VOLTAGE_SWELLS);
    }

    if (strncmp(telegram, "1-0:72.36.0", strlen("1-0:72.36.0")) == 0) {
        L3_VOLTAGE_SWELLS = get_value(telegram, len, '(', ')');
        update_register(addr_L3_VOLTAGE_SWELLS, L3_VOLTAGE_SWELLS);
    }

    if (strncmp(telegram, "1-0:32.7.0", strlen("1-0:32.7.0")) == 0) {
        L1_VOLTAGE = get_value(telegram, len, '(', '*');
        update_register(addr_L1_VOLTAGE, L1_VOLTAGE);
    }

    if (strncmp(telegram, "1-0:52.7.0", strlen("1-0:52.7.0")) == 0) {
        L2_VOLTAGE = get_value(telegram, len, '(', '*');
        update_register(addr_L2_VOLTAGE, L2_VOLTAGE);
    }

    if (strncmp(telegram, "1-0:72.7.0", strlen("1-0:72.7.0")) == 0) {
        L3_VOLTAGE = get_value(telegram, len, '(', '*');
        update_register(addr_L3_VOLTAGE, L3_VOLTAGE);
    }

    if (strncmp(telegram, "1-0:31.7.0", strlen("1-0:31.7.0")) == 0) {
        L1_CURRENT = get_value(telegram, len, '(', '*');
        update_register(addr_L1_CURRENT, L1_CURRENT);
    }

    if (strncmp(telegram, "1-0:51.7.0", strlen("1-0:51.7.0")) == 0) {
        L2_CURRENT = get_value(telegram, len, '(', '*');
        update_register(addr_L2_CURRENT, L2_CURRENT);
    }

    if (strncmp(telegram, "1-0:71.7.0", strlen("1-0:71.7.0")) == 0) {
        L3_CURRENT = get_value(telegram, len, '(', '*');
        update_register(addr_L3_CURRENT, L3_CURRENT);
    }

    if (strncmp(telegram, "1-0:21.7.0", strlen("1-0:21.7.0")) == 0) {
        L1_IMPORT_POWER_ACTIVE = get_value(telegram, len, '(', '*');
        update_register(addr_L1_IMPORT_POWER_ACTIVE, L1_IMPORT_POWER_ACTIVE);
    }

    if (strncmp(telegram, "1-0:41.7.0", strlen("1-0:41.7.0")) == 0) {
        L2_IMPORT_POWER_ACTIVE = get_value(telegram, len, '(', '*');
        update_register(addr_L2_IMPORT_POWER_ACTIVE, L2_IMPORT_POWER_ACTIVE);
    }

    if (strncmp(telegram, "1-0:61.7.0", strlen("1-0:61.7.0")) == 0) {
        L3_IMPORT_POWER_ACTIVE = get_value(telegram, len, '(', '*');
        update_register(addr_L3_IMPORT_POWER_ACTIVE, L3_IMPORT_POWER_ACTIVE);
    }

    if (strncmp(telegram, "1-0:22.7.0", strlen("1-0:22.7.0")) == 0) {
        L1_EXPORT_POWER_ACTIVE = get_value(telegram, len, '(', '*');
        update_register(addr_L1_EXPORT_POWER_ACTIVE, L1_EXPORT_POWER_ACTIVE);
    }

    if (strncmp(telegram, "1-0:42.7.0", strlen("1-0:42.7.0")) == 0) {
        L2_EXPORT_POWER_ACTIVE = get_value(telegram, len, '(', '*');
        update_register(addr_L2_EXPORT_POWER_ACTIVE, L2_EXPORT_POWER_ACTIVE);
    }

    if (strncmp(telegram, "1-0:62.7.0", strlen("1-0:62.7.0")) == 0) {
        L3_EXPORT_POWER_ACTIVE = get_value(telegram, len, '(', '*');
        update_register(addr_L3_EXPORT_POWER_ACTIVE, L3_EXPORT_POWER_ACTIVE);
    }
    
    if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0) {
        IMPORT_GAS_VOLUME = get_value(telegram, len, '(', '*');
        update_register(addr_IMPORT_GAS_VOLUME, IMPORT_GAS_VOLUME);
    }
    
    return validCRCFound;
}

int find_char_in_array_rev(char array[], char c, int len) {
    for (int i = len - 1; i >= 0; i--) {
        if (array[i] == c) {
            return i;
        }
    }
    
    return -1;
}

unsigned int crc16(unsigned int crc, unsigned char *buf, int len) {
    for (int pos = 0; pos < len; pos++) {
        crc ^= (unsigned int)buf[pos];
        
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

bool is_number(char *res, int len) {
    for (int i = 0; i < len; i++) {
        if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0)) {
            return false;
        }
    }

    return true;
}

long get_value(char *buffer, int maxlen, char startchar, char endchar) {
    int s = find_char_in_array_rev(buffer, startchar, maxlen - 2);
    int l = find_char_in_array_rev(buffer, endchar, maxlen - 2) - s - 1;
    
    char res[16];
    memset(res, 0, sizeof(res));

    if (strncpy(res, buffer + s + 1, l)) {
        if (endchar == '*') {
            if (is_number(res, l)) {
                return (1000 * atof(res));
            }
        } else if (endchar == ')') {
            if (is_number(res, l)) {
                return atof(res);
            }
        }
    }

    return 0;
}

void update_register(int address, long value) {
    int16_t v_high = value >> 16;
    int16_t v_low = (int16_t)value;
    mbServer.Hreg(address, v_high);
    mbServer.Hreg(address + 1, v_low);
}

void setup_ota() {
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
}