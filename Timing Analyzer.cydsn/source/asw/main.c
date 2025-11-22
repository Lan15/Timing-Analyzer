/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "stdio.h"
#include "global.h"
#include "TimingAnalyzer.h"

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here */
    
    RC_t res = RC_SUCCESS;
    
    // MCU system init
    
    // Initialize timing system
    res = TimingAnalyzer_initialize();     
    
    char strMessage[100];
    
    // Initialize UART protocol
    UART_LOG_Start();
    
    UART_LOG_PutString("\r\n\r\nTiming Analyzer\r\n");                                                // (N1)
    sprintf(strMessage, "CPU-Frequency: %luMHz\r\n", ((unsigned long)BCLK__BUS_CLK__HZ / 1000000));   // (N2)
    UART_LOG_PutString(strMessage); 
    CyDelay(100);           // Wating for UART to complete

    TimingAnalyzer_t analyzer1;
    
    res = TimingAnalyzer_create(&analyzer1, MODE_DWT, PIN_RED, "Task1");

    res = TimingAnalyzer_start(&analyzer1);
    // Code region you want to measure
    // do something
    CyDelay(100);
    
    res = TimingAnalyzer_pause(&analyzer1);
    
    CyDelay(100);   // Should NOT be counted
    
    res = TimingAnalyzer_resume(&analyzer1);
    
    CyDelay(100);
    
    res = TimingAnalyzer_stop(&analyzer1);
    
    // Printing
    res = TimingAnalyzer_printStatus(&analyzer1);
    //res = TimingAnalyzer_printAll(&analyzer1);
    
    // Info for debugging
    if(res != RC_SUCCESS){
        char strError[20];
        sprintf(strError, "Error: code %u\r\n", res);
        UART_LOG_PutString(strError);
    }
}

/* RC_t DWT_Enable(uint16_t value) {
    RC_t res = RC_SUCCESS;
    
    // Enable debug and trace
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0)   return RC_ERROR_NOT_INITIALIZERD;
    
    // Enable the cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0)  return RC_ERROR_NOT_INITIALIZERD;
    
    // Reset the cycle counter
    DWT->CYCCNT = 0; 
    
    if(!value){
        return DWT->CYCCNT;
    }
    
    return res;
} */

/* NOTE
 * 
 * 1. \r is a carriage return escape sequence used in programming and text files to move the cursor 
 * to the beginning of the current line, allowing new text to overwrite what was previously there. 
 * It is often used in conjunction with \n (newline) to create a new line, with Windows typically 
 * using the \r\n combination for a new line. 
 *
 * 2. CPU frequency can be accessed via the define BCLK__BUS_CLK__HZ
 */

/* [main.c] END OF FILE */
