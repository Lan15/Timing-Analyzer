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
#include "Pins.h"
#include "UART_LOG.h"

/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/ 
// MISRA Rule 12.1/12.2 : Avoid magic numbers. Use symbolic constants. (We replaced all numeric literals with symbolic constants.)
// MISRA Rule 8.12 (2004) : Use #define or const for magic numbers. (Fully compliant (uses macros))
// MISRA Rule 10.3 : Implicit conversions should be avoided. (All constants use explicit suffix UL (unsigned long).)    // ???
#define MAX_ANALYZERS           (10UL)
#define MAX_PINS                (3UL)
#define MAX_MODES               (5UL)
#define SYSTICK_RELOAD_VALUE    ((BCLK__BUS_CLK__HZ / 1000UL) - 1UL)
#define SYSTICK_RESET_VALUE     (0UL)
#define DWT_RESET_VALUE         (0UL)
#define MAX_32BIT_VALUE         (4294967295UL)   /* 0xFFFFFFFFUL */
#define COUNTER_INCREMENT       (1UL)

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
static uint8_t g_analyzerCount = 0U;                    //          // ???
volatile uint32_t system_ms = 0UL;                      // Global millisecond counter


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
    
    // MISRA Rule 9.1 : Variables should be initialized before use. (All registers and constants are initialized before usage.)
    // Initialize SysTick timer (1 ms)
    // MISRA Violation Rule 2.2 : No dead or unused code. (All lines have a clear function (no commented-out or dead code).)
    CySysTickInit();									// Activate Systick counter                 // (N7)
	CySysTickSetCallback(0, SysTick_Handler);           // Set ISR for Systick
	CySysTickSetReload(BCLK__BUS_CLK__HZ / 1000 - 1);   // Set 1ms cycle Time
	CySysTickEnable();									// Enable the Callback of the systick

    /*SysTick->CTRL  = 0;                                 // Disable Systick
    SysTick->LOAD  = (BCLK__BUS_CLK__KHZ-1); // SYSTICK_RELOAD_VALUE; // Counts from 23999 down to 0, 1 ms        // (N8)
    SysTick->VAL   = SYSTICK_RESET_VALUE;               // Clear current value
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;        // Selects the CPU/Preprocessor clock (24 MHz) as the counter source
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;          // Enables the interrupt every 1 ms
    CySysTickSetCallback(0, SysTick_Handler);           // Set ISR for Systick
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;           // Starts the SysTick counter running
    
    // MISRA Violation Rule 14.7 - Functions should have a single exit point.
    // Cycle counter must be enabled
    if ((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == 0)  return RC_ERROR_INVALID_STATE; */

    system_ms = 0;      // Reset the global time counter
    
    // Enable DWT Cycle Counter                                                                 // (N2)
    CoreDebug->DEMCR    |= CoreDebug_DEMCR_TRCENA_Msk;     // Activate the trace unit
    DWT->CYCCNT          = DWT_RESET_VALUE;                // Resets the cycle counter          // (N3)
    DWT->CTRL           |= DWT_CTRL_CYCCNTENA_Msk;         // Starts the DWT counter running    // (N4)    // ???
    
    // Check if hardware were initialized
    // Trace must be enabled
    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0) 
    { 
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; 
    }

    // Cycle counter must be enabled
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0) 
    { 
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; 
    } 

    // Set the pins low initially
    void Pins_Init();
    
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
RC_t TimingAnalyzer_create(TimingAnalyzer_t *me, AnalyzerMode_t const mode, PinFunc_t const pin_ctrl, const char* const name){ // ???
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL_PTR || name == NULL_PTR) return RC_ERROR_NULL ;
    
    if ((mode == MODE_DWT_PIN || mode == MODE_SYSTICK_PIN ||  mode == MODE_PIN)
    && (pin_ctrl == NULL_PTR)) return RC_ERROR_NULL;
    
    if (strlen(name) == 0UL || mode > MAX_MODES) 
        return RC_ERROR_BAD_PARAM;
    
    if ((mode == MODE_DWT || mode == MODE_SYSTICK) 
    && (pin_ctrl == NULL_PTR)) return RC_ERROR_BAD_PARAM; 
    
    if(g_analyzerCount >= MAX_ANALYZERS)
    {
        res = RC_ERROR_BUFFER_FULL;
    }
    // Add to global analyzer list
    else
    {
        g_analyzers[g_analyzerCount++] = me;
    
        // Clear the entire struct first
        memset(me, 0, sizeof(TimingAnalyzer_t));
    
        // Storing the info
        me->mode    = mode;
        me->state   = STATE_CONFIGURED;
        me->name    = name;
        me->active  = TRUE;     // (N6)
    
        // Initialize the timing fields
        // DWT counter is already running globally
        me->start_cycles = 0UL;
        me->stop_cycles  = 0UL;
        // SysTick already initialized globally in TimingAnalyzer_init()
        me->start_tick = 0UL;
        me->stop_tick  = 0UL;
        
        me->elapsed_time_ms = 0.0F;
        me->elapsed_cycles  = 0UL;
    
        me->accumulated_time_ms = 0.0F;
        me->accumulated_cycles = 0UL;
        
        // No internal timer used, only external pin toggling
        // Assign pin control function pointers
        me->pin_control_func = pin_ctrl;

        /* // Configuring timing source based on mode
        switch (mode)
        {
            case MODE_DWT:
            case MODE_DWT_PIN:
                // DWT counter is already running globally
                me->start_cycles = 0UL;
                me->stop_cycles  = 0UL;
                break;

            case MODE_SYSTICK:
            case MODE_SYSTICK_PIN:
                // SysTick already initialized globally in TimingAnalyzer_init()
                me->start_tick = 0UL;
                me->stop_tick  = 0UL;
                break;
        
            case MODE_PIN:
                break;

            default:
                // do nothing
                break;
        } */
    }
    
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
    if (me == NULL_PTR) return RC_ERROR_NULL ;
    
    // Must be a valid, active analyzer
    if (me->active == FALSE) return RC_ERROR_INVALID_STATE;
    
    // Check state - If already running, invalid call
    if (me->state == STATE_RUNNING) return RC_ERROR_BUSY;
    
    // Cycle counter must be enabled
    //if ((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == 0)  return RC_ERROR_INVALID_STATE;
    
    // May not happen
    /*// Re-enable DWT for safety, incase reset by other causes
    if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
    {
        DWT->CYCCNT  = DWT_RESET_VALUE;
        DWT->CTRL   |= DWT_CTRL_CYCCNTENA_Msk;
    }
    
    // Check if DWT hardware was initialized
    // Cycle counter must be enabled
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0)  return RC_ERROR_INVALID_STATE; */
    
    // Handle pin output first for precision
    if ((me->mode == MODE_SYSTICK_PIN) || (me->mode == MODE_DWT_PIN) || (me->mode == MODE_PIN))
    {
        if (me->pin_control_func != NULL_PTR)
        {
           me->pin_control_func(1U);   /* Turn ON pin */
        }
    }
    
    // Fetching the counter FIRST for precision
    if (me->mode == MODE_SYSTICK || me->mode == MODE_SYSTICK_PIN)
    {
        // SysTick counts in ms
        me->start_tick = system_ms;  // Start a cycle interval
    }
    else if (me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        // DWT cycle counter
        // DWT->CYCCNT = current CPU cycle count
        me->start_cycles = DWT->CYCCNT;  // Start a cycle interval
    }
    else 
    {
        // MODE_PIN only - do nothing
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
    if (me == NULL_PTR) return RC_ERROR_NULL ;
    
    // Must be a valid, active analyzer
    if (me->active == FALSE) return RC_ERROR_INVALID_STATE;
    
    // Check state - Can only stop if running or paused
    if (me->state != STATE_RUNNING && me->state != STATE_PAUSED)
    {
        return RC_ERROR_INVALID_STATE;
    }
    
    // Fetching the counter FIRST for precision
    if (me->mode == MODE_SYSTICK || me->mode == MODE_SYSTICK_PIN)
    {
        // SysTick counts in ms
        me->stop_tick = system_ms;  // Stop the cycle interval
        
        // Calculate the elapsed time in ms
        float32_t interval_ticks;
        if (me->stop_tick >= me->start_tick)
        {
            interval_ticks = (float32_t)me->stop_tick - (float32_t)me->start_tick;
        }
        else
        {
            /* Handle 32-bit wrap-around */
            interval_ticks = (MAX_32BIT_VALUE - (float32_t)me->start_tick) + (float32_t)me->stop_tick + COUNTER_INCREMENT;
        }
    
        me->elapsed_time_ms = interval_ticks + (float32_t)me->accumulated_time_ms;
    }
    else if (me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        // DWT cycle counter
        // DWT->CYCCNT = current CPU cycle count
        me->stop_cycles = DWT->CYCCNT;  // Stop the cycle interval
        
        // Calculate the elapsed cycles
        float32_t interval_cycles;
        
        if (me->stop_cycles >= me->start_cycles)
        {
            interval_cycles = (float32_t)me->stop_cycles - (float32_t)me->start_cycles;
        }
        else
        {
            /* Handle 32-bit wrap-around */
            interval_cycles = (float32_t)(MAX_32BIT_VALUE - (float32_t)me->start_cycles) + (float32_t)me->stop_cycles + COUNTER_INCREMENT;
        }
    
        me->elapsed_cycles = interval_cycles + (float32_t)me->accumulated_cycles;
        
        // Duration for the iteration in ms
        me->elapsed_time_ms = ((float32_t)(me->elapsed_cycles) / (BCLK__BUS_CLK__KHZ));
        
        /* char strMessageA[100];
        sprintf(strMessageA, "elapsed_time_ms: %f\r\n", me->elapsed_time_ms);
        UART_LOG_PutString(strMessageA); */
    
        me->accumulated_cycles = me->elapsed_cycles;
    }
    else 
    {
        // MODE_PIN only - do nothing
    }
    
    // Clear pin after stopping
    if ((me->mode == MODE_SYSTICK_PIN) || (me->mode == MODE_DWT_PIN) || (me->mode == MODE_PIN))
    {
        if (me->pin_control_func != NULL_PTR)
        {
            me->pin_control_func(0U); /* Turn OFF pin */
        }
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
    if (me == NULL_PTR) return RC_ERROR_NULL ;
    
    // Must be a valid, active analyzer
    if (me->active == FALSE) return RC_ERROR_INVALID_STATE;
    
    // Check state - Can only pause when running
    if (me->state != STATE_RUNNING)
    {
        return RC_ERROR_INVALID_STATE;
    }
    
    // Fetching the counter FIRST for precision
    if (me->mode == MODE_SYSTICK || me->mode == MODE_SYSTICK_PIN)
    {
        // SysTick counts in ms
        me->stop_tick = system_ms;  // Stop current cycle interval
        
        // Calculate the elapsed time in ms
        float32_t interval_ticks;
        if (me->stop_tick >= me->start_tick)
        {
            interval_ticks = (float32_t)me->stop_tick - (float32_t)me->start_tick;
        }
        else
        {
            /* Handle 32-bit wrap-around */
            interval_ticks = (MAX_32BIT_VALUE - (float32_t)me->start_tick) + (float32_t)me->stop_tick + COUNTER_INCREMENT;
        }
    
        me->accumulated_time_ms += interval_ticks;
    }
    else if (me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        // DWT cycle counter
        me->stop_cycles = DWT->CYCCNT;  // Stop current cycle interval
        
        // Calculate the elapsed cycles
        float32_t interval_cycles;
        
        if (me->stop_cycles >= me->start_cycles)
        {
            interval_cycles = (float32_t)me->stop_cycles - (float32_t)me->start_cycles;
        }
        else
        {
            /* Handle 32-bit wrap-around */
            interval_cycles = (MAX_32BIT_VALUE - (float32_t)me->start_cycles) + (float32_t)me->stop_cycles + COUNTER_INCREMENT;
        }
    
        me->accumulated_cycles += interval_cycles;
    }
    else 
    {
        // MODE_PIN only - do nothing
    }
    
    // Pin LOW to show pause
    if ((me->mode == MODE_SYSTICK_PIN) || (me->mode == MODE_DWT_PIN) || (me->mode == MODE_PIN))
    {
        if (me->pin_control_func != NULL_PTR)
        {
            me->pin_control_func(0U); /* Turn OFF pin */
        }
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
    if (me == NULL_PTR) return RC_ERROR_NULL ;
    
    // Must be a valid, active analyzer
    if (me->active == FALSE) return RC_ERROR_INVALID_STATE;
    
    // Check state - Can only resume if paused
    if (me->state != STATE_PAUSED)
    {
        return RC_ERROR_INVALID_STATE;
    }
    
    // Pin HIGH to indicate resumed timing
    if ((me->mode == MODE_SYSTICK_PIN) || (me->mode == MODE_DWT_PIN) || (me->mode == MODE_PIN))
    {
        if (me->pin_control_func != NULL_PTR)
        {
           me->pin_control_func(1U);   /* Turn ON pin */
        }
    }
    
    // Fetching the counter FIRST for precision
    if (me->mode == MODE_SYSTICK || me->mode == MODE_SYSTICK_PIN)
    {
       // SysTick counts in ms
        me->start_tick = system_ms;  // Start new cycle interval
    }
    else if (me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        // DWT cycle counter
        me->start_cycles = DWT->CYCCNT;  // Start new cycle interval
    }
    else 
    {
        // MODE_PIN only - do nothing
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
RC_t TimingAnalyzer_printStatus(TimingAnalyzer_t const *me){
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL_PTR) return RC_ERROR_NULL ;
    
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
    if(me->mode == MODE_DWT || me->mode == MODE_DWT_PIN)
    {
        snprintf(buffer, sizeof(buffer),
            "Name: %s | State: %s | Elapsed time: %.6fms | Cycles: %ul\r\n",
             me->name, strState, me->elapsed_time_ms, me->elapsed_cycles);
    }
    else if(me->mode == MODE_PIN)
    {
        snprintf(buffer, sizeof(buffer),
            "Name: %s | State: %s\r\n",
            me->name, strState);
    }
    else
    {
        snprintf(buffer, sizeof(buffer),
            "Name: %s | State: %s | Elapsed time: %.6fms\r\n",
            me->name, strState, me->elapsed_time_ms);
    }

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
    
    boolean_t any_active = FALSE;
    
    for (uint8_t i = 0; i < g_analyzerCount; i++)
    {
        TimingAnalyzer_t *me = g_analyzers[i];
        if (me != NULL_PTR && me->active) // again ???
        {
            any_active = TRUE;
            TimingAnalyzer_printStatus(me);
        }
    }
    
    // MISRA Rule 13.7 : Boolean expressions should be explicit. (Use to avoid ambiguity.)
    if (any_active == FALSE)
    {
        // Array is empty
        UART_LOG_PutString("\r\nInfo: No analyzer instance available to print.\r\n");
        
        res = RC_ERROR_BUFFER_EMTPY;
    }
    
    return res;
}

/**
 * Func Systick Handler - used to increment the milliseconds counter each millisecond
 * \param None
 * \return None
*/
void SysTick_Handler(void)
{
    system_ms++;
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
 * 
 * 6. The active flag ensures the analyzer exists. The state ensures it’s in the correct
 * mode to perform the requested operation.
 *
 * 7. PSoC’s high-level SysTick API - the PSoC Creator system library wrapper around the 
 * Cortex-M3 SysTick. Slightly more overhead, Less control and precision.
 *
 * 8. Direct CMSIS register access. Make code non portable.
 *
 * 9. DWT cycle counter is the Cortex-M 32-bit cycle counter. SysTick is the Cortex-M 
 * 24-bit counter
 *
 * > MISRA-C:2004 compliancy - ~85–90%
 */

/* [TimingAnalyzer.c] END OF FILE */