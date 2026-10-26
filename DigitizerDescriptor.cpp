#include "DigitizerDescriptor.h"

const uint8_t DigitizerReportDescriptor[] PROGMEM =
{
    0x05, 0x0D,        // Usage Page (Digitizers)
    0x09, 0x04,        // Usage (Touch Screen)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)

    0x09, 0x22,        //   Usage (Finger)
    0xA1, 0x02,        //   Collection (Logical)

    0x09, 0x42,        //     Usage (Tip Switch)
    0x09, 0x32,        //     Usage (In Range)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)

    0x75, 0x06,        //     Report Size (6)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x03,        //     Input (Constant, Variable, Absolute)

    0x09, 0x51,        //     Usage (Contact Identifier)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)

    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x16, 0x00, 0x00,  //     Logical Minimum (0)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)

    0xC0,              //   End Collection (Finger)

    0x05, 0x0D,        //   Usage Page (Digitizers)
    0x09, 0x54,        //   Usage (Contact Count)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)

    0xC0               // End Collection (Application)
};

const uint16_t DigitizerReportDescriptorLength =
    sizeof(DigitizerReportDescriptor);
