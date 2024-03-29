/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "Board.h"

#include <stdint.h>

#include <ti/uia/runtime/LogUC.h>
#include <ti/uia/events/UIABenchmark.h>

#define TASKSTACKSIZE     768

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/* Global memory storage for a PIN_Config table */
static PIN_State ledPinState;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config ledPinTable[] = {
Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
                             Board_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
                             PIN_TERMINATE };

/*
 *  ======== echoFxn ========
 *  Task for this function is created statically. See the project's .cfg file.
 */
Void echoFxn(UArg arg0, UArg arg1){
  char input;
  UART_Handle uart;
  UART_Params uartParams;
  const char echoPrompt[] = "Echoing characters:\r\n";

  /* Create a UART with data processing off. */
  UART_Params_init(&uartParams);
  uartParams.writeDataMode = UART_DATA_BINARY;
  uartParams.readDataMode = UART_DATA_BINARY;
  uartParams.readReturnMode = UART_RETURN_FULL;
  uartParams.readEcho = UART_ECHO_OFF;
  uartParams.baudRate = 9600;
  uart = UART_open(Board_UART0, &uartParams);

  if (uart == NULL) {
    System_abort("Error opening the UART");
  }

  /* loop until we reach termination timestamp */
  Log_write1(UIABenchmark_start, (IArg) "prompt");
  UART_write(uart, echoPrompt, sizeof(echoPrompt));
  Log_write1(UIABenchmark_stop, (IArg) "prompt");

  /* Loop forever echoing */
  while (1) {
    UART_read(uart, &input, 1);
    Log_write1(UIABenchmark_start, (IArg) "write");
    UART_write(uart, &input, 1);
    Log_write1(UIABenchmark_stop, (IArg) "write");
  }
}

/*
 *  ======== main ========
 */
int main(void){
  PIN_Handle ledPinHandle;
  Task_Params taskParams;

  /* Call board init functions */
  Board_initGeneral()
  ;
  Board_initUART();

  /* Construct BIOS objects */
  Task_Params_init(&taskParams);
  taskParams.stackSize = TASKSTACKSIZE;
  taskParams.stack = &task0Stack;
  Task_construct(&task0Struct, (Task_FuncPtr) echoFxn, &taskParams, NULL);

  /* Open LED pins */
  ledPinHandle = PIN_open(&ledPinState, ledPinTable);
  if (!ledPinHandle) {
    System_abort("Error initializing board LED pins\n");
  }

  PIN_setOutputValue(ledPinHandle, Board_LED1, 1);

  /* This example has logging and many other debug capabilities enabled */
  System_printf("This example does not attempt to minimize code or data "
                "footprint\n");
  System_flush();

  System_printf("Starting the UART Echo example\nSystem provider is set to "
                "SysMin. Halt the target to view any SysMin contents in "
                "ROV.\n");
  /* SysMin will only print to the console when you call flush or exit */
  System_flush();

  /* Start BIOS */
  BIOS_start();

  return (0);
}
