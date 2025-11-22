/**
* \file <CAnalyzer.c>
* \author <AGILAN V S>
* \date <22-10-2025>
*
* \brief <Symbolic File name>   // ???
*
* \copyright Copyright ©2016
* Department of electrical engineering and information technology, Hochschule Darmstadt - University of applied sciences (h_da). All Rights Reserved.
* Permission to use, copy, modify, and distribute this software and its documentation for educational, and research purposes in the context of non-commercial
* (unless permitted by h_da) and official h_da projects, is hereby granted for enrolled students of h_da, provided that the above copyright notice,
* this paragraph and the following paragraph appear in all copies, modifications, and distributions.
* Contact Prof.Dr.-Ing. Peter Fromm, peter.fromm@h-da.de, Birkenweg 8 64295 Darmstadt - GERMANY for commercial requests.
*
* \warning This software is a PROTOTYPE version and is not designed or intended for use in production, especially not for safety-critical applications!
* The user represents and warrants that it will NOT use or redistribute the Software for such purposes.
* This prototype is for research purposes only. This software is provided "AS IS," without a warranty of any kind.
*/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "project.h"
#include "TimingAnalyzer.h"
//#include "core_cm3.h"       //  (N1)
#include "RED_LED.h"
#include "YELLOW_LED.h"
#include "GREEN_LED.h"
#include "UART_LOG.h"

/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/
#define MAX_ANALYZERS 10

/*****************************************************************************/
/* Global variable definitions (declared in header file with 'extern')       */
/*****************************************************************************/

/*****************************************************************************/
/* Local type definitions ('typedef')                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Local variable definitions ('static')                                     */
/*****************************************************************************/
static TimingAnalyzer_t* g_analyzers[MAX_ANALYZERS];    // (N5)     // ???
static uint8_t g_analyzer_count = 0;

/*****************************************************************************/
/* Local function prototypes ('static')                                      */
/*****************************************************************************/

/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/

