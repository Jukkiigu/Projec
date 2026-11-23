#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include "tkjhat/sdk.h"

#define DEFAULT_STACK_SIZE 2048 

enum state { IDLE=1, Luetila, Piste, Valilyonti, Viiva, X, Laheta };
enum state programState = IDLE;
char asento = X;


static void asento_task(void *pvparameters){ (void)pvparameters;

   float ax, ay, az, gx, gy, gz, t;


    init_ICM42670();
    ICM42670_start_with_default_values();

   // jos nappia painetaan, niin luetaan sensorin arvo ja muutetaan ohjelman tila

    while (1)
    {
        if (programState == IDLE){
            onko nappia painettu? jos nappia painettu:
            programState = Luetila;
        }

        if(programState == Luetila){
            // luetaan sensorin arvo-->>

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
        
        vTaskDelay(pdMS_TO_TICKS(300));
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }


/*static void merkki_task(void *arg){

lisätään merkki puskuriin

    (void)arg;

    int pituus;

    for (;;)
    {
        xSemaphoreTake(stateMutex, portMAX_DELAY);

        // Tarkistetaan ohjelman tila
        if (programState == Piste ||
            programState == Viiva ||
            programState == Valilyonti)
        {

                if (programState == Piste)
                {
                    puskuri[] = '.';
                }
                else if (programState == Viiva)
                {
                    puskuri[] = '-';
                }
                else if (programState == Valilyonti)
                {
                    puskuri[] = ' ';
                }

                // Siirrytään lähettämistilaan
                programState = Laheta;
            }
        }

        xSemaphoreGive(stateMutex);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
*/

/*static void UART_task(void *arg)
{
    (void)arg;

    char lahetysPuskuri[BUFFER_SIZE];

    for (;;)
    {
        xSemaphoreTake(stateMutex, portMAX_DELAY);

        if (programState == Laheta)
        {
            // Kopioidaan puskuri paikalliseen muuttujaan
            strcpy(lahetysPuskuri, puskuri);

            // Tyhjennetään alkuperäinen puskuri
            puskuri[0] = '\0';

            xSemaphoreGive(stateMutex);

            // Lähetetään työasemalle USB:n kautta
            printf("%s", lahetysPuskuri);
            fflush(stdout);

            xSemaphoreTake(stateMutex, portMAX_DELAY);

            // Palataan alkutilaan
            programState = IDLE;

            xSemaphoreGive(stateMutex);
        }
        else
        {
            xSemaphoreGive(stateMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
*/
int main() {
    stdio_init_all();
    init_hat_sdk();
    init_button1();
    init_button2();
    sleep_ms(300);

    TaskHandle_t TaskHandle = NULL;
    
        xTaskCreate(asento_task, "Asento", 1024, NULL, 2, &TaskHandle);
        //xTaskCreate(merkki_task, "Merkki", 512, NULL, 2, &Taskhandle);
        //xTaskCreate(UART_task, "UART", 512, NULL, 2, &Taskhandle);

         vTaskStartScheduler();
    return 0;
    }


