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

TODO: provide more information.

## Header File

```c++
#include <MCCI_Catena_SGPC3.h>
```

## Library Dependencies

None, beyond the normal Arduino library `<Wire.h>`.  It can be used with [Catena-Arduino-Platform](https://github.com/mcci-catena/Catena-Arduino-Platform), but it doesn't require it.

## Example Scripts


## Namespace

All definitions are wrapped in a namespace. Normally, after including the header file, you'll want to say:

```c++
using namespace McciCatenaSht3x;
```
