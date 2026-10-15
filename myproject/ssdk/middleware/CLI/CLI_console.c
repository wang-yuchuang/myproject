/*
 * console.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: console function.
 *
 * Revision History:
 * -----------------
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <cmsis_os2.h>
#include <CLI.h>
#include <debug.h>
#include <driver/uart.h>
#include <board.h>
#ifdef CONFIG_CU
#include <cu.h>
#endif

/* BS and DEL ASCII Number */
#define CLI_ASCII_BS    0x08
#define CLI_ASCII_DEL   0x7F

/* Command Buffer */
#define CLI_INPUT_BUFFER_SIZE       256
#define CLI_OUTPUT_BUFFER_SIZE      1024
static char gcInputBuffer[CLI_INPUT_BUFFER_SIZE];
static char gcOutputBuffer[CLI_OUTPUT_BUFFER_SIZE];
static uint8_t gucInputIndex;

/* a linear array of statically defined command blocks,
   defined in the linker script.
 */
extern CLI_Command_Definition_t __commands_start[];
extern CLI_Command_Definition_t __commands_end[];

/* Uart Recv Semaphore */
static osSemaphoreId_t gxRxSemaphore;

/* Uart Console */
static uart_dev_t *gpxUartConsole;

/* Const messages output by the command console. */
static const char * const gpcWelcomeMessage = "\r\nFreeRTOS CLI.\r\nType Help to view a list of registered commands.\r\n>";
static const char * const gpcNewLine = "\r\n>";
static const char * const gpcErrMessage = "\r\nInput Command Error\r\n>";

#ifdef CONFIG_SDSHELL_BACKEND
bool is_sdshell_backend_running(void);
#endif

/*
 * CLI Set Uart Console.
 */
static inline void CLISetUartConsole(uart_dev_t *dev)
{
    gpxUartConsole = dev;
}

/*
 * CLI Uart Write.
 */
static inline void CLIUartWrite(const char *data, size_t len)
{
#ifdef CONFIG_CU
    if (g_console_occupid && len > 0) {
        uart_transmit(gpxUartConsole, data, len);
    }
    else if (len > 0) {
        cu_buffer_print_log(data, len);
    }
#else
    if (len > 0) {
        uart_transmit(gpxUartConsole, data, len);
    }
#endif
}

/*
 * Uart Recv Callback function.
 */
static void CLIUartReadCallback(void *arg)
{
    osSemaphoreRelease(gxRxSemaphore);
}

/*
 * CLI Uart Read.
 */
static inline bool CLIUartRead(char *data, size_t len)
{
    if (len <= 0)
        return false;

#ifdef CONFIG_CLI_SYNC_MODE
        uart_receive(gpxUartConsole, data, len);
#else
        uart_receive_async(gpxUartConsole, data, len, CLIUartReadCallback);
        if (osSemaphoreAcquire(gxRxSemaphore, osWaitForever) != osOK) {
            return false;
        }
#endif

    return true;
}

/*
 * Register CLI commands.
 */
static void CLIRegisterCommands(void)
{
    CLI_Command_Definition_t *cmd;
    for (cmd = __commands_start; cmd != __commands_end; cmd++) {
        CLIRegisterCommand(cmd);
    }
}

/*
 * console task entry.
 *
 * @ arg   input param.
 */
static void CLICommandHandle(char cRxedChar)
{
    int xReturned;

    /* Is end of the line? */
    if(cRxedChar == '\n' || cRxedChar == '\r')
    {
        if (gucInputIndex) {
            CLIUartWrite("\r\n", strlen("\r\n"));
            gcInputBuffer[gucInputIndex] = '\0';
            do {
                gcOutputBuffer[0] = '\0';

                /* Get the next output string from the command interpreter. */
                xReturned = CLIProcessCommand(gcInputBuffer, gcOutputBuffer, CLI_OUTPUT_BUFFER_SIZE);

                /* Write the generated string to the UART. */
                CLIUartWrite(gcOutputBuffer, strlen(gcOutputBuffer));
            } while(xReturned != CLI_FALSE);

            gucInputIndex = 0;
        }
        /* Start New Line */
        CLIUartWrite(gpcNewLine, strlen(gpcNewLine));
    }
    else if((cRxedChar == CLI_ASCII_BS) || (cRxedChar == CLI_ASCII_DEL))
    {
        /* Backspace was pressed.  Erase the last character in the
        string - if any. */
        if(gucInputIndex > 0)
        {
            gucInputIndex--;
            gcInputBuffer[gucInputIndex] = '\0';
            char bs = CLI_ASCII_BS;
            char space = ' ';
            CLIUartWrite(&bs, 1);
            CLIUartWrite(&space, 1);
            CLIUartWrite(&bs, 1);
        }
    }
    else
    {
        /* echo */
        CLIUartWrite(&cRxedChar, 1);

        if (gucInputIndex < CLI_INPUT_BUFFER_SIZE) {
            gcInputBuffer[gucInputIndex] = cRxedChar;
            gucInputIndex++;
        }
        else {
            CLIUartWrite(gpcErrMessage, strlen(gpcErrMessage));
            gucInputIndex = 0;
        }
    }
}

#ifdef CONFIG_SDRV_VIRT_CONSOLE
int sdshell_save_char(char *buf, int len)
{
    int i;

    for (i = 0; i < len; i++)
        CLICommandHandle(buf[i]);

    CLICommandHandle('\n');

    return 0;
}
#endif

/*
 * console task entry.
 *
 * @ arg   input param.
 */
static void CLIConsoleTask(void *pcArg)
{
    static char cRxedChar;

    for( ;; )
    {
#ifdef CONFIG_SDSHELL_BACKEND
        if (is_sdshell_backend_running()) {
            osDelay(10);
            continue;
        }
#endif
        if (CLIUartRead(&cRxedChar, 1)) {
            CLICommandHandle(cRxedChar);
        }
#ifdef CONFIG_OS_POLL
        else {
            break;
        }
#endif
    }
}

/*
 * Tokenize Command.
 *
 * @ pcCommandString   input string.
 * @ argv argv array.
 */
int CLITokenizeCommand(const char *pcCommandString, char *argv[])
{
    int arg_num = 0;
    int arg_len = 0;
    char *arg_ptr = (char *)pcCommandString;

    /* Index the character pointer past the current word.  If this is the start
    of the command string then the first word is the command itself. */
    while(((*arg_ptr) != 0x00) && ((*arg_ptr) != ' '))
    {
        arg_ptr++;
    }

    while ((arg_ptr = (char *)CLIGetParameter(arg_ptr, 1, &arg_len)) != NULL) {
        argv[arg_num++] = arg_ptr;
        arg_ptr += arg_len;
        if (*arg_ptr == ' ') {
            *arg_ptr = '\0';
            arg_ptr++;
        }
        if (arg_num >= CLI_MAX_ARGS) {
            ssdk_printf(SSDK_WARNING, "CLI command extend max args num\n");
            break;
        }
    }

    return arg_num;
}

/*
 * start console task.
 *
 */
void CLIConsoleStart(void)
{
    CLISetUartConsole(SSDK_CONSOLE);

    CLIRegisterCommands();

    gxRxSemaphore = osSemaphoreNew(1, 0, NULL);

    osThreadAttr_t attr = {
        .name = "CLI",
        .stack_size = CONFIG_CLI_STACK_SIZE,
        .priority = CONFIG_CLI_PRIORITY,
    };

    CLIUartWrite(gpcWelcomeMessage, strlen(gpcWelcomeMessage));

    osThreadNew(CLIConsoleTask, NULL, &attr);
}
