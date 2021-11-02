#include <SPI.h>

SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

char bejovo_uzenet[100];
char kimeno_uzenet[100]="Felelet az arduinotol\n";

volatile byte index_bejovo;
volatile byte index_kimeno;
volatile bool fogadunk;
volatile bool kuldunk;
volatile bool vege;

void setup (void)
{
  Serial.begin (9600);
  SPCR |= bit(SPE);         //bekapcsolja az SPIt
  //SPI.begin();
  pinMode(MISO, OUTPUT);    //MISOn valaszol
  
  index_bejovo = 0;
  index_kimeno = 0;
  
  fogadunk = true; //elobb fogadunk
  kuldunk = false; //aztan kuldunk
  vege = false; //aztan vege
  
  SPI.attachInterrupt();   //ha jon az SPIn  valami beugrik a fuggvenybe
  pinMode(3,OUTPUT);
}

void loop ()
{
if (vege)    
  {
    for(int i=0;i<index_bejovo;i++)
      Serial.print(bejovo_uzenet[i]);   

    int feny = analogRead(A1);
    Serial.println();
    Serial.print(feny);
    
    index_bejovo=0;
    vege = false;
    fogadunk = true; 
    kuldunk = false;
    digitalWrite(3,0);
  }

delay(100);  

}

// SPI interrupt routine
ISR(SPI_STC_vect)
{

  //kiolvassuk a kapott karaktert
  char c = SPDR;
  
  if (fogadunk)
  {
    bejovo_uzenet[index_bejovo] = c;
    index_bejovo++;
  } 

  if (kuldunk)
  {
    if (index_kimeno<22) SPDR = kimeno_uzenet[index_kimeno];
    if (index_kimeno==99) vege = true;
    index_kimeno++;
  } 

  if (c == '\n')
  {     
     fogadunk = false;
     kuldunk = true;
     index_kimeno = 0;
     digitalWrite(3,1);
  }
}
