# stm32F103C8_NRF24L01_and_LoRa_E220_900T22D
This project was created in STM32CubeMX.
Devices: 
1. STM32F103C8.
2. Lora radio module: E220-900T22D (Powered by USB) (with antenna).
3. NRF radio module: NRF24L01 (with antenna).
4. Temperature and humidity sensor: AM2302.
5. OLED 0.96" 128x64 I2C: SSD1306.
6. Three buttons.
7. UART to USB module. (Power source).

The main features:
1. In project are LoRa E220-900T22D radio module. Module can work in RX and TX modes, depends on choise in menu. (Work dictance about 9 km)
2. In project are  NRF24L01 radio module, in RX and TX modes, depends on choise in menu. (Work dictance about 400 - 600 m)
3. Menu. For create menu was used four-linked list. (see on menu.c file)
4. LoRa and NFR modules can transmit test data(counter) or real measured temterature and humidity by AM3202 sensor. For transmiting T and H must be trur on periodic meassure:  (3.AM3202 sensor -> 2. Per Meas: ON) after, select LoRa or NRF menu, and Select TX T and H menu item.
For better radio communication need use large capacitors on power line near radio modules!
All project was tested.

![alt text](https://github.com/OlegDemk/stm32F103C8_NRF24L01_and_LoRa_E220_900T22D/blob/main/schem_device.png)


![alt text](https://github.com/OlegDemk/stm32F103C8_NRF24L01_and_LoRa_E220_900T22D/blob/main/%20program_structure.png)

![alt text](https://github.com/OlegDemk/stm32F103C8_NRF24L01_and_LoRa_E220_900T22D/blob/main/20210913_162651.jpg)

![alt text](https://github.com/OlegDemk/stm32F103C8_NRF24L01_and_LoRa_E220_900T22D/blob/main/20210913_163217.jpg)
