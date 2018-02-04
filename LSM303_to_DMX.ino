/******************************************************************************

    LSM303_to_DMX
        send LSM303 data as DMX-Stream.r LEDBoard_4x4_16bit
        debugout on usbserial interface: 115200baud

    hardware:
        Board:
            Arduino compatible (with hw serial port)
            LED on pin 13
            used: Pololu A-Star MINI UV
        Connections:
            D0 RX --> MAX485E Pin1 (RS485 receive)
            D1 TX --> MAX485E Pin4 (RS485 transmit)
            D4 --> MAX485E Pin2&3 (RS485 direction)
            D2 SDA --> LSM303D SDA
            D3 SCL --> LSM303D SCL


    libraries used:
        ~ slight_DebugMenu
        ~ slight_FilterMedian
        ~ slight_Button
            License: MIT
            written by stefan krueger (s-light),
                github@s-light.eu, http://s-light.eu, https://github.com/s-light/
        ~ DMXSerial
            Copyright (c) 2005-2012 by Matthias Hertel,
            http://www.mathertel.de
            license:
                See http://www.mathertel.de/License.aspx
                Software License Agreement (BSD License)

    written by stefan krueger (s-light),
        github@s-light.eu, http://s-light.eu, https://github.com/s-light/

    changelog / history
        check git commit messages


******************************************************************************/
/******************************************************************************
    The MIT License (MIT)

    Copyright (c) 2018 Stefan Krüger

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
******************************************************************************/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Includes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// use "file.h" for files in same directory as .ino
// #include "file.h"
// use <file.h> for files in library directory
// #include <file.h>

#include <slight_DebugMenu.h>
#include <slight_ButtonInput.h>
// #include <slight_filter.h>
#include "./slight_filter.h"

// #include <DMXSerial.h>

#include <Wire.h>
#include <LSM303.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Info
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void sketchinfo_print(Print &out) {
    out.println();
    //             "|~~~~~~~~~|~~~~~~~~~|~~~..~~~|~~~~~~~~~|~~~~~~~~~|"
    out.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    out.println(F("|                       ^ ^                      |"));
    out.println(F("|                      (0,0)                     |"));
    out.println(F("|                      ( _ )                     |"));
    out.println(F("|                       \" \"                      |"));
    out.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    out.println(F("| LSM303_to_DMX.ino"));
    out.println(F("|   send LSM303 data as DMX-Stream."));
    out.println(F("|"));
    out.println(F("| This Sketch has a debug-menu:"));
    out.println(F("| send '?'+Return for help"));
    out.println(F("|"));
    out.println(F("| dream on & have fun :-)"));
    out.println(F("|"));
    out.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    out.println(F("|"));
    //out.println(F("| Version: Nov 11 2013  20:35:04"));
    out.print(F("| version: "));
    out.print(F(__DATE__));
    out.print(F("  "));
    out.print(F(__TIME__));
    out.println();
    out.println(F("|"));
    out.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    out.println();

    //out.println(__DATE__); Nov 11 2013
    //out.println(__TIME__); 20:35:04
}


// Serial.print to Flash: Notepad++ Replace RegEx
//     Find what:        Serial.print(.*)\("(.*)"\);
//     Replace with:    Serial.print\1\(F\("\2"\)\);



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// definitions (global)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Debug Output

Print &DebugOut = Serial;
Stream &DebugIn = Serial;
// attention: in setup_DebugOut 'Serial' is hardcoded used for initialisation


boolean infoled_state = 0;
const byte infoled_pin = 13;

unsigned long debugOut_LiveSign_TimeStamp_LastAction = 0;
const uint16_t debugOut_LiveSign_UpdateInterval = 1000; //ms

boolean debugOut_LiveSign_Serial_Enabled = 0;
boolean debugOut_LiveSign_LED_Enabled = 1;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Menu

// slight_DebugMenu(Stream &in_ref, Print &out_ref, uint8_t input_length_new);
// slight_DebugMenu myDebugMenu(Serial, Serial, 15);
slight_DebugMenu myDebugMenu(DebugIn, DebugOut, 15);


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// button

const uint8_t button_pin = A5;

