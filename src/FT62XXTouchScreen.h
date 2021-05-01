 /*
  * Cribbed from https://github.com/adafruit/Adafruit_FT6206_Library but simplified a little for testing purposes.
  */
#ifndef _FT62XXTouchScreen_H_

#include <Wire.h>

  #define _FT62XXTouchScreen_H_ 1

  #define FT62XX_ADDR 0x38
  #define FT62XX_REG_MODE 0x00        //!< Device mode, either WORKING or FACTORY
  #define FT62XX_REG_CALIBRATE 0x02   //!< Calibrate mode
  #define FT62XX_REG_WORKMODE 0x00    //!< Work mode
  #define FT62XX_REG_FACTORYMODE 0x40 //!< Factory mode
  #define FT62XX_REG_THRESHHOLD 0x80  //!< Threshold for touch detection
  #define FT62XX_REG_POINTRATE 0x88   //!< Point rate
  #define FT62XX_REG_FIRMVERS 0xA6    //!< Firmware version
  #define FT62XX_REG_CHIPID 0xA3      //!< Chip selecting
  #define FT62XX_REG_VENDID 0xA8      //!< FocalTech's panel ID
  
  #define FT62XX_VENDID 0x11  //!< FocalTech's panel ID
  #define FT6206_CHIPID 0x06  //!< Chip selecting
  #define FT6236_CHIPID 0x36  //!< Chip selecting
  #define FT6236U_CHIPID 0x64 //!< Chip selecting

  typedef struct TouchPoint {
    uint16_t xPos;
    uint16_t yPos;
    uint16_t touched;
  } TouchPoint;

  class FT62XXTouchScreen {

    public:
      FT62XXTouchScreen(uint16_t displayHeight, uint8_t sda, uint8_t scl) : m_displayHeight(displayHeight), m_sda(sda), m_scl(scl) {
      }
      
      bool begin() {
        Wire.begin(m_sda, m_scl);
        #ifdef TOUCHSCREEN_DEBUG
          Serial.print("Vend ID: 0x");
          Serial.println(readByteFromTouch(FT62XX_REG_VENDID), HEX);
          Serial.print("Chip ID: 0x");
          Serial.println(readByteFromTouch(FT62XX_REG_CHIPID), HEX);
          Serial.print("Firm V: ");
          Serial.println(readByteFromTouch(FT62XX_REG_FIRMVERS));
          Serial.print("Point Rate Hz: ");
          Serial.println(readByteFromTouch(FT62XX_REG_POINTRATE));
          Serial.print("Thresh: ");
          Serial.println(readByteFromTouch(FT62XX_REG_THRESHHOLD));
        
          // dump all registers
          for (int16_t i = 0; i < 0x10; i++) {
            Serial.print("I2C $");
            Serial.print(i, HEX);
            Serial.print(" = 0x");
            Serial.println(readByteFromTouch(i), HEX);
          }
        #endif
      
        // change threshhold to be higher/lower
        //writeByteToTouch(FT62XX_REG_THRESHHOLD, FT62XX_DEFAULT_THRESHOLD);
      
        if (readByteFromTouch(FT62XX_REG_VENDID) != FT62XX_VENDID) {
          return false;
        }
        uint8_t id = readByteFromTouch(FT62XX_REG_CHIPID);
        if ((id != FT6206_CHIPID) && (id != FT6236_CHIPID) &&
            (id != FT6236U_CHIPID)) {
          return false;
        }
        return true;
      }

      TouchPoint read(void) {
  
        TouchPoint retPoint = {0};
      
        uint8_t i2cdat[16];
        Wire.beginTransmission(FT62XX_ADDR);
        Wire.write((byte)0);
        Wire.endTransmission();
      
        Wire.requestFrom((byte)FT62XX_ADDR, (byte)16);
        for (uint8_t i = 0; i < 16; i++) {
          i2cdat[i] = Wire.read();
        }
      
        #ifdef TOUCHSCREEN_DEBUG
          for (int16_t i = 0; i < 16; i++) {
            Serial.print("I2C $");
            Serial.print(i, HEX);
            Serial.print(" = 0x");
            Serial.println(i2cdat[i], HEX);
          }
        #endif
      
        uint8_t touches = i2cdat[0x02];
      
        if (touches != 1) {
          return retPoint;
        }
        
        #ifdef TOUCHSCREEN_DEBUG
          Serial.print("# Touches: ");
          Serial.println(touches);
        
          for (uint8_t i = 0; i < 16; i++) {
            Serial.print("0x");
            Serial.print(i2cdat[i], HEX);
            Serial.print(" ");
          }
          Serial.println();
          if (i2cdat[0x01] != 0x00) {
            Serial.print("Gesture #");
            Serial.println(i2cdat[0x01]);
          }
        #endif
  
        uint16_t touchY = i2cdat[0x03] & 0x0F;
        touchY <<= 8;
        touchY |= i2cdat[0x04];
        uint16_t touchX = i2cdat[0x05] & 0x0F;
        touchX <<= 8;
        touchX |= i2cdat[0x06];
      
        #ifdef TOUCHSCREEN_DEBUG
          Serial.println();
          uint16_t touchID = i2cdat[0x05] >> 4;
          for (uint8_t i = 0; i < touches; i++) {
            Serial.print("ID #");
            Serial.print(touchID);
            Serial.print("\t(");
            Serial.print(touchX);
            Serial.print(", ");
            Serial.print(touchY);
            Serial.print(") ");
          }
          Serial.println();
        #endif
      
        touchY = m_displayHeight - touchY;

        retPoint.xPos = touchX;
        retPoint.yPos = touchY;
        retPoint.touched = -1;
      
        return retPoint;
      }

    private:
      uint16_t m_displayHeight;
      uint8_t m_sda;
      uint8_t m_scl;

      uint8_t readByteFromTouch(uint8_t reg) {
        Wire.beginTransmission(FT62XX_ADDR);
        Wire.write((byte)reg);
        Wire.endTransmission();
      
        Wire.requestFrom((byte)FT62XX_ADDR, (byte)1);
        uint8_t x = Wire.read();
        #ifdef TOUCHSCREEN_DEBUG
          Serial.print("$");
          Serial.print(reg, HEX);
          Serial.print(": 0x");
          Serial.println(x, HEX);
        #endif
        return x;
      }
      
      void writeByteToTouch(uint8_t reg, uint8_t val) {
        Wire.beginTransmission(FT62XX_ADDR);
        Wire.write((byte)reg);
        Wire.write((byte)val);
        Wire.endTransmission();
      }
      
  
  };

#endif
