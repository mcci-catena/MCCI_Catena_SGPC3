/*

Module: MCCI_Catena_SGPC3.h

Function:
        Definitions for the Catena library for the Sensirion SGPC3 TVOC sensor.

Copyright and License:
        See accompanying LICENSE file.

Author:
        Terry Moore, MCCI Corporation   May 2020

*/

#ifndef _MCCI_Catena_SGPC3_h_
# define _MCCI_Catena_SGPC3_h_
# pragma once

/// \file

// AVR doesn't have <cstdint> but we want to use it... so work around it.
#ifndef __CATENA_HAVE_CSTDINT
# ifdef __AVR__
#  define _CATENA_HAVE_CSTDINT	0
# else
#  define _CATENA_HAVE_CSTDINT 1
# endif
#endif

#if _CATENA_HAVE_CSTDINT
# include <cstdint>
#else
# include <stdint.h>
namespace std {
  using ::int8_t;
  using ::uint8_t;
  using ::int16_t;
  using ::uint16_t;
  using ::int32_t;
  using ::uint32_t;
}
#endif

#include <Wire.h>


/// \brief Namespace used for all definitions in this module
///
/// \details
///     All names contributed by this library are in the namespace
///     \ref McciCatenaSGPC3. It's expected that most library clients will
///     use a statement like `using namespace McciCatenaSCPC3;` to
///     bring the names into the default namespace, but this is not
///     mandatory.
///
namespace McciCatenaSGPC3 {

/****************************************************************************\
|
|   Version boilerplate
|
\****************************************************************************/

/// \defgroup version Version numbers for the library
///
/// \brief The version number framework allows compile-time checks of library versions.
///
/// \details
///     It's often desirable to check at compile time that the library version
///     in use is up to date. The alternative may be compile errors that will
///     be puzzling to casual users.
///
///     The semantics of version numbers are based on [Semantic Versioning](www.semver.org) version 2,
///     extended to support a fourth field, "local", and omitting the pre-release
///     field. In general, the major, minor, and patch fields are reserved for
///     public (github) version tags. The local version is used for tagging version
///     numbers that are on HEAD or public branches, and therefore must be
///     distinguished. They're similar to pre-release tags, but they
///     represent points *after* a release, rather than points *before* a release.
///
/// \{

/// \brief Create a version number suitable for comparison.
///
/// \param      major   Major version number. See [Semantic Versioning V2.0](https://semver.org/spec/v2.0.0.html).
/// \param      minor   Minor version number. See [Semantic Versioning V2.0](https://semver.org/spec/v2.0.0.html).
/// \param      patch   Patch number. See [Semantic Versioning V2.0](https://semver.org/spec/v2.0.0.html).
/// \param      local   Local release (optional).
///
static constexpr std::uint32_t
makeVersion(
    std::uint8_t major, std::uint8_t minor, std::uint8_t patch, std::uint8_t local = 0
    )
    {
    return ((std::uint32_t)major << 24u) | ((std::uint32_t)minor << 16u) | ((std::uint32_t)patch << 8u) | (std::uint32_t)local;
    }

/// \brief Extract major number from version.
///
/// \details
///     This is only intended to be used when printing a version number.
static constexpr std::uint8_t
getMajor(std::uint32_t v)
    {
    return std::uint8_t(v >> 24u);
    }

/// \brief Extract minor number from version.
///
/// \details
///     This is only intended to be used when printing a version number.
static constexpr std::uint8_t
getMinor(std::uint32_t v)
    {
    return std::uint8_t(v >> 16u);
    }

/// \brief Extract patch number from version.
///
/// \details
///     This is only intended to be used when printing a version number.
static constexpr std::uint8_t
getPatch(std::uint32_t v)
    {
    return std::uint8_t(v >> 8u);
    }

/// \brief Extract local number from version.
///
/// \details
///     This is only intended to be used when printing a version number.
static constexpr std::uint8_t
getLocal(std::uint32_t v)
    {
    return std::uint8_t(v);
    }

/// \brief Official version of library, for use by clients in `static_assert()`
///
/// \details
///     Check that you have a version of the library that suits your needs
///     using a statement like this:
///
///     ```c++
///     static_assert(
///             McciCatenaSGPC3::kVersion >= McciCatenaSGPC3::makeVersion(0,1,0),
///             "MCCI_Catena_SGPC3 library version is out of date");
///     ```
static constexpr std::uint32_t kVersion = makeVersion(0,1,0,0);

//  endgroup version
/// \}

/****************************************************************************\
|
|   The sensor class.
|
\****************************************************************************/

/// \defgroup scpc3 SCPC3 Sensor APIs
/// \{

/// \brief Helper base class, defines relevant mechanisms for creating command constants.
///
/// \details
///     We want to avoid macros, and we want to use a \c constexpr function to define
///     the command constants that we use. It's hard to do this from within the class
///     that defines the constexpr. So ... 
class cSGPC3_cmds
    {
protected:
    enum class Command_t : std::uint32_t;                       // forward reference.
    static constexpr std::uint32_t kCommandMask = 0x3FFF << 0;
    static constexpr std::uint32_t kParamLenMask = 0x1 << 16;
    static constexpr std::uint32_t kResponseLenMask = 0x7 << 17;
    static constexpr std::uint32_t kFeatureMask = 0xF << 20;
    static constexpr std::uint32_t kDelayMask = 0xFF << 24;

