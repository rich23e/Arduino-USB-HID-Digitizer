#include "Digitizer.h"

#if defined(USBCON)

#include "DigitizerDescriptor.h"
#include "USBAPI.h"
#include "USBCore.h"

namespace
{
constexpr uint8_t HID_DESCRIPTOR_TYPE = 0x21;
constexpr uint8_t HID_REPORT_DESCRIPTOR_TYPE = 0x22;
constexpr uint8_t HID_SUBCLASS_NONE = 0;
constexpr uint8_t HID_PROTOCOL_NONE = 0;
constexpr uint8_t HID_REPORT_PROTOCOL = 1;

constexpr uint8_t HID_GET_REPORT = 0x01;
constexpr uint8_t HID_GET_IDLE = 0x02;
constexpr uint8_t HID_GET_PROTOCOL = 0x03;
constexpr uint8_t HID_SET_REPORT = 0x09;
constexpr uint8_t HID_SET_IDLE = 0x0A;
constexpr uint8_t HID_SET_PROTOCOL = 0x0B;

constexpr uint8_t STATUS_TIP_SWITCH = 0x01;
constexpr uint8_t STATUS_IN_RANGE = 0x02;
constexpr uint8_t CONTACT_ID_SINGLE = 0;

struct HIDClassDescriptor
{
    uint8_t length;
    uint8_t descriptorType;
    uint16_t hidVersion;
    uint8_t countryCode;
    uint8_t descriptorCount;
    uint8_t reportDescriptorType;
    uint16_t reportDescriptorLength;
} __attribute__((packed));

struct DigitizerInterfaceDescriptor
{
    InterfaceDescriptor interface;
    HIDClassDescriptor hid;
    EndpointDescriptor endpoint;
} __attribute__((packed));

// Field order must match DigitizerReportDescriptor.
struct DigitizerInputReport
{
    uint8_t reportId;
    uint8_t status;
    uint8_t contactId;
    uint16_t x;
    uint16_t y;
    uint8_t contactCount;
} __attribute__((packed));

static_assert(
    sizeof(DigitizerInputReport) == DIGITIZER_INPUT_REPORT_BYTES,
    "Digitizer input report size does not match the HID descriptor");
}

Digitizer_ Digitizer;

Digitizer_::Digitizer_()
    : PluggableUSBModule(1, 1, endpointType_),
      protocol_(HID_REPORT_PROTOCOL),
      idle_(1),
      x_(0),
      y_(0),
      pressed_(false),
      lastSentX_(0),
      lastSentY_(0),
      lastSentPressed_(false),
      reportSent_(false)
{
    endpointType_[0] = EP_TYPE_INTERRUPT_IN;
    PluggableUSB().plug(this);
}

void Digitizer_::begin()
{
    x_ = 0;
    y_ = 0;
    pressed_ = false;
    reportSent_ = false;
}

void Digitizer_::end()
{
    release();
}

void Digitizer_::press(uint16_t x, uint16_t y)
{
    x_ = limitCoordinate(x);
    y_ = limitCoordinate(y);
    pressed_ = true;
    sendInputReport();
}

void Digitizer_::moveTo(uint16_t x, uint16_t y)
{
    x_ = limitCoordinate(x);
    y_ = limitCoordinate(y);

    // While released, remember the position but do not send redundant
    // no-contact reports. The next press begins at this coordinate.
    if (pressed_)
        sendInputReport();
}

void Digitizer_::release()
{
    if (!pressed_)
        return;

    pressed_ = false;
    sendInputReport(true);
}

void Digitizer_::tap(uint16_t x, uint16_t y, uint16_t holdMs)
{
    press(x, y);
    delay(holdMs);
    release();
}

void Digitizer_::swipe(uint16_t startX, uint16_t startY,
                       uint16_t endX, uint16_t endY,
                       uint16_t durationMs, uint8_t steps)
{
    if (steps == 0)
        steps = 1;

    press(startX, startY);

    const uint16_t stepDelay = durationMs / steps;
    for (uint8_t step = 1; step <= steps; ++step)
    {
        const int32_t x = int32_t(startX) +
            ((int32_t(endX) - int32_t(startX)) * step) / steps;
        const int32_t y = int32_t(startY) +
            ((int32_t(endY) - int32_t(startY)) * step) / steps;

        moveTo(uint16_t(x), uint16_t(y));
        if (stepDelay != 0)
            delay(stepDelay);
    }

    release();
}

