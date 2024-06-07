TaskHandle_t Task0;

void core0_Task0( void * parameter )
{
    for (;;) {
        runSteppers();
        
        EVERY_N_SECONDS(1){
            Serial.print("\t\t\tSteppers running on Core ");
            Serial.println( xPortGetCoreID());
        }
        delay(10);
    }
}

void dualCoreInit(){
    xTaskCreatePinnedToCore(
        core0_Task0,    /* Task function. */
        "core0Task0",   /* name of task. */
        2048,           /* Stack size of task */
        NULL,           /* parameter of the task */
        10,             /* priority of the task */
        &Task0,         /* Task handle to keep track of created task */
        0               /* pin task to core 0 */
    );
    delay(500);  // needed to start-up task0
}