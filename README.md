# MCCI Catena&reg; SGPC3 Gas Sensor Library

This library provides a simple interface to Sensirion SGPC3 gas sensors. Although we tested this on the MCCI Catena 4630, there are no known dependencies on MCCI hardware; this should work equally well with Adafruit breakout boards, etc. This library is written, to the maximum extent practical, using pure C++, and avoiding introducing new `#define`'d or global namespace symbols.

**Contents:**

<!--
  This TOC uses the VS Code markdown TOC extension AlanWalk.markdown-toc.
  We strongly recommend updating using VS Code, the markdown-toc extension and the
  bierner.markdown-preview-github-styles extension. Note that if you are using
  VS Code 1.29 and Markdown TOC 1.5.6, https://github.com/AlanWalk/markdown-toc/issues/65
  applies -- you must change your line-ending to some non-auto value in Settings>
  Text Editor>Files.  `\n` works for me.
-->
<!-- markdownlint-disable MD033 MD004 -->
<!-- markdownlint-capture -->
<!-- markdownlint-disable -->
<!-- TOC depthFrom:2 updateOnSave:true -->

- [Introduction](#introduction)
- [Header File](#header-file)
- [Library Dependencies](#library-dependencies)
- [Example Scripts](#example-scripts)
- [Namespace](#namespace)

<!-- /TOC -->
<!-- markdownlint-restore -->
<!-- Due to a bug in Markdown TOC, the table is formatted incorrectly if tab indentation is set other than 4. Due to another bug, this comment must be *after* the TOC entry. -->

## Introduction

Clients interact with the gas sensor via the following sequence.

1. Initially, the client creates an instance object of type `cSGPC3` for the sensor. When creating the object, the client passes a `Wire` object (representing the I2C bus used for communication). The SGPC3 supports only a single I2C address, namely `0x58`. The sensor is initially in standby.

2. The SGPC3 does not a have a reset pin. It is reset either by power-cycling or by a soft reset.

3. If the client has a saved baseline, the user should load the value into the library using `cSGPC3::setBaseline()`.

4. To begin measurements, the client calls the `cSGPC3::begin()` method.

5. If the client only needs to take occasional measurements, the client calls either the `cSHT3x::getTemperatureHumidity()` method (which returns temperature and humidity scaled in engineering units), or `cSHT3x::getTemperatureHumidityRaw()` (which returns temperature and humidity as `uint16_t` unscaled values).  Generally, the former is used if data is to be processed locally on the Arduino, and the latter is used if data is to be transmitted via a LPWAN network.

6. If the client needs to make periodic measurements, the client first calls `cSHT3x::startPeriodicMeasurement()` to set the parameters for the periodic measurement, and start the acquisition process. The result of this call is the number of milliseconds per measurement.

   To collect results, the client occasionaly calls `cSHT3x::getPeriodicMeasurement()` or `cSHT3x::getPeriodicMeasurementRaw()`. If a measurement is available, it will be returned, and the method returns `true`; otherwise, the method returns `false`.  To save power, the client should delay the appropriate number of milliseconds between calls (as indicated by the result of `cSHT3x::startPeriodicMeasurement()`).

Measurements are returned in structures (`cSHT3x::Measurements` or `cSHT3x::MeasurementsRaw`, respectively.) These structures have some utility methods:

- `cSHT3x::Measurments::set(const cSHT3x::MeasurementsRaw &mRaw)` sets the target `Measurement` to the engineering-units equivalent of `mRaw`.
- `cSHT3x::Measurments::extract(float &t, float &rh) const` sets `t` to the temperature (in Celsius), and `rh` to the relative humidity (in percent).
- `cSHT3x::MeasurmentsRaw::extract(std::uint16_t &t, std::uint16_t &rh) const` sets `t` and `rh` to the raw measurement from the device.

A number of utility methods allow the client to manage the sensor.

- `cSHT3x::reset()` issues a soft reset to the device.
- `cSHT3x::end()` idles the device, and is typically used prior to sleeping the system.
- By default, the library checks CRCs on received data. `cSHT3x::getCrcMode()` and `cSHT3x::setCrcMode()` allow the client to query and change whether the library checks (`true`) or ignores (`false`) CRC.
- The sensor includes a heater that's intended for diagnostic purposes. (Turn on the heater, and make sure the temperature changes.) `cSHT3x::getHeater()` queries the current state of the heater, and `cSHT3x::setHeater(bool fOn)` turns it on or off.
- `cSHT3x::getStatus()` reads the current value of the status register. The value is returned as an opaque structure of type `cSHT3x::Status_t`. Methods are provided to allow clients to query individual bits. A status also has an explicit `invalid` state, which can be separately queried.
- For convenience, static methods are provided to convert between raw (`uint16_t`) data and engineering units. `cSHT3x::rawToCelsius()` and `cSHT3x::rawRHtoPercent()` convert raw data to engineering units. `cSHT3x::celsiusToRawT()` and `cSHT3x::percentRHtoRaw()` convert engineering units to raw data. (This may be useful for precalculating alarms, to save on floating point calculations at run time.)
- `cSHT3x::isDebug()` returns `true` if this is a debug build, `false` otherwise. It's a `constexpr`, so using this in an `if()` statement is equivalent to a `#if` -- the compiler will optimize away the code if this is not a debug build.

## Header File

```c++
#include <Catena-SHT3x.h>
```

## Library Dependencies

None, beyond the normal Arduino library `<Wire.h>`.  It can be used with [Catena-Arduino-Platform](https://github.com/mcci-catena/Catena-Arduino-Platform), but it doesn't require it.

## Example Scripts

See [sht3x-simple](./examples/sht3x-simple/sht3x-simple.ino). Sht3x-simple reads and displays the TVOC once a second, using the simple APIs.

![Screenshot of sht3x-simple.ino in operation](./assets/sht3x-simple-screenshot.png)

## Namespace

All definitions are wrapped in a namespace. Normally, after including the header file, you'll want to say:

```c++
using namespace McciCatenaSht3x;
```

