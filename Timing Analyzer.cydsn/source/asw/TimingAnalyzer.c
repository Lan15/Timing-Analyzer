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
#include "project.h"        // includes core_cm3.h   //  (N1)
#include "TimingAnalyzer.h"
#include "Pins.h"
#include "UART_LOG.h"

/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/ 
// MISRA Rule 12.1/12.2 : Avoid magic numbers. Use symbolic constants. (We replaced all numeric literals with symbolic constants.)
// MISRA Rule 8.12 (2004) : Use #define or const for magic numbers. (Fully compliant (uses macros))
// MISRA Rule 10.3 : Implicit conversions should be avoided. (All constants use explicit suffix UL (unsigned long).)    // ???
#define TA_MAX_ANALYZERS           (10UL)
#define TA_MAX_PINS                (3UL)
#define TA_MAX_MODES               (5UL)
#define TA_SYSTICK_RELOAD_VALUE    ((BCLK__BUS_CLK__HZ / 1000UL) - 1UL)
#define TA_SYSTICK_RESET_VALUE     (0UL)
#define TA_DWT_RESET_VALUE         (0UL)
#define TA_MAX_32BIT_VALUE         (4294967295UL)   /* 0xFFFFFFFFUL */
#define TA_COUNTER_INCREMENT       (1UL)
#define TA_SCALE_FACTOR            (1000000UL)

/*****************************************************************************/
/* Global variable definitions (declared in header file with 'extern')       */
/*****************************************************************************/

/*****************************************************************************/
/* Local type definitions ('typedef')                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Local variable definitions ('static')                                     */
/*****************************************************************************/
static TA_t* ta_g_analyzers[TA_MAX_ANALYZERS];    //  (N5)
static uint8_t ta_g_analyzer_count = 0U;
volatile static uint32_t ta_g_system_ms = 0UL; // Global millisecond counter

/*****************************************************************************/
/* Local function prototypes ('static')                                      */
/*****************************************************************************/

/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/

/**
 * Func to initialize the necessary peripherals like Set up SysTick timer (1 ms), enable DWT counter, and configure GPIO pins.
 * \param None
 * \return RC_SUCCESS when success and RC_ERROR_INVALID_STATE when Hardware not properly initilized
*/
RC_t TA_init(void)
{
    RC_t res = RC_SUCCESS;
    
    // Initialize SysTick timer (1 ms)
    CySysTickInit();									// Activate Systick counter     // (N7)
	CySysTickSetCallback(0, SysTick_Handler);           // Set ISR for Systick
	CySysTickSetReload(BCLK__BUS_CLK__HZ / 1000 - 1);   // Set 1ms cycle Time
	CySysTickEnable();									// Enable the Callback of the systick

    ta_g_system_ms = 0;      // Reset the global time counter
    
    // Enable DWT Cycle Counter                                                                 // (N2)
    CoreDebug->DEMCR    |= CoreDebug_DEMCR_TRCENA_Msk;     // Activate the trace unit
    DWT->CYCCNT          = TA_DWT_RESET_VALUE;             // Resets the cycle counter          // (N3)
    DWT->CTRL           |= DWT_CTRL_CYCCNTENA_Msk;         // Starts the DWT counter running    // (N4)

    // Set the pins low initially
    Pins_Init();
    
    return res;
}

