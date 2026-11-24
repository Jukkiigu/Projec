#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include "tkjhat/sdk.h"

#define SW1_PIN 2
#define SW2_PIN 22
#define BUTTON1 SW1_PIN
#define BUTTON2 SW2_PIN
#define DEFAULT_STACK_SIZE 2048 
#define BUFFER_SIZE 20
#define RED_LED_PIN 14
#define LED1 RED_LED_PIN

enum state { IDLE=1, Luetila, Piste, Valilyonti, Viiva, X, Laheta };
enum state programState = IDLE;

static char puskuri[BUFFER_SIZE];
static int paikka = 0;

static volatile bool nappi1 = false;
static volatile bool nappi2 = false;

static void nappifun(uint gpio, uint32_t eventMask) {
    if (gpio  == BUTTON1)
        nappi1 = true;
    else if (gpio == BUTTON2)
        nappi2 = true;
}


static void asento_task(void *pvparameters){ 
   (void)pvparameters;

   float ax, ay, az, gx, gy, gz, t;

    init_ICM42670();
    ICM42670_start_with_default_values();

    for(;;)
    {
        if (programState == IDLE){
            if(nappi1){
               nappi1=false;
               programState = Luetila;
            }
        }

        if(programState == Luetila){

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
            else{
                programState = X;
            }
            printf("X=%f, Y=%f, Z=%f\n", ax, ay, az);
            printf("asento on %d\n", programState);
        }
    }
        vTaskDelay(pdMS_TO_TICKS(20));
 }

static void merkki_task(void *arg){
   (void)arg;
    for (;;){
        if (programState == Piste || programState == Viiva || programState == Valilyonti)
        {
               
         if(paikka+2 < BUFFER_SIZE){
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
            blink_led(2);
            programState = IDLE;
            }
         else{
            programState = Laheta;
        }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
static void UART_task(void *arg){

    char lahetysPuskuri[BUFFER_SIZE+4];
    lahetysPuskuri[0] = '\0';

    (void)arg;

    for (;;){
        if (programState == Laheta || nappi2){
           nappi2 = false;
           
            snprintf(lahetysPuskuri, sizeof(lahetysPuskuri), "%s  \n", puskuri);
            puskuri[0] = '\0';
            paikka = 0;
            printf("%s", lahetysPuskuri);
            fflush(stdout);
            programState = IDLE;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
int main() {
    stdio_init_all();
    init_hat_sdk();
    init_button1();
    init_button2();
    sleep_ms(300);
    init_led();
    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_RISE, true, nappifun);
    gpio_set_irq_enabled(BUTTON2, GPIO_IRQ_EDGE_RISE, true);

    TaskHandle_t asentoHandle = NULL;
    TaskHandle_t merkkiHandle = NULL;
    TaskHandle_t uartHandle = NULL;

    xTaskCreate(asento_task, "Asento", DEFAULT_STACK_SIZE, NULL, 2, &asentoHandle);
    xTaskCreate(merkki_task, "Merkki", DEFAULT_STACK_SIZE, NULL, 2, &merkkiHandle);
    xTaskCreate(UART_task, "UART", DEFAULT_STACK_SIZE, NULL, 2, &uartHandle);

         vTaskStartScheduler();
    return 0;
    }
