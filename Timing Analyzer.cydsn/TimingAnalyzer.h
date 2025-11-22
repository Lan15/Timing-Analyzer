/**
* \file <CAnalyzer.h>
* \author <AGILAN V S>
* \date <22-10-2025>
*
* \brief <Symbolic File name>
*
* detailed description what the file does
*
* \note <notes>
* Programming rules (may be deleted in the final release of the file)
* ===================================================================
*
* 1. Naming conventions:
*    - Prefix of your module in front of every function and static data. 
*    - Scope _ for public and __ for private functions / data / types, e.g. 
*       Public:  void CONTROL_straightPark_Init();
*       Private: static void CONTROL__calcDistance();
*       Public:  typedef enum {RED, GREEN, YELLOW} CONTROL_color_t
*    - Own type definitions e.g. for structs or enums get a postfix _t
*    - #define's and enums are written in CAPITAL letters
* 2. Code structure
*    - Be aware of the scope of your modules and functions. Provide only functions which belong to your module to your files
*    - Prepare your design before starting to code
*    - Implement the simple most solution (Too many if then else nestings are an indicator that you have not properly analysed your task)
*    - Avoid magic numbers, use enums and #define's instead
*    - Make sure, that all error conditions are properly handled
*    - If your module provides data structures, which are required in many other files, it is recommended to place them in a file_type.h file
*	  - If your module contains configurable parts, is is recommended to place these in a file_config.h|.c file
* 3. Data conventions
*    - Minimize the scope of data (and functions)
*    - Global data is not allowed outside of the signal layer (in case a signal layer is part of your design)
*    - All static objects have to be placed in a valid linker sections
*    - Data which is accessed in more than one task has to be volatile and needs to be protected (e.g. by using messages or semaphores)
*    - Do not mix signed and unsigned data in the same operation
* 4. Documentation
*    - Use self explaining function and variable names
*    - Use proper indentation
*    - Provide Javadoc / Doxygen compatible comments in your header file and C-File
*    		- Every  File has to be documented in the header
*			- Every function parameter and return value must be documented, the valid range needs to be specified
*     		- Logical code blocks in the C-File must be commented
*    - For a detailed list of doxygen commands check http://www.stack.nl/~dimitri/doxygen/index.html 
* 5. Qualification
*    - Perform and document design and code reviews for every module
*    - Provide test specifications for every module (focus on error conditions)
*
* Further information:
*    - Check the programming rules defined in the MIMIR project guide
*         - Code structure: https://mimir.h-da.io/public/methods/eng_codestructure/method.htm
*         - MISRA for C: https://mimir.h-da.io/public/methods/eng_c_rules/method.htm
*         - MISRA for C++: https://mimir.h-da.io/public/methods/eng_cpp_rules/method.htm
*
* \todo <todos>
* \warning <warnings, e.g. dependencies, order of execution etc.>
*
*  Changelog:\n
*  - <version; data of change; author>
*            - <description of the change>
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
**/


 
#ifndef TIMINGANALYZER_H
#define TIMINGANALYZER_H

#include "global.h"

/*****************************************************************************/
/* Global pre-processor symbols/macros and type declarations                 */
/*****************************************************************************/

//####################### Defines/Macros
/** 
 * \brief a brief description of what the define is representing
*
* If needed, a more detailed description can be given below */
#define TOP_DOCUMENTED_DEFINE                    0x1
#define AFTER_DOCUMENTED_DEFINE                  0x2         /**< \brief by putting a < next to the comment-start. The documentation referes to the left instead to the next line. */

//####################### Enumerations
/**
* \Analyzer configurations enum
*
* Enum to hold different configuration modes possible with the analyzer
*/
 enum eMode{
  MODE_DWT,             /**< \DWT Cycle Counter. */
  MODE_DWT_PIN,         /**< \DWT Cycle Counter + Output pin. */
  MODE_SYSTICK,         /**< \SYSTICK timer (1ms tick). */
  MODE_SYSTICK_PIN,     /**< \SYSTICK + Output pin config. */
  MODE_PIN              /**< \Output pin only (external measurement). */
} ;
typedef enum eMode AnalyzerMode_t;

/**
* \Analyzer state enum
*
* Enum to hold different states that the analyzer could take
*/
 enum eState{
  STATE_CONFIGURED,     /**< \Analyser in Configured state. */
  STATE_RUNNING,        /**< \Analyser in Running state. */
  STATE_PAUSED,         /**< \Analyser in Paused state. */
  STATE_STOPPED         /**< \Analyser in Paused state. */
} ;
typedef enum eState AnalyzerState_t;

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