/**
 * Func to initializes an analyzer struct with configuration and assign function pointers for pin control.
 * \param TA_t *const me                : [IN/OUT] struct of Analyzer related parameters
 * \param TA_Mode_t const mode          : [IN] type of the configuration mode we need to run the analyzer
 * \param TA_PinFunc_t const pin_ctrl   : [IN] func pointer to control a GPIO pin
 * \param const char const *name        : [IN] name of the Analyzer
 * \return RC_SUCCESS when success, RC_ERROR_NULL when a pointer param is null
 *         RC_ERROR_BAD_PARAM when unexpected params are passed and 
 *         RC_ERROR_BUFFER_FULL when max analyzer count is reached
*/
RC_t TA_create(TA_t *const me, TA_Mode_t const mode, TA_PinFunc_t const pin_ctrl, const char* const name)
{
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL_PTR || name == NULL_PTR) // warning Nullprt == me? All comparisions - datatypes and comparion?
    {
        return RC_ERROR_NULL ;              // Can we compare a conatant value with the desired value?
    }
    
    if ((TA_MODE_DWT_PIN == mode || TA_MODE_SYSTICK_PIN == mode || TA_MODE_PIN == mode) && (pin_ctrl == NULL_PTR))
    {
        return RC_ERROR_NULL;
    }
    
    if (strlen(name) == 0UL || mode > TA_MAX_MODES)
    {
        return RC_ERROR_BAD_PARAM;
    }
    
    if ((TA_MODE_DWT == mode || TA_MODE_SYSTICK == mode) && (pin_ctrl == NULL_PTR)) 
    {
        return RC_ERROR_BAD_PARAM;
    }
    
    if(ta_g_analyzer_count >= TA_MAX_ANALYZERS)
    {
        res = RC_ERROR_BUFFER_FULL;
    }
    
    // Add to global analyzer list
    else
    {
        ta_g_analyzers[ta_g_analyzer_count++] = me;
    
        // Clear the entire struct first
        memset(me, 0, sizeof(TA_t));
    
        // Storing the info
        me->mode    = mode;
        me->state   = TA_STATE_IDLE;
        me->name    = name;
    
        // Initialize the timing fields
        me->start_time      = 0UL;
        me->stop_time       = 0UL;
        me->elapsed_time    = 0UL;
        
        // Assign pin control function pointers
        me->pin_control_func = pin_ctrl;
    }
    return res;
}

/**
 * Func to start counting using SysTick or DWT and set pin HIGH if configured.
 * \param TA_t *const me            : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null, 
 *         RC_ERROR_INVALID_STATE when analyzer is not in any active state and/or when Hardware not properly initilized and 
 *         RC_ERROR_BUSY when an analyzer is already in running state
*/
RC_t TA_start(TA_t *const me)
{
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL_PTR)
    {
        return RC_ERROR_NULL;
    }
    
    // Check state - If already running, invalid call
    if (TA_STATE_RUNNING == me->state)
    {
        return RC_ERROR_BUSY;
    }
    
    // Handle pin output first for precision
    if ((TA_MODE_SYSTICK_PIN == me->mode) || (TA_MODE_DWT_PIN  == me->mode) || (TA_MODE_PIN  == me->mode))
    {
        if (me->pin_control_func != NULL_PTR)
        {
            me->pin_control_func(1U);   /* Turn ON pin */
        } else {
            return RC_ERROR_NULL;
        }
    }
    
    // Fetching the counter FIRST for precision
    if (TA_MODE_SYSTICK == me->mode || TA_MODE_SYSTICK_PIN  == me->mode) {
        // SysTick counts in ms
        me->start_time = ta_g_system_ms;  // Start a cycle interval
    } else if (TA_MODE_DWT  == me->mode || TA_MODE_DWT_PIN  == me->mode) {
        // DWT->CYCCNT = current CPU cycle count
        me->start_time = DWT->CYCCNT;  // Start a cycle interval
    } else {
        // TA_MODE_PIN only - do nothing
    }

    // Update state
    me->state = TA_STATE_RUNNING;
    
    return res;
}