    /// \brief Construct an SGPC3 command description constant.
    ///
    /// \param code [in]        The command code, from the datasheet.
    /// \param paramLen [in]    The number of bytes of parameter (including CRC), from datasheet.
    /// \param responseLen [in] The bytes of expected response.
    /// \param delayMs [in]     The number of milliseconds to delay between issuing the command/parameters and looking for a response.
    /// \param featureSet [in]  The chip version neded to support this command.
    ///
    static constexpr std::uint32_t CommandInit(std::uint16_t code, std::uint8_t paramLen, std::uint8_t responseLen, std::uint8_t delayMs, std::uint8_t featureSet = 0)
        {
        return std::uint32_t(
                (code & kCommandMask) |
                ((std::uint32_t(paramLen / 3) << 16) & kParamLenMask) |
                ((std::uint32_t(responseLen / 3) << 17) & kResponseLenMask) |
                ((std::uint32_t(featureSet) << 20) & kFeatureMask) | 
                ((std::uint32_t(delayMs) << 24) & kDelayMask)
                );
        }
    /// \brief Extract command bytes from command constant.
    static constexpr std::uint16_t getCommand(Command_t c)
        {
        return std::uint16_t(c) & kCommandMask;
        }
    /// \brief Extract write parameter length from command constant.
    static constexpr std::uint8_t getParameterLength(Command_t c)
        {
        return ((std::uint32_t(c) & kParamLenMask) >> 16);
        }
    /// \brief Extract read response length from command constant.
    static constexpr std::uint8_t getResponseLength(Command_t c)
        {
        return ((std::uint32_t(c) & kResponseLenMask) >> 17);
        }
    /// \brief Extract required feature set leval from command constant.
    static constexpr std::uint8_t getFeatureSet(Command_t c)
        {
        return ((std::uint32_t(c) & kFeatureMask) >> 20);
        }
    /// \brief Extract delay (in milliseconds) from command constant.
    static constexpr std::uint32_t getDelayMs(Command_t c)
        {
        return (std::uint32_t(c) & kDelayMask) >> 24;
        }
    };

//---- the commands, in numerical order ----
/// \brief Internal respresentation of SGPC3 commands.
///
/// \details
///     Commands are are represented as 32-bit constants encoding the comamand value,
///     parameter length, result length, and delay in milliseconds. We use an `enum class`
///     to distinguish these values from ordinary uint32_t values.
///
enum class cSGPC3_cmds::Command_t : std::uint32_t {
    measure_tvoc                   = CommandInit(0x2008, 0, 3, 50),    ///< Measure TVOC.
    get_tvoc_baseline              = CommandInit(0x2015, 0, 3, 10),    ///< Get the baseline value.
    set_tvoc_baseline              = CommandInit(0x201e, 3, 0, 10),    ///< Set the baseline value.
    get_feature_set_version        = CommandInit(0x202f, 0, 3, 10),    ///< Get feature-set version for this device.
    measure_test                   = CommandInit(0x2032, 0, 3, 220),   ///< Test mode; manufacturing.
    measure_tvoc_and_raw           = CommandInit(0x2046, 0, 6, 50),    ///< Get TVOC and raw data.
    measure_raw                    = CommandInit(0x204d, 0, 3, 50),    ///< Measure raw concentration.
    set_absolute_humidity          = CommandInit(0x2061, 3, 0, 10, 6), ///< Set absolute humidity.
    
    /// \brief Set power mode (low or ultralow)
    /// \note The sample code requires feature set >= 6.
    set_power_mode                 = CommandInit(0x209f, 3, 0, 10, 6),  ///< Set power mode (low or ultralow).

    /// \brief Initialize continuous operation mode.
    /// \note
    ///     The sample code requires feature set >= 6. It also defines two addtional commands, `tvoc_init_no_preheat`
    ///     and `SGPC3_CMD_IAQ_INIT_64`; the latter is unused and otherwise undocumented. 
    tvoc_init_continuous           = CommandInit(0x20ae, 0, 0, 10),    ///< Initialize continuous operation mode

