#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h> //täällä kaikki mahdolliset kirjastot
#include <queue.h>
#include <task.h>
#include "tkjhat/sdk.h"

#define SW1_PIN 2
#define SW2_PIN 22
#define BUTTON1 SW1_PIN
#define BUTTON2 SW2_PIN //määritykset
#define DEFAULT_STACK_SIZE 2048 
#define BUFFER_SIZE 40 //viestin pituutta voidaan muuttaa tästä
#define RED_LED_PIN 14
#define LED1 RED_LED_PIN

enum state { IDLE=1, Luetila, Piste, Valilyonti, Viiva, Laheta };// ei tarvetta suunnitellulle X-tilalle, sillä kaikki asennot vastaavat jotain merkkiä,
enum state programState = IDLE;

static char puskuri[BUFFER_SIZE];
static int paikka = 0; //puskuriin lisättävän merkin paikka

static volatile bool nappi1 = false;
static volatile bool nappi2 = false;

static void nappifun(uint gpio, uint32_t eventMask) {// tarkistetaan onko nappia painettu
    if (gpio  == BUTTON1)
        nappi1 = true;
    else if (gpio == BUTTON2)
        nappi2 = true;
}


static void asento_task(void *pvparameters){ //tämä task lukee sensoria muuttaa ohjelman tilaa sen mukaan
   (void)pvparameters;

   float ax, ay, az, gx, gy, gz, t;

    init_ICM42670();
    ICM42670_start_with_default_values();

    for(;;)
    {
        if (programState == IDLE){
            if(nappi1){
               nappi1=false;
               programState = Luetila; // vaihdetaan tila -> Luetila jotta päästään lukemaan sensoria
            }
        }

        if(programState == Luetila){ //luetaan sensorin tila

        if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0) {

            if(ax >= 0.7 ){
                programState = Piste;
            }
            else if(ax <= -0.7){
                programState = Viiva;
            }
            else if(ax >= -0.7 && ax <= 0.7){
                programState = Valilyonti;
            }
            //printf("X=%f, Y=%f, Z=%f\n", ax, ay, az);
            //printf("asento on %d\n", programState);  // vasemmalle kallistus = 5 = viiva,  oikealle = 3 = piste,  keskiasento = 4 = valilyonti
        }
    }
        vTaskDelay(pdMS_TO_TICKS(20));
 }
}

static void merkki_task(void *arg){ //asentotask on muuttanut ohjelman tilaa, ja tämä task lisää oikean merkin puskuriin
   (void)arg;
    for (;;){
        if (programState == Piste || programState == Viiva || programState == Valilyonti)
        {
               
         if(paikka+2 < BUFFER_SIZE){ // tarkistetaan ettei puskurin koko ylity ja lisätään oikea merkki puskuriin
                if (programState == Piste)
                {
                    puskuri[paikka++] = '.';
                    puskuri[paikka++] = ' ';
                }
                else if (programState == Viiva)
                {
                    puskuri[paikka++] = '-';
                    puskuri[paikka++] = ' ';
                }
                else if (programState == Valilyonti)
                {
                    puskuri[paikka++] = ' ';
                    puskuri[paikka++] = ' ';
                }
            puskuri[paikka] = '\0';
            blink_led(2); //väläytettään ledillä kun merkki on lisätty puskuriin
            programState = IDLE; // palataan odottamaan napin painallusta
            }
         else{
            programState = Laheta; // siirrytään automaattisesti lähettämään jos puskuri on täynnä.
        }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void UART_task(void *arg){ //Valmis viesti lähetetään työasemalle

    char lahetysPuskuri[BUFFER_SIZE+4]; // lahetyspuskuri jossa on tilaa rivinvaihdolle ja lopetusmerkille
    lahetysPuskuri[0] = '\0'; // alustetaan puskuri tyhjäksi

    (void)arg;

    for (;;){
        if (programState == Laheta || nappi2){
           nappi2 = false;
           
            snprintf(lahetysPuskuri, sizeof(lahetysPuskuri), "%s  \n", puskuri); //kopioidaan puskuri lahetyspuskuriin ja lisätään rivinvaihto
            puskuri[0] = '\0'; // tyhjennetään puskuri
            paikka = 0; // nollataan paikka
            printf("%s", lahetysPuskuri); // tulostetaan puskurin sisältö
            fflush(stdout);
            programState = IDLE; // Ohjelma alkaa alusta, valmiina odottamaan seuraavaa merkkiä
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

int main() {
    stdio_init_all();
    init_hat_sdk();
    init_button1(); // oheislaitteiden alustukset 
    init_button2();
    init_led();
    sleep_ms(300);
    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_RISE, true, nappifun); //keskeytykset nappien painalluksille
    gpio_set_irq_enabled(BUTTON2, GPIO_IRQ_EDGE_RISE, true);
//handlet
    TaskHandle_t asentoHandle = NULL;
    TaskHandle_t merkkiHandle = NULL;
    TaskHandle_t uartHandle = NULL;
//Luodaan taskit
    xTaskCreate(asento_task, "Asento", DEFAULT_STACK_SIZE, NULL, 2, &asentoHandle);
    xTaskCreate(merkki_task, "Merkki", DEFAULT_STACK_SIZE, NULL, 2, &merkkiHandle);
    xTaskCreate(UART_task, "UART", DEFAULT_STACK_SIZE, NULL, 2, &uartHandle);

         vTaskStartScheduler();
    return 0;
    }
