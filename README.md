# EDU_IoT
## UBB IoT  project - Smart Office

### Project structure
- NodeMCU:
    - Up to date version: `/mcus/nodemcu/spi_master`
    - Example code snippets: `/mcus/nodemcu/examples/`

- Arduino Uno:
    - Up to date version: `/mcus/arduino_uno/spi_slave`
    - Example code snippets: `/mcus/arduino_uno/examples/`

- Libraries:
    - path: /mcus/libraries
    - Usage instructions:
        - Windows: Copy all folders from `/mcus/libraries` to your local `/Documents/Arduino/libraries/` folder. Reopen your arduino IDE. You should see the libraries under the `Sketch->Include Library` menu item of the IDE.
        - Other OSes: check where the user libraries for arduino are stored and copy the content of `/mcus/libraries/` there (Imma captain obvious). Alternatively you can create zip archives from the libraries (for ex `EDU_IoT_DataPacket.zip`) and import them into
        the Arduino IDE by using the `Sketch->Include Library->Add .ZIP Library...` function.

If you want to use VSCode for Arduino: https://www.youtube.com/watch?v=FnEvJXpxxNM&t=300s