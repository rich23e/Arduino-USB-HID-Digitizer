#pragma once

#include <Arduino.h>

#if defined(USBCON)
#include "PluggableUSB.h"

class Digitizer_ : public PluggableUSBModule
{
public:
    Digitizer_();

    void begin();
    void end();

    void press(uint16_t x, uint16_t y);
    void moveTo(uint16_t x, uint16_t y);
    void release();

    void tap(uint16_t x, uint16_t y, uint16_t holdMs = 80);
    void swipe(uint16_t startX, uint16_t startY,
               uint16_t endX, uint16_t endY,
               uint16_t durationMs = 250,
               uint8_t steps = 20);

    uint16_t mapAnalog(uint16_t reading, bool invert = false) const;
    uint16_t mapAnalog(uint16_t reading,
                       uint16_t inputMinimum,
                       uint16_t inputMaximum,
                       bool invert = false) const;

    bool isPressed() const;

protected:
    int getInterface(uint8_t* interfaceCount) override;
    int getDescriptor(USBSetup& setup) override;
    bool setup(USBSetup& setup) override;
    uint8_t getShortName(char* name) override;

private:
    void sendInputReport(bool force = false);
    static uint16_t limitCoordinate(uint32_t coordinate);

    uint8_t endpointType_[1];
    uint8_t protocol_;
    uint8_t idle_;

    uint16_t x_;
    uint16_t y_;
    bool pressed_;

    uint16_t lastSentX_;
    uint16_t lastSentY_;
    bool lastSentPressed_;
    bool reportSent_;
};

extern Digitizer_ Digitizer;

#endif
