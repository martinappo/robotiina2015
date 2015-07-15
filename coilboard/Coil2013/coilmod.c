/*
 * coilmod.c
 * 
 * eeprom
 * 0 - id
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "usb_serial.h"

#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

#define F_CPU 16000000UL

//pin definitions
#define PWM PB5
#define CHARGE PB6
#define KICK PB7
#define DONE PB4
#define LED1 PC2
#define LED2 PC4
#define LED3 PC7
#define LED4 PD4
#define BALL PB0

int atoi(const char * str);

char response[16];
uint8_t fail_counter = 0;
uint8_t failsafe = 1;
uint8_t autocharge = 1;
uint8_t inTribbler = 0;

ISR(TIMER0_COMPA_vect) {
	fail_counter++;
	
}
ISR(TIMER1_COMPA_vect) {
	//ir ball detector
	uint8_t signalReceived = bit_get(PINB, BIT(BALL));		//PB0
	if (!(signalReceived & 0b00000001)) {
		inTribbler = 0;
	}
	else{
		inTribbler = 1;
	}
}


ISR(PCINT0_vect) {
	//PB4 - DONE
	uint8_t done = bit_get(PINB, BIT(4));
	if (!(done & 0b00010000)) {
		bit_clear(PORTC, BIT(LED1)); //CHARGE LED OFF
		bit_clear(PORTB, BIT(CHARGE)); //CHARGE OFF
	}
}

void my_delay_us(int n) {
	while(n--) {
		_delay_us(50);
	}
}

void usb_write(const char *str) {
	while (*str) {
		usb_serial_putchar(*str);
		str++;
	}
}

uint8_t recv_str(char *buf, uint8_t size) {
	char data;
	uint8_t count = 0;
	
	while (count < size) {
		data = usb_serial_getchar();
		if (data == '\r' || data == '\n') {
			*buf = '\0';
			return size;
		}
		if (data >= ' ' && data <= '~') {
			*buf++ = data;
			count++;
		}
	}
	return count;
}

void discharge() {
	//graceful discharge
	bit_clear(PORTB, BIT(CHARGE)); //CHARGE OFF
	for(int i = 0; i < 300; i++) {
		bit_clear(PORTC, BIT(LED2)); //KICK LED ON
		bit_set(PORTB, BIT(KICK)); //KICK ON
		_delay_ms(3);
		bit_clear(PORTB, BIT(KICK)); //KICK OFF
		bit_clear(PORTC, BIT(LED2)); //KICK LED OFF
		_delay_ms(50);
	}
	bit_clear(PORTB, BIT(KICK)); //KICK OFF
	sprintf(response, "%s\n", "discharged");
	usb_write(response);
}

void parse_and_execute_command(char *buf) {
	char *command;
	int16_t par1;
	command = buf;
	if (command[0] == 'c') {
		//charge
		bit_set(PORTC, BIT(LED1)); //CHARGE LED ON
		bit_set(PORTC, BIT(LED2)); //KICK LED OFF
		bit_clear(PORTB, BIT(KICK)); //KICK OFF
		bit_set(PORTB, BIT(CHARGE)); //CHARGE ON
	} else if (command[0] == 'k') {
		//kick
		bit_set(PORTC, BIT(LED2)); //KICK LED ON
		bit_clear(PORTC, BIT(LED1)); //CHARGE LED OFF
		par1 = atoi(command + 1);
		bit_clear(PORTB, BIT(CHARGE)); //CHARGE OFF
		bit_set(PORTB, BIT(KICK)); //KICK ON
		my_delay_us(par1);
		bit_clear(PORTB, BIT(KICK)); //KICK OFF
		bit_clear(PORTC, BIT(LED2)); //KICK LED OFF
		//autocharge
		if (autocharge) {
			bit_set(PORTC, BIT(LED1)); //CHARGE LED ON
			bit_set(PORTB, BIT(CHARGE)); //CHARGE ON
		}		
	} else if (command[0] == 'e') {
		//end
		bit_clear(PORTC, BIT(LED1)); //CHARGE LED OFF
		bit_clear(PORTC, BIT(LED2)); //KICK LED OFF
		bit_clear(PORTB, BIT(CHARGE)); //CHARGE OFF
		bit_clear(PORTB, BIT(KICK)); //KICK OFF
	} else if (command[0] == 'd') {
		//graceful discharge
		discharge();
	} else if ((command[0] == 'i') && (command[1] == 'd')) {
		//set id
		par1 = atoi(command+2);
		eeprom_update_byte((uint8_t*)0, par1);
	} else if (command[0] == '?') {
		//get info: id
		bit_flip(PORTC, BIT(LED3));
		par1 = eeprom_read_byte((uint8_t*)0);
		sprintf(response, "<id:%d>\n", par1);
		usb_write(response);
	} else if (command[0] == 'p') {
		//ping
		bit_flip(PORTD, BIT(LED4));
		fail_counter = 0;
	} else if ((command[0] == 'a') && (command[1] == 'c')) {
		//toggle autocharge
		autocharge = par1;
	} else if ((command[0] == 'f') && (command[1] == 's')) {
		//toggle failsafe
		par1 = atoi(command+2);
		failsafe = par1;
	} else if(command[0] == 'm'){
		if(command[1] == '0'){
			bit_clear(PORTB, BIT(PWM));
		}
		else if(command[1] == '1'){
			bit_set(PORTB, BIT(PWM));
		}
		//bit_flip(PORTB, BIT(PWM));
	} else if(command[0] == 'b'){
		if (inTribbler == 1) {
			sprintf(response, "%s\n", "true");
			usb_write(response);
		}
		else{
			sprintf(response, "%s\n", "false");
			usb_write(response);
		}
	}
	else {
		sprintf(response, "%s\n", command);
		usb_write(response);
	}

}

int main(void) {
	CLKPR = 0x80;
	CLKPR = 0x00;
	
	bit_set(DDRC, BIT(LED1));
	bit_set(DDRC, BIT(LED2));
	bit_set(DDRC, BIT(LED3));
	bit_set(DDRD, BIT(LED4));
	
	bit_clear(PORTB, BIT(KICK)); //KICK OFF
	bit_clear(DDRB, BIT(DONE)); //DONE input
	bit_set(DDRB, BIT(CHARGE)); //CHARGE output
	bit_set(DDRB, BIT(KICK)); //CHARGE output
	bit_clear(PORTB, BIT(CHARGE)); //CHARGE OFF
	bit_set(DDRB, BIT(PWM));//DRIBBLER output
	
	bit_clear(DDRB, BIT(BALL));//BALL IN TRIBBLER input
	bit_set(PORTB, BIT(BALL));//pullup
	bit_clear(MCUCR, PUD);
	
	usb_init();
	
	//Wait for USB to be configured
	while (!usb_configured()) /* wait */ ;
	_delay_ms(500);
	
	//timer0
	TCCR0A = 0b00000010;
	TCCR0B = 0b00000101; //prescale 1024
	OCR0A = 250;
	TIMSK0 = 0b00000010;
	TCNT0 = 0;
	
	//timer1
	TCCR1A = 0b00000010;
	TCCR1B = 0b00000101; //prescale 1024
	OCR1A = 250;
	TIMSK1 = 0b00000010;
	TCNT1 = 0;
	
	//init PCINT4
	DDRB &= 0b11101111;
	PCICR = 1; //enable pin change interrupt
	PCMSK0 = 0b00010000;
	
	bit_set(PORTC, BIT(LED1));
	bit_set(PORTC, BIT(LED2));
	bit_set(PORTC, BIT(LED3));
	bit_set(PORTD, BIT(LED4));
	
	sei();
	
	uint8_t n;
	char buf[16];	
	
	while (1) {
		if(usb_serial_available()) {
			n = recv_str(buf, sizeof(buf));
			if (n == sizeof(buf)) {
				parse_and_execute_command(buf);
			}
		}
		if ((fail_counter == 100) && failsafe) {
			discharge();
			fail_counter = 0;
		}

	}
}
