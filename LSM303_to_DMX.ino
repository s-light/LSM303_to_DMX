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

#include "./dmx_handling.h"
#include "./lsm303_handling.h"


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

const uint16_t button_duration_Debounce      =   30;
const uint16_t button_duration_HoldingDown   =  500;
const uint16_t button_duration_ClickSingle   =   40;
const uint16_t button_duration_ClickLong     = 1000;
const uint16_t button_duration_ClickDouble   =  200;

const uint8_t button_1 = 0;
// const uint8_t button_2 = 1;
// const uint8_t button_3 = 2;
// const uint8_t button_4 = 3;

const uint8_t buttons_COUNT = 1;
slight_ButtonInput buttons[buttons_COUNT] = {
    // button_1
    slight_ButtonInput(
        button_1,
        A5,
        button_getInput,
        button_onEvent,
        button_duration_Debounce,
        button_duration_HoldingDown,
        button_duration_ClickSingle,
        button_duration_ClickLong,
        button_duration_ClickDouble
    ),
};

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
        delay(1000);
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

void handle_debugout(Print &out) {
    if (
        (millis() - debugOut_LiveSign_TimeStamp_LastAction) >
        debugOut_LiveSign_UpdateInterval
    ) {
        debugOut_LiveSign_TimeStamp_LastAction = millis();

        if ( debugOut_LiveSign_Serial_Enabled ) {
            out.print(millis());
            out.print(F("ms;"));
            out.print(F("  free RAM = "));
            out.print(freeRam());
            // out.print(F("; bat votlage: "));
            // out.print(bat_voltage/100.0);
            // out.print(F("V"));
            out.println();
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
            // out.println(F("\t filter tests:"));
            // out.println(F("\t 'q': print values"));
            // out.println(F("\t 'w': add random value"));
            // out.println(F("\t 'e': print filterd value"));
            out.println();
            out.print(F("\t 'r': toggle lsm303 read ("));
            out.print(lsm303_handling::read_enabled);
            out.println(F(")"));
            out.print(F("\t 'R': set lsm303 read interval 'i65535' ("));
            out.print(lsm303_handling::read_interval);
            out.println(F(")"));
            out.print(F("\t 'a': toggle lsm303 serial output ("));
            out.print(lsm303_handling::serial_out_enabled);
            out.println(F(")"));
            out.print(F("\t 'A': set lsm303 serial output interval 'i65535' ("));
            out.print(lsm303_handling::serial_out_interval);
            out.println(F(")"));
            out.print(F("\t 'd': toggle lsm303 dmx output ("));
            out.print(lsm303_handling::dmx_send_enabled);
            out.println(F(")"));
            out.print(F("\t 'D': set lsm303 dmx send interval 'i65535' ("));
            out.print(lsm303_handling::dmx_send_interval);
            out.println(F(")"));
            out.print(F("\t 'e': toggle dmx serial out ("));
            out.print(dmx_handling::serial_out_enabled);
            out.println(F(")"));
            out.print(F("\t 'E': set dmx serial out interval 'i65535' ("));
            out.print(dmx_handling::serial_out_interval);
            out.println(F(")"));
            out.println(F("\t 't': test send values 't255'"));
            out.println(F("\t 'T': test fill filters 't-30000'"));
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

            // dmx_handling::send_int16_mapped_to_uint8(
            //     dmx_handling::ch_heading,
            //     lsm303_handling::filter_heading.get_filterd_value(),
            //      0, 359);
            //
            // dmx_handling::send_int16_mapped_to_uint8(
            //     dmx_handling::ch_temp,
            //     lsm303_handling::filter_temp.get_filterd_value(),
            //     -50, 50);

            // uint16_t value = 65535;
            // value = atoi(&command[1]);
            // out.print(F("value: "));
            // out.print(value);
            // out.println();
            //
            // out.print(F("1: "));
            // out.print((byte)value);
            // out.println();
            //
            // out.print(F("2: "));
            // out.print((byte)(value>>8));
            // out.println();
            //
            // out.println();

            out.println(F("__________"));
        } break;
        //-------------------------------------------------------------
        // case 'q': {
        //     out.println(F("print values:"));
        //     out.println(F(" x_raw"));
        //     slight_DebugMenu::print_int16_array(out, x_raw, x_size);
        //     out.println();
        //     out.println(F(" x_sorted"));
        //     slight_DebugMenu::print_int16_array(out, x_sorted, x_size);
        //     out.println();
        // } break;
        // case 'w': {
        //     out.print(F("add one new value:"));
        //     size_t value = random(0, 255);
        //     out.print(value);
        //     out.println();
        //     x_filter.add_value(value);
        // } break;
        // case 'e': {
        //     out.print(F("print filterd value:"));
        //     out.print(x_filter.get_filterd_value());
        //     out.println();
        // } break;
        //-------------------------------------------------------------
        case 'r': {
            out.println(F("\t toggle lsm303 read enable"));
            lsm303_handling::read_enabled =
                !lsm303_handling::read_enabled;
        } break;
        case 'R': {
            out.print(F("\t set lsm303 read interval "));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            lsm303_handling::read_interval = value;
        } break;
        case 'a': {
            out.println(F("\t toggle lsm303 serial output"));
            lsm303_handling::serial_out_enabled =
                !lsm303_handling::serial_out_enabled;
        } break;
        case 'A': {
            out.print(F("\t set lsm303 serial output interval "));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            lsm303_handling::serial_out_interval = value;
        } break;
        case 'd': {
            out.println(F("\t toggle lsm303 dmx send"));
            lsm303_handling::dmx_send_enabled =
                !lsm303_handling::dmx_send_enabled;
        } break;
        case 'D': {
            out.print(F("\t set lsm303 dmx send interval "));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            lsm303_handling::dmx_send_interval = value;
        } break;
        case 'e': {
            out.println(F("\t toggle dmx serial out"));
            dmx_handling::serial_out_enabled =
                !dmx_handling::serial_out_enabled;
        } break;
        case 'E': {
            out.print(F("\t set dmx serial out interval "));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            dmx_handling::serial_out_interval = value;
        } break;
        case 't': {
            out.print(F("\t test send dmx values "));
            // int16_t value = filter_a_y.get_filterd_value();
            uint8_t value = 0;
            value = dmx_handling::map_int16_to_uint8(
                lsm303_handling::filter_a_y.get_filterd_value(), -17000, 17000);
            if (&command[1] != '\0') {
                // value = random(-10000, +30000);
                value = atoi(&command[1]);
            }
            out.print(value);
            // out.print(F(" = "));
            // out.print(uint16_t(value));
            // out.print(F(" = "));
            // out.print(uint8_t(value));
            // out.print(F(" : "));
            // out.print(uint8_t(value >> 8));
            out.println();

            // dmx_handling::dmx_send_int16(dmx_handling::ch_a_x, value);
            // dmx_handling::dmx_send_int16(dmx_handling::ch_a_y, value);
            // dmx_handling::dmx_send_int16(dmx_handling::ch_a_z, value);
            // dmx_handling::dmx_send_int16(dmx_handling::ch_m_x, value);
            // dmx_handling::dmx_send_int16(dmx_handling::ch_m_y, value);
            // dmx_handling::dmx_send_int16(dmx_handling::ch_m_z, value);
            // dmx_handling::dmx_send_int16(dmx_handling::ch_heading, value);
            // dmx_handling::dmx_send_int16(dmx_handling::ch_temp, value);
            dmx_handling::send_uint8(dmx_handling::ch_a_x, value);
            dmx_handling::send_uint8(dmx_handling::ch_a_y, value);
            dmx_handling::send_uint8(dmx_handling::ch_a_z, value);
            dmx_handling::send_uint8(dmx_handling::ch_heading, value);
            dmx_handling::send_uint8(dmx_handling::ch_temp, value);
        } break;
        case 'T': {
            out.print(F("\t test fill filters "));
            // int16_t value = filter_a_y.get_filterd_value();
            int16_t value = random(0, 255);
            if (&command[1] != '\0') {
                value = atoi(&command[1]);
            }
            out.print(value);
            out.println();

            for (size_t i = 0; i < lsm303_handling::filter_size; i++) {
                lsm303_handling::filter_a_x.add_value(value);
                lsm303_handling::filter_a_y.add_value(value);
                lsm303_handling::filter_a_z.add_value(value);
                // lsm303_handling::filter_m_x.add_value(value);
                // lsm303_handling::filter_m_y.add_value(value);
                // lsm303_handling::filter_m_z.add_value(value);
                lsm303_handling::filter_heading.add_value(value);
                // lsm303_handling::filter_temp.add_value(value);
            }
            out.print(F("\t ... filled "));
            out.print(lsm303_handling::filter_size);
            out.println();
            lsm303_handling::filter_a_x.update();
            lsm303_handling::filter_a_y.update();
            lsm303_handling::filter_a_z.update();
            // lsm303_handling::filter_m_x.update();
            // lsm303_handling::filter_m_y.update();
            // lsm303_handling::filter_m_z.update();
            lsm303_handling::filter_heading.update();
            // lsm303_handling::filter_temp.update();
            out.println(F("\t all updated"));
            lsm303_handling::dmx_send();
            out.println(F("\t dmx_send"));
            out.println(F("\t done."));
        } break;
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
        // out.println(F("\t set button pin"));
        // pinMode(button_pin, INPUT_PULLUP);
        // out.println(F("\t button begin"));
        // button.begin();
        for (size_t index = 0; index < buttons_COUNT; index++) {
            pinMode(buttons[index].getPin(), INPUT_PULLUP);
            buttons[index].begin();
        }
    }
    out.println(F("\t finished."));
}

void buttons_update() {
    for (size_t index = 0; index < buttons_COUNT; index++) {
        buttons[index].update();
    }
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

    dmx_handling::setup(DebugOut);

    lsm303_handling::setup(DebugOut);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // go
    DebugOut.println(F("Loop:"));

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main loop
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // menu input
    myDebugMenu.update();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // update sub parts
    buttons_update();

    lsm303_handling::update(DebugOut);
    dmx_handling::update(DebugOut);

    handle_debugout(DebugOut);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
