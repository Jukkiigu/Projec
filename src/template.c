
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

static void asento_task(void *arg){ (void)arg;

    float ax, ay, az;

    bool nappi1_edellinen = false;
    bool nappi2_edellinen = false;

    bool nappi1_nyt;
    bool nappi2_nyt;

    char uusiAsento;

    for (;;)
    {
        // Luetaan anturin kiihtyvyys
        if (imu_read_accel(&ax, &ay, &az))
        {
            uusiAsento = tunnista_asento(ax, ay, az);

            xSemaphoreTake(stateMutex, portMAX_DELAY);

            asento = uusiAsento;

            xSemaphoreGive(stateMutex);
        }

        // Luetaan painikkeiden tilat
        nappi1_nyt = !gpio_get(14);
        nappi2_nyt = !gpio_get(15);

        xSemaphoreTake(stateMutex, portMAX_DELAY);

        // Käsitellään painikkeet vain IDLE-tilassa
        if (programState == IDLE)
        {
            // Painike 1: hyväksytään asento
            if (nappi1_nyt && !nappi1_edellinen)
            {
                if (asento == '.')
                {
                    programState = Piste;
                }
                else if (asento == '-')
                {
                    programState = Viiva;
                }
            }

            // Painike 2: välilyönti
            if (nappi2_nyt && !nappi2_edellinen)
            {
                programState = Valilyonti;
            }
        }

        // Jos asentoa ei tunnisteta
        if (asento == 'X' && programState == IDLE)
        {
            programState = X;
            programState = IDLE;
        }

        xSemaphoreGive(stateMutex);

        nappi1_edellinen = nappi1_nyt;
        nappi2_edellinen = nappi2_nyt;

        // Odotetaan 20 ms
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


static void merkki_task(void *arg){
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
            pituus = strlen(puskuri);

            // Tarkistetaan puskurin tila
            if (pituus < BUFFER_SIZE - 1)
            {
                if (programState == Piste)
                {
                    puskuri[pituus] = '.';
                }
                else if (programState == Viiva)
                {
                    puskuri[pituus] = '-';
                }
                else if (programState == Valilyonti)
                {
                    puskuri[pituus] = ' ';
                }

                // Lisätään merkkijonon lopetusmerkki
                puskuri[pituus + 1] = '\0';

                // Siirrytään lähettämistilaan
                programState = Laheta;
            }
            else
            {
                // Puskuri täynnä
                programState = IDLE;
            }
        }

        xSemaphoreGive(stateMutex);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


static void UART_task(void *arg)
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

int main() {
    stdio_init_all();
    init_hat_sdk();
    sleep_ms(300);

    TaskHandle_t TaskHandle = NULL;
    BaseType_t result = xTaskCreate(example_task,       // (en) Task function
                "example",              // (en) Name of the task 
                DEFAULT_STACK_SIZE, // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                NULL,               // (en) Arguments of the task 
                2,                  // (en) Priority of this task
                &TaskHandle);    // (en) A handle to control the execution of this task
    
                xTaskCreate(asento_task, "Asento", 512, NULL, 2, &Taskhandle);
                xTaskCreate(merkki_task, "Merkki", 512, NULL, 2, &Taskhandle);
                xTaskCreate(UART_task, "UART", 512, NULL, 2, &Taskhandle);

    if(result != pdPASS) {
        printf("Example Task creation failed\n");
        return 0;
    }


    vTaskStartScheduler();


    return 0;
}

