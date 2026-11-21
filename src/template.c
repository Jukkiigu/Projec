
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

static void asento_task(void *arg){
    tänne päädytään kun tila on IDLE
    (void)arg;

    for(;;){
        tight_loop_contents(); 
        
        kerää data asennosta
        asennon perusteella:
        globaalimuuttuja asento = joko "." 1  tai "-" 2 tai "valilyonti" 3  tai "X" 4
        if(asento = X) {
            enum state programState = IDLE;
        }
        else{
            enum state programState = asento;
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void merkki_task(void *arg){
    tänne päädytään kun state = Piste tai Valilyonti tai Viiva
    (void)arg;

    for(;;){
        tight_loop_contents();
        merkki on tallessa asento muuttujassa - tai . tai "  "
        lisätään merkki puskuriin
        lopuksi muutetaan tila jotta päästään --> UART_task
        enum state programState = Laheta;
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void UART_task(void *arg){
    tänne päädytään kun state = Laheta
    (void)arg;

    for(;;){
        tight_loop_contents();
        lähetetään puskurin merkki työasemalle
        tyhjennetään puskuri
        enum state programState = IDLE;
        vTaskDelay(pdMS_TO_TICKS(2000));
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

