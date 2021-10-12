// Morse Code Machine Translator
// School project for ECE 198 course.

// Written by Allyn Bao, October 2021 
// Based on template provided by Bernie Roehl, August 2021

#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions

#include "ece198.h"

// functions
uint32_t time_difference( uint32_t x, uint32_t y ) { 
    // calculate the difference between two uint32_t (time)
    if (x >= y) { return x - y; }
    else { return y - x; }
    }


int power( int base, int power ) {
    if ( power == 0 ) { return 1; }
    int result = base;
    for ( unsigned int i=2; i <= power; i++ ) {
        result *= base;
    }
    return result;
}


void reset_intervals_array( int intervals[], const unsigned int MAX_INTERVALS ) {
    for (unsigned int i=0; i<MAX_INTERVALS; i++ ) {
        intervals[i] = 0;
    }
}


char check_match( int intervals[], int cur_interval, const char CHARACTORS[], const int MORSE_CODE_INT[], const unsigned int NUM_CHARS) {
    
    int code_int = 0;
    for ( int i=0; i <cur_interval ; i++ ) {
        code_int += intervals[i] * power(10, (cur_interval-1) - i);
    }
    // debug print(code_int)
    /*
    char buff[5];
    sprintf(buff, " %d ", code_int);
    SerialPuts(buff);
    */
    
    for ( unsigned int j=0; j < NUM_CHARS; j++ ) {
        if ( code_int == MORSE_CODE_INT[j] ) {
            return CHARACTORS[j];

            // debug print(MORSE_CODE_INT[j])
            char buff[1];
            sprintf(buff, " %s ", CHARACTORS[j]);
            SerialPuts(buff);
        }
    }
    SerialPutc('*');
    return ' ';
}


int main(void) {
    HAL_Init(); // initialize the Hardware Abstraction Layer

    // Peripherals (including GPIOs) are disabled by default to save power, so we
    // use the Reset and Clock Control registers to enable the GPIO peripherals that we're using.

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable port B (for the rotary encoder inputs, for example)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable port C (for the on-board blue pushbutton, for example)

    // initialize the pins to be input, output, alternate function, etc...

    InitializePin(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // on-board LED

    // note: the on-board pushbutton is fine with the default values (no internal pull-up resistor
    // is required, since there's one on the board)

    // set up for serial communication to the host computer
    // (anything we write to the serial port will appear in the terminal (i.e. serial monitor) in VSCode)

    SerialSetup(9600);

    // as mentioned above, only one of the following code sections will be used
    // (depending on which of the #define statements at the top of this file has been uncommented)


    // Morse code
    const unsigned int NUM_CHARS = 36;
    const char CHARACTORS[36] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
    // short => 1, long => 2; Ex) A => ._ => 12
    const int MORSE_CODE_INT[36] = {12, 2111, 2121, 211, 1, 1121, 221, 1111,
                                    11, 1222, 212, 1211, 22, 21, 222, 1221,
                                    2212, 121, 111, 2, 112, 1112, 122, 2112,
                                    2122, 2211, 12222, 11222, 11122, 11112, 11111, 21111, 22111, 22211, 22221, 22222};

    // LED lights when the button is pressed, and turn off when LED is off

    // time counter
    const uint32_t short_press = 250;
    const uint32_t long_press = 800;
    const uint32_t short_gap = 500;
    const uint32_t long_gap = 1500;

    // const unsigned int correct_pattern_length = 5;
    // int correct_pattern[correct_pattern_length];

    const int MAX_INTERVALS = 5;
    bool exceed_limit = false;
    int intervals[MAX_INTERVALS]; // value 1 == short press; value 2 == long press;
    unsigned int cur_interval = 0;

    char cur_char;

    uint32_t cur_time;

    uint32_t time_button_pressed;
    uint32_t time_button_released;
    uint32_t cur_button_down_length;

    uint32_t cur_press_to_short_press;
    uint32_t cur_press_to_long_press;

    uint32_t time_gap_start = HAL_GetTick();;
    uint32_t time_gap_end;
    uint32_t cur_gap_length;

    uint32_t cur_gap_to_short_gap;
    uint32_t cur_gap_to_long_gap;

    // uint32_t correct_prompt_start;
    // uint32_t correct_prompt_length = 5000;

    // loop
    while (true) {
        // wait for button press
        while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {
            cur_time = HAL_GetTick();
            if (exceed_limit || (cur_interval != 0 && cur_time - time_gap_start > long_gap)) {
                cur_char = check_match(intervals, cur_interval, CHARACTORS, MORSE_CODE_INT, NUM_CHARS);
                // debug char
                SerialPutc(' ');
                SerialPutc(cur_char);
                SerialPutc(' ');

                // reset intervals[] all elements to 0
                cur_interval = 0;
                reset_intervals_array(intervals, MAX_INTERVALS);
                exceed_limit = false;
            }
        }
        // record time button is pressed
        time_button_pressed = HAL_GetTick();

        // record time when gap ends
        time_gap_end = time_button_pressed;

        // wait for button to release
        while (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, true);   // turn on LED
        }
        // record time button is released
        time_button_released = HAL_GetTick();

        // record time when gap starts
        time_gap_start = time_button_released;

        // calculated the length of time the button was pressed in current interval
        cur_button_down_length = time_button_released - time_button_pressed;

        // check if the button pressed was a short press or long press
        // if the length of time is closing to a short press OR length of time is less than a short press
        if (cur_button_down_length <= short_press) {
            intervals[cur_interval] = 1; // short press
            SerialPutc('.');
        } else {
            intervals[cur_interval] = 2; // long press
            SerialPutc('_');
        }
        cur_interval++;

        // check if interval exceeds limit
        if (cur_interval >= MAX_INTERVALS) { exceed_limit = true; }

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, false);  // turn off LED
    }

    return 0;
}
// This function is called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
}
