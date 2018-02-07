/******************************************************************************

    lsm303_handling

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

#include "./lsm303_handling.h"

namespace lsm303_handling {
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LSM303 compass;

bool read_enabled = true;
uint32_t read_timestamp_last = 0;
uint16_t read_interval = 20;

bool serial_out_enabled = false;
uint32_t serial_out_timestamp_last = 0;
uint16_t serial_out_interval = 1000;

bool dmx_send_enabled = true;
uint32_t dmx_send_timestamp_last = 0;
uint16_t dmx_send_interval = 50;


int16_t filter_array_a_x_raw[filter_size];
int16_t filter_array_a_x_sorted[filter_size];
slight_FilterMedianRingbuffer <int16_t> filter_a_x(
    filter_array_a_x_raw,
    filter_array_a_x_sorted,
    filter_size,
    filter_average_frame_length
);
int16_t filter_array_a_y_raw[filter_size];
int16_t filter_array_a_y_sorted[filter_size];
slight_FilterMedianRingbuffer <int16_t> filter_a_y(
    filter_array_a_y_raw,
    filter_array_a_y_sorted,
    filter_size,
    filter_average_frame_length
);
int16_t filter_array_a_z_raw[filter_size];
int16_t filter_array_a_z_sorted[filter_size];
slight_FilterMedianRingbuffer <int16_t> filter_a_z(
    filter_array_a_z_raw,
    filter_array_a_z_sorted,
    filter_size,
    filter_average_frame_length
);

// int16_t filter_array_m_x_raw[filter_size];
// int16_t filter_array_m_x_sorted[filter_size];
// slight_FilterMedianRingbuffer <int16_t> filter_m_x(
//     filter_array_m_x_raw,
//     filter_array_m_x_sorted,
//     filter_size,
//     filter_average_frame_length
// );
// int16_t filter_array_m_y_raw[filter_size];
// int16_t filter_array_m_y_sorted[filter_size];
// slight_FilterMedianRingbuffer <int16_t> filter_m_y(
//     filter_array_m_y_raw,
//     filter_array_m_y_sorted,
//     filter_size,
//     filter_average_frame_length
// );
// int16_t filter_array_m_z_raw[filter_size];
// int16_t filter_array_m_z_sorted[filter_size];
// slight_FilterMedianRingbuffer <int16_t> filter_m_z(
//     filter_array_m_z_raw,
//     filter_array_m_z_sorted,
//     filter_size,
//     filter_average_frame_length
// );

int16_t filter_array_heading_raw[filter_size];
int16_t filter_array_heading_sorted[filter_size];
slight_FilterMedianRingbuffer <int16_t> filter_heading(
    filter_array_heading_raw,
    filter_array_heading_sorted,
    filter_size,
    filter_average_frame_length
);

// int16_t filter_array_temp_raw[filter_size];
// int16_t filter_array_temp_sorted[filter_size];
// slight_FilterMedianRingbuffer <int16_t> filter_temp(
//     filter_array_temp_raw,
//     filter_array_temp_sorted,
//     filter_size,
//     filter_average_frame_length
// );


void serial_out_print(Print &out);
void dmx_send();
void sensor_read();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup(Print &out) {
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

void update(Print &out) {
    if (read_enabled) {
        if ((millis() - read_timestamp_last) > read_interval) {
            read_timestamp_last =  millis();
            sensor_read();
        }
    }

    if (serial_out_enabled) {
        if ((millis() - serial_out_timestamp_last) > serial_out_interval) {
            serial_out_timestamp_last =  millis();
            serial_out_print(out);
        }
    }

    if (dmx_send_enabled) {
        if ((millis() - dmx_send_timestamp_last) > dmx_send_interval) {
            dmx_send_timestamp_last =  millis();
            dmx_send();
        }
    }
}

void sensor_read() {
    compass.read();
    filter_a_x.add_value(compass.a.x);
    filter_a_y.add_value(compass.a.y);
    filter_a_z.add_value(compass.a.z);
    filter_a_x.update();
    filter_a_y.update();
    filter_a_z.update();
    // filter_m_x.add_value(compass.m.x);
    // filter_m_y.add_value(compass.m.y);
    // filter_m_z.add_value(compass.m.z);
    // filter_m_x.update();
    // filter_m_y.update();
    // filter_m_z.update();
    filter_heading.add_value(compass.heading());
    filter_heading.update();
    // filter_temp.add_value(42);
    // filter_temp.update();
}


void serial_out_print(Print &out) {
        // char line[24];
        // snprintf(
        //     line,
        //     sizeof(line),
        //     "A: %6d %6d %6d;",
        //     compass.a.x,
        //     compass.a.y,
        //     compass.a.z
        // );

        // char line[60];
        // snprintf(
        //     line,
        //     sizeof(line),
        //     // "A: %6d %6d %6d; AF: %6d %6d %6d;",
        //     "A: %6d %6d",
        //     compass.a.y,
        //     filter_a_y.get_filterd_value()
        // );

        // char line[60];
        // snprintf(
        //     line,
        //     sizeof(line),
        //     "A: %6d %6d %6d F: %6d %6d %6d",
        //     compass.a.x,
        //     compass.a.y,
        //     compass.a.z,
        //     filter_a_x.get_filterd_value(),
        //     filter_a_y.get_filterd_value(),
        //     filter_a_z.get_filterd_value()
        // );


        // char line[60];
        // snprintf(
        //     line,
        //     sizeof(line),
        //     "A: %6d %6d %6d M: %6d %6d %6d H: %3d",
        //     filter_a_x.get_filterd_value(),
        //     filter_a_y.get_filterd_value(),
        //     filter_a_z.get_filterd_value(),
        //     compass.m.x,
        //     compass.m.y,
        //     compass.m.z,
        //     filter_heading.get_filterd_value()
        // );

        // char line[100];
        // snprintf(
        //     line,
        //     sizeof(line),
        //     "A: %6d %6d %6d M: %6d %6d %6d H: %6d T: %6d",
        //     filter_a_x.get_filterd_value(),
        //     filter_a_y.get_filterd_value(),
        //     filter_a_z.get_filterd_value(),
        //     filter_m_x.get_filterd_value(),
        //     filter_m_y.get_filterd_value(),
        //     filter_m_z.get_filterd_value(),
        //     filter_heading.get_filterd_value(),
        //     filter_temp.get_filterd_value());

        char line[100];
        snprintf(
            line,
            sizeof(line),
            "A: %6d %6d %6d H: %6d",
            filter_a_x.get_filterd_value(),
            filter_a_y.get_filterd_value(),
            filter_a_z.get_filterd_value(),
            filter_heading.get_filterd_value());

        out.println(line);
}


void dmx_send() {
    // dmx_handling::dmx_send_int16(
    //     dmx_handling::ch_a_x, filter_a_x.get_filterd_value());
    // dmx_handling::dmx_send_int16(
    //     dmx_handling::ch_a_y, filter_a_y.get_filterd_value());
    // dmx_handling::dmx_send_int16(
    //     dmx_handling::ch_a_z, filter_a_z.get_filterd_value());
    // dmx_handling::dmx_send_int16(
    //     dmx_handling::ch_m_x, filter_m_x.get_filterd_value());
    // dmx_handling::dmx_send_int16(
    //     dmx_handling::ch_m_y, filter_m_y.get_filterd_value());
    // dmx_handling::dmx_send_int16(
    //     dmx_handling::ch_m_z, filter_m_z.get_filterd_value());
    // dmx_handling::dmx_send_int16(
    //     dmx_handling::ch_heading, filter_heading.get_filterd_value());
    // dmx_handling::dmx_send_int16(
    //     dmx_handling::ch_temp, filter_temp.get_filterd_value());


    dmx_handling::send_int16_mapped_to_uint8(
        dmx_handling::ch_a_x,
        filter_a_x.get_filterd_value(),
        -17000, 17000);
    dmx_handling::send_int16_mapped_to_uint8(
        dmx_handling::ch_a_y,
        filter_a_y.get_filterd_value(),
        -17000, 17000);
    dmx_handling::send_int16_mapped_to_uint8(
        dmx_handling::ch_a_z,
        filter_a_z.get_filterd_value(),
        -17000, 17000);

    dmx_handling::send_int16_mapped_to_uint8(
        dmx_handling::ch_heading,
        filter_heading.get_filterd_value(),
         0, 359);

    // dmx_handling::send_int16_mapped_to_uint8(
    //     dmx_handling::ch_temp,
    //     filter_temp.get_filterd_value(),
    //     0, 255);
    dmx_handling::send_uint8(dmx_handling::ch_temp, 42);
}



}  // namespace lsm303_handling
