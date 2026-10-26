#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

constexpr uint8_t DIGITIZER_REPORT_ID = 1;
constexpr uint16_t DIGITIZER_COORDINATE_MAX = 32767;
constexpr uint8_t DIGITIZER_INPUT_REPORT_BYTES = 8;

extern const uint8_t DigitizerReportDescriptor[] PROGMEM;
extern const uint16_t DigitizerReportDescriptorLength;
