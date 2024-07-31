#include "mbed.h"  
#include "GlobalDefines.h"
#include "BMP280.hh"
#include "USBSerial.h"
#include <chrono>
#include <cstdio>
#include <inttypes.h>
#include <I2CEEBlockDevice.h>
#include <SPIFBlockDevice.h>

//mudei os define: eles representam numeros de escolha na main, só pra ficar mais claro nas funções
#define TESTLED 1
#define TESTBUZZER 2
#define TESTUSB 1
#define USEUSB 2
#define TESTBMP 1
#define TESTDETECT 1
#define USEDETECT 2
#define ERASEEPROM 1
#define TESTEEPROM 2
#define READEEPROM 3
#define ERASEFLASH 1
#define TESTEFLASH 2
#define READFLASH 3

DigitalOut led(LED); //define o led de debug
DigitalOut buzzer(BUZZER); //define o pino do buzzer

//pinos Drogue
DigitalOut droguep(DROGUE); 
DigitalIn drogue_detect(DROGUE_DETECT);
DigitalOut drogue_signal(DROGUE_SIGNAL); 

//pinos main
DigitalOut mainp(MAIN);
DigitalIn main_detect(MAIN_DETECT);
DigitalOut main_signal(MAIN_SIGNAL);

//Seção para o usb serial
static constexpr auto UsbSerialBlocking = false;
static bool isConsoleInitialised = false;

class MyConsole : public USBSerial {
    using USBSerial::USBSerial;
 
    virtual int _putc(int c) override {
        USBSerial::_putc(c);
        return c;
    }
};

static USBSerial* get_console() {
    static MyConsole console(UsbSerialBlocking);

    if(!UsbSerialBlocking && !isConsoleInitialised) {
        console.init();
        console.connect();
        isConsoleInitialised = true;
    }

    return &console;
}
 
namespace mbed
{
   FileHandle *mbed_override_console(int fd)
    {
        return get_console();
    }
}
//Seção para o usb serial

int continuar_encerrar() {
    char continuar;

    printf("\n \nContinuar testes (s/n)? ");
    scanf(" %c", &continuar); 

    if (continuar == 'n' || continuar == 'N') {
        return 0; 
    }
    return 1;
}
 
void teste_Led_ou_Buzzer(int option) {

    printf ("Executando teste do Led e do Buzzer") ;
    for(int k = 0; k < 10; k++) {
        if (option == TESTLED){
        //led = !led;
        }
        if (option == TESTBUZZER){
        buzzer = !buzzer;
        }

        ThisThread::sleep_for(500ms);
        }      
}

void usbTESTE(int optionUSB) {

    //USBSerial key(false);
    wait_us(1e6);

    if (optionUSB == TESTUSB){
        for(int k = 0; k < 10; k++) {
        printf("\nInterface USB funcional");

        ThisThread::sleep_for(1s);
        }
    }

    if (optionUSB == USEUSB){
        printf("Interface USB funcional");
        ThisThread::sleep_for(1s);
    }

}

void bmpTESTE(int optionBMP) {

    // bmp(sda, scl)
    BMP280 bmp(BMP_SDA, BMP_SCL, FAST_FREQUENCY);

    while(bmp.init() == 0);
    printf("\nIniciando o BMP280...\r\n");
    //bmp.init();

    ThisThread::sleep_for(1s);

    // printf("Entrou1\r\n");

    float pressure;
    int32_t *pAux;
    float temperature;
    // printf("Entrou2\r\n");
    if (optionBMP == TESTBMP){
        for(int i = 0; i < 30; i++){
            bmp.startMeasure();
        //printf("Entrou3\r\n");
            bmp.getPressure(&pressure);
            wait_us(7000);
        //printf("Entrou4\r\n");
            bmp.getTemperature(&temperature);
        //printf("Entrou5\r\n");
        
            printf("Pressure:  %.2f  Temperature:  %.2f\r\n", pressure, temperature);
        };
    }
    //Fecha o TESTBMP

}