uint16_t Digitizer_::mapAnalog(uint16_t reading, bool invert) const
{
    return mapAnalog(reading, 0, 1023, invert);
}

uint16_t Digitizer_::mapAnalog(uint16_t reading,
                               uint16_t inputMinimum,
                               uint16_t inputMaximum,
                               bool invert) const
{
    if (inputMaximum <= inputMinimum)
        return 0;

    if (reading < inputMinimum)
        reading = inputMinimum;
    else if (reading > inputMaximum)
        reading = inputMaximum;

    uint32_t scaled = (uint32_t(reading - inputMinimum) *
                       DIGITIZER_COORDINATE_MAX) /
                      (inputMaximum - inputMinimum);

    if (invert)
        scaled = DIGITIZER_COORDINATE_MAX - scaled;

    return uint16_t(scaled);
}

bool Digitizer_::isPressed() const
{
    return pressed_;
}

void Digitizer_::sendInputReport(bool force)
{
    if (!force && reportSent_ &&
        x_ == lastSentX_ &&
        y_ == lastSentY_ &&
        pressed_ == lastSentPressed_)
    {
        return;
    }

    const DigitizerInputReport report =
    {
        DIGITIZER_REPORT_ID,
        pressed_ ? uint8_t(STATUS_TIP_SWITCH | STATUS_IN_RANGE) : uint8_t(0),
        CONTACT_ID_SINGLE,
        x_,
        y_,
        pressed_ ? uint8_t(1) : uint8_t(0)
    };

    USB_Send(
        pluggedEndpoint | TRANSFER_RELEASE,
        &report,
        sizeof(report));

    lastSentX_ = x_;
    lastSentY_ = y_;
    lastSentPressed_ = pressed_;
    reportSent_ = true;
}

uint16_t Digitizer_::limitCoordinate(uint32_t coordinate)
{
    return coordinate > DIGITIZER_COORDINATE_MAX
        ? DIGITIZER_COORDINATE_MAX
        : coordinate;
}

int Digitizer_::getInterface(uint8_t* interfaceCount)
{
    *interfaceCount += 1;

    const DigitizerInterfaceDescriptor descriptor =
    {
        D_INTERFACE(
            pluggedInterface,
            1,
            USB_DEVICE_CLASS_HUMAN_INTERFACE,
            HID_SUBCLASS_NONE,
            HID_PROTOCOL_NONE),

        {
            9,
            HID_DESCRIPTOR_TYPE,
            0x0111,
            0,
            1,
            HID_REPORT_DESCRIPTOR_TYPE,
            DigitizerReportDescriptorLength
        },

        D_ENDPOINT(
            USB_ENDPOINT_IN(pluggedEndpoint),
            USB_ENDPOINT_TYPE_INTERRUPT,
            USB_EP_SIZE,
            5)
    };

    return USB_SendControl(0, &descriptor, sizeof(descriptor));
}

int Digitizer_::getDescriptor(USBSetup& setup)
{
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE)
        return 0;

    if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE)
        return 0;

    if (setup.wIndex != pluggedInterface)
        return 0;

    protocol_ = HID_REPORT_PROTOCOL;

    return USB_SendControl(
        TRANSFER_PGM,
        DigitizerReportDescriptor,
        DigitizerReportDescriptorLength);
}

bool Digitizer_::setup(USBSetup& setup)
{
    if (setup.wIndex != pluggedInterface)
        return false;

    if (setup.bmRequestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
    {
        switch (setup.bRequest)
        {
            case HID_GET_REPORT:
                return true;

            case HID_GET_PROTOCOL:
                USB_SendControl(0, &protocol_, 1);
                return true;

            case HID_GET_IDLE:
                USB_SendControl(0, &idle_, 1);
                return true;
        }
    }

    if (setup.bmRequestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
    {
        switch (setup.bRequest)
        {
            case HID_SET_PROTOCOL:
                protocol_ = setup.wValueL;
                return true;

            case HID_SET_IDLE:
                idle_ = setup.wValueL;
                return true;

            case HID_SET_REPORT:
                return true;
        }
    }

    return false;
}

uint8_t Digitizer_::getShortName(char* name)
{
    name[0] = 'D';
    name[1] = 'I';
    name[2] = 'G';
    return 3;
}

#endif
