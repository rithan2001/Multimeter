//AUTHOR NAME:  Rithanathith.S
//DOMAIN:  Embedded and Analog electronics-TASK0
//GLOBAL VARIABLE:  time_overflow -to count of overflows

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

#define r1 10000
//r1 is resistance1
#define r2 10000
//r2 is resistance 2


//FUNCTION NAME : adc
//EXAMPLE CALL: adc()
//LOGIC: Enabling the adc and setting prescalar to 128
//since adc uses input clock frequency between 50kHz and 200KHz

void adc(void)
{
 ADCSRA |=1<<ADPS0|1<<ADPS1|1<<ADPS2;//prescalar of 128
 ADCSRA|=(1<<ADEN)|(1<<ADATE);//auto trigger mode & ADC enable
 ADMUX|=(1<<REFS0)|(1<<ADLAR); //referance voltage 5v & left shifted
 adcstart();
}

//FUNCTION NAME : adcstart
//EXAMPLE CALL: adcstart() 
//LOGIC: It will start adc conversion*

void adcstart(void)
{
   ADCSRA|=(1<<ADSC);
}

//FUNCTION NAME : convert
//INPUT: x -it has the measured voltage* 
//OUTPUT: x -The measured voltage is returned 
//LOGIC : Since resulted value is stored in ADCH which is of 8 bit memory,resolution of adc is (2^8)=256
//TO convert obtained adc value in term of 5v ,convert() fn does the following convertion
//EXAMPLE CALL: convert(3.5)

float convert(float x)
{
  float value;

   while((ADCSRA &(1<<ADIF))==0);
  value=ADCH;
   x=(5.0*value)/256.0;
   return x;
}

//FUNCTION NAME : uart_intialize
//LOGIC: Enabling receiver and transmitter via asynchronous USART communication
//Character size of 8Bit data is sent ,no parity mode,1 bit stopbit
//UBBR0 holds baud value for  Baud rate  9600bps and 16MHz system clock  frequency.
//UBBR0=(fo/16BAUD RATE)-1, where fo is system clock frequency
//downcounter start as 103 is assigned to UBBR 
//EXAMPLE CALL: uart_intialize()

void uart_intialize()
{
  UCSR0A=0X00;
  UCSR0B=(1<<RXEN0)|(1<<TXEN0);
  UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);//8 bit data 
  UBRR0=103;
  
}

//FUNCTION NAME : uart_read
//LOGIC: FN TO RECEIVE DATA
//This fn waits till each data bit is received completely and return stored value in USART data register
//EXAMPLE CALL: uart_read()

unsigned char  uart_read(void)
{
  while(!(UCSR0A&(1<<RXC0)));
  return UDR0; 
}


//FUNCTION NAME : uart_write
//INPUT : data as character
//LOGIC: //FN TO SEND data
//This fn waits till data register is empty completely and writes data value in USART data register
//EXAMPLE CALL: uart_write()

void uart_write(char data)
{
  while(!(UCSR0A&(1<<UDRE0)));
  UDR0=data;
}

//FUNCTION NAME : uart_string
//INPUT:  stringptr --declare a string pointer* 
//LOGIC: Fn TO send string
//This fn takes each character in string and calls uart_write() fn and moves the pointer to next position
//EXAMPLE CALL: uart_string(&char)

 
void uart_string(char* stringptr)
{
  while((*stringptr)!=0x00)
  {
    uart_write(*stringptr);
    stringptr++;
  }

}

//FUNCTION NAME : reverse
//INPUT:  str as string pointer,len as string length 
//LOGIC: Reverse the string using string length
//EXAMPLE CALL: reverse(&char,5)

void reverse(char *str, int len) 
{ 
    int i=0, j=len-1, temp; 
    while (i<j) 
    { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; j--; 
    } 
} 

//FUNCTION NAME : intToStr
//INPUT:x,d as integer,str as char,
//OUTPUT:return converted integer i. 
//LOGIC: Converts a given integer x to string string str. 
//d is the number of digits required in output
//EXAMPLE CALL:intToStr(5,"apple",2)

int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    while (x) 
    { 
        str[i++] = (x%10) + '0'; 
        x = x/10; 
    } 
  
    while (i < d) 
        str[i++] = '0'; 
  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
} 


//FUNCTION NAME : ftoa
//INPUT:floating point no n,string variable to store floating point no,afterpoint as int 
//LOGIC: convert floating point no to string
//EXAMPLE CALL: ftoa(8.9645,&char,2)
 
