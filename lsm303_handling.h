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

#ifndef LSM303_HANDLING_H_
#define LSM303_HANDLING_H_

#include <Arduino.h>


#include <Wire.h>
#include <LSM303.h>

// #include <slight_filter.h>
#include "./slight_filter.h"

#include "./dmx_handling.h"

namespace lsm303_handling {

extern LSM303 compass;

const uint16_t read_interval = 20;
// 20ms = 50Hz = update rate for accelerometer

extern bool serial_out_enabled;
extern uint16_t serial_out_interval;

extern bool dmx_send_enabled;
extern uint16_t dmx_send_interval;


const size_t filter_size = 25;
const size_t filter_average_frame_length = 5;
extern int16_t filter_array_a_x_raw[filter_size];
extern int16_t filter_array_a_x_sorted[filter_size];
extern slight_FilterMedianRingbuffer <int16_t> filter_a_x;
extern int16_t filter_array_a_y_raw[filter_size];
extern int16_t filter_array_a_y_sorted[filter_size];
extern slight_FilterMedianRingbuffer <int16_t> filter_a_y;
extern int16_t filter_array_a_z_raw[filter_size];
extern int16_t filter_array_a_z_sorted[filter_size];
extern slight_FilterMedianRingbuffer <int16_t> filter_a_z;
extern int16_t filter_array_m_x_raw[filter_size];
extern int16_t filter_array_m_x_sorted[filter_size];
extern slight_FilterMedianRingbuffer <int16_t> filter_m_x;
extern int16_t filter_array_m_y_raw[filter_size];
extern int16_t filter_array_m_y_sorted[filter_size];
extern slight_FilterMedianRingbuffer <int16_t> filter_m_y;
extern int16_t filter_array_m_z_raw[filter_size];
extern int16_t filter_array_m_z_sorted[filter_size];
extern slight_FilterMedianRingbuffer <int16_t> filter_m_z;
extern int16_t filter_array_heading_raw[filter_size];
extern int16_t filter_array_heading_sorted[filter_size];
extern slight_FilterMedianRingbuffer <int16_t> filter_heading;
extern int16_t filter_array_temp_raw[filter_size];
extern int16_t filter_array_temp_sorted[filter_size];
extern slight_FilterMedianRingbuffer <int16_t> filter_temp;


// const size_t x_size = 6;
// int16_t x_raw[x_size];
// int16_t x_sorted[x_size];
// slight_FilterMedianRingbuffer <int16_t> x_filter(
//     x_raw,
//     x_sorted,
//     x_size
// );


void setup(Print &out);
void update(Print &out);

}  // namespace lsm303_handling

#endif  // LSM303_HANDLING_H_
