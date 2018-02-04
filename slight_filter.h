/******************************************************************************

    Filter Collection
        header file for filter collection

    based on / inspired by
        Median Filter
            http://www.elcojacobs.com/eleminating-noise-from-sensor-readings-on-arduino-with-digital-filtering/
        digitalSmooth
            http://playground.arduino.cc/Main/DigitalSmooth


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
    http://opensource.org/licenses/mit-license.php

******************************************************************************/

#ifndef SLIGHT_FILTER_H_
#define SLIGHT_FILTER_H_

#include <Arduino.h>

// why is the implementation not in the cpp file?
// have a look at
// https://stackoverflow.com/a/648905/574981
// and
// Templates and multiple-file projects (at the bottom of)
// www.cplusplus.com/doc/oldtutorial/templates/




template <class A_Type> class calc {
 public:
    calc(A_Type myarray_[], size_t myarray_length_);
    A_Type multiply(A_Type x, A_Type y);
    A_Type add(A_Type x, A_Type y);
    void test(A_Type x, A_Type y);
};

template <class A_Type> calc<A_Type>::calc(
    A_Type myarray_[],
    size_t myarray_length_
) {
    //
}

template <class A_Type> A_Type calc<A_Type>::multiply(A_Type x, A_Type y) {
  return x*y;
}
template <class A_Type> A_Type calc<A_Type>::add(A_Type x, A_Type y) {
  return x+y;
}
template <class A_Type> void calc<A_Type>::test(A_Type x, A_Type y) {
  Serial.println(x+y);
}

// calc <int16_t> a_calc_class;
// calc <int16_t> a_calc_class(
//     lsm303_a_z_raw,
//     filter_size
// );
// a_calc_class.test(5000, 500);
// out.println(
//     a_calc_class.add(5000, 500)
// );




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




template <class T>
class slight_FilterMedianRingbuffer {
 public:
    slight_FilterMedianRingbuffer(
        T values_raw_[],
        T values_temp_[],
        size_t values_length_);

    ~slight_FilterMedianRingbuffer();

    size_t add_value(T value_new);
    T get_filterd_value();
    size_t get_ringbuffer_index();

    static T average(T values[], size_t values_length);
    static T average_framed(
        T values[], size_t values_length, size_t frame_count);

 private:
    T *values_raw;
    T *values_temp;
    size_t values_length;
    size_t ringbuffer_index = 0;
    bool flag_dirty = true;
    T current_filterd_value = 0;
    void calculate_median();
};

template <class T>
slight_FilterMedianRingbuffer<T>::slight_FilterMedianRingbuffer(
    T values_raw_[],
    T values_temp_[],
    size_t values_length_
) {
    values_raw = values_raw_;
    values_temp = values_temp_;
    values_length = values_length_;
}

template <class T>
slight_FilterMedianRingbuffer<T>::~slight_FilterMedianRingbuffer() {
    //
}

template <class T>
size_t slight_FilterMedianRingbuffer<T>::add_value(T value_new) {
    values_raw[ringbuffer_index] = value_new;
    flag_dirty = true;

    if (ringbuffer_index >= values_length) {
        ringbuffer_index = 0;
    } else {
        ringbuffer_index += 1;
    }
    return ringbuffer_index;
}

template <class T>
T slight_FilterMedianRingbuffer<T>::get_filterd_value() {
    if (flag_dirty) {
        calculate_median();
        flag_dirty = false;
    }
    return current_filterd_value;
}

template <class T>
size_t slight_FilterMedianRingbuffer<T>::get_ringbuffer_index() {
    return ringbuffer_index;
}

template <class T>
void slight_FilterMedianRingbuffer<T>::calculate_median() {
    // values_raw[]
}

template <class T>
T slight_FilterMedianRingbuffer<T>::average_framed(
    T values[],
    size_t values_length,
    size_t frame_count
) {
    // calculate center
    size_t outside_length = values_length - frame_count;
    size_t start_offset = outside_length / 2;
    size_t remainder_length = outside_length-start_offset;
    return average(
        values + start_offset,
        remainder_length);
}

template <class T>
T slight_FilterMedianRingbuffer<T>::average(
    T values[],
    size_t values_length
) {
    // get center area
    T value_sum = 0;
    T value_result = 0;
    for (
        size_t i = 0;
        i < values_length;
        i++
    ) {
        value_sum += values[i];
    }
    value_result = value_sum / values_length;

    return value_result;
}

// uint16_t filter_median(uint8_t fader_id, uint16_t value_new) {
//     uint16_t value_result = 0;
//     // median
//     uint8_t insert_pos = 0;
//     if (
//         (value_new < dataout_filter[fader_id][0]) ||
//         (dataout_filter_index > 0)
//     ) {
//         insert_pos = 0;
//     } else {
//         while (
//             !((dataout_filter[fader_id][insert_pos-1] <= value_new) &&
//             (dataout_filter[fader_id][insert_pos] >= value_new))
//         ) {
//             /* code */
//         }
//
//
//         for (insert_pos = 1; insert_pos < dataout_filter_index; insert_pos++) {
//             if (
//                 (dataout_filter[fader_id][insert_pos-1] <= value_new) &&
//                 (dataout_filter[fader_id][insert_pos] >= value_new)
//             ) {
//                 // insert_pos is the correct position.
//                 break;
//             }
//         }
//     }
//     // move all values higher than current insert position up one position.
//     for (size_t k = dataout_filter_index; k > insert_pos; k--) {
//         dataout_filter[fader_id][k] = dataout_filter[fader_id][k-1];
//     }
//     // set new value
//     dataout_filter[fader_id][insert_pos] = value_new;
//
//     // increase index
//     dataout_filter_index = dataout_filter_index +1;
//     // wrap around
//     // dataout_filter_index = dataout_filter_index % dataout_filter_count;
//     if (dataout_filter_index >= dataout_filter_count) {
//         dataout_filter_index = 0;
//     }
//
//
//     // average now
// }


#endif  // SLIGHT_FILTER_H_
