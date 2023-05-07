#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

#include "qutyserial.h" 

/***
 * Tutorial 08: Introduction
 *
 * In this week's tutorial you will be working with the TCA0 and
 * ADC0 peripherals. You will use these to control the brightness
 * of the 7-segment display, produce a tone from the buzzer and
 * read the position of the potentiometer.
 * 
 * Interfacing with these hardware devices on the QUTy is a core
 * requirement for Assessment 2. 
 */



/***
 * Ex 8.0
 * 
 * Write a function named "pwm_init" below which will configure
 * TCA0 to produce a 3X0 Hz, 1Y % duty cycle PWM output on net DISP EN, 
 * Where XY is the last two digits of your student number. Your 
 * function should take no arguments and return no values.
 * 
 * Example: If your student number were n12345678 you should configure 
 *          TCA0 to produce a 370 Hz, 18 % duty cycle output.
 */
void pwm_init()
{
    PORTB.OUTSET = PIN1_bm; // DISP EN disabled
    PORTB.DIRSET = PIN1_bm; // DISP EN as output
    TCA0.SPLIT.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc; // No prescaler
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc ; // Set timer to single slope mode
    /* Set timer to compare CMP1, 
    Make waveform output match corresponding pin (CMPx).
    DISP EN connected to W01, enable compare for CMP1
    */
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP1EN_bm;
    // TCA0.SINGLE.CMP0 = TCA_SINGLE_CMP1EN_bm;
    /* Set the frequency (350 Hz)
    f_pwm = (1 / TOP * T_clk, timer)
    350 = (3.3333\times10^{-6} / TOP)
    TOP = 9009; ?
    */
   TCA0.SINGLE.PER = 9794;
   /* Set the duty cycle
        11% duty cycle
            CMPx = TCA0.SINGLE.PER * (duty cycle / 100);
            CMP1 = VAR //
   */
  TCA0.SINGLE.CMP1 = 1522;
  // Set up the timer to operate with compare to CMP0
  PORTB.DIRSET = PIN0_bm;
  TCA0.SINGLE.CMP0 = 4761;
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // Enable the timer, do this last when configuring one
}

// Write your code for Ex 8.0 above this line.



/***
 * Ex 8.1
 * 
 * Write a function named "adc_init" below which will configure
 * ADC0 in 8-bit single conversion mode. The ADC should be 
 * configured to sample the voltage on the POT net, be put
 * in free-running mode, and started.
 */
void adc_init()
{
    ADC0.CTRLA = ADC_ENABLE_bm; // Enables the ADC
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;
    ADC0.CTRLC = ADC_TIMEBASE4_bm | ADC_REFSEL_VDD_gc;
    ADC0.CTRLE = 64; // Sample duration
    ADC0.CTRLF = ADC_FREERUN_bm; // Sets the ADC xto free running mode
    ADC0.CTRLF |= ADC_LEFTADJ_bm; // Left adjusts the contents of the result
    /* Sets the ADC to read from the pin connected to the potentiometer */
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;
    ADC0.COMMAND |= ADC_MODE_SINGLE_8BIT_gc; // Set resolution of the result
    ADC0.COMMAND |= ADC_START_IMMEDIATE_gc; // Start the ADC
}   

// Write your code for Ex 8.1 above this line.



int main(void) {

    uint8_t result;

    serial_init();
    PORTA.DIRSET = PIN1_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
    _delay_ms(3000); // 3 second delay

    /***
     * Ex 8.2
     * 
     * Call the two initialisation functions you have written above
     * such that the TCA0 and ADC0 are enabled and operational.
     * 
     * On completion of this exercise the 7-segment display should show
     * an 8 on the RHS that is initially bright, but dims after 3 seconds,
     * and you should be able to test your ADC configuration via the serial
     * terminal.
     */

    pwm_init();
    adc_init();
    // Write your code for Ex 8.2 above this line.

    printf("Turn the potentiometer R1 fully counter-clockwise, then press S4.\n");
    while (VPORTA.IN & PIN7_bm); result = ADC0.RESULT; printf("Ex 8.2.0: result = 0x%02X, expected = 0x00\n", result);
    _delay_ms(1000); printf("Turn the potentiometer R1 fully clockwise, then press S4.\n");
    while (VPORTA.IN & PIN7_bm); result = ADC0.RESULT; printf("Ex 8.2.1: result = 0x%02X, expected = 0xFF\n", result);
    _delay_ms(1000); printf("Turn the potentiometer R1 to the half-way position, then press S4.\n");
    while (VPORTA.IN & PIN7_bm); result = ADC0.RESULT; printf("Ex 8.2.2: result = 0x%02X, expected = ~0x80\n", result);



    // main loop    
    while(1) {

        /***
         * Ex 8.3
         * 
         * Write code below such that the position of the potentiometer controls
         * the brightness of the 7-segment display. The display should be at 
         * maximum brightness when the potentiometer is turned fully counter-
         * clockwise, and zero brightness (i.e. off) when the pot is turned fully
         * clockwise.
         */


        // Write your code for Ex 8.3 above this line.

        result = ADC0.RESULT;
        // Translate 8 bit value from within result
        // RESULT will be between 0 and 255 (potentiometer 0% to 100% on);
        // CMP1 wll need to be between 0 and 9514
        TCA0.SINGLE.CMP1BUF = (255 - result) * 9523.71428 / 255;
        /***
         * Ex 8.4
         * 
         * Write code below such that the buzzer produces a 3X0 Hz tone
         * when the potentiometer position is between 87.5% and 100% (where
         * 100% is fully CW and 0% is fully CCW). When the pot position is
         * outside of this range the buzzer should be silent. The buzzer should
         * be driven with a 50 % duty cycle square wave.
         * 
         * TIP: You will probably also want to modify the code you wrote in 
         *      pwm_init() to achieve this functionality.
         * 
         * TIP: Note that the frequency specified above is the same as for Ex 8.0. 
         */

        if (result > 223) // 223  (87.5% of 255)
        {
            TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0_bm;    // Enable PWM on pin connected to 
        } else 
        {
            TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP0_bm; // Disable PWM 
        }
        // Write your code for Ex 8.4 above this line.
        

    } // end main loop

} // end main()