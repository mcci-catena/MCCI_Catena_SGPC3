/*

Module: MCCI_Catena_SGPC3.cpp

Function:
    Implementation functions for the Catena library for the Sensirion SGPC3 TVOC sensor.

Copyright and License:
    See accompanying LICENSE file.

Author:
    Terry Moore, MCCI Corporation   May 2020

*/

/// \file

#include "../MCCI_Catena_SGPC3.h"

using namespace McciCatenaSGPC3;

/// \brief  Initialze the SGPC3, and fetch the feature set. 
/// \details
///     This function fetches the version of the SGPC3. If the version suitable,
///     the chip is initialized for continuous operation in the specified power mode.
///     Remember that Sensirion says it's a bad idea to switch modes on a given device,
///     but also remember that we have no good way to know what mode a device is in
///     by examining the device. On return, it's a
cSGPC3::Error_t cSGPC3::begin(PowerMode_t mode)
    {
    // treat this as a chip reset.
    this->handleChipReset();

    // get the version
    std::uint16_t featureSet;
    std::uint8_t productVersion;
    auto result = this->sendAndGetSynchronous<Command_t::get_feature_set_version>(featureSet);

    this->m_featureSet = 0;

    if (! isSuccess(result))
        return result;

    if (featureSet_getProductType(featureSet) != ProductType_t::SGPC3)
        return Error_t::WrongDeviceType;

    // sample code checks version 4; but this library is only tested with version 6.
    productVersion = featureSet_getProductVersion(featureSet);
    if (productVersion < 6)
        return Error_t::WrongDeviceType;

    this->m_featureSet = productVersion;

    // set the mode.
    result = this->set_power_mode_synchronous(mode);

    // start the sensor
    result = this->tvoc_init_continuous();

    return result;
    }

/// \details
///     The command is sent to the sensor, and the appropriate delay is inserted.
///
cSGPC3::Error_t cSGPC3::sendCommand(
    cSGPC3::Command_t c,
    const std::uint8_t *pParamBytes,
    std::uint8_t *pResultBytes
    )
    {
    std::uint8_t i2c_result;
    const std::uint16_t cmd = getCommand(c);

    // wait for the bus to be available.
    auto tNow = millis();
    while (std::int32_t(this->m_tAvail - tNow < 0))
        tNow = millis();

    this->m_wire->beginTransmission(this->kAddress);
    this->m_wire->write(std::uint8_t(cmd >> 8));
    this->m_wire->write(std::uint8_t(cmd));

    if (pParamBytes != nullptr)
        {
        for (auto nParam = getParameterLength(c); nParam > 0; --nParam)
            {
            this->m_wire->write(*pParamBytes++);
            this->m_wire->write(*pParamBytes++);
            this->m_wire->write(*pParamBytes++);
            }
        }

    i2c_result = this->m_wire->endTransmission();

    // update available time.
    this->m_tAvail = tNow + getDelayMs(c) + 1;

    // check for success.
    if (i2c_result != 0)
        {
        if (this->isDebug())
            {
            Serial.print("sendCommand: error writing command 0x");
            Serial.print(cmd, HEX);
            Serial.print(", i2c result: ");
            Serial.println(i2c_result);
            }
        return Error_t::WriteError;
        }

    // wait.
    delay(getDelayMs(c));

    auto const nResult = getResponseLength(c);
    if (nResult == 0)
        return Error_t::Success;

    // now we need to read the response
    std::uint8_t nReadFrom = this->m_wire->requestFrom(this->kAddress, nResult * 3);
    if (nReadFrom != nResult * 3)
        {
        if (this->isDebug())
            {
            Serial.print("sendCommand: nReadFrom(");
            Serial.print(unsigned(nReadFrom));
            Serial.print(") != nBuf(");
            Serial.print(nResult * 3);
            Serial.println(")");
            }
        return Error_t::ReadError;
        }

    return Error_t::Success;
    }

/// \param c [in]       Description of the command.
cSGPC3::Error_t cSGPC3::sendCommandBare(cSGPC3::Command_t c)
    {
    return this->sendCommand(c, nullptr, nullptr);
    }

/// \param c [in]       Description of the command.
/// \param param [in]   The parameter to accompany the command.
/// \details
///     We depend on the caller to have used one of the standard command codes;
///     in that case, it's not possible for us to be confused about how many bytes
///     to send or receive. We assume that the command requires exactly 3 bytes of
///     parameters data..
cSGPC3::Error_t cSGPC3::sendCommandWithParam(cSGPC3::Command_t c, std::uint16_t param)
    {
    std::uint8_t paramBuf[3];

    this->putbe16(paramBuf, param);
    paramBuf[2] = this->crc(paramBuf, 2);
    return this->sendCommand(c, paramBuf, nullptr);
    }

