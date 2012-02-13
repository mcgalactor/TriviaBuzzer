/*
TriavaBuzzer - allows 4 Players to use a buzzer which activates a LED identifing the player who pressed its buzzer first.

    Copyright (C) 2011,2012  Jan-Hendrik Hanne

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include  <msp430x20x2.h>
#include <string.h>
//#define     RED_LED                  BIT0
//#define     GRN_LED                  BIT6

#define     LED0                     BIT0
#define     LED1                     BIT1
#define     LED2                     BIT2
#define     LED3                     BIT3

#define     BUTTON0                  BIT4
#define     BUTTON1                  BIT5
#define     BUTTON2                  BIT6
#define     BUTTON3                  BIT7

#define     APP_STANDBY_MODE      0
#define     APP_APPLICATION_MODE  1

unsigned char applicationMode = APP_STANDBY_MODE;

int8_t pressed=0;

void testLEDS(void){
    P1DIR|=(LED1+LED2+LED3+LED0); ;
    P1OUT |= (LED1+LED2+LED3+LED0);
}

void initLEDS(void){
    P1DIR|=(LED1+LED2+LED3+LED0); ;
    P1OUT&= ~(LED1+LED2+LED3+LED0);
}


/* the mode the chip initially enters to blink the LEDs */
void PreApplicationMode(void) {
    /* Set both LED pins as output pins and turn red on, green off */
    /* The basic clock system control register 3 controls the oscillator
     * the auxilliary clock uses.  Change ACLK from external timer crystal
     * oscillator to internal very low-power oscillator (VLO) clock,
     * which runs at appoximately 12kHz.
     */
    BCSCTL3 |= LFXT1S_2;

    /* Basic clock system control register 1 controls the auxilliary clock
     * divider.  Set ACLK to use Divider 1, i.e. divided by 2^1. */
    BCSCTL1 |= DIVA_1;

    /* Set TimerA counter period to 1200 or about 1/5 second */
    
    TACCR0 = 5000;
    /* Set TimerA to use auxilliary clock TASSEL_1 and count up mode MC_1 */
    TACTL = TASSEL_1 | MC_0;
    TACCTL1 = CCIE;
    
 }

/* Timer A interrupt service routine 1.  The function prototype
 * tells the compiler that this will service the Timer A1 interrupt,
 * and then the function follows.
 */
void ta1_isr(void) __attribute__((interrupt(TIMERA1_VECTOR)));
void ta1_isr(void)
{
    /* clear the timer interrupt flag */
    TACCTL1 &= ~CCIFG;
    //stop timer
    TACTL = TASSEL_1 | MC_0;
    //enable button interupt
    
    //clear button interrupt flag
    P1IFG &= ~(BUTTON0+BUTTON1+BUTTON2+BUTTON3);
    P1IE |= (BUTTON0+BUTTON1+BUTTON2+BUTTON3);
    P1OUT&= ~(LED0+LED1+LED2+LED3);
}
/* *************************************************************
 * Port Interrupt for Button Press
 * 1. During standby mode: to exit and enter application mode
 * 2. During application mode: to recalibrate temp sensor
 * *********************************************************** */
void PORT1_ISR(void) __attribute__((interrupt(PORT1_VECTOR)));
void PORT1_ISR(void)
{
    //disable button interupt 
    
    if (P1IFG&BUTTON0){
        P1OUT^=LED0;
    }else if (P1IFG&BUTTON1){
        P1OUT^=LED1;
    }else if (P1IFG&BUTTON2){
        P1OUT^=LED2;
    }else if (P1IFG&BUTTON3){
        P1OUT^=LED3;
    }
    
    
    
    P1IFG &= ~(BUTTON0+BUTTON1+BUTTON2+BUTTON3);
    P1IE &= ~(BUTTON0+BUTTON1+BUTTON2+BUTTON3);
    
    
    
    TACTL = TASSEL_1 | MC_0;
    TACCR0 = 18000;
    TACTL = TASSEL_1 | MC_1;  
    
    }




/* This function configures the button so it will trigger interrupts
 * when pressed.  Those interrupts will be handled by PORT1_ISR() */
void InitializeButton(void) {
    /* Set button pin as an input pin */
    P1DIR &= ~BUTTON0;
    P1DIR &= ~BUTTON1;
    P1DIR &= ~BUTTON2;
    P1DIR &= ~BUTTON3;
    /* set output for button */
    P1OUT |= (BUTTON0+BUTTON1+BUTTON2+BUTTON3);
    
    /* enable pull up resistor for button to keep pin high until pressed */
    P1REN |=  (BUTTON0+BUTTON1+BUTTON2+BUTTON3);
    
    /* Interrupt should trigger from high (unpressed) to low (pressed) */
    P1IES |=  (BUTTON0+BUTTON1+BUTTON2+BUTTON3);
    /* Clear the interrupt flag for the button */
    P1IFG &= ~BUTTON0;
    P1IFG &= ~BUTTON1;
    P1IFG &= ~BUTTON2;
    P1IFG &= ~BUTTON3;
    /* Enable interrupts on port 1 for the button */
    P1IE |= (BUTTON0+BUTTON1+BUTTON2+BUTTON3);
    
  
}


/* This function catches watchdog timer interrupts, which are
 * set to happen 681ms after the user presses the button.  The
 * button has had time to "bounce" and we can turn the button
 * interrupts back on.
 */

void WDT_ISR(void) __attribute__((interrupt(WDT_VECTOR)));
void WDT_ISR(void)
{
    /* Disable interrupts on the watchdog timer */
    /*IE1 &= ~WDTIE;
    /* clear the interrupt flag for watchdog timer */
    /*IFG1 &= ~WDTIFG;
    /* resume holding the watchdog timer so it doesn't reset the chip */
    /*WDTCTL = WDTPW + WDTHOLD;
    /* and re-enable interrupts for the button */
    //P1IE |= BUTTON;
}


/************** main program begins here ***************/
int main( void ) {
    /* access the watchdog timer control without password and hold
     * the count (so the watchdog timer doesn't reset the chip)
     */
    
    WDTCTL = WDTPW + WDTHOLD;
    initLEDS();
    InitializeButton();
       
    PreApplicationMode();
    //testLEDS();
    /*
    P1OUT&=~(LED1+LED2+LED3+LED4+LED5+LED6);
    P1OUT|=LED1;
    P1OUT&=~GRN_LED;
    */
     __bis_SR_register(GIE);
     
     while (1){
     
     }
     return 1;
}   
