#include <mega32.h>
#include<delay.h>

void main()
{
  //FOR TIMER1
   TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        //NON Inverted PWM
   TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10); //PRESCALER=64 MODE 14(FAST PWM)
   ICR1H=0x13;  //fPWM=50Hz
   ICR1L=0x87;  //fPWM=50Hz 
   DDRD|=0xFF;   //PWM Pins as Output
   while(1)
   {
      OCR1A=316;  //90 degree
      delay_ms(5000);
      OCR1A=97;   //0 degree
      delay_ms(5000);
     OCR1A=535;  //180 degree
      delay_ms(5000);
   }
}