/**
 * Func to initialize of the necessary peripherals like Set up SysTick timer (1 ms), enable DWT counter, and configure GPIO pins.
 * \param None
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_initialize(void){
    
    // Initialize SysTick timer (1 ms)


    // Enable DWT Cycle Counter                                                     (N2)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Activate the trace unit
    DWT->CYCCNT = 0;                                // Resets the cycle counter     (N3)
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;           // Activate the counting        (N4)    // ???
    
    // Trace must be enabled
    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0)   return RC_ERROR_NOT_INITIALIZERD;

    // Cycle counter must be enabled
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0)  return RC_ERROR_NOT_INITIALIZERD;

    // Set the pins low initially
    RED_LED_Write(0);
    YELLOW_LED_Write(0);
    GREEN_LED_Write(0);
    
    return RC_SUCCESS;
}

/**
 * Func to initializes an analyzer struct with configuration and assign function pointers for pin control.
 * \param TimingAnalyzer_t *me	: [IN/OUT] structure of Analyzer related parameters
 * \param AnalyzerMode_t mode   : [IN] type of the configuration mode we need to run the analyzer
 * \param AnalyzerPin_t pin     : [IN] GPIO pin incase of Output pin config use
 * \param const char name       : [IN] name of the Analyzer
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_create(TimingAnalyzer_t *me, AnalyzerMode_t mode, AnalyzerPin_t pin, const char *name){
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL) return RC_ERROR_NULL ;      // ???
    
    // Add to global analyzer list
    if (g_analyzer_count < MAX_ANALYZERS)
    {
        g_analyzers[g_analyzer_count++] = me;
    }
    
    // Clear the entire struct first ---
    memset(me, 0, sizeof(TimingAnalyzer_t));
    
    // Storing the info
    me->mode  = mode;
    me->pin   = pin;
    me->state = STATE_CONFIGURED;
    me->name  = name;   // Array + Strcpy or Pointer Assignment     // ???
    
    // Initialize the timing fields
    // Configuring timing source based on mode
    switch (mode)
    {
        case MODE_DWT:
        case MODE_DWT_PIN:
            // DWT counter is already running globally
            me->start_cycles = 0;
            me->stop_cycles  = 0;
            break;

        case MODE_SYSTICK:
        case MODE_SYSTICK_PIN:
            // SysTick already initialized globally in TimingAnalyzer_init()
            me->start_tick = 0;
            me->stop_tick  = 0;
            break;
        
        case MODE_PIN:
            // No internal timer used, only external pin toggling
            break;

        default:
            // do nothing
            break;
    }
    
    me->elapsed_time_ms = 0.0;
    me->elapsed_cycles  = 0;
    
    me->accumulated_time_ms = 0.0;
    me->accumulated_cycles = 0;
    
    // Pin
    
    // SysTick setup is done globally in TimingAnalyzer_init(), not here
    
    return res;
}

/**
 * This func will allow us to start an Analyser
 * \param TimingAnalyzer_t *me	: [IN/OUT] structure of Analyzer related parameters
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_start(TimingAnalyzer_t *me){
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL) return RC_ERROR_NULL ;
    
    // Check state
    if (me->state == STATE_RUNNING)
    {
        // Already running - do nothing
        return RC_ERROR_INVALID_STATE;
    }
    
    // Re-enable DWT for safety, incase reset by other causes
    if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
    {
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
    
    // Cycle counter must be enabled
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0)  return RC_ERROR_NOT_INITIALIZERD;
    
    //
    
    // Fetching the counter FIRST for precision
    if (me->mode == MODE_SYSTICK || me->mode == MODE_SYSTICK_PIN)
    {
        // SysTick counts in ms

    }
    else if (me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        // DWT cycle counter
        // DWT->CYCCNT = current CPU cycle count
        me->start_cycles = DWT->CYCCNT;  // Start a cycle interval
    }
    else if (me->mode == MODE_PIN)
    {

    }

    // Update state
    me->state = STATE_RUNNING;
    
    return res;
}

/**
 * This func will allow us to start an Analyser
 * \param TimingAnalyzer_t *me	: [IN/OUT] structure of Analyzer related parameters
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_stop(TimingAnalyzer_t *me){
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL) return RC_ERROR_NULL ;
    
    // Check state
    if (me->state != STATE_RUNNING && me->state != STATE_PAUSED)
    {
        // Neither running nor paused - do nothing
        return RC_ERROR_INVALID_STATE;
    }
    
    //
    
    // Fetching the counter FIRST for precision
    if (me->mode == MODE_SYSTICK || me->mode == MODE_SYSTICK_PIN)
    {
        // SysTick counts in ms
        
        // Calculate the elapsed cycles
    }
    else if (me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        // DWT cycle counter
        // DWT->CYCCNT = current CPU cycle count
        me->stop_cycles = DWT->CYCCNT;  // Stop the cycle interval
        
        // Calculate the elapsed cycles
        uint32_t interval_cycles = me->stop_cycles - me->start_cycles;
    
        me->elapsed_cycles = interval_cycles + me->accumulated_cycles;
        
        // Duration for the iteration in ms
        me->elapsed_time_ms = 1.0; //((me->elapsed_cycles) / (BCLK__BUS_CLK__KHZ) * 1000.0);
        
        char strMessageA[100];
        sprintf(strMessageA, "elapsed_time_ms: %f\r\n", me->elapsed_time_ms);
        UART_LOG_PutString(strMessageA); 
    
        me->accumulated_cycles = me->elapsed_cycles;
    }
    else if (me->mode == MODE_PIN)
    {

    }
    
    // Update state
    me->state = STATE_STOPPED;
    
    return res;
}

/**
 * This func will allow us to start an Analyser
 * \param TimingAnalyzer_t *me	: [IN/OUT] structure of Analyzer related parameters
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_pause(TimingAnalyzer_t *me){
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL) return RC_ERROR_NULL ;
    
    // Check state
    if (me->state != STATE_RUNNING)
    {
        // Already running - do nothing
        return RC_ERROR_INVALID_STATE;
    }
    
    //
    
    // Fetching the counter FIRST for precision
    if (me->mode == MODE_SYSTICK || me->mode == MODE_SYSTICK_PIN)
    {
        // SysTick counts in ms
        
        // Calculate the elapsed cycles
    }
    else if (me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        // DWT cycle counter
        me->stop_cycles = DWT->CYCCNT;  // Stop current cycle interval
        
        // Calculate the elapsed cycles
        uint32_t interval_cycles = me->stop_cycles - me->start_cycles;
    
        me->accumulated_cycles += interval_cycles;
    }
    else if (me->mode == MODE_PIN)
    {

    }
    
    // Update state
    me->state = STATE_PAUSED;
    
    return res;
}

/**
 * This func will allow us to start an Analyser
 * \param TimingAnalyzer_t *me	: [IN/OUT] structure of Analyzer related parameters
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_resume(TimingAnalyzer_t *me){
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL) return RC_ERROR_NULL ;
    
    // Check state
    if (me->state != STATE_PAUSED)
    {
        // Already paused - do nothing
        return RC_ERROR_INVALID_STATE;
    }
    
    //
    
    // Fetching the counter FIRST for precision
    if (me->mode == MODE_SYSTICK || me->mode == MODE_SYSTICK_PIN)
    {
        // SysTick counts in ms
        
        // Calculate the elapsed cycles
    }
    else if (me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        // DWT cycle counter
        me->start_cycles = DWT->CYCCNT;  // Start new cycle interval
    }
    else if (me->mode == MODE_PIN)
    {

    }
    
    // Update state
    me->state = STATE_RUNNING;
    
    return res;
}

/**
 * Func to print the status of an Analyser using UART
 * \param TimingAnalyzer_t *me	: [IN] structure of Analyzer related parameters
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_printStatus(TimingAnalyzer_t *me){
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL) return RC_ERROR_NULL ;
    
    // Buffer to assemble string
    char buffer[100]; 

    // Format state as string
    const char* strState = "UNKNOWN";
    switch(me->state)
    {
        case STATE_CONFIGURED:
            strState = "CONFIGURED"; 
            break;
        case STATE_STOPPED: 
            strState = "STOPPED"; 
            break;
        case STATE_RUNNING: 
            strState = "RUNNING"; 
            break;
        case STATE_PAUSED:  
            strState = "PAUSED"; 
            break;
    }

    // Assemble full status string
    snprintf(buffer, sizeof(buffer),
            "Name: %s | State: %s | Elapsed time: %.6fms | Cycles: %lu\r\n",
            me->name,
            strState,
            me->elapsed_time_ms,
            me->elapsed_cycles);

    // Send string to UART (only once)
    UART_LOG_PutString(buffer);
    
    return res;
}

/**
 * Func to loop through all analyzers and print status of each
 * \param TimingAnalyzer_t *me	: [IN] structure of Analyzer related parameters
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_printAll(void){
    RC_t res = RC_SUCCESS;
    
    if(g_analyzers[0] == 0 && g_analyzer_count == 0)
    {
        // Array is empty
        UART_LOG_PutString("\r\nInfo: No analyzer instance available to print.\r\n");
    }
    
    for (uint8_t i = 0; i < g_analyzer_count; i++)
    {
        TimingAnalyzer_t *me = g_analyzers[i];
        if (me != NULL) // again ???
        {
            TimingAnalyzer_printStatus(me);
        }
    }
    
    return res;
}

/* NOTE
 * 
 * 1. core_cm3.h - Gives access to DWT, CoreDebug, and other Cortex-M3 registers.
 * PSOC 5LP supports CMSIS (Cortex Microcontroller Software Interface Standard) library.
 *
 * 2. The DWT (Data Watchpoint and Trace) is a peripheral unit present in ARM Cortex-M 
 * cores allowing for debugging, performance analysis, and watchpoint triggers.
 * 
 * 3. CYCCNT (Cycle Count Register) - Holds the number of CPU cycles since the last 
 * reset or enabling of the counter.
 * 
 * 4. CTRL (Control Register) - Enables various DWT functionalities, like the cycle 
 * counter.
 * 5. Global array - All elements are automatically set to 0 by the C runtime.
 */

/* [TimingAnalyzer.c] END OF FILE */