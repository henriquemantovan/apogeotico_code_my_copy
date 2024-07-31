#define LED PB_15
#define BUZZER PB_5 

//BMP280 interface
#define BMP_SDA PC_9
#define BMP_SCL PA_8

//Flash memory interface
#define FLASH_MOSI PC_12
#define FLASH_MISO PC_11
#define FLASH_SCK PC_10
#define FLASH_CS PA_15
#define FLASH_WP PB_3

//EEPROM interface
#define EEPROM_SDA PB_9
#define EEPROM_SCL PB_8
#define EEPROM_WP PB_0

//Main e-match GPIOs
#define MAIN PC_1
#define MAIN_DETECT PC_3
#define MAIN_SIGNAL PA_1

//Drogue e-match GPIOs
#define DROGUE PC_4
#define DROGUE_DETECT PA_6
#define DROGUE_SIGNAL PA_4

#define DEBUG_MESSAGES

// If debug messages are active, debugPrint is used as printf, otherwise it
// will be treated as an empty function
#ifdef DEBUG_MESSAGES
#define debugPrint(...) printf(__VA_ARGS__)
#else
#define debugPrint(...)
#endif

#define FILE_PRINT
#ifdef FILE_PRINT
#define filePrint(...) printf(__VA_ARGS__)
#else
#define filePrint(...)
#endif