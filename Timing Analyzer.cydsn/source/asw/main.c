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
// Standard header files
#include "project.h"
#include "stdio.h"
#include <math.h>
#include "global.h"

// Project specific header files
#include "TimingAnalyzer.h"
#include "Pins.h"

volatile TA_t analyzerDwt;     // Creating obj inside main can not be used to refer an isr. !!!
volatile TA_t analyzerSystick;   // (N3)
volatile TA_t analyzerPin;
volatile TA_t analyzerMath;
volatile TA_t analyzerIsr1msDWT;
volatile TA_t analyzerIsr1msSYS;
volatile TA_t analyzerIsr2secsDWT;
volatile TA_t analyzerIsr2secsSYS;

extern volatile uint32_t system_ms;

CY_ISR(ISR_1ms_handler);
CY_ISR(ISR_2secs_handler);

// Change this define to activate the different Snippets
#define CodeSnippetMain
//#define CodeSnippetMath
//#define CodeSnippetIsr


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    RC_t res = RC_SUCCESS;
    
    // Initialize timing system
    res = TA_init();     
    
    char strMessage[100];
    
    // Initialize UART protocol
    UART_LOG_Start();
    
    UART_LOG_PutString("\r\n\r\nTiming Analyzer\r\n");                                                // (N1)
    sprintf(strMessage, "CPU-Frequency: %luMHz\r\n", ((unsigned long)BCLK__BUS_CLK__HZ / 1000000));   // (N2)
    UART_LOG_PutString(strMessage); 
    CyDelay(100);       // Wating for UART to complete  
    
    #ifdef CodeSnippetIsr
    //res = TA_create((TA_t *)&analyzerIsr1msDWT, TA_MODE_DWT_PIN, Pin_3_Control, "ISR 1ms DWT Test");
    //res = TA_create((TA_t *)&analyzerIsr1msSYS, TA_TA_MODE_SYSTICK_PIN, Pin_3_Control, "ISR 1ms SYS Test");
    //res = TA_create((TA_t *)&analyzerIsr2secsDWT, TA_MODE_DWT_PIN, Pin_2_Control, "ISR 2secs DWT Test");
    //res = TA_create((TA_t *)&analyzerIsr2secsSYS, TA_TA_MODE_SYSTICK_PIN, Pin_2_Control, "ISR 2secs SYS Test");
    
    res = TA_create((TA_t *)&analyzerIsr1msDWT, TA_MODE_DWT_PIN, YELLOW_LED_Write, "ISR 1ms DWT Func Test");
    res = TA_create((TA_t *)&analyzerIsr2secsDWT, TA_MODE_DWT_PIN, GREEN_LED_Write, "ISR 2secs DWT Func Test");
    
    Timer_1ms_Start();                      // Start timer hardware
    Timer_2secs_Start();
    
    // Initialize isr
    isr_1ms_StartEx(ISR_1ms_handler);       // Register the ISR     //  ???
    isr_2secs_StartEx(ISR_2secs_handler);
    
    #endif
        
    #ifdef CodeSnippetMain
    // Code Main
        
    // Analyzer's creation
    res = TA_create((TA_t *)&analyzerDwt, TA_MODE_DWT_PIN, Pin_3_Control, "DWT Task");
    //res = TA_create((TA_t *)&analyzerSystick, TA_MODE_SYSTICK_PIN, Pin_2_Control, "SYSTICK Task");
    //res = TA_create((TA_t *)&analyzerPin, TA_MODE_PIN, Pin_3_Control, "PIN Task");
    
    res = TA_start((TA_t *)&analyzerDwt);
    //res = TA_start((TA_t *)&analyzerSystick);
    //res = TA_start((TA_t *)&analyzerPin);
    // Code region you want to measure
    // do something
    CyDelay(1000);
    
    //res = TA_pause((TA_t *)&analyzerDwt);
    
    //CyDelay(1000);   // Should NOT be counted
    
    //res = TA_resume((TA_t *)&analyzerDwt);

    CyDelay(1000);
    
    res = TA_stop((TA_t *)&analyzerDwt);
    //res = TA_stop((TA_t *)&analyzerSystick);
    //CyDelay(5000);
    //res = TA_stop((TA_t *)&analyzerPin);
    
    // Printing
    res = TA_printStatus((TA_t *)&analyzerDwt);
    //res = TA_printStatus((TA_t *)&analyzerSystick);
    #endif
    
    #ifdef CodeSnippetMath
    res = TA_create((TA_t *)&analyzerMath, TA_MODE_DWT_PIN, Pin_3_Control, "Math Task");
        
    res = TA_start((TA_t *)&analyzerMath);
    
    volatile int add = 1;   // ???
    int mul = 2;
    int div = 10000;
    float addf = 1.2;
    float mulf = 2.2;
    float divf = 1000.2020;  
    double root;
    float sine = 1000;
    float sinefunc = 1000;
    
    for (int i = 0; i <= 1000; i++)
    {
        add = i + 1;
        //mul = mul * mul;  
        //div = div / 2;
        //++addf;
        //mulf = mulf * mulf; 
        //divf = divf / 2.2;

        //root = sqrt(1000);
        //sine = sin(sine);
        //sinefunc = sinf(sinefunc);
    }
    //CyDelay(100);
    res = TA_stop((TA_t *)&analyzerMath);
    
    // Printing
    res = TA_printStatus((TA_t *)&analyzerMath);
    #endif
    
    //TA_delete((TA_t *)&analyzerSystick);
    
    UART_LOG_PutString("\r\nPrinting All Analyzers\r\n"); 
    res = TA_printAll();
    
    // Info for debugging
    if(res != RC_SUCCESS)
    {
        char strError[20];
        sprintf(strError, "Error: code %u\r\n", res);
        UART_LOG_PutString(strError);
    }
}


#ifdef CodeSnippetIsr
/**
* @brief ISR running at 1 ms.
*/
CY_ISR(ISR_1ms_handler)
{
    // Starting time measurement
    TA_start((TA_t *)&analyzerIsr1msDWT);
    //TA_start((TA_t *)&analyzerIsr1msSYS);
    
    // Clear interrupt flag
    (void)Timer_1ms_ReadStatusRegister();
    isr_1ms_ClearPending();
    
    // Work - do nothing
    
    // Stoping time measurement
    TA_stop((TA_t *)&analyzerIsr1msDWT);
    //TA_stop((TA_t *)&analyzerIsr1msSYS);
}

/**
* @brief ISR running at 2 secs.
*/
CY_ISR(ISR_2secs_handler)
{
    // Starting time measurement
    TA_start((TA_t *)&analyzerIsr2secsDWT);
    //TA_start((TA_t *)&analyzerIsr2secsSYS);
    
    // Clear interrupt flag
    (void)Timer_2secs_ReadStatusRegister();
    isr_2secs_ClearPending();
    
    // Work
    CyDelay(1000);

    // Stoping time measurement
    TA_stop((TA_t *)&analyzerIsr2secsDWT);
    //TA_stop((TA_t *)&analyzerIsr2secsSYS);
    
    TA_printStatus((TA_t *)&analyzerIsr2secsDWT);
    TA_printStatus((TA_t *)&analyzerIsr2secsSYS);
}
#endif

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
 * 4. ISR steps - 1. Add a Timer in TopDesign
 *                2. Write the ISR (Interrupt Service Routine) and declare it
 *                3. Connect the ISR to the Interrupt (in main loop)
 *                4. Clear Interrupt Flag (inside ISR)
 *
 * > MISRA-C:2004 compliancy - ~85–90%
 */

/* [main.c] END OF FILE */