slight_ButtonInput button(
    0,  // byte cbID_New
    button_pin,  // byte cbPin_New,
    button_getInput,  // tCbfuncGetInput cbfuncGetInput_New,
    button_onEvent,  // tcbfOnEvent cbfCallbackOnEvent_New,
      30,  // const uint16_t cwDuration_Debounce_New = 30,
     500,  // const uint16_t cwDuration_HoldingDown_New = 1000,
      50,  // const uint16_t cwDuration_ClickSingle_New =   50,
     500,  // const uint16_t cwDuration_ClickLong_New =   3000,
     500   // const uint16_t cwDuration_ClickDouble_New = 1000
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// DMX

const uint8_t dmx_pin_direction = 4;

// timeout in milliseconds
const uint32_t dmx_valid_timeout = 1000;

bool dmx_valid = false;

uint16_t dmx_start_channel = 80;
uint8_t dmx_value = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LSM303 compass

LSM303 compass;

uint32_t lsm303_read_timestamp_last = 0;
const uint16_t lsm303_read_interval = 20;
// 20ms = 50Hz = update rate for accelerometer


bool lsm303_serial_out_enabled = false;
uint32_t lsm303_serial_out_timestamp_last = 0;
const uint16_t lsm303_serial_out_interval = 500;

bool lsm303_dmx_send_enabled = false;
uint32_t lsm303_dmx_send_timestamp_last = 0;
const uint16_t lsm303_dmx_send_interval = 500;


const size_t filter_size = 20;
int16_t lsm303_a_x_raw[filter_size];
int16_t lsm303_a_x_temp[filter_size];
slight_FilterMedianRingbuffer <int16_t> filter_a_x(
    lsm303_a_x_raw,
    lsm303_a_x_temp,
    filter_size
);
int16_t lsm303_a_y_raw[filter_size];
int16_t lsm303_a_y_temp[filter_size];
slight_FilterMedianRingbuffer <int16_t> filter_a_y(
    lsm303_a_y_raw,
    lsm303_a_y_temp,
    filter_size
);
int16_t lsm303_a_z_raw[filter_size];
int16_t lsm303_a_z_temp[filter_size];
slight_FilterMedianRingbuffer <int16_t> filter_a_z(
    lsm303_a_z_raw,
    lsm303_a_z_temp,
    filter_size
);


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// other things..

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// DebugOut

// freeRam found at
// http://forum.arduino.cc/index.php?topic=183790.msg1362282#msg1362282
// posted by mrburnette
int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup_DebugOut(Print &out) {
    // for ATmega32U4 devices:
    #if defined (__AVR_ATmega32U4__)
        // wait for arduino IDE to release all serial ports after upload.
        // delay(2000);
    #endif

    Serial.begin(115200);

    // for ATmega32U4 devices:
    #if defined (__AVR_ATmega32U4__)
        // Wait for Serial Connection to be Opend from Host or
        // timeout after 6second
        uint32_t timeStamp_Start = millis();
        while( (! Serial) && ( (millis() - timeStamp_Start) < 1000 ) ) {
            // nothing to do
        }
    #endif


    out.println();

    out.print(F("# Free RAM = "));
    out.println(freeRam());
}

void handle_debugout() {
    if (
        (millis() - debugOut_LiveSign_TimeStamp_LastAction) >
        debugOut_LiveSign_UpdateInterval
    ) {
        debugOut_LiveSign_TimeStamp_LastAction = millis();

        if ( debugOut_LiveSign_Serial_Enabled ) {
            DebugOut.print(millis());
            DebugOut.print(F("ms;"));
            DebugOut.print(F("  free RAM = "));
            DebugOut.print(freeRam());
            // DebugOut.print(F("; bat votlage: "));
            // DebugOut.print(bat_voltage/100.0);
            // DebugOut.print(F("V"));
            DebugOut.println();
        }

        if ( debugOut_LiveSign_LED_Enabled ) {
            infoled_state = ! infoled_state;
            if (infoled_state) {
                //set LED to HIGH
                digitalWrite(infoled_pin, HIGH);
            } else {
                //set LED to LOW
                digitalWrite(infoled_pin, LOW);
            }
        }

    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Menu System

void setup_DebugMenu(Print &out) {
    out.print(F("# Free RAM = "));
    out.println(freeRam());

    out.println(F("setup DebugMenu:")); {
        // myDebugMenu.set_user_EOC_char(';');
        myDebugMenu.set_callback(handleMenu_Main);
        myDebugMenu.begin();
    }
    out.println(F("\t finished."));
}

// Main Menu
void handleMenu_Main(slight_DebugMenu *pInstance) {
    Print &out = pInstance->get_stream_out_ref();
    char *command = pInstance->get_command_current_pointer();
    // out.print("command: '");
    // out.print(command);
    // out.println("'");
    switch (command[0]) {
        case 'h':
        case 'H':
        case '?': {
            // help
            out.println(F("____________________________________________________________"));
            out.println();
            out.println(F("Help for Commands:"));
            out.println();
            out.println(F("\t '?': this help"));
            out.println(F("\t 'i': sketch info"));
            out.println(F("\t 'y': toggle DebugOut livesign print"));
            out.println(F("\t 'Y': toggle DebugOut livesign LED"));
            out.println(F("\t 'x': tests"));
            out.println();
            out.println(F("\t 'a': toggle accelerometer serial output "));
            out.println(F("\t 'A': toggle accelerometer dmx output "));
            // out.println(F("\t 's': set channel 's1:65535'"));
            // out.println(F("\t 'f': DemoFadeTo(ID, value) 'f1:65535'"));
            out.println();
            out.println(F("____________________________________________________________"));
        } break;
        case 'i': {
            sketchinfo_print(out);
        } break;
        case 'y': {
            out.println(F("\t toggle DebugOut livesign Serial:"));
            debugOut_LiveSign_Serial_Enabled = !debugOut_LiveSign_Serial_Enabled;
            out.print(F("\t debugOut_LiveSign_Serial_Enabled:"));
            out.println(debugOut_LiveSign_Serial_Enabled);
        } break;
        case 'Y': {
            out.println(F("\t toggle DebugOut livesign LED:"));
            debugOut_LiveSign_LED_Enabled = !debugOut_LiveSign_LED_Enabled;
            out.print(F("\t debugOut_LiveSign_LED_Enabled:"));
            out.println(debugOut_LiveSign_LED_Enabled);
        } break;
        case 'x': {
            // get state
            out.println(F("__________"));
            out.println(F("Tests:"));

            out.println(F("nothing to do."));

            // uint16_t wTest = 65535;
            uint16_t wTest = atoi(&command[1]);
            out.print(F("wTest: "));
            out.print(wTest);
            out.println();

            out.print(F("1: "));
            out.print((byte)wTest);
            out.println();

            out.print(F("2: "));
            out.print((byte)(wTest>>8));
            out.println();

            out.println();

            out.println(F("__________"));
        } break;
        //---------------------------------------------------------------------
        // case 'A': {
        //     out.println(F("\t Hello World! :-)"));
        // } break;
        // ------------------------------------------
        // case 's': {
        //     out.print(F("\t set channel "));
        //     // convert part of string to int
        //     // (up to first char that is not a number)
        //     uint8_t command_offset = 1;
        //     uint8_t ch = atoi(&command[command_offset]);
        //     // convert single character to int representation
        //     // uint8_t id = &command[1] - '0';
        //     command_offset = 3;
        //     if (ch > 9) {
        //         command_offset = command_offset +1;
        //     }
        //     out.print(ch);
        //     out.print(F(" : "));
        //     uint16_t value = atoi(&command[command_offset]);
        //     out.print(value);
        //     out.println();
        //
        //     if (output_enabled) {
        //         tlc.setChannel(ch, value);
        //         tlc.write();
        //     }
        // } break;
        // case 'f': {
        //     out.print(F("\t DemoFadeTo "));
        //     // convert part of string to int
        //     // (up to first char that is not a number)
        //     uint8_t id = atoi(&command[1]);
        //     // convert single character to int representation
        //     // uint8_t id = &command[1] - '0';
        //     out.print(id);
        //     out.print(F(" : "));
        //     uint16_t value = atoi(&command[3]);
        //     out.print(value);
        //     out.println();
        //     //demo_fadeTo(id, value);
        //     tlc.setChannel()
        //     out.println(F("\t demo for parsing values --> finished."));
        // } break;
        //---------------------------------------------------------------------
        default: {
            if(strlen(command) > 0) {
                out.print(F("command '"));
                out.print(command);
                out.println(F("' not recognized. try again."));
            }
            pInstance->get_command_input_pointer()[0] = '?';
            pInstance->set_flag_EOC(true);
        }
    } // end switch

    // end Command Parser
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// buttons

void setup_buttons(Print &out) {
    out.print(F("# Free RAM = "));
    out.println(freeRam());

    out.println(F("setup button:")); {
        out.println(F("\t set button pin"));
        pinMode(button_pin, INPUT_PULLUP);
        out.println(F("\t button begin"));
        button.begin();
    }
    out.println(F("\t finished."));
}

boolean button_getInput(uint8_t id, uint8_t pin) {
    // read input invert reading - button closes to GND.
    // check HWB
    // return ! (PINE & B00000100);
    return ! digitalRead(pin);
}

void button_onEvent(slight_ButtonInput *pInstance, byte bEvent) {
    // Serial.print(F("FRL button:"));
    // Serial.println((*pInstance).getID());
    //
    // Serial.print(F("Event: "));
    // Serial.print(bEvent);
    // // (*pInstance).printEvent(Serial, bEvent);
    // Serial.println();

    // uint8_t button_id = (*pInstance).getID();

    // show event additional infos:
    switch (bEvent) {
        // case slight_ButtonInput::event_StateChanged : {
        //     Serial.println(F("\t state: "));
        //     (*pInstance).printlnState(Serial);
        //     Serial.println();
        // } break;
        case slight_ButtonInput::event_Down : {
            // Serial.println(F("FRL down"));
        } break;
        case slight_ButtonInput::event_HoldingDown : {
            // uint32_t duration = (*pInstance).getDurationActive();
            // Serial.println(F("duration active: "));
            // Serial.println(duration);
        } break;
        case slight_ButtonInput::event_Up : {
            // Serial.println(F("up"));
        } break;
        case slight_ButtonInput::event_Click : {
            // Serial.println(F("FRL click"));
        } break;
        case slight_ButtonInput::event_ClickLong : {
            // Serial.println(F("click long"));
        } break;
        case slight_ButtonInput::event_ClickDouble : {
            // Serial.println(F("click double"));
        } break;
        case slight_ButtonInput::event_ClickTriple : {
            // Serial.println(F("click triple"));
        } break;
        case slight_ButtonInput::event_ClickMulti : {
            // Serial.print(F("click count: "));
            // Serial.println((*pInstance).getClickCount());
        } break;
    }  // end switch
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// DMX

void setup_DMX(Print &out) {
    out.println(F("setup DMX:"));

    // pin for direction
    pinMode(dmx_pin_direction, OUTPUT);

    // set to receive mode
    // Serial.println(F("\t set direction pin to Low = 'Receive' "));
    // digitalWrite(dmx_pin_direction, LOW);
    // Serial.println(F("\t init as DMXReceiver"));
    // DMXSerial.init(DMXReceiver, dmx_pin_direction);

    // set to send mode
    // Serial.println(F("\t set direction pin to High = 'Send' "));
    // digitalWrite(dmx_pin_direction, HIGH);
    // Serial.println(F("\t init as DMXController"));
    // DMXSerial.init(DMXController, dmx_pin_direction);



    // Serial.println(F("\t set some values"));
    // DMXSerial.write(10, 255);
    // DMXSerial.write(11, 255);
    // DMXSerial.write(12, 1);
    // read dmx values
    // DMXSerial.read(1);

    out.println(F("\t finished."));
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LSM303 compass

void setup_LSM303(Print &out) {
    out.println(F("setup LSM303D:")); {
        out.println(F("\t  start I2C"));
        Wire.begin();
        out.println(F("\t  init compass"));
        compass.init();
        out.println(F("\t  enable defaults"));
        compass.enableDefault();
    }
    out.println(F("\tfinished."));
}

void handle_LSM303() {
    if(
        (millis() - lsm303_read_timestamp_last) > lsm303_read_interval
    ) {
        lsm303_read_timestamp_last =  millis();
        lsm303_read();
    }

    if (lsm303_serial_out_enabled) {
        if(
            (millis() - lsm303_serial_out_timestamp_last) > lsm303_serial_out_interval
        ) {
            lsm303_serial_out_timestamp_last =  millis();
            lsm303_serial_out_print();
        }
    }

    if (lsm303_dmx_send_enabled) {
        if(
            (millis() - lsm303_dmx_send_timestamp_last) > lsm303_dmx_send_interval
        ) {
            lsm303_dmx_send_timestamp_last =  millis();
            lsm303_dmx_send();
        }
    }
}


void lsm303_read() {
    compass.read();
    filter_a_x.add_value(compass.a.x);
    filter_a_y.add_value(compass.a.y);
    filter_a_z.add_value(compass.a.z);
    filter_a_x.update();
    filter_a_y.update();
    filter_a_z.update();
}



void lsm303_serial_out_print() {
        // char line[24];
        // snprintf(
        //     line,
        //     sizeof(line),
        //     "A: %6d %6d %6d;",
        //     compass.a.x,
        //     compass.a.y,
        //     compass.a.z
        // );
        // DebugOut.println(line);
        char line[60];
        snprintf(
            line,
            sizeof(line),
            "A: %6d %6d %6d; AF: %6d %6d %6d;",
            compass.a.x,
            compass.a.y,
            compass.a.z,
            filter_a_x.get_filterd_value(),
            filter_a_y.get_filterd_value(),
            filter_a_z.get_filterd_value()
        );
        DebugOut.println(line);
}


void lsm303_dmx_send() {
        compass.read();

        char line[24];
        snprintf(
            line,
            sizeof(line),
            "A: %6d %6d %6d;",
            compass.a.x,
            compass.a.y,
            compass.a.z
        );
        DebugOut.println(line);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// other things..




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// setup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // initialise PINs

    //LiveSign
    pinMode(infoled_pin, OUTPUT);
    digitalWrite(infoled_pin, HIGH);

    // as of arduino 1.0.1 you can use INPUT_PULLUP

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    setup_DebugOut(DebugOut);

    setup_DebugMenu(DebugOut);

    sketchinfo_print(DebugOut);

    setup_buttons(DebugOut);

    setup_DMX(DebugOut);

    setup_LSM303(DebugOut);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // go
    DebugOut.println(F("Loop:"));

} /** setup **/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main loop
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // menu input
    myDebugMenu.update();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // update sub parts

    button.update();

    handle_debugout();

    handle_LSM303();


} /** loop **/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
