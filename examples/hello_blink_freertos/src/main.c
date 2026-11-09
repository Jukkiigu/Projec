#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#ifndef LED_DELAY_MS
#define LED_DELAY_MS 1000
#endif


static int pico_led_init(void) {
    return cyw43_arch_init(); // for Pico W
}


static void pico_set_led(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}


static void led_task(void *arg) {
    (void)arg;
    
    pico_led_init();

    for (;;) {
        pico_set_led(true);
        printf("Tack-FreeRTOS\n");
        vTaskDelay(pdMS_TO_TICKS(LED_DELAY_MS));
        pico_set_led(false);
        vTaskDelay(pdMS_TO_TICKS(LED_DELAY_MS));
    }
}

int main(void) {
    stdio_init_all();

    TaskHandle_t myTaskHandle = NULL;

    BaseType_t result =xTaskCreate(led_task,      
                "led",        
                512,          
                NULL,          
                2,              
                &myTaskHandle); 

    if(result != pdPASS) {
        printf("Task create failed\n");
        return 0;
    }

    vTaskStartScheduler();

}

/*
enum{
    idle=0,
    viiva,
    piste,
    x,
    valilyonti,
    UART,
}status;
*/ 