    /// \brief Get the inceptive base line.
    /// \note
    ///     The SGPC3 datasheet 1.0 May 2020 says that this command also takes a 3-byte parameter.
    ///     However, the sample code shows this as not taking a parameter. We follow the sample code.
    ///     The sample code also requires feature set >= 5.
    get_tvoc_inceptive_baseline    = CommandInit(0x20b3, 0, 3, 10, 5),
    get_serial_id                  = CommandInit(0x3682, 0, 9, 1),     ///< Get the serial number ID.
};

/*!

\brief Control a Sensirion SGPC3 TVOC Sensor.

\details
    This object provides an interface for controlling a single SGPC3 sensor
    and making measurements. It requires an I2C interface compatible with
    \c TwoWire.
    
    The implementation is divided into lower and upper parts. The lower
    part manages the I2C interface to the sensor, providing and checking
    CRCs, managing power-up timing, and so forth. The upper part performs
    high-level measurements usign the lower part.

*/


///
class cSGPC3 : public cSGPC3_cmds
    {
private:
    using Millisecond_t = decltype(millis());

public:
    /// \brief Control result of isDebug(); use for compiling debug code in/out.
    static constexpr bool kfDebug = false;

    /// \brief The SCPC3 I2C address. This is fixed by design.
    static constexpr std::int8_t kAddress = 0x58;

    /// \brief Delay (in milliseconds) after hard reset before accessing device. 
    static constexpr Millisecond_t kTpuMs = 600;

    /// \brief Delay (in milliseconds) after soft reset before accessing device. 
    static constexpr Millisecond_t kTsrMs = 600;

    /// \brief Time (in milleseconds) between measurements in low-power mode.
    static constexpr Millisecond_t kTlowPowerMs = 2000;

    /// \brief Time (in milliseconds) between measurements in ultra-low-power mode
    static constexpr Millisecond_t kTultraLowPowerMs = 30000;

    /// \brief Common result codes for this library.
    enum class Error_t
        {
        Success,                    ///< The operation was a succcess.
        Failure,                    ///< The operation failed for no specific reason.
        InvalidParmameter,          ///< The operation failed because a paramter was not valid.
        NotSupported,               ///< The operation failed because the sensor doesn't support the command.
        };

    /// \brief Test an Error_t value to see whether it represents a successful operation.
    static constexpr bool isSuccess(Error_t e)
        {
        return e == Error_t::Success;
        }

    /// \brief Power modes.
    ///
    /// \details
    ///     The SGPC3 has two power modes, low power (the default) and ultra-low power. If low-power
    ///     mode is selected, the device updates measurements every two seonds. If ultra-low-power
    ///     mode is seleted, the device updates measurements every thirty seconds. The defauult
    ///     is low-power mode.
    enum class PowerMode_t
        {
        UltraLow = 0,               ///< Ultra-low power mode.
        Low = 1,                    ///< Low-power mode.
        };

public:
    /// \brief Construct an instance on a given I2C bus.
    /// \param wire [in]  I2C bus (or repeater) to be used for this sensor.
    cSGPC3(TwoWire &wire)
            : m_wire(&wire)
            {}

    /// \brief Instances of this class are neither copyable nor movable.
    cSGPC3(const cSGPC3&) = delete;
    /// \brief Instances of this class are neither copyable nor movable.
    cSGPC3& operator=(const cSGPC3&) = delete;
    /// \brief Instances of this class are neither copyable nor movable.
    cSGPC3(const cSGPC3&&) = delete;
    /// \brief Instances of this class are neither copyable nor movable.
    cSGPC3& operator=(const cSGPC3&&) = delete;

    /// \brief Initialze the SGPC3.
    bool begin();

