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

#ifndef PINS_H
#define PINS_H

#include "cytypes.h"
#include "project.h"   /* Required for RED_LED_Write(), etc. */

/**
* \Hardware pin/LED enum
*
* Enum to hold different hardware pins/LEDs used for external timing
*/
 enum ePin{
    PIN_NONE = 0,       /**< \No pin selected. */
    PIN_RED,            /**< \Pin red selected. */
    PIN_YELLOW,         /**< \Pin yellow selected. */
    PIN_GREEN           /**< \Pin green selected. */
} ;
typedef enum ePin AnalyzerPin_t;
    
/* MISRA-C:2004 Rule 8.8 — external linkage explicitly declared */
extern void Pins_Init(void);

/* Individual pin set/clear functions */
extern void Pin_1_Control(uint8_t state);

extern void Pin_2_Control(uint8_t state);

extern void Pin_3_Control(uint8_t state);

#endif /* PINS_H */


/* [Pins.h] END OF FILE */