/**
 * Func to stop counting temporarily and add elapsed time since start to total time.
 * \param TA_t *const me            : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null and 
 *         RC_ERROR_INVALID_STATE when analyzer is not in any active state and/or not running
*/
RC_t TA_pause(TA_t *const me)
{
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL_PTR)
    {
        return RC_ERROR_NULL;
    }
    
    // Check state - Can only pause when running
    if (TA_STATE_RUNNING != me->state)
    {
        return RC_ERROR_INVALID_STATE;
    }
    
    // Fetching the counter FIRST for precision
    if (TA_MODE_SYSTICK == me->mode || TA_MODE_SYSTICK_PIN == me->mode) {
        // SysTick counts in ms
        me->stop_time = ta_g_system_ms;  // Stop current cycle interval
        
    } else if (TA_MODE_DWT == me->mode || TA_MODE_DWT_PIN == me->mode) {
        // DWT cycle counter
        me->stop_time = DWT->CYCCNT;  // Stop current cycle interval
        
    } else {
        // TA_MODE_PIN only - do nothing
    }
    
    res = TA_calculateElapsedTime(me);
    
    // Pin LOW to show pause
    if ((TA_MODE_SYSTICK_PIN == me->mode) || (TA_MODE_DWT_PIN == me->mode) || (TA_MODE_PIN == me->mode))
    {
        if (me->pin_control_func != NULL_PTR)
        {
            me->pin_control_func(0U); /* Turn OFF pin */
        } else {
            return RC_ERROR_NULL;
        }
    }
    
    // Update state
    me->state = TA_STATE_PAUSED;
    
    return res;
}

/**
 * Func to resume measurement from paused state.
 * \param TA_t *const me            : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null, 
 *         RC_ERROR_INVALID_STATE when analyzer is not in any active state and/or not paused
*/
RC_t TA_resume(TA_t *const me)
{
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL_PTR)
    {
        return RC_ERROR_NULL;
    }
    
    // Check state - Can only resume if paused
    if (TA_STATE_PAUSED != me->state)
    {
        return RC_ERROR_INVALID_STATE;
    }
    
    // Pin HIGH to indicate resumed timing
    if ((TA_MODE_SYSTICK_PIN == me->mode) || (TA_MODE_DWT_PIN == me->mode) || (TA_MODE_PIN == me->mode))
    {
        if (me->pin_control_func != NULL_PTR)
        {
           me->pin_control_func(1U);   /* Turn ON pin */
        } else {
            return RC_ERROR_NULL;
        }
    }
    
    // Fetching the counter FIRST for precision
    if (TA_MODE_SYSTICK == me->mode || TA_MODE_SYSTICK_PIN == me->mode) {
       // SysTick counts in ms
        me->start_time = ta_g_system_ms;  // Start new cycle interval
    } else if (TA_MODE_DWT == me->mode || TA_MODE_DWT_PIN == me->mode) {
        // DWT cycle counter
        me->start_time = DWT->CYCCNT;  // Start new cycle interval
    } else {      
        // TA_MODE_PIN only - do nothing
    }
    
    // Update state
    me->state = TA_STATE_RUNNING;
    
    return res;
}

/**
 * Func to stop counting, calculate total time, and set pin LOW.
 * \param TA_t *const me            : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null and 
 *         RC_ERROR_INVALID_STATE when analyzer is not in any active state and/or already stopped
*/
RC_t TA_stop(TA_t *const me)
{
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL_PTR)
    {
        return RC_ERROR_NULL;
    }
    
    // Check state - Can only stop if running or paused
    if (TA_STATE_RUNNING != me->state && TA_STATE_PAUSED != me->state) 
    {
        return RC_ERROR_INVALID_STATE;
    }
    
    // Fetching the counter FIRST for precision
    if (TA_MODE_SYSTICK == me->mode || TA_MODE_SYSTICK_PIN == me->mode) {
        // SysTick counts in ms
        me->stop_time = ta_g_system_ms;  // Stop the cycle interval        
    } else if (TA_MODE_DWT == me->mode || TA_MODE_DWT_PIN == me->mode) {
        // DWT cycle counter
        me->stop_time = DWT->CYCCNT;  // Stop the cycle interval
    } else {
        // TA_MODE_PIN only - do nothing
    }
    
    res = TA_calculateElapsedTime(me);
    
    // Clear pin after stopping
    if ((TA_MODE_SYSTICK_PIN == me->mode) || (TA_MODE_DWT_PIN == me->mode) || (TA_MODE_PIN == me->mode))
    {
        if (me->pin_control_func != NULL_PTR)
        {
            me->pin_control_func(0U); /* Turn OFF pin */
        } else {
            return RC_ERROR_NULL;
        }
    }
    
    // Update state
    me->state = TA_STATE_STOPPED;
    
    return res;
}

