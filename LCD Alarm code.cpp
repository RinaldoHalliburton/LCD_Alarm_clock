/*
 * Prototype code for clock.cpp
 *
 * Created: 20/10/2023 9:43:17 pm
 * Author : kyngt
 */
#define ENABLE PB0
#define REG_SEL PB1
#define READ_WRITE PB2
#define BUZZER PB3
#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
// #include <string.h>
#include <avr/interrupt.h>

volatile unsigned char seconds = 0, minutes = 0, hours = 12;       // Time variables
volatile unsigned char A_seconds = 0, A_minutes = 0, A_hours = 12; // Alarm variables
volatile bool meridian = true;                                     // true = AM, false = PM
volatile bool A_meridian = true;                                   // Meridian for alarm
volatile bool is_24 = false;                                       // bool for format of clock
volatile bool set = true;                                          // bool to set clock
volatile bool alarm = false;                                       // bool for alarm
volatile bool var = false;                                         // variable to turn of alarm display
volatile bool buzz = false;                                        // variable for buzzer
volatile bool alarm_enabled = false;                               // variable for buzzer
char type[][15] = {"Clock-", "Alarm-OFF     "};                    // Display Strings

void lcd_cmd(unsigned char cmd) // function to pass a command to the LCD in 4 bit mode
{

    PORTB = (cmd & 0xF0); // First nibble of command
    PORTB |= (1 << ENABLE);
    PORTB &= ~((1 << READ_WRITE));
    PORTB &= ~(1 << REG_SEL);
    _delay_ms(2);
    PORTB &= ~(1 << ENABLE);
    _delay_ms(3);
    PORTB = ((cmd << 4) & 0xF0); // Second nibble of command
    PORTB |= (1 << ENABLE);
    PORTB &= ~((1 << READ_WRITE));
    PORTB &= ~(1 << REG_SEL);
    _delay_ms(2);
    PORTB &= ~(1 << ENABLE);
    _delay_ms(3);
}

void lcd_data(unsigned char data) // function to pass a display data to the LCD in 4 bit mode
{
    PORTB = (data & 0xF0); // First nibble of data
    PORTB |= (1 << ENABLE);
    PORTB &= ~(1 << READ_WRITE);
    PORTB |= (1 << REG_SEL);
    _delay_ms(2);
    PORTB &= ~(1 << ENABLE);
    _delay_ms(1);
    PORTB = ((data << 4) & 0xF0); // Second nibble of data
    PORTB |= (1 << ENABLE);
    PORTB &= ~((1 << READ_WRITE));
    PORTB |= (1 << REG_SEL);
    _delay_ms(2);
    PORTB &= ~(1 << ENABLE);
    _delay_ms(1);
}

void string_out(char *str) // Function to display a string on the LCD
{
    unsigned char i = 0;
    while (str[i] != 0)
    {
        lcd_data(str[i]);
        i++;
    }
}

void initialize() // Initialize cursor and display for 4 bit mode
{
    _delay_ms(50);
    lcd_cmd(0x33);
    lcd_cmd(0x32);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x01);
    _delay_ms(2);
    lcd_cmd(0x06);

    string_out(type[0]); // display CLOCK STRING
    lcd_cmd(0xC0);       // Set cursor to new location
    string_out(type[1]); // display ALARM STRING

    MCUCR |= (1 << ISC11 | 1 << ISC01); // enable button interrupts on falling edge
    GIMSK |= (1 << INT0 | 1 << INT1);   // enable button interrupts
}
void start_timer() // start timer for clock by enabling timer interrupt
{
    TCCR1B = (1 << CS12 | 1 << CS10 | 1 << WGM12);
    OCR1A = 7812;
    TIMSK = (1 << OCIE1A);
}

