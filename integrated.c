#include <avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#define SET_BIT(PORT,PIN)  PORT|=(1<<PIN)
#define CLR_BIT(PORT,PIN)  PORT&=~(1<<PIN)
void initADC();
uint16_t ReadADC(uint8_t ch);
void initInterrupt();
void init_pwm();
void airbag();
void AEB();
volatile uint8_t obstacle_flag=0;
volatile uint8_t cc_flag=0;
uint16_t brake_pressure=0;
int main(void)
{
 CLR_BIT(DDRC,PC6);
 SET_BIT(PORTC,PC6);
 SET_BIT(DDRD,PD7);
 CLR_BIT(DDRD,PD6);
 SET_BIT(PORTD,PD6);
 SET_BIT(DDRB,PB0);
 CLR_BIT(DDRC,PC0);

 SET_BIT(PORTD,PD2);
 SET_BIT(DDRC,PC5);
 SET_BIT(DDRC,PC3);
 SET_BIT(DDRB,PB1);

 CLR_BIT(DDRD,PD2);
 CLR_BIT(DDRD,PD3);
 SET_BIT(PORTD,PD3);
 CLR_BIT(DDRC,PC1);
 CLR_BIT(DDRC,PC4);
 uint8_t read_switch;
 uint16_t colli_pressure_value;

 colli_pressure_value=0x00;
 initInterrupt();
 initADC();
 init_pwm();
 read_switch=0x00;
    while(1)
    {
        colli_pressure_value = ReadADC(0);
        _delay_ms(100);
        brake_pressure=ReadADC(1);
         _delay_ms(100);
        read_switch=PIND;

        if(!(PINC&(1<<PC6)))
        {
        if(!(read_switch&(1<<PD6)))
        {
        if(obstacle_flag)
        {
            AEB();
            if(colli_pressure_value>500)
            {
                airbag();
                break;
            }
        }
         if(cc_flag && (brake_pressure<=50) )
         {
             OCR1A=128;
         }
         else
         {
            OCR1A=ReadADC(4);
             _delay_ms(100);
         }
    }
        }
        }
    return 0;
}
void airbag()
{

    SET_BIT(PORTD,PD7);
    SET_BIT(PORTB,PB0);
    _delay_ms(500);


}
void AEB()
{
    SET_BIT(PORTC,PC5);

    if(brake_pressure<500)
    {
        SET_BIT(PORTC,PC3);
        _delay_ms(2000);
    }
    else
    {
        CLR_BIT(PORTC,PC3);
        _delay_ms(1000);
        CLR_BIT(PORTC,PC5);
    }
    obstacle_flag=0;
    CLR_BIT(PORTC,PC5);
    CLR_BIT(PORTC,PC3);
}
void initADC()
{
    ADMUX=(1<<REFS0);
    ADCSRA=(1<<ADEN)|(7<<ADPS0);
}
uint16_t ReadADC(uint8_t ch)
{
    ADMUX&=0xf8;
    ch=ch&0b00000111;
    ADMUX|=ch;

    ADCSRA|=(1<<ADSC);
    while(!(ADCSRA&(1<<ADIF)));
    ADCSRA|=(1<<ADIF);
    uint16_t TEMP=0;
    TEMP|=(ADCH<<8);
    TEMP|=(ADCL);
    return(TEMP);
}
void init_pwm()
{
    TCCR1A|=((1<<COM1A1)|(1<<WGM11)|(1<<WGM10));
    TCCR1B|=((1<<WGM12)|(1<<CS01)|(1<<CS00));
    TCNT1=0x00;
}
void initInterrupt()
{
    EICRA|=(1<<ISC00);
    EIMSK|=(1<<INT0);
    EICRA|=(1<<ISC10);
    EIMSK|=(1<<INT1);
    sei();
}
ISR(INT0_vect)
{
     if(!(PIND&(1<<2)))
       obstacle_flag=1;
   else
       obstacle_flag=0;
}
ISR(INT1_vect)
{
    if(!(PIND&(1<<3)))
       cc_flag=1;
   else
       cc_flag=0;
}
