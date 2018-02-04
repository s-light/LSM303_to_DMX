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
******************************************************************************/

#include "./slight_filter.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// median filter
// based on
// http://www.elcojacobs.com/eleminating-noise-from-sensor-readings-on-arduino-with-digital-filtering/
// mode = median + averag
// median
// int sortedValues[NUM_READS];
// for(int i=0;i<NUM_READS;i++){
//   int value = analogRead(sensorpin);
//   int j;
//   if(value<sortedValues[0] || i==0){
//      j=0; //insert at first position
//   }
//   else{
//     for(j=1;j<i;j++){
//        if(sortedValues[j-1]<=value && sortedValues[j]>=value){
//          // j is insert position
//          break;
//        }
//     }
//   }
//   for(int k=i;k>j;k--){
//     // move all values higher than current reading up one position
//     sortedValues[k]=sortedValues[k-1];
//   }
//   sortedValues[j]=value; //insert current reading
// }
// averag
// return scaled mode of 10 values
// float returnval = 0;
// for(int i=NUM_READS/2-5;i<(NUM_READS/2+5);i++){
//   returnval +=sortedValues[i];
// }
// returnval = returnval/10;
// ##########################
// 480,  477,  476,  476,  476,  475,  476,  475,  475,  478,  475,  477,  480,
// 475,  476,  476,  474,  478,  473,  475,  472,  478,  472,  470,  476,  475,
// 479,  476,  476,  473,  474,  475,  473,  476,  477,  474,  472,  474,  477,
// 474,  476,  477,  478,  473,  472,  475,  476,  475,  474,    0
//
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
//         for (
//             insert_pos = 1;
//             insert_pos < dataout_filter_index;
//             insert_pos++
//         ) {
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





//