void set_sec(unsigned char sec, unsigned char A_sec) // convert seconds variable to time and display it
{
    int sec_1 = sec / 10;  // to get the first digit of second
    int sec_2 = sec % 10;  // to get the second digit of second
    lcd_cmd(0x8C);         // set cursor location
    lcd_data(sec_1 + '0'); // convert seconds variable of Time to ASCII
    lcd_cmd(0x8D);         // set cursor location
    lcd_data(sec_2 + '0'); // convert seconds variable of Time to ASCII
    if (alarm == true)
    {
        int A_sec_1 = A_sec / 10;
        int A_sec_2 = A_sec % 10;
        lcd_cmd(0xCC);           // set cursor location
        lcd_data(A_sec_1 + '0'); // convert seconds variable of Alarm to ASCII
        lcd_cmd(0xCD);           // set cursor location
        lcd_data(A_sec_2 + '0'); // convert seconds variable of Alarm to ASCII
    }
}
void set_min(unsigned char min, unsigned char A_min) // convert minutes variable to time and display it
{
    int min_1 = min / 10;  // to get the first digit of minute
    int min_2 = min % 10;  // to get the second digit of minute
    lcd_cmd(0x89);         // set cursor location
    lcd_data(min_1 + '0'); // convert minutes variable of Time to ASCII
    lcd_cmd(0x8A);         // set cursor location
    lcd_data(min_2 + '0'); // convert minutes variable of Time to ASCII
    if (alarm == true)
    {
        int A_min_1 = A_min / 10;
        int A_min_2 = A_min % 10;
        lcd_cmd(0xC9);
        lcd_data(A_min_1 + '0');
        lcd_cmd(0xCA);
        lcd_data(A_min_2 + '0');
    }
}
void set_hr(unsigned char hr, unsigned char A_hr) // convert hours variable to time and display it
{
    int hr_1 = hr / 10;
    int hr_2 = hr % 10;
    lcd_cmd(0x86);
    lcd_data(hr_1 + '0');
    lcd_cmd(0x87);
    lcd_data(hr_2 + '0');
    if (alarm == true)
    {
        int A_hr_1 = A_hr / 10;
        int A_hr_2 = A_hr % 10;
        lcd_cmd(0xC6);
        lcd_data(A_hr_1 + '0');
        lcd_cmd(0xC7);
        lcd_data(A_hr_2 + '0');
    }
}

void seprator() // display colon for time separator
{
    lcd_cmd(0x88); // set cursor location
    string_out(":");
    lcd_cmd(0x8B); // set cursor location
    string_out(":");
    if (alarm == true)
    {
        lcd_cmd(0xC8); // set cursor location
        string_out(":");
        lcd_cmd(0xCB);
        string_out(":");
    }
}

void set_meridian() // display AM or PM or disable if it's 24 hour
{
    lcd_cmd(0x8E); // set cursor location
    if (is_24 == false)
    {

        if (meridian == true)
        {
            string_out("AM");
        }
        if (meridian == false)
        {
            string_out("PM");
        }
    }

    if (is_24 == true)
    {
        string_out("  ");
    }

    lcd_cmd(0xCE); // set cursor location
    if (is_24 == false)
    {
        if (A_meridian == true)
        {
            string_out("AM");
        }
        if (A_meridian == false)
        {
            string_out("PM");
        }
    }
    if (is_24 == true)
    {
        string_out("  ");
    } // turn AM or PM off if clock is in 24 hour mode
}

void updatedis() // function to refresh display
{
    set_sec(seconds, A_seconds);
    set_min(minutes, A_minutes);
    set_hr(hours, A_hours);
    set_meridian();
    seprator();
}

void set_format() // function to change between 12 and 24 hour format
{
    if (is_24 == true)
    {
        if (hours > 12)
        {
            hours = hours - 12;
            meridian = false;
        }
        if (hours == 0)
        {
            hours = 12;
            meridian = true;
        }
        if (A_hours > 12)
        {
            A_hours = A_hours - 12;
            A_meridian = false;
        }
        if (A_hours == 0)
        {
            A_hours = 12;
            A_meridian = true;
        }
    }

    if (is_24 == false)
    {

        if (A_hours < 12 && A_meridian == false)
        {
            A_hours = A_hours + 12;
        }
        if (A_hours == 12 && A_meridian == true)
        {
            A_hours = 0;
        }

        if (hours < 12 && meridian == false)
        {
            hours = hours + 12;
        }
        if (hours == 12 && meridian == true)
        {
            hours = 0;
        }
    }
    is_24 = !is_24;
}