void detectTESTE(int optionDETECT){
    if (optionDETECT == TESTDETECT){
        mainp = 0;
        droguep = 0;
        drogue_signal = 1;
        main_signal = 1;
        ThisThread::sleep_for(1s);
        if (drogue_detect == 0 && main_detect == 0){
            printf("Drogue AND Main detected :)\r\n");
        }
        if (drogue_detect == 1){
        printf("Drogue NOT detected\r\n");
        }
        if (main_detect == 1){
            printf("Main NOT detected\r\n");
        }
    }
    if (optionDETECT == USEDETECT){
        buzzer = 0;
        drogue_signal = 1;
        main_signal = 1;
        if (drogue_detect == 0 && main_detect == 0){
            printf("ACTIVATED!\r\n");
            buzzer = 1;
            wait_us(3e6);
            buzzer = 0;
            droguep = 1;
            mainp = 1;
            wait_us(1e6);
            droguep = 0;
            mainp = 0;
        }
        else {
            printf("NOT ACTIVATED!\r\n");
        }
    }
}

void epromTESTE(int optionEPROM){

    DigitalOut eepromWP(EEPROM_WP);
    eepromWP = 0;
    typedef union{
        float pressure;
        int8_t pressure_bytes[sizeof(float)];
    } float4bytes;

    Timer t;

    float4bytes p;
    p.pressure = 0.0;
    float storedPressure;
    I2CEEBlockDevice eeprom(EEPROM_SDA, EEPROM_SCL, 0xA0, 64*1024, 128, 400000);
    int addr = 0;
    int lastAddr = 64000 - 1;

    // BMP280 bmp(BMP_SDA, BMP_SCL, FAST_FREQUENCY);
    // while(bmp.init() == 0);
    // printf("BMP280 Initialized\r\n");

    eeprom.init();
    printf("EEPROM Initialized\r\n");

    if (optionEPROM == ERASEEPROM){
        for(int i = 0; i < 64000; i+= 4) {
            eeprom.program(p.pressure_bytes, i, 4);
        }
    }

    if (optionEPROM == TESTEEPROM){
        printf("TESTING EEPROM\r\n");   
        for(int k = 0; k < 100; k++) {

            //espera BMP medir pressao
            while(!bmp.startMeasure());

            t.reset();
            t.start();

            wait_us(6400);

            while(t.elapsed_time() < 9ms);

            while(!bmp.getPressure(&p.pressure));

            memcpy(&storedPressure, p.pressure_bytes, sizeof(float));

            eeprom.program(p.pressure_bytes, k*4, 4);
            ThisThread::sleep_for(1ms);
            addr += 4;
            // printf("%d ADDRESS\r\n", k/4);
        }
    }

    if (optionEPROM == READEEPROM){
        int k = 0;
        uint8_t* buffer = (uint8_t*)malloc(4);
        addr = 0;

        printf("READING EEPROM\r\n");
        while(k < 10000){
            eeprom.init();
            eeprom.read(buffer,addr,4);
            memcpy(&storedPressure, buffer, 4);
            ThisThread::sleep_for(5ms);
            //wait_us()
            //printf("%f\r\n", 1e-5*(storedPressure));
            printf("%d: %f\r\n", k, (storedPressure));
            addr += 4;
            eeprom.deinit();
            k++;
        }

        eeprom.deinit();
        printf("FIM!\r\n");
    }
}

void flashTESTE(int optionFLASH){

    DigitalOut flashWP (FLASH_WP);
    flashWP = 1;
    printf("INIT TESTFLASH\r\n");
    SPIFBlockDevice flash(FLASH_MOSI, FLASH_MISO, FLASH_SCK, FLASH_CS);
    flash.init();

    typedef union{
        float pressure;
        uint8_t pressure_bytes[sizeof(float)];
    } float4bytes;

    float4bytes pUnion;
    float p = 0.0;
    float readP = 69.0;    

    int addr = 0;

    Timer tempoDeEscrita;
    
    if (optionFLASH == ERASEFLASH){
        flash.erase(0, flash.size());
    }

    if (optionFLASH == TESTEFLASH){
        for(int k = 0; k < 100; k++) {
            bmp.startMeasure();
            wait_us(7000);
            bmp.getPressure(&p);

            pUnion.pressure = p*1e-5;
    
            printf("Pressure UPDATED for flash writing: %f\r\n", pUnion.pressure);
            tempoDeEscrita.reset();
            tempoDeEscrita.start();
            flash.program(pUnion.pressure_bytes, k*4, 4);

            tempoDeEscrita.stop();

            printf("Tempo de escrita: %llu us\r\n", std::chrono::duration_cast<std::chrono::microseconds>(tempoDeEscrita.elapsed_time()).count());

        }
    }

    if (optionFLASH == READFLASH){
        printf("READING Flash\r\n");
        for(int i = 0; i < 100; i++) {
                flash.read(&readP, i*4, 4);
        printf("Dado lido na memoria: %f\r\n", readP);        
        }   
    }
}

