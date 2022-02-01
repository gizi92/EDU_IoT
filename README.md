# EDU_IoT
## UBB IoT  project - Smart Office

### Project structure
- NodeMCU:
    - Up to date version: `/mcus/nodemcu/spi_master`
    - Example code snippets: `/mcus/nodemcu/examples/`
    - To run the examples, you need to install the following libraries:
        - LittleFS by lorol: https://github.com/lorol/LITTLEFS
        - ESP8266WiFi: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
        - ESPAsyncTCP by me-no-dev: https://github.com/me-no-dev/ESPAsyncTCP
        - ESPAsyncWebServer by me-no-dev: https://github.com/me-no-dev/ESPAsyncWebServer
        - Arduino_JSON by arduino-libraries: https://github.com/arduino-libraries/Arduino_JSON
        - PubSubClient by knolleary: https://github.com/knolleary/pubsubclient

- Arduino Uno:
    - Up to date version: `/mcus/arduino_uno/spi_slave`
    - Example code snippets: `/mcus/arduino_uno/examples/`
    - To run the examples, you need to install the following libraries:
        - LiquidCrystal I2C by Marco Schwartz: https://github.com/johnrickman/LiquidCrystal_I2C
        - Arduino-DS3231 by Korneliusz Jarzebski: https://github.com/jarzebski/Arduino-DS3231
        - SimpleDHT by Winlin: https://github.com/winlinvip/SimpleDHT
        - Adafruit_TSL2561 by Adafruit: https://github.com/adafruit/Adafruit_TSL2561
        - Arduino-IRRemote by Arduino: https://github.com/Arduino-IRremote/Arduino-IRremote
- Libraries:
    - path: /mcus/libraries
    - Usage instructions:
        - Windows: Copy all folders from `/mcus/libraries` to your local `/Documents/Arduino/libraries/` folder. Reopen your arduino IDE. You should see the libraries under the `Sketch->Include Library` menu item of the IDE.
        - Other OSes: check where the user libraries for arduino are stored and copy the content of `/mcus/libraries/` there (Imma captain obvious). Alternatively you can create zip archives from the libraries (for ex `EDU_IoT_DataPacket.zip`) and import them into
        the Arduino IDE by using the `Sketch->Include Library->Add .ZIP Library...` function.

# Tools
- If you want to use VSCode for Arduino: https://www.youtube.com/watch?v=FnEvJXpxxNM&t=300s
- PlatformIO is a cool VSCode plugin for embedded programming: https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/
- vagrant and virtualbox for mqtt server.