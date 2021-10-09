#define USE_HOLDING_REGISTERS_ONLY
#define P1_BAUD_RATE 115200
#define P1_MAX_LEN   1050

#define HOSTNAME ""
#define OTA_PASSWORD ""
#define MODBUS_TCP_PORT 1502
#define UPDATE_INTERVAL 1000

#define STA_SSID ""
#define STA_PSK  ""


long IMPORT_ENERGY_ACTIVE_LOW;
long IMPORT_ENERGY_ACTIVE_HIGH;
long EXPORT_ENERGY_ACTIVE_LOW;    
long EXPORT_ENERGY_ACTIVE_HIGH;   
long TARIFF;                      
long IMPORT_POWER_ACTIVE;         
long EXPORT_POWER_ACTIVE;         
long TOTAL_POWER_FAILURES;        
long TOTAL_POWER_FAILURES_LONG;   
long L1_VOLTAGE_SAGS;             
long L2_VOLTAGE_SAGS;             
long L3_VOLTAGE_SAGS;             
long L1_VOLTAGE_SWELLS;           
long L2_VOLTAGE_SWELLS;           
long L3_VOLTAGE_SWELLS;           
long L1_VOLTAGE;                  
long L2_VOLTAGE;                  
long L3_VOLTAGE;                  
long L1_CURRENT;                  
long L2_CURRENT;                  
long L3_CURRENT;                  
long L1_IMPORT_POWER_ACTIVE;      
long L2_IMPORT_POWER_ACTIVE;      
long L3_IMPORT_POWER_ACTIVE;      
long L1_EXPORT_POWER_ACTIVE;      
long L2_EXPORT_POWER_ACTIVE;      
long L3_EXPORT_POWER_ACTIVE;      
long IMPORT_GAS_VOLUME;


const int addr_IMPORT_ENERGY_ACTIVE_LOW = 0;
const int addr_IMPORT_ENERGY_ACTIVE_HIGH = 2;
const int addr_EXPORT_ENERGY_ACTIVE_LOW = 4;
const int addr_EXPORT_ENERGY_ACTIVE_HIGH = 6;
const int addr_TARIFF = 8;
const int addr_IMPORT_POWER_ACTIVE = 10;
const int addr_EXPORT_POWER_ACTIVE = 12;
const int addr_TOTAL_POWER_FAILURES = 14;
const int addr_TOTAL_POWER_FAILURES_LONG = 16;
const int addr_L1_VOLTAGE_SAGS = 18;
const int addr_L2_VOLTAGE_SAGS = 20;
const int addr_L3_VOLTAGE_SAGS = 22;
const int addr_L1_VOLTAGE_SWELLS = 24;
const int addr_L2_VOLTAGE_SWELLS = 26;
const int addr_L3_VOLTAGE_SWELLS = 28;
const int addr_L1_VOLTAGE = 30;
const int addr_L2_VOLTAGE = 32;
const int addr_L3_VOLTAGE = 34;
const int addr_L1_CURRENT = 36;
const int addr_L2_CURRENT = 38;
const int addr_L3_CURRENT = 40;
const int addr_L1_IMPORT_POWER_ACTIVE = 42;
const int addr_L2_IMPORT_POWER_ACTIVE = 44;
const int addr_L3_IMPORT_POWER_ACTIVE = 46;
const int addr_L1_EXPORT_POWER_ACTIVE = 48;
const int addr_L2_EXPORT_POWER_ACTIVE = 50;
const int addr_L3_EXPORT_POWER_ACTIVE = 52;
const int addr_IMPORT_GAS_VOLUME = 54;