/**
 * Func to calculate the elapsed ticks/cycles between start and stop time.
 * \param TA_t *const me            : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success
*/
RC_t TA_calculateElapsedTime(TA_t *const me)
{
    RC_t res = RC_SUCCESS;
    
    // Calculate the elapsed time
    if (me->stop_time >= me->start_time)
    {
        me->elapsed_time = me->elapsed_time + (me->stop_time - me->start_time);
    } else {
        /* Handle 32-bit wrap-around */
        me->elapsed_time = me->elapsed_time + ((TA_MAX_32BIT_VALUE - me->start_time) + me->stop_time + TA_COUNTER_INCREMENT);
    }
    return res;
}

/**
 * Func which returns elapsed time based on SysTick or DWT reading.
 * \param TA_t *const me        : [IN] struct of Analyzer related parameters
 * \return RC_SUCCESS when success and RC_ERROR_NULL when the me pointer is null
*/
RC_t TA_printStatus(TA_t *const me)
{
    RC_t res = RC_SUCCESS;
    
    // Validate the input
    if (me == NULL_PTR)
    {
        return RC_ERROR_NULL;
    }
    
    // Buffer to assemble string
    char buffer[150]; 

    // Format state as string
    const char* strState;
    switch(me->state)
    {
        case TA_STATE_IDLE:
            strState = "IDLE"; 
            break;
        case TA_STATE_STOPPED: 
            strState = "STOPPED"; 
            break;
        case TA_STATE_RUNNING: 
            strState = "RUNNING"; 
            break;
        case TA_STATE_PAUSED:  
            strState = "PAUSED"; 
            break;
        default:
            strState = "UNKNOWN"; 
            break;
    }    
    
    // Assemble full status string
    if(me->mode == TA_MODE_DWT || me->mode == TA_MODE_DWT_PIN)
    {
        uint32_t int_ms, frac_ms, rem_cycles;
        int_ms = me->elapsed_time / BCLK__BUS_CLK__KHZ;
        rem_cycles = me->elapsed_time % BCLK__BUS_CLK__KHZ;
        frac_ms = (rem_cycles * TA_SCALE_FACTOR) / BCLK__BUS_CLK__KHZ;
        
        snprintf(buffer, sizeof(buffer), "Name: %s | State: %s | Elapsed time: %u.%06ums | Cycles: %u\r\n",
             me->name, strState, int_ms, frac_ms, me->elapsed_time);
    } else if(me->mode == TA_MODE_PIN) {
        snprintf(buffer, sizeof(buffer), "Name: %s | State: %s\r\n", // N10
            me->name, strState);
    } else {
        snprintf(buffer, sizeof(buffer), "Name: %s | State: %s | Elapsed time: %lums\r\n",
            me->name, strState, me->elapsed_time);
    }

    // Send string to UART
    UART_LOG_PutString(buffer);
    
    return res;
}

/**
 * Func to print all the available Analysers
 * \param None
 * \return RC_SUCCESS when success and RC_ERROR_BUFFER_EMTPY when analyzer array is empty
*/
RC_t TA_printAll(void)
{
    RC_t res = RC_SUCCESS;
    
    boolean_t anyActive = FALSE;
    
    for (uint8_t i = 0; i < ta_g_analyzer_count; i++)
    {
        TA_t *me = ta_g_analyzers[i];
        if ((me != NULL_PTR) && me->state)
        {
            anyActive = TRUE;
            TA_printStatus(me);
        } else {
            return RC_ERROR_NULL;
        }
    }
    
    // MISRA Rule 13.7 : Boolean expressions should be explicit. (Use to avoid ambiguity.)
    if (anyActive == FALSE)
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
    ta_g_system_ms++;
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
 * 24-bit counter.
 *
 * 10. snprintf - avoids buffer overflow.
 *
 * > MISRA-C:2004 compliancy - ~85–90%
 */

/* [TimingAnalyzer.c] END OF FILE */