/// \param c [in]       Description of the command.
/// \param response [out]  Set to the 16-bit response (if no errors).
/// \details
///     We depend on the caller to have used one of the standard command codes;
///     in that case, it's not possible for us to be confused about how many bytes
///     to send or receive. We assume that the command returns exactly 3 bytes of
///     result data.
cSGPC3::Error_t cSGPC3::sendCommandWithResponse(cSGPC3::Command_t c, std::uint16_t &response)
    {
    std::uint8_t responseBuf[3];

    auto result = this->sendCommand(c, nullptr, responseBuf);

    if (! this->isSuccess(result))
        return result;

    if (this->crc(responseBuf, 2) != responseBuf[2])
        return Error_t::BadCRC;

    response = this->getbe16(responseBuf);
    return Error_t::Success;
    }

/// \param c [in]       Description of the command.
/// \param response1 [out]  Set to the first 16-bit response (if no errors).
/// \param response2 [out]  Set to the second 16-bit response (if no errors).
/// \details
///     We depend on the caller to have used one of the standard command codes;
///     in that case, it's not possible for us to be confused about how many bytes
///     to send or receive. We assume that the command requires exactly 6 bytes of
///     result data.
cSGPC3::Error_t cSGPC3::sendCommandWithTwoResponses(cSGPC3::Command_t c, std::uint16_t &response1, std::uint16_t &response2)
    {
    std::uint8_t responseBuf[6];

    auto result = this->sendCommand(c, nullptr, responseBuf);

    if (! this->isSuccess(result))
        return result;

    if (this->crc(responseBuf, 2) != responseBuf[2])
        return Error_t::BadCRC;
    if (this->crc(responseBuf + 3, 2) != responseBuf[5])
        return Error_t::BadCRC;

    response1 = this->getbe16(responseBuf);
    response2 = this->getbe16(responseBuf + 3);
    return Error_t::Success;
    }

/// \param c [in]       Description of the command.
/// \param response [out]  Set to the response recieved for the command (if no errors).
///                     The high-order 16 bits of the response are zero, and the remaining
///                     48 bits are set to the response received from the device.
/// \details
///     We depend on the caller to have used one of the standard command codes;
///     in that case, it's not possible for us to be confused about how many bytes
///     to send or receive. We assume that the command requires exactly 9 bytes of
///     result data.
cSGPC3::Error_t cSGPC3::sendCommandWithThreeResponses(cSGPC3::Command_t c, std::uint64_t &response)
    {
    std::uint8_t responseBuf[9];

    auto result = this->sendCommand(c, nullptr, responseBuf);

    if (! this->isSuccess(result))
        return result;

    if (this->crc(responseBuf, 2) != responseBuf[2])
        return Error_t::BadCRC;
    if (this->crc(responseBuf + 3, 2) != responseBuf[5])
        return Error_t::BadCRC;
    if (this->crc(responseBuf + 6, 2) != responseBuf[8])
        return Error_t::BadCRC;

    response = (std::uint64_t(this->getbe16(responseBuf)) << 32) |
               (std::uint64_t(this->getbe16(responseBuf + 3)) << 16) |
               this->getbe16(responseBuf + 6);

    return Error_t::Success;
    }

/// \param buf [in]     Buffer to be CRC'ed.
/// \param nBuf [in]    Number of bytes in buffer.
/// \param crc8 [in]    The initial CRC value; normally the default is used.
///
/// \details
///     This routine updates the CRC 4 bits at a time. Althought it's always called
///     for exactly two bytes, there's nothing in this routine that enforces that.
std::uint8_t cSGPC3::crc(const std::uint8_t * buf, size_t nBuf, std::uint8_t crc8)
    {
    /* see cSHT3x CRC-8-Calc.md for a little info on this */
    static const std::uint8_t crcTable[16] =
        {
        0x00, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97,
        0xb9, 0x88, 0xdb, 0xea, 0x7d, 0x4c, 0x1f, 0x2e,
        };

    for (size_t i = nBuf; i > 0; --i, ++buf)
        {
        uint8_t b, p;

        // calculate first nibble
        b = *buf;
        p = (b ^ crc8) >> 4;
        crc8 = (crc8 << 4) ^ crcTable[p];

        // calculate second nibble
        // this could be written as:
        //      b <<= 4;
        //      p = (b ^ crc8) >> 4;
        // but it's more effective as:
        p = ((crc8 >> 4) ^ b) & 0xF;
        crc8 = (crc8 << 4) ^ crcTable[p];
        }

    return crc8;
    }
