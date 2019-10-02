




#include "mcu_tm4c123gh6pm.h"
#include "gpio_tm4c123gh6pm.h"
#include "uart_tm4c123gh6pm.h"
#include <string.h>


#define MAX_SIZE 40

char string[MAX_SIZE];


void init_clock(void)
{
    // clock set up
    sysctl_t *pSYSCTL = SYSCTL;

    // 40 Hz
    pSYSCTL->RCC = 0x00000540 | 0x00400000 | (0x04 << 23);

}



int8_t init_gpio(void)
{
    gpio_handle_t led, pb1, uart;

    uart_handle_t console;

    int8_t retval = 0;

    /* On board LED */
    memset(&led, 0, sizeof(led));

    led.p_gpio_x = GPIOF;
    led.gpio_pin_config.pin_mode  = DIGITAL_ENABLE;
    led.gpio_pin_config.direction = DIRECTION_OUTPUT;

    led.gpio_pin_config.pin_number = 1;
    gpio_init(&led);

    led.gpio_pin_config.pin_number = 2;
    gpio_init(&led);

    led.gpio_pin_config.pin_number = 3;
    gpio_init(&led);


    /* Push button */
    memset(&pb1, 0, sizeof(pb1));

    pb1.p_gpio_x = GPIOF;
    pb1.gpio_pin_config.pin_number   = 4;
    pb1.gpio_pin_config.direction    = DIRECTION_INPUT;
    pb1.gpio_pin_config.pin_mode     = DIGITAL_ENABLE;
    pb1.gpio_pin_config.pull_up_down = PULLUP_ENABLE;
    gpio_init(&pb1);


    /* UART configs */
    memset(&uart, 0, sizeof(uart));

    uart.p_gpio_x                           = GPIOA;
    uart.gpio_pin_config.pin_mode           = DIGITAL_ENABLE;
    uart.gpio_pin_config.alternate_function = ALTERNATE_FUNCTION_ENABLE;

    uart.gpio_pin_config.pctl_val   = UART0RX_PA0;
    uart.gpio_pin_config.pin_number = 0;
    gpio_init(&uart);

    uart.gpio_pin_config.pctl_val   = UART0TX_PA1;
    uart.gpio_pin_config.pin_number = 1;
    gpio_init(&uart);

    console.p_uart_x                      = UART0;
    console.uart_config.stop_bits         = ONE_STOP_BIT;
    console.uart_config.uart_baudrate     = 115200;
    console.uart_config.uart_clock_source = CLOCK_SYSTEM;
    console.uart_config.uart_fifo         = FIFO_ENABLE;
    console.uart_config.word_length       = EIGHT_BITS;
    console.uart_config.uart_direction    = UART_TRANSCEIVER;

    uart_init(&console);

    return retval;
}


void putcUart0(char c)
{
    uart_putchar(UART0, c);
}

// Blocking function that writes a string when the UART buffer is not full
void putsUart0(char* str)
{

    uart_write(UART0, str, strlen(str));
}

// Blocking function that returns with serial data once the buffer is not empty
char getcUart0(void)
{
    return uart_getchar(UART0);
}


void project_info(void)
{
    putsUart0("\033]2; Test Console (c) 2019 \007");                                                               // Window Title Information
    putsUart0("\033]10;#FFFFFF\007");                                                                                      // Text Color (RGB)
    //putsUart0("\033]11;#E14141\007");                                                                                    // Background Color

    putsUart0("\r\n");
    putsUart0("Project : Driver Development \r\n");
    putsUart0("email   : \033[38;5;51;4maditya.mall@mavs.uta.edu\033[0m \r\n");
    putsUart0("Version : 1.0 \r\n");

    putsUart0("\r\n");
    putsUart0("\033[33;1m!! This Program requires Local Echo, please enable Local Echo from settings !!\033[0m \r\n");     // Foreground color:Yellow
    putsUart0("\r\n");

}

void clear_screen(void)
{
    putsUart0("\033[2J\033[H");                                      // ANSI VT100 escape sequence, clear screen and set cursor to home.
}



// Function of reseting the input buffers and variables
void reset_buffer(void)
{
    uint8_t i = 0;
    uint8_t len = 0;

    len = strlen(string);

    for (i = 0; i < len; i++)
    {
        string[i] = '\0';
    }

}

void command_line(void)
{
    char char_input = 0;
    int char_count  = 0;

    putsUart0("\r\n");
    putsUart0("\033[1;32m$>\033[0m");

    while(1)
    {
        char_input = uart_getchar(UART0);

        //putcUart0(char_input);                        // Enable hardware echo, Optional

        if (char_input == 13)
        {
            putsUart0("\r\n");
            string[char_count] = '\0';
            char_count = 0;
            break;
        }

        // Cursor processing
        if (char_input == 27)
        {
            char next_1 = getcUart0();
            char next_2 = getcUart0();

            if(next_1 == 91 && next_2 == 65)            // Up, don't process UP key press
            {
                putsUart0("\033[B");
                putsUart0("\033[D");
                char_input = '\0';
                continue;
            }
            else if(next_1 == 91 && next_2 == 66)       // Down, Don't process Down key press
            {
                putsUart0("\033[A");
                char_input = '\0';
                char_count = char_count - 1;
            }
            else if(next_1 == 91 && next_2 == 68)
            {


            }
        }

        // Backspace processing
        if (char_input == 8 || char_input == 127)
        {
            if(char_count <= 0)
            {
                putsUart0("\033[C");
                continue;
            }
            else
            {
                putcUart0(' ');
                putsUart0("\033[D");
                char_count--;
                continue;
            }
        }

        else
            string[char_count++] = char_input;

        // Check for max buffer size
        if (char_count == MAX_SIZE)
        {
            putsUart0("\r\n");
            putsUart0("\r\nCan't exceed more than 40 characters");    // Let the User know that character count has been exceeded
            putsUart0("\r\n");

            reset_buffer();                                           // Reset the buffer, call function

            *string = 0;

            break;
        }

    }

}


int main()
{

    int retval = 0;

    char message[40] = {0};

    char num[5];

    num[0] = 'a';
    num[1] = 'b';
    num[2] =  0 ;
    num[3] = 'd';
    num[4] = 'e';

    init_clock();

    init_gpio();

    gpio_write_pin(GPIOF, 1, ENABLE);

    clear_screen();

    project_info();

    while(1)
    {
        while(gpio_read_pin(GPIOF, 4));

        gpio_write_pin(GPIOF, 1, DISABLE);
        gpio_write_pin(GPIOF, 3, ENABLE);

        uart_read(UART0, message, 20);

        uart_putchar(UART0, '\r');
        uart_putchar(UART0, '\n');

        uart_write(UART0, message, 20);

        uart_putchar(UART0, '\r');
        uart_putchar(UART0, '\n');

        memset(message, 0, sizeof(message));

    }
}