void eletrorecTESTE(){
    #define COUNT_TIME(t) std::chrono::duration_cast<std::chrono::milliseconds>(Timer::t.elapsed_time()).count

    //Também para evitar mesmo erro chrono
    using namespace std::chrono;
            
            
        //I2C EEPROM(EEPROM_SDA, EEPROM_SCL);
    I2CEEBlockDevice eeprom(EEPROM_SDA, EEPROM_SCL, 0xA0, 64*1024, 128, 400000);
    int addr = 0;
    SPIFBlockDevice flash(FLASH_MOSI, FLASH_MISO, FLASH_SCK, FLASH_CS);
    flash.init();
    DigitalOut flashWP (FLASH_WP);
    flashWP = 1;

    typedef union{
    float pressure;
    int8_t pressure_bytes[sizeof(float)];
    } float4bytes;


    DigitalIn detectDrogue(DROGUE_DETECT);
    DigitalOut signalDrogue(DROGUE_SIGNAL);
    DigitalOut activateDrogue(DROGUE);

    DigitalIn detectMain(MAIN_DETECT);
    DigitalOut signalMain(MAIN_SIGNAL);
    DigitalOut activateMain(MAIN);

    DigitalOut eepromWP(EEPROM_WP);

    DigitalOut buzz(BUZZER);

    mbed::Timer tempoTotal;
    //Declaração da EEPROM
    eeprom.init();
    eepromWP = 0;    

    float4bytes p;
    int lastAddr = 64000 - 1;
    float storedPressure;
    int i = 0;
    int m = 0;
    int n = 0;

    buzz = 0;
    activateDrogue = 0;
    activateMain = 0;
    signalDrogue = 1;
    signalMain = 1;

    BMP280 bmp(BMP_SDA, BMP_SCL, FAST_FREQUENCY);

    while(bmp.init() == 0);

    buzz = 1;
    wait_us(3e6);
    buzz = 0;


    mbed::Timer tempo;

    while(!bmp.startMeasure());

    // wait measurement time (6.4ms)
    wait_us(6400);
    while(!bmp.getPressure(&p.pressure));
    tempoTotal.start();

    if (detectDrogue == 0 && detectMain == 0){
        if (i == 0){
            buzz = 1;
            wait_us(1e6);
            buzz = 0;
            wait_us(1e6);
            buzz = 1;
            wait_us(1e6);
            buzz = 0;
            wait_us(1e6);
            signalDrogue = 0;
            signalMain = 0;
            i = 1;
        }

        // tempo.start();

        while(tempoTotal.elapsed_time() < 50000ms) {
            //printf("Passou pelo while\r\n");            

            // reinicia timer
            tempo.reset();

            // inicia timer do tempo necessário para leitura de pressão
            tempo.start();

            while(!bmp.startMeasure());

            if (addr < lastAddr){
                memcpy(&storedPressure, p.pressure_bytes, sizeof(float));
                eeprom.program(p.pressure_bytes,addr,4);
                flash.program(p.pressure_bytes,addr,4);
                addr += 4;
                //printf("gravou\r\n");
            };
            if(tempoTotal.elapsed_time() > 15000ms && tempoTotal.elapsed_time() < 20000ms) buzz = 1;
            if (tempoTotal.elapsed_time() > 20000ms && tempoTotal.elapsed_time() < 35000ms && n == 0) {
                //printf("PASSOU 1\r\n");
                // wait_us(5e6);
                activateDrogue = 1;
                buzz = 0;
                n = 1;
            }
            if(tempoTotal.elapsed_time() > 35000ms && tempoTotal.elapsed_time() < 40000ms) buzz = 1;
            if (tempoTotal.elapsed_time() > 40000ms && tempoTotal.elapsed_time() < 45000ms && m == 0){
                //printf("PASSOU 2\r\n");
                buzz = 0;
                // wait_us(5e6);
                activateMain = 1;
                // buzz = 0;
                m = 1;
            }
            // wait_us(1e6); 
            // activateDrogue = 0;
            // activateMain = 0; 

            while(tempo.elapsed_time() < 9ms);

            tempo.stop();

            while(!bmp.getPressure(&p.pressure));
        }
        //printf("Terminou os 50 seg\r\n");
    tempoTotal.stop();
    }    
}