void ftoa(float n, char *res, int afterpoint) 
{ 
    
    int ipart = (int)n;
    float fpart = n - (float)ipart; 
    int i = intToStr(ipart, res, 0); 
 
    if (afterpoint != 0) 
    { 
        res[i] = '.'; 
     fpart = fpart * pow(10, afterpoint); 
    intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
}


// global variable to count the number of overflows
volatile uint8_t time_overflow;

//FUNCTION NAME : timer1_init
//EXAMPLE CALL: timer1_init() 
//LOGIC:  initialize timer & set prescalar of 8 ,overflow interrupt 

void timer1_init()
{
                    
    TCCR1B |= (1 << CS11); //timer1 with prescaler of 8
    TCNT1 = 0;
    TIMSK1 |= (1 << TOIE1); // enable overflow interrupt
     sei(); // global interrupts
    time_overflow = 0;
}
 
//ISR is called when TCNT1 overflows
ISR(TIMER1_OVF_vect)
{
    // number of overflows
    time_overflow++;
    
}

//main function

 int main()
 { 
  
  char user[250]="\n enter ur choice 1.resistance(ohms) 2.voltmeter(volts) 3.ammeter(milliamps) 4.capacitance (microfarad) \n";
  //fn call 
  uart_intialize();
  uart_string(user);
  int choice;

 float  unknownresistance=0;
 float y,a,b;
 
 choice=uart_read();//store entered choice in choice
  
//logic: To calculate unknown  resistance using known resistance of 10k ohms
//By calculating volage between two resistance ,unknown resistance is found by ohm law
//expected Range between 9900 and 11000 ohms

 if(choice=='1') 
 {
  char resistance[500];
    adc();
    y=convert(0);
    unknownresistance= (5.0-y)*r1/y;
   ftoa(unknownresistance,resistance,3);
    uart_string(resistance);
  
  }

//logic: To calculate unknown voltage using 2 known resistance of 10k ohms
//By calculating volage between two resistance and connecting the one of the end to voltage source,
//unknown voltage is found by ohm law
//expected Range between 9 and 9.5v
  
else if(choice=='2')
 { 
  float voltage=0;
  char unknownvoltage[500];
  ADMUX=1<<MUX0;
  adc();
    a=convert(0);
    voltage= ((r1+r2)*a)/r2;
     ftoa(voltage,unknownvoltage,3);
    uart_string(unknownvoltage);
       
  }
//logic: To calculate unknown current using 2 known resistance of 10k ohms and 1k ohms.
//By calculating voltage between two resistance and connecting the one of the end to referance voltage source(5v),
//unknown current is found by ohm law
//expected Range between 0.03 and 0.04 milliamps
 
else if(choice=='3')
 {
  char unknowncurrent[500];
  float current=0;
  ADMUX=1<<MUX1;
  adc();
 
    a=convert(0);
    current=a*1000/r2;
    //milliamps
 ftoa(current,unknowncurrent,5);
    uart_string(unknowncurrent);
    
 }
//logic: To calculate unknown capacitance using 1 known resistance of 10k ohms
//By calculating volage across capacitance,
//unknown  capacitance is found using time canstanst
//LOGIC : 1 time constant =63% of volatge across capacitance
//expected Range between 90 and 100
else if (choice=='4')
 { 
   
    DDRC |= (1 << 0)|(0<<3)|(0<<5);
  
   int count=0;
   float b=0;
   
   timer1_init();

  char unknowncapacitance[500]; 
  float capacitance=0,timeconstant=0;
  
    while(1)
    { 

      //Since prescalar was 8 ,16MHz/8=2MHz 
      //65535/2MHz =32.7millisec
      //to tick every sec,32.7 millisec is very less ,
      //so 1sec/32.7millisec=30.5 is timer overflow limit
   
      if (time_overflow >= 31) 
    {
        PORTC ^= (1 << 0); //timer is reset every time it overflows
        count++;
        time_overflow = 0;  
         
       adc();
         b=convert(0);
       //  1 time constant =63% of volatge across capacitance
       // equivalent analog value for ref volt(5v)is 256 
       //Hence  ,0.63*256=3.15
        
          while(b>3.15)
          {
            PORTC=0XFF;
        timeconstant=count;
         capacitance=(timeconstant *1000000.0)/r1;//convert to micro farad
         ftoa(capacitance,unknowncapacitance,3);
         uart_string(unknowncapacitance);
        break;
         }
     break;    
       
    } 
     
    }
        
}
 }
