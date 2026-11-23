
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
    // Setting up the sensor. 
    init_ICM42670();
    ICM42670_start_with_default_values();
  
    while (1)
    {
        if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0) {
            
            printf("Gyro: X=%f, Y=%f, Z=%f\n",gx, gy, gz);

            if(gx >= 2.0 ){
                printf("gx on enemmän kuin 2.0\n");
            }

        } else {
            printf("Virhe");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}


/*static void merkki_task(void *arg){
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
    sleep_ms(300);

    TaskHandle_t TaskHandle = NULL;
    
        xTaskCreate(asento_task, "Asento", 1024, NULL, 2, &TaskHandle);
        //xTaskCreate(merkki_task, "Merkki", 512, NULL, 2, &Taskhandle);
        //xTaskCreate(UART_task, "UART", 512, NULL, 2, &Taskhandle);

         vTaskStartScheduler();
    return 0;
    }


