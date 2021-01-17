#include <mega32.h>
#include <stdio.h>
#include <delay.h>

// Alphanumeric LCD functions
#include <alcd.h>

// Declare your global variables here
int globalTimer = 0;    // Global timer with period of 8 ms
int second = 0;         // Seconds passed 

int currentFloor = 0;   // The current floor
int target = 0;         // This is the target floor
bit locked = 0;         // The Elevator is now locked
bit firstIter = 1;      // First Iter is for reseting the timer and servo position
bit DCMotors = 0;       // Controlling the DC motors
bit shouldMove = 0;     // This bit is for making sure we are capturing the inputs
bit doorOpen = 1;       // Checking door

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    // Place your code here 
    globalTimer++;
    if(globalTimer%125==0) {
        second++;
        globalTimer = 0;
    }
}

// Reset timer and globalTimer and second
void reset_timer()
{
    TCNT0 = 0x00;       // Set the counter to zero
    globalTimer = 0;    // 8ms counter is reset 
    second = 0;         // Resetting the second
    lcd_clear();
}

// For controlling the DC Motors
void DC_motor(int position)
{
    // The PWM is in range of 0 to 19999!
    OCR1B = (position) * 10 * 20; 
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

// Processing the input from buttons
void process_input()
{
    
    // First of all we have to check whether we are in locked mode or not
    if(!locked && !shouldMove) {
        int tempTarget = 255-PINB; // PINB is the input/target
        
        switch(tempTarget)
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
        
        if(target!=currentFloor) shouldMove = 1;
    }
}

// Processing the floor to be displayed in LCD and furthur processings
void process_floors()
{
    if(target>currentFloor) currentFloor++;
    else currentFloor--;
}

// Moving the servo motors
void process_servo_motors()
{
    if(doorOpen)    servo_position(0);
    else    servo_position(90);
}

// Moving the DC motors as well as controlling the speed
void process_dc_motors()
{
    // Difference of floors
    int difference = target - difference;
    // If negative, negate it
    if(difference < 0) difference*=-1; 
     
    // If DC motors should be moving start moving them
    if(DCMotors){
        // If we are in the first second, accelerate and if we are in the middle of the transportation
        // constant speed. Otherwise, decelerate
        
        if(second <= 1)
            DC_motor(80 * globalTimer * 10);
        else if(second == 4 && difference == 1) DC_motor(80 * (125-globalTimer) * 10);
        else DC_motor(80*125*10);
    }
}

// Writing to LCD
void LCD_controller(int level)
{
    char currFloor[16] = "";                    // What we are going to display
    char myMan[16] = "";
     
    sprintf(currFloor,"F: %d, %d, %d",level, globalTimer, second);       // Making the text ready for display
    lcd_gotoxy(0,0);                            //lcd_gotoxy ham k cursor ro b xy mibarad 
    lcd_puts(currFloor);                        // chap bar roye lcd
    
    sprintf(myMan,"CF: %d,T: %d",currentFloor, target);    // Making the text ready for display
    lcd_gotoxy(0,1);                            //lcd_gotoxy ham k cursor ro b xy mibarad 
    lcd_puts(myMan); 
}

// Well this is obvious
void main(void)
{
    // Declare your local variables here
    DDRA = 0xFF;
    DDRB = 0x00;
    DDRD = 0xFF;
    PORTB = 0xFF;

    // Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 31.250 kHz
// Mode: CTC top=OCR0
// OC0 output: Disconnected
// Timer Period: 8 ms
TCCR0=(0<<WGM00) | (0<<COM01) | (0<<COM00) | (1<<WGM01) | (1<<CS02) | (0<<CS01) | (0<<CS00);
TCNT0=0x00;
OCR0=0xF9;

    // Configure Timer1
    TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);     //NON Inverted PWM
    TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11);        //PRESCALER=8 MODE 14(FAST PWM)

    // ICR = 19,999 so fPWM = 8,000,000/ (19,999+1) / 8 = 50Hz
    ICR1H=0x4E;
    ICR1L=0x1F;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<OCIE0) | (1<<TOIE0);


    lcd_init(16);

    // Start the servo at rest
    /*
    servo_position(0);
    DC_motor(0);
    */

    // Global enable interrupts
    #asm("sei")

    while (1) {
        /*
        // Place your code here
        process_input();
        LCD_controller(currentFloor);
        process_dc_motors();
        process_servo_motors();

        if(target==currentFloor){}
        else {
            // Checking for first iteration to avoid malfunction
            if(firstIter) {
                doorOpen = 0;
                reset_timer();
                firstIter = 0;                
            }
        }
        */
        LCD_controller(currentFloor);
        
        if(second%2){
            servo_position(90);
            DC_motor(90);        
        }
        else {
            servo_position(0);
            DC_motor(0);        
        }
        
    }
}