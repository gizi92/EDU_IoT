#include <SPI.h>

#define LED_PIN 3
#define LIGHT_SENSOR_PIN A1

char buff [100];
volatile byte index;
volatile bool receivedone;  /* use reception complete flag */

void setup() 
{
  Serial.begin(9600);
  Serial.println("Setup");
  pinMode(LED_PIN, OUTPUT);
  
  SPCR |= bit(SPE);         /* Enable SPI */
  pinMode(MISO, OUTPUT);    /* Make MISO pin as OUTPUT */
  index = 0;
  receivedone = false;
  SPI.attachInterrupt();    /* Attach SPI interrupt */
}

int light_sensor_value = 0;
void loop()
{
  light_sensor_value = analogRead(LIGHT_SENSOR_PIN)/10.23;
  
  Serial.println(light_sensor_value);
  analogWrite(LED_PIN, light_sensor_value*2.55);

  if (receivedone)          /* Check and print received buffer if any */
  {
    buff[index] = 0;
    Serial.println(buff);
    index = 0;
    receivedone = false;
  }
}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  uint8_t oldsrg = SREG;
  cli();
  char c = SPDR;
  if (index <sizeof buff)
  {
    buff [index++] = c;
    if (c == '\n'){     /* Check for newline character as end of msg */
     receivedone = true;
    }
  }
  SREG = oldsrg;
}
