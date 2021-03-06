#include <Wire.h>
#include <EEPROM.h>

/**** Pin definitions *******/
// Encoder pins
#define P_ENC_LEFT   2
#define P_ENC_RIGHT  3
#define P_ENC_BUTTON 4

// 7Seg pins
#define P_ADDR0  1
#define P_ADDR1  2
#define P_ADDR2  3
#define P_EN     4
#define P_CS0    1
#define P_CS1    2
#define P_CS2    3
#define P_CS3    3
/**** End of Pin definitions *******/

/*** Instructions ***/
#define I_NONE        0
#define I_GET_VAL     1
#define I_GET_MINMAX  2
#define I_SET_VAL     3
#define I_SET_MINMAX  4
#define I_PING      254
#define I_ERR       255
/*** End of Instructions ***/

//       
//    ---0---
//    |1    |2
//    ---3---
//    |4    |5
//    ---6---   7

byte numbers[][10] = {
    {0,1,2,4,5,6},      // 0
    {2,5},              // 1
    {0,2,3,4,6},        // 2
    {0,2,3,5,6},        // 3
    {1,2,3,5},          // 4
    {0,1,3,5,6},        // 5
    {0,1,3,4,5,6},      // 6
    {0,2,5},            // 7
    {0,1,2,3,4,5,6,7},  // 8
    {0,1,2,3,5,6}       // 9
};

// byte numbers[] = {
//     B10001000,  //  0
//     B11101011,  //  1
//     B01001100,  //  2
//     B01001001,  //  3
//     B00101011,  //  4
//     B00011001,  //  5
//     B00011000,  //  6
//     B11001011,  //  7
//     B00001000,  //  8
//     B00001011,  //  9
// };

byte addr;
byte instr = I_NONE;

int16_t val = 0;
int16_t minVal = 0;
int16_t maxVal = 100;

void helloWorld() {
    displayDigit(1, 0);
    displayDigit(2, 1);
    displayDigit(3, 2);
    displayDigit(4, 3);
}

void setup() {
    //addr = EEPROM.read(0);
    addr = 1;

    attachInterrupt(digitalPinToInterrupt(P_ENC_LEFT), onPulseLeft, HIGH);
    attachInterrupt(digitalPinToInterrupt(P_ENC_RIGHT), onPulseRight, HIGH);
      
    Wire.begin(addr);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    
    delay(1000);
    helloWorld();
}

void loop() {
    updateVal();
}

void updateVal() {
    // split val into digits
    byte units = byte(val % 10);
    byte tens = byte(floor(val / 10));
    byte hundreds = byte(floor(val / 100));
    byte thousands = byte(floor(val / 1000));
    
    displayDigit(units, 0);
    displayDigit(tens, 1);
    displayDigit(hundreds, 2);
    displayDigit(thousands, 3);
}

void displayDigit(byte digit, byte segment) {
    // digit must be [0..9]
    // segment must be [0..3]

    // select the segment
    digitalWrite(P_CS0, segment==0);
    digitalWrite(P_CS1, segment==1);
    digitalWrite(P_CS2, segment==2);
    digitalWrite(P_CS3, segment==3);
    
    for (int i=0; i<numbers[digit].size(); i++) {
        digitalWrite(P_EN, LOW);
        digitalWrite(P_ADDR0, bitRead(numbers[digit][i], 0));
        digitalWrite(P_ADDR1, bitRead(numbers[digit][i], 1));
        digitalWrite(P_ADDR2, bitRead(numbers[digit][i], 2));
        digitalWrite(P_EN, HIGH);
        delay(1);
    }

    // for (int i=0; i<8; i++) {
    //     if(bitRead(numbers[digit], i)) {
    //         // output i to mux address
    //         digitalWrite(P_ADDR0, bitRead(i, 0));
    //         digitalWrite(P_ADDR1, bitRead(i, 1));
    //         digitalWrite(P_ADDR2, bitRead(i, 2));
    //         digitalWrite(P_EN, HIGH);
    //     }
    // }
}

void onPulseLeft() {
    val--;
    if (val < minVal)
        val = minVal;
}

void onPulseRight() {
    val++;
    if (val > maxVal)
        val = maxVal;
}

int16_t read16() {
    byte h = Wire.read();  
    byte l = Wire.read();
    int16_t result = h << 8 + l;
    return result;
}

void receiveEvent(int n) {
    instr = Wire.read();
    switch(instr) {
        case(I_SET_VAL):
            val = read16();
        break;
        case(I_SET_MINMAX):
            minVal = read16();
            maxVal = read16();
        break;
    }
}

void requestEvent() {
    switch(instr) {
        case(I_PING):
            Wire.write(I_PING);
        break;
        case(I_GET_VAL):
            Wire.write(highByte(val));
            Wire.write(lowByte(val));
        break;
        case(I_GET_MINMAX):
            Wire.write(highByte(minVal));
            Wire.write(lowByte(minVal));
            Wire.write(highByte(maxVal));
            Wire.write(lowByte(maxVal));
        break;
        default:
            Wire.write(I_ERR);
        break;
    }
    instr = I_NONE;
}
