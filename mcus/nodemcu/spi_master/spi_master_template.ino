#include<SPI.h>

char buff[]="Uzenet a Nodetol\n";
char kuldes_tesztje[20];
char returnbuff[100];

SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0); 
//100 kHz legyen a sebesseg, a Node tud 80MHzt de az Arduino csak 16MHzt

void setup() 
{
   Serial.begin(9600);  
   SPI.begin();
}

void loop() 
{
   SPI.beginTransaction(spi_settings);

   //elkuldi az uzenetet
   for(int i=0; i<sizeof(buff); i++)
   {  
      kuldes_tesztje[i] = SPI.transfer(buff[i]);
      delay(1);
   }
    
   //kuld meg 100 pontot,ezeket az Arduino felulirja
   //ami visszajon azt kiolvassuk betesszuk a returnbuffbe
   for(int i=0; i<100; i++)
   {  
      returnbuff[i] = SPI.transfer('.');
      delay(1);
   }
    
   SPI.endTransaction();

   //itt kiirjuk. Az SPI sokkal gyorsabb volt mint a soros port
   //ezert ott kozben nem probaljuk meg kiirni, a soros kiiratas lassu
   for(int i=0; i<20; i++)  
      Serial.print(kuldes_tesztje[i]);

   for(int i=0; i<100; i++)  
      Serial.print(returnbuff[i]);

   Serial.println();
   delay(1000);  
}
