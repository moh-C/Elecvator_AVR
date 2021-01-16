#include <mega32.h>
#include <delay.h>
#include <stdio.h>

// Declare your global variables here
int globalTimer = 0;    // Global timer with period of 8 ms
int second = 0;         // Seconds passed 

int currentFloor = 0;   // The current floor
int target = 0;         // This is the target floor
bit locked = 0;         // The Elevator is now locked
bit firstIter = 1;      // First Iter is for reseting the timer and servo position

// Alphanumeric LCD functions
#include <alcd.h>

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    // Reinitialize Timer 0 value
    TCNT0=0x06;
    
    globalTimer++;          // After an interval of 8 ms, globalTimer is incremented 
    if(globalTimer==125) second++;  // 125 * 8 ms = 1s so we are keeping account of the seconds
}

// Reseting the main timer to avoid malfunction
void reset_timer()
{
    TCNT0 = 0x06;       // Set the counter to zero
    globalTimer = 0;    // 8ms counter is reset 
    second = 0;         // Resetting the second
}

// Writing to LCD
void LCD_controller(int level)
{
    char currFloor[16] = "";                    // What we are going to display 
    sprintf(currFloor,"Floor: %d",level);       // Making the text ready for display
    lcd_gotoxy(0,0);                            //lcd_gotoxy ham k cursor ro b xy mibarad 
    lcd_puts(currFloor);                        // chap bar roye lcd
}

// For controlling the DC Motors
void DC_motor(int position)
{
    // The PWM is in range of 0 to 19999!
    OCR1B = 1499 + position * 50/9; 
}

// Given a degree, the servo with move to the desired degree/step.
// For example 0 will result in 0 degrees and 90 is result in 90 degrees 
void servo_position(int position)
{
    // Servo has to move in range of 0 to 90 degrees thus according to datasheet,
    // a pulse with a width of 1.5ms will result in 0 degree while a pulse with a
    // width of 2 ms will result in 90 degrees!   
    OCR1A = 1499 + position * 50/9; 
}

void process_input()
{
    // First of all we have to check whether we are in locked mode or not
    if(!locked) {
        target = 255-PINB; // PINB is the input/target
        
        switch(target)
        {
            case 1:
                target = 0;
                break;
            case 2:
                target = 1;
                break;
            case 4:
                target = 2;
                break;
            case 8:
                target = 3;
                break;
            case 16:
                target = 4;
                break;    
        }
    }
}

void main(void)
{

    // PORTB is input port
    DDRB=0x00;
    PORTB=0xFF;

    // PORTD is output for PWM
    DDRD=0xFF;

    // Configure Timer1
    TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);     //NON Inverted PWM
    TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11);        //PRESCALER=8 MODE 14(FAST PWM)

    // ICR = 19,999 so fPWM = 8,000,000/ (19,999+1) / 8 = 50Hz
    ICR1H=0x4E;
    ICR1L=0x1F;
    
    // Configure Timer0
    // Clock value: 31.250 kHz
    // Timer Period: 8 ms
    TCCR0=(1<<CS02);
    TCNT0=0x06;
    OCR0=0x00;

    // Alphanumeric LCD initialization
    // Characters/line: 16
    lcd_init(16);


    // Global enable interrupts
    #asm("sei")

    while (1)
    {
        // Place your code here
        
        process_input();
        LCD_controller(target);
        
        if(target==currentFloor){} // We could display a message here
        else {
            // Checking for first iteration to avoid malfunction
            if(firstIter) {
                reset_timer();
                firstIter = 0;
                servo_position(90);
            }
            if(second==1) {
                // Start the DC Motor
                locked = 1;            
            }
                                        
        }                
    }
}