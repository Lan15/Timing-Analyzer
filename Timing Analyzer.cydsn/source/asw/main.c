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
#include "Pins.h"

volatile TimingAnalyzer_t analyzerMain;     // Creating obj inside main can not be used to refer an isr.   !!!
volatile TimingAnalyzer_t analyzerIsrLow;   // (N3)
volatile TimingAnalyzer_t analyzerIsrHigh;

extern volatile uint32_t system_ms;     // ???

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
    
    res = TimingAnalyzer_create((TimingAnalyzer_t *)&analyzerMain, MODE_DWT_PIN, Pin_1_Control, "DWT Task");
    res = TimingAnalyzer_create((TimingAnalyzer_t *)&analyzerIsrLow, MODE_SYSTICK_PIN, Pin_2_Control, "SYSTICK Task");
    res = TimingAnalyzer_create((TimingAnalyzer_t *)&analyzerIsrHigh, MODE_PIN, Pin_3_Control, "PIN Task");

    res = TimingAnalyzer_start((TimingAnalyzer_t *)&analyzerMain);
    res = TimingAnalyzer_start((TimingAnalyzer_t *)&analyzerIsrLow);
    res = TimingAnalyzer_start((TimingAnalyzer_t *)&analyzerIsrHigh);
    // Code region you want to measure
    // do something
    CyDelay(1000);
    
    res = TimingAnalyzer_pause((TimingAnalyzer_t *)&analyzerMain);
    
    CyDelay(100);   // Should NOT be counted
    
    res = TimingAnalyzer_resume((TimingAnalyzer_t *)&analyzerMain);

    CyDelay(1000);
    //CyDelay(5000);
    res = TimingAnalyzer_stop((TimingAnalyzer_t *)&analyzerMain);
    res = TimingAnalyzer_stop((TimingAnalyzer_t *)&analyzerIsrLow);
    CyDelay(5000);
    res = TimingAnalyzer_stop((TimingAnalyzer_t *)&analyzerIsrHigh);
    
    // Printing
    res = TimingAnalyzer_printStatus((TimingAnalyzer_t *)&analyzerMain);
    res = TimingAnalyzer_printStatus((TimingAnalyzer_t *)&analyzerIsrLow);
    
    UART_LOG_PutString("\r\nPrinting All Analyzers\r\n"); 
    res = TimingAnalyzer_printAll();
    
    // Info for debugging
    if(res != RC_SUCCESS){
        char strError[20];
        sprintf(strError, "Error: code %u\r\n", res);
        UART_LOG_PutString(strError);
    }
}

// MISRA Violation Rule 2.2 : No dead or unused code. (All lines have a clear function (no commented-out or dead code).)
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
 * 
 * 3. Error: passing argument 1 of 'TimingAnalyzer_create' discards 'volatile' qualifier from 
 * pointer target type - make sure the implementation also uses volatile.
 *
 * 4.
 *
 * > MISRA-C:2004 compliancy - ~85–90%
 */

/* [main.c] END OF FILE */
