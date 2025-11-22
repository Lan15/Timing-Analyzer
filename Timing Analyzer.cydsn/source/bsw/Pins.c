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

#include "Pins.h"

/* Initialize pins */
void Pins_Init(void)
{
    /* Set each pin to low */
    RED_LED_Write(0U);
    YELLOW_LED_Write(0U);
    GREEN_LED_Write(0U);
}

/* Wrappers for RED LED */
void Pin_1_Control(uint8_t state)
{
    RED_LED_Write(state);
}

/* Wrappers for YELLOW LED */
void Pin_2_Control(uint8_t state)
{
    YELLOW_LED_Write(state);
}

/* Wrappers for GREEN LED */
void Pin_3_Control(uint8_t state)
{
    GREEN_LED_Write(state);
}

/* [Pins.c] END OF FILE */
