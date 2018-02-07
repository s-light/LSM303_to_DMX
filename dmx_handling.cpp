/******************************************************************************

    dmx_handling

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

#include "./dmx_handling.h"

namespace dmx_handling {
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool dmx_valid = false;
uint16_t dmx_start_channel = 1;
bool effect_control = false;


// const size_t values_count = dmx_maxchannel_count / (int16_t);
// const size_t values_count = 8;
size_t values_dirty = 0b00000000;
// int16_t values[values_count];
uint8_t values[values_count];

bool serial_out_enabled = false;
uint32_t serial_out_timestamp_last = 0;
uint16_t serial_out_interval = 1000;


void print_values(Print &out);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


size_t chname2chindex(channel_names name) {
    return (name*2);
}


void send_uint8(channel_names name, uint8_t value) {
    values[name] = value;
    DMXSerial.write(uint16_t(name) + 1, value);
}

void send_uint16(size_t ch, uint16_t value) {
    DMXSerial.write(ch + 0, uint8_t(value >> 8));
    DMXSerial.write(ch + 1, uint8_t(value));
}

void send_int16(size_t ch, int16_t value) {
    send_uint16(ch, uint16_t(value));
}

void send_int16(channel_names name, int16_t value) {
    values[name] = value;
    send_uint16(chname2chindex(name) + 1, uint16_t(value));
}

uint8_t map_int16_to_uint8(int16_t value, int16_t low, int16_t high) {
    return map(constrain(value, low, high), low, high, 0, 255);
}

void send_int16_mapped_to_uint8(
    channel_names name,
    int16_t value,
    int16_t low,
    int16_t high
) {
    uint8_t value_uint8 = map_int16_to_uint8(value, low, high);
    values[name] = value_uint8;
    DMXSerial.write(uint16_t(name) + 1, value_uint8);
}




void print_values(Print &out) {
    char line[100];
    // snprintf(
    //     line,
    //     sizeof(line),
    //     "A: %6d %6d %6d M: %6d %6d %6d H: %6d T: %6d",
    //     values[ch_a_x],
    //     values[ch_a_y],
    //     values[ch_a_z],
    //     values[ch_m_x],
    //     values[ch_m_y],
    //     values[ch_m_z],
    //     values[ch_heading],
    //     values[ch_temp]);
    snprintf(
        line,
        sizeof(line),
        "dmx -> A: %3u %3u %3u H: %3u T: %3u",
        values[ch_a_x],
        values[ch_a_y],
        values[ch_a_z],
        values[ch_heading],
        values[ch_temp]);
    out.println(line);
}

void print_raw(Print &out) {
    char line[100];
    snprintf(
        line,
        sizeof(line),
        "[%3u, %3u, %3u, %3u, %3u, %3u, %3u, %3u, %3u, %3u]",
        DMXSerial.read(1),
        DMXSerial.read(2),
        DMXSerial.read(3),
        DMXSerial.read(4),
        DMXSerial.read(5),
        DMXSerial.read(6),
        DMXSerial.read(7),
        DMXSerial.read(8),
        DMXSerial.read(9),
        DMXSerial.read(10));
    out.println(line);
}


// private functions
void handle_new_values() {
    // for (size_t i = 0; i < values_count; i = i + 2) {
    //     if (bitRead(values_dirty, i)) {
    //         // clear
    //         bitClear(values_dirty, i);
    //         // handle
    //     }
    // }
    // if (effect_control) {
    //     if (bitRead(values_dirty, ch_a_x)) {
    //         // clear
    //         bitClear(values_dirty, ch_a_x);
    //         // handle
    //         sequencer_interval = map(values[ch_a_x], -17000, 17000, 0, 1000);
    //     }
    // }
}



void setup(Print &out) {
    out.println(F("setup DMX:"));

    // pinMode(dmx_pin_valid_led, OUTPUT);
    // digitalWrite(dmx_pin_valid_led, HIGH);
    // delay(100);
    // digitalWrite(dmx_pin_valid_led, LOW);
    // delay(100);
    // digitalWrite(dmx_pin_valid_led, HIGH);
    // delay(100);
    // digitalWrite(dmx_pin_valid_led, LOW);
    // delay(100);
    // digitalWrite(dmx_pin_valid_led, HIGH);

    // set to receive mode
    // Serial.println(F("\t init as DMXReceiver"));
    // DMXSerial.init(DMXReceiver, dmx_pin_direction);

    // set to send mode
    Serial.println(F("\t init as DMXController"));
    DMXSerial.init(DMXController, dmx_pin_direction);

    Serial.print(F("\t set maxChannel "));
    Serial.println(dmx_maxchannel_count);
    DMXSerial.maxChannel(dmx_maxchannel_count);

    // Serial.println(F("\t set some values"));
    // DMXSerial.write(10, 255);
    // DMXSerial.write(11, 255);
    // DMXSerial.write(12, 1);
    // read dmx values
    // DMXSerial.read(1);

    out.println(F("\t finished."));
}

void update(Print &out) {
    if (serial_out_enabled) {
        if (
            (millis() - serial_out_timestamp_last) > serial_out_interval
        ) {
            serial_out_timestamp_last =  millis();
            print_values(out);
            // print_raw(out);
        }
    }
    // bool dmx_valid_new = false;
    // if (DMXSerial.noDataSince() < dmx_valid_timeout) {
    //     dmx_valid_new = true;
    // }
    //
    // // check if dmx_valid state has changed
    // if (dmx_valid != dmx_valid_new) {
    //     dmx_valid = dmx_valid_new;
    //     // print_mode(lcd);
    //     // print_DMXSignal(lcd);
    // }
    //
    // if (dmx_valid) {
    //     // digitalWrite(dmx_pin_valid_led, LOW);
    //     // check if values are new
    //     // this triggers really to much with no changing data...
    //     // if (DMXSerial.dataUpdated()) {
    //     //     DMXSerial.resetUpdated();
    //     //     print_DMXValues(lcd);
    //     // }
    //
    //     // check if values are new
    //     for (size_t i = 0; i < values_count; i = i + 2) {
    //         uint16_t value_new = 0;
    //         value_new |= DMXSerial.read(dmx_start_channel + i + 0) << 8;
    //         value_new |= DMXSerial.read(dmx_start_channel + i + 1);
    //         if (values[i] != value_new) {
    //             values[i] = value_new;
    //             bitSet(values_dirty, i);
    //         } else {
    //             bitClear(values_dirty, i);
    //         }
    //     }
    //     if (values_dirty) {
    //         handle_new_values();
    //     }
    // } else {
    //     // digitalWrite(dmx_pin_valid_led, HIGH);
    // }
    //
    // // combine 16bit value
    // // uiDMXValue_Pan  = DMXSerial.read(11) << 8;
    // // uiDMXValue_Pan  = uiDMXValue_Pan | DMXSerial.read(12);
    // // read 8bit value
    // // uiDMXValue_Pan  = DMXSerial.read(12);
}

}  // namespace dmx_handling