//####################### Structures
/**
* \Analyzer instance struct
*
* Main structure that holds everything about one analyzer instance.
*/
struct sAnalyzer {
    /* Configuration Data */
    const char *name;               // String name for print/log
    AnalyzerMode_t mode;            // Selected measurement mode (SysTick, DWT, etc.)
    AnalyzerPin_t pin;              // Associated output pin (or PIN_NONE)
    AnalyzerState_t state;          // Current analyzer state

    /* Measurement Data */
    uint32_t start_tick;            // Start tick (for SysTick mode)
    uint32_t stop_tick;             // Stop tick (for SysTick mode)
    uint32_t start_cycles;          // Start cycle count (for DWT mode)
    uint32_t stop_cycles;           // Stop cycle count (for DWT mode)
    float elapsed_time_ms;          // Elapsed time in milliseconds
    uint32_t elapsed_cycles;        // Elapsed CPU cycles (for DWT mode)
    
    /* Pause/Resume Support */
    double accumulated_time_ms;     // Total time across pauses
    uint32_t accumulated_cycles;    // Total cycles across pauses

};
typedef struct sAnalyzer TimingAnalyzer_t;

// Wrapper to allow representing the file in Together as class
#ifdef TOGETHER

class CAnalyzer
{
public:
    
#endif /* Together */

/*****************************************************************************/
/* Extern global variables                                                   */
/*****************************************************************************/

/**
 * <description>
 */
//extern type FILE_variable;

/*****************************************************************************/
/* API functions                                                             */
/*****************************************************************************/

/**
 * Func to initialize of the necessary peripherals like Set up SysTick timer (1 ms), enable DWT counter, and configure GPIO pins.
 * \param None
 * \return RC_SUCCESS when success and RC_ERROR_NOT_INITIALIZERD when Hardware not properly initilized      // ???
*/
RC_t TimingAnalyzer_initialize(void);

/**
 * Func to initializes an analyzer struct with configuration and assign function pointers for pin control.
 * \param TimingAnalyzer_t *me	    : [IN/OUT] struct of Analyzer related parameters
 * \param AnalyzerMode_t mode       : [IN] type of the configuration mode we need to run the analyzer
 * \param AnalyzerPin_t pin         : [IN] GPIO pin incase of Output pin config use
 * \param const char name           : [IN] name of the Analyzer
 * \return RC_SUCCESS when success and RC_ERROR_NULL when the me pointer is null
*/
RC_t TimingAnalyzer_create(TimingAnalyzer_t *me, AnalyzerMode_t mode, AnalyzerPin_t pin, const char *name);

/**
 * Func to start counting using SysTick or DWT and set pin HIGH if configured.
 * \param TimingAnalyzer_t *me	    : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null, 
 *         RC_ERROR_INVALID_STATE when not respective state and 
 *         RC_ERROR_NOT_INITIALIZERD when Hardware not properly initilized
*/
RC_t TimingAnalyzer_start(TimingAnalyzer_t *me);

/**
 * Func to stop counting, calculate total time, and set pin LOW.
 * \param TimingAnalyzer_t *me	    : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null and 
 *         RC_ERROR_INVALID_STATE when not respective state
*/
RC_t TimingAnalyzer_stop(TimingAnalyzer_t *me);

/**
 * Func to stop counting temporarily and add elapsed time since start to total time.
 * \param TimingAnalyzer_t *me	    : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null and 
 *         RC_ERROR_INVALID_STATE when not respective state
*/
RC_t TimingAnalyzer_pause(TimingAnalyzer_t *me);

/**
 * Func to resume measurement from paused state.
 * \param TimingAnalyzer_t *me	    : [IN/OUT] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null, 
 *         RC_ERROR_INVALID_STATE when not respective state
*/
RC_t TimingAnalyzer_resume(TimingAnalyzer_t *me);

/**
 * Func which returns elapsed time based on SysTick or DWT reading.
 * \param TimingAnalyzer_t *me	    : [IN] struct of Analyzer related parameters
 * \return RC_SUCCESS when success, RC_ERROR_NULL when the me pointer is null
*/
RC_t TimingAnalyzer_printStatus(TimingAnalyzer_t *me);

/**
 * Func to print all the available Analysers
 * \param None
 * \return RC_SUCCESS when success
*/
RC_t TimingAnalyzer_printAll(void);

/*****************************************************************************/
/* Private stuff, only visible for Together, declared static in cpp - File   */
/*****************************************************************************/

#ifdef TOGETHER
//Not visible for compiler, only used for document generation
private:


};
#endif /* Together */

#endif /* CANALYZER_H */

/* [TimingAnalyzer.h] END OF FILE */
