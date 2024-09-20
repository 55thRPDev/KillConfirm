This resource is free forever and always for personal use
Your support is always appreciated it <3 [![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/T6T8KZZF4)    


# Kill Confirm  
Kill Confirm score counter for 2 teams using RFID. Arduino Uno Based.

# Demo Video
https://youtu.be/_Dhgae85r1A

# Current Functionality

- Scan Red/Blue Access Card to enter Score Card Scanning Mode
- Scan x1 Score card to add a point to the appopritate teams score
- Game end when a team scores x10 score cards in total
- The same card can not be scored twice by either team
- Score cards will not score until an access card is scanned
- After 60 seconds if no score card is scanned, device will return to main score screen

## Parts Needed  
- x1 Arduino Uno  
- x1 16x2 LCD (i2c)  
- x1 RC522 RFID Module
- x22 RFID Cards

## Wiring Information  

### RC522 RFID Module to Arduino Uno

| **RFID-RC522 PIN** | **Arduino Uno PIN** |
|--------------------|---------------------|
| SDA                | 10                  |
| SCK                | 13                  |
| MOSI               | 11                  |
| MISO               | 12                  |
| IRQ                | UNUSED              |
| GND                | GND                 |
| RST                | 9                   |
| 3.3 V              | 3.3 V               |


### 16x2 LCD I2C to Arduino Uno

| **LCD I2C PIN** | **Arduino Uno PIN** |
|-----------------|---------------------|
| SDA             | A4                  |
| SCL             | A5                  |
| VCC             | 5V                  |
| GND             | GND                 |


Source code and required libraries can be found in /Arduino