int main() { 
    int option = 0;

    while (true) {
        printf ("\nEscolha um teste para executar:\n") ; //o define antigo
        printf ("1. Teste Led\n"); //TESTLED
        printf ("2. Teste Buzzer\n"); //TESTBUZZER
        printf ("3. Teste e/ou utilização do USB serial\n"); //TESTUSB //USEUSB
        printf ("4. Teste e/ou utilização de leitura do Barômetro\n"); //TESTBMP //USEBMP //esse até o ultimo precisam do USEUSB
        printf ("5. Teste e/ou utilização da detecção dos e-matchs\n"); //TESTDETECT //USEDETECT
        printf ("6. Apagamento, teste e/ou leitura da EPROM\n"); //ERASEEPROM //TESTEEPROM //READEEPROM
        printf ("7. Apagamento, teste e/ou leitura  da FLASH\n"); //ERASEFLASH //TESTEFLASH //READFLASH
        printf ("7. Teste ELETROREC\n"); //ELETROREC
        printf ("0. ENCERRAR\n"); 


        printf ("\nDigite o número do que você quer testar: ");
        scanf ("%d", &option);
        printf ("\n");

        switch (option) {
            case 1:
                teste_Led_ou_Buzzer(TESTLED);
                if (continuar_encerrar() == 0){
                    return 0;
                } 
                break;
            case 2:
                teste_Led_ou_Buzzer(TESTBUZZER);
                if (continuar_encerrar() == 0){
                    return 0;
                }
                break;
            case 3:
                printf("Testar o USB ou começar a utiliza-lo:\n1. Testar\n2. Utilizar\n");
                printf ("\nDigite o número referente a sua escolha: ");
                int optionUSB;
                scanf ("%d", &optionUSB);

                usbTESTE(optionUSB);
                if (continuar_encerrar() == 0){
                    return 0;
                }
                break;
            case 4:
                usb(USEUSB);

                printf("\nTestar o BMP ou começar a utiliza-lo:\n1. Testar\n2. Utilizar\n");
                printf ("\nDigite o número referente a sua escolha: ");
                int optionBMP;
                scanf ("%d", &optionBMP);
                
                bmpTESTE(optionBMP);
                if (continuar_encerrar() == 0){
                    return 0;
                }
                break;
            case 5:
                usb(USEUSB);

                printf("\nTestar a detecção ou começar a utiliza-la:\n1. Testar\n2. Utilizar\n");
                printf ("\nDigite o número referente a sua escolha: ");
                int optionDETECT;
                scanf ("%d", &optionDETECT);
                printf("\n");

                detectTESTE(optionDETECT);
                if (continuar_encerrar() == 0){
                    return 0;
                }
                break;
            case 6:
                usb(USEUSB);

                printf("\nApagar a EPROM, testar ou lê-la:\n1. Apagar\n2. Testar\n3. Ler\n");
                printf ("\nDigite o número referente a sua escolha: ");
                int optionEPROM;
                scanf ("%d", &optionEPROM);
                printf("\n");

                epromTESTE(optionEPROM);
                if (continuar_encerrar() == 0){
                    return 0;
                }
                break;
            case 7:
                usb(USEUSB);

                printf("\nApagar a FLASH, testar ou lê-la:\n1. Apagar\n2. Testar\n3. Ler\n");
                printf ("\nDigite o número referente a sua escolha: ");
                int optionFLASH;
                scanf ("%d", &optionFLASH);
                printf("\n");

                flashTESTE(optionFLASH);
                if (continuar_encerrar() == 0){
                    return 0;
                }
                break;
            case 8:
                eletrorecTESTE();
                if (continuar_encerrar() == 0){
                    return 0;
                }
                break;
            case 0:
                return 0;
            default:
                printf( "Não tem essa opção") ;
                break;
        }
    }

    return 0;
}