void set_alarm() // function to allow setting of alarm
{
    if (alarm == true)
    {
        alarm_enabled = true;
    }
    if (!(PIND & (1 << PD4)) && alarm == true)
    {
        A_seconds++;
        if (A_seconds >= 60)
        {
            A_seconds = 0;
        }
    }
    if (!(PIND & (1 << PD5)) && alarm == true)
    {
        A_minutes++;
        if (A_minutes >= 60)
        {
            A_minutes = 0;
        }
    }
    if (!(PIND & (1 << PD6)) && alarm == true)
    {
        A_hours++;
        if (A_hours == 13 && is_24 == false)
        {
            A_hours = 1;
        }
        if (A_hours >= 24 && is_24 == true)
        {
            A_hours = 0;
        }
    }
    if (!(PIND & (1 << PD0)) && alarm == true)
    {
        A_meridian = !A_meridian;
    }
}

void set_time() // function that allows to set time
{
    if (set == false) // start timer if set variable is false
    {
        start_timer();
    }

    if (!(PIND & (1 << PD4)) && set == true)
    {
        seconds++;
        if (seconds >= 60)
        {
            seconds = 0;
        }
    }
    if (!(PIND & (1 << PD5)) && set == true)
    {
        minutes++;
        if (minutes >= 60)
        {
            minutes = 0;
        }
    }
    if (!(PIND & (1 << PD6)) && set == true)
    {
        hours++;
        if (hours == 13 && is_24 == false)
        {
            hours = 1;
        }
        if (hours >= 24 && is_24 == true)
        {
            hours = 0;
            meridian = true;
        }
    }
    if (!(PIND & (1 << PD0)) && set == true)
    {
        meridian = !meridian;
    }
}

int main(void) // main function
{
    DDRB = 0xFF;                                                            // PORT B, DATA OR CMD PORT AS OUTPUT
    PORTB = 0x00;                                                           // DISBALE PULL UP
    DDRD = 0x00;                                                            // BUTTON INTERRUPTS AS INPUTS
    PORTD = (1 << 2 | 1 << 3 | 1 << 0 | 1 << 1 | 1 << 4 | 1 << 5 | 1 << 6); // ENABLE Pull up resistors

    sei();
    initialize();

    while (1)
    {
        if ((A_hours == hours) && (A_minutes == minutes) && (A_seconds == seconds) && (A_meridian == meridian) && (alarm == false) && (alarm_enabled == true)) // turn of buzzer when clock matches with alarm
        {
            buzz = true;
        }

        if (buzz == true) // turn buzzer on
        {
            PORTB |= (1 << BUZZER);
            _delay_ms(100);
        }
        if (var == true) // reset display after alarm is disabled
        {
            lcd_cmd(0xC0); // set cursor location
            string_out(type[1]);
            A_hours = 12;
            if (is_24 == true)
            {
                A_hours = 0;
            }
            A_minutes = 0;
            A_seconds = 0;
            // A_meridian = false;
            var = false;
            alarm_enabled = false;
        }
        if (!(PIND & (1 << PD1))) // polling interrupt to set format
        {
            set_format();
        }

        set_time();  // allow time setting
        set_alarm(); // allow alarm setting
        updatedis(); // update display
    }
}

ISR(TIMER1_COMPA_vect) // overflow timer interrupt
{
    seconds++;

    if (seconds == 60)
    {
        seconds = 0;
        minutes++;
    }
    if (minutes == 60)
    {
        minutes = 0;
        hours++;
    }
    if (hours == 12 && seconds == 0 && minutes == 0)
    {
        meridian = !(meridian);
    }

    if (hours >= 13 && is_24 == false)
    {
        hours = 1;
    }
    if (hours >= 24)
    {
        hours = 0;
        // meridian=!meridian;
        meridian = true;
    }
}

ISR(INT0_vect) /*ISR for alarm*/
{
    _delay_ms(3);
    alarm = !alarm;
    if (buzz == true) // disable buzzer when it goes off
    {
        buzz = false;
        alarm = false;
        var = true;
    }
}

ISR(INT1_vect) /* ISR to set time*/
{
    _delay_ms(3);
    TCCR1B = 0;
    set = !set;
}