    /// \brief Deinitialize the SGPC3.
    void end();

protected:
    /// \brief Send command with neither parameter nor response.
    /// \details
    ///     Check (at compile time) whether the command can be used with this routine.
    ///     Check (at run time) whether the command is supported by the discovered sensor.
    ///     Launch the command and delay until completed.
    ///
    /// \returns
    ///     This operation returns \ref Error_t::Success, some other code for failure.
    ///     Use isSuccess() to check for success or failure.
    ///
    template <Command_t c>
    Error_t sendSynchronous()
        {
        static_assert(getParameterLength(c) == 0, "command takes parameters");
        static_assert(getResponseLength(c) == 0, "command returns response");
        auto eSupported = this->isSupported(c);
        if (! isSupported(eSupported))
            return eSupported; 
        return this->sendCommandBare(c);
        }
    /// \brief Send a commmand synchronously, with one parameter
    /// \tparam c   The command to be sent.
    /// \param param [in]   The parameter value, in host-native byte order.
    /// \copydetails sendSynchronous()
    template <Command_t c>
    Error_t sendSynchronous(std::uint16_t param)
        {
        static_assert(getParameterLength(c) == 1, "wrong number of parameters for command");
        static_assert(getResponseLength(c) == 0, "command returns response");
        auto eSupported = this->isSupported(c);
        if (! isSupported(eSupported))
            return eSupported; 
        return this->sendCommandParam(c, param);
        }
    /// \brief Send a commmand synchronously, with one response.
    /// \tparam c   The command to be sent.
    /// \param response [out]   Set to the response, in host-native byte order. 
    /// \copydetails sendSynchronous()
    template <Command_t c>
    Error_t sendAndGetSynchronous(std::uint16_t &response)
        {
        static_assert(getParameterLength(c) == 0, "command takes parameters");
        static_assert(getResponseLength(c) == 1, "command response length != 1");
        auto eSupported = this->isSupported(c);
        if (! isSupported(eSupported))
            return eSupported; 
        return this->sendCommandWithResponse(c, response);
        }
    /// \brief Send a commmand synchronously, with two responses.
    /// \tparam c   The command to be sent.
    /// \param response1 [out]  Set to the first response word, in host-native byte order.
    /// \param response2 [out]  Set to the second response word, in host-native byte order.
    /// \copydetails sendSynchronous()
    template <Command_t c>
    Error_t sendAndGetSynchronous(std::uint16_t &response1, std::uint16_t &response2)
        {
        static_assert(getParameterLength(c) == 0, "command takes parameters");
        static_assert(getResponseLength(c) == 2, "command response length != 2");
        auto eSupported = this->isSupported(c);
        if (! isSupported(eSupported))
            return eSupported; 
        return this->sendCommandWithTwoResponses(c, response1, response2);
        }
    /// \brief Send a commmand synchronously, with a 64-bit response
    /// \tparam c   The command to be sent.
    /// \param response [out]   set to the 3-word response formatted as a uint64_t in host-native byte order.
    /// \copydetails sendSynchronous()
    template <Command_t c>
    Error_t sendAndGetSynchronous(std::uint64_t &response)
        {
        static_assert(getParameterLength(c) == 0, "command takes parameters");
        static_assert(getResponseLength(c) == 3, "command response length != 3");
        auto eSupported = this->isSupported(c);
        if (! isSupported(eSupported))
            return eSupported; 
        return this->sendCommandWithThreeResponses(c, response);
        }

private:
    /// \brief Send command with param, no response
    Error_t sendCommandWithParam(Command_t c, uint16_t param);
    /// \brief Send command with one response word
    Error_t sendCommandWithResponse(Command_t c, uint16_t &response);
    /// \brief Send command with two response words
    Error_t sendCommandWithTwoResponses(Command_t c, uint16_t &response1, std::uint16_t &response2);
    /// \brief Send command with three response words
    Error_t sendCommandWithThreeResponses(Command_t c, uint64_t &response);
    /// \brief Send `sgpc3_tvoc_init_continuous` command

public:
    /// \brief Set the sensor into continuous measurement mode.
    Error_t tvoc_init_continuous(void)
        {
        auto result = this->isSupported(Command_t::tvoc_init_continuous));
        if (isSuccess(result))
            result = this->sendSynchronous<Command_t::tvoc_init_continuous>();
        return result;
        }

    /// \brief Get a TVOC measurement
    ///
    /// \param result [out]     Set to the TVOC in ppb (0 to 60000).
    Error_t measure_tvoc_synchronous(std::uint16_t &result)
        {
        return this->sendAndGetSynchronous<Command_t::measure_tvoc>(result);
        }

    /// \brief Set the power-consumption level of the sensor.
    ///
    /// \param [in] mode    The target power mode.
    Error_t set_power_mode_synchronous(PowerMode_t mode)
        {
        auto result = this->sendSynchronous<Command_t::set_power_mode>(std::uint16_t(mode));
        if (isSuccess(result))
            this->m_powerMode = mode;

        return result;
        }

    /// \brief  Inform driver of a chip reset
    ///
    /// \param when [in]    System time as returned by \c millis() when chip reset finished.
    ///                     If not provided, default is the current time.
    ///
    /// \note
    ///     The library doesn't distinguish power-on and soft resets.
    ///
    void handleChipReset(Millisecond_t when = millis())
        {
        this->m_powerMode = PowerMode_t::Low;
        this->m_tAvail = when + kTpuMs;
        }

private:
    Error_t isSupported(Command_t c) const
        {
        if (this->m_featureSet < getFeatureSet(c))
            return Error_t::NotSupported;
        else
            return Error_t::Success;
        }
private:
    /// \brief the I2C bus to use for communication.
    TwoWire *m_wire;
    /// \brief the current power mode.
    PowerMode_t m_powerMode;
    /// \brief The time, in `millis()`, when the sensor will be available again.
    Millisecond_t m_tAvail;
    /// \brief The feature set byte; 0 if chip not recognized or not initialized.
    std::uint8_t m_featureSet;
    };

// end group scpc3
/// \}

} // McciCatenaSGPC3

#endif // _MCCI_Catena_SGPC3_h_
