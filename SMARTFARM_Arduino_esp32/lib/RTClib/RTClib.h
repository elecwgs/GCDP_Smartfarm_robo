#ifndef RTCLIB_H
#define RTCLIB_H

#include <Arduino.h>
#include <Wire.h>

// DS1307 I2C Address
#define DS1307_ADDRESS 0x68

// DS1307 Register Addresses
#define DS1307_SECONDS_REG     0x00
#define DS1307_MINUTES_REG     0x01
#define DS1307_HOURS_REG       0x02
#define DS1307_DAYOFWEEK_REG   0x03
#define DS1307_DAY_REG         0x04
#define DS1307_MONTH_REG       0x05
#define DS1307_YEAR_REG        0x06
#define DS1307_CONTROL_REG     0x07

// Days of the week
enum DayOfWeek {
    SUNDAY = 0,
    MONDAY = 1,
    TUESDAY = 2,
    WEDNESDAY = 3,
    THURSDAY = 4,
    FRIDAY = 5,
    SATURDAY = 6
};

// Month names
enum Month {
    JANUARY = 1,
    FEBRUARY = 2,
    MARCH = 3,
    APRIL = 4,
    MAY = 5,
    JUNE = 6,
    JULY = 7,
    AUGUST = 8,
    SEPTEMBER = 9,
    OCTOBER = 10,
    NOVEMBER = 11,
    DECEMBER = 12
};

/**
 * @brief DateTime class for handling date and time
 * 
 * This class provides comprehensive date and time functionality
 * compatible with the DS1307 RTC module.
 */
class DateTime {
private:
    uint16_t _year;        ///< Year (e.g., 2025)
    uint8_t _month;        ///< Month (1-12)
    uint8_t _day;          ///< Day of month (1-31)
    uint8_t _hour;         ///< Hour (0-23)
    uint8_t _minute;       ///< Minute (0-59)
    uint8_t _second;       ///< Second (0-59)
    uint8_t _dayOfWeek;    ///< Day of week (0-6, 0=Sunday)

public:
    /**
     * @brief Default constructor - creates DateTime with current time
     */
    DateTime();
    
    /**
     * @brief Constructor with date and time parameters
     * @param year Year (2000-2099)
     * @param month Month (1-12)
     * @param day Day (1-31)
     * @param hour Hour (0-23)
     * @param minute Minute (0-59)
     * @param second Second (0-59)
     */
    DateTime(uint16_t year, uint8_t month, uint8_t day, 
             uint8_t hour = 0, uint8_t minute = 0, uint8_t second = 0);
    
    /**
     * @brief Constructor from Unix timestamp
     * @param unixTime Unix timestamp (seconds since Jan 1, 1970)
     */
    DateTime(uint32_t unixTime);
    
    /**
     * @brief Constructor from date and time strings
     * @param date Date string "MMM DD YYYY" (e.g., "Jan 14 2025")
     * @param time Time string "HH:MM:SS" (e.g., "12:30:45")
     */
    DateTime(const char* date, const char* time);
    
    // ========== Accessor Methods ==========
    /**
     * @brief Get year
     * @return Year (2000-2099)
     */
    uint16_t year() const { return _year; }
    
    /**
     * @brief Get month
     * @return Month (1-12)
     */
    uint8_t month() const { return _month; }
    
    /**
     * @brief Get day of month
     * @return Day (1-31)
     */
    uint8_t day() const { return _day; }
    
    /**
     * @brief Get hour
     * @return Hour (0-23)
     */
    uint8_t hour() const { return _hour; }
    
    /**
     * @brief Get minute
     * @return Minute (0-59)
     */
    uint8_t minute() const { return _minute; }
    
    /**
     * @brief Get second
     * @return Second (0-59)
     */
    uint8_t second() const { return _second; }
    
    /**
     * @brief Get day of week
     * @return Day of week (0-6, 0=Sunday)
     */
    uint8_t dayOfTheWeek() const { return _dayOfWeek; }
    
    // ========== Conversion Methods ==========
    /**
     * @brief Convert to Unix timestamp
     * @return Unix timestamp (seconds since Jan 1, 1970)
     */
    uint32_t unixtime() const;
    
    /**
     * @brief Get day of year (1-366)
     * @return Day of year
     */
    uint16_t dayOfYear() const;
    
    /**
     * @brief Check if year is leap year
     * @return true if leap year, false otherwise
     */
    bool isLeapYear() const;
    
    /**
     * @brief Get number of days in current month
     * @return Days in month (28-31)
     */
    uint8_t daysInMonth() const;
    
    // ========== Formatting Methods ==========
    /**
     * @brief Format date as string
     * @param format Format string (e.g., "YYYY-MM-DD")
     * @return Formatted date string
     */
    String formatDate(const String& format = "YYYY-MM-DD") const;
    
    /**
     * @brief Format time as string
     * @param format Format string (e.g., "HH:MM:SS")
     * @return Formatted time string
     */
    String formatTime(const String& format = "HH:MM:SS") const;
    
    /**
     * @brief Format date and time as string
     * @param format Format string (e.g., "YYYY-MM-DD HH:MM:SS")
     * @return Formatted datetime string
     */
    String formatDateTime(const String& format = "YYYY-MM-DD HH:MM:SS") const;
    
    /**
     * @brief Get month name
     * @param abbreviated true for short name (Jan), false for full (January)
     * @return Month name string
     */
    String monthName(bool abbreviated = false) const;
    
    /**
     * @brief Get day of week name
     * @param abbreviated true for short name (Sun), false for full (Sunday)
     * @return Day name string
     */
    String dayName(bool abbreviated = false) const;
    
    // ========== Arithmetic Operations ==========
    /**
     * @brief Add specified number of seconds
     * @param seconds Seconds to add
     * @return New DateTime object
     */
    DateTime addSeconds(int32_t seconds) const;
    
    /**
     * @brief Add specified number of minutes
     * @param minutes Minutes to add
     * @return New DateTime object
     */
    DateTime addMinutes(int32_t minutes) const;
    
    /**
     * @brief Add specified number of hours
     * @param hours Hours to add
     * @return New DateTime object
     */
    DateTime addHours(int32_t hours) const;
    
    /**
     * @brief Add specified number of days
     * @param days Days to add
     * @return New DateTime object
     */
    DateTime addDays(int32_t days) const;
    
    /**
     * @brief Add specified number of months
     * @param months Months to add
     * @return New DateTime object
     */
    DateTime addMonths(int32_t months) const;
    
    /**
     * @brief Add specified number of years
     * @param years Years to add
     * @return New DateTime object
     */
    DateTime addYears(int32_t years) const;
    
    // ========== Comparison Operators ==========
    bool operator==(const DateTime& other) const;
    bool operator!=(const DateTime& other) const;
    bool operator<(const DateTime& other) const;
    bool operator<=(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;
    
    // ========== Utility Methods ==========
    /**
     * @brief Check if DateTime is valid
     * @return true if valid, false otherwise
     */
    bool isValid() const;
    
    /**
     * @brief Get difference in seconds between two DateTime objects
     * @param other Other DateTime object
     * @return Difference in seconds
     */
    int32_t secondsBetween(const DateTime& other) const;
    
    /**
     * @brief Get difference in days between two DateTime objects
     * @param other Other DateTime object
     * @return Difference in days
     */
    int32_t daysBetween(const DateTime& other) const;

private:
    /**
     * @brief Calculate day of week from date
     * @param year Year
     * @param month Month
     * @param day Day
     * @return Day of week (0-6)
     */
    static uint8_t calculateDayOfWeek(uint16_t year, uint8_t month, uint8_t day);
    
    /**
     * @brief Parse date string
     * @param date Date string to parse
     */
    void parseDate(const char* date);
    
    /**
     * @brief Parse time string
     * @param time Time string to parse
     */
    void parseTime(const char* time);
    
    /**
     * @brief Get number of days in specified month
     * @param month Month (1-12)
     * @param year Year (for leap year calculation)
     * @return Days in month
     */
    static uint8_t getDaysInMonth(uint8_t month, uint16_t year);
    
    /**
     * @brief Check if specified year is leap year
     * @param year Year to check
     * @return true if leap year
     */
    static bool isLeapYear(uint16_t year);
};

/**
 * @brief RTC DS1307 class for interfacing with DS1307 Real-Time Clock
 * 
 * This class provides methods to read from and write to the DS1307 RTC module
 * via I2C communication.
 */
class RTC_DS1307 {
private:
    bool _isInitialized;        ///< Initialization status
    bool _isRunning;           ///< Clock running status
    uint8_t _lastError;        ///< Last error code
    
public:
    /**
     * @brief Constructor
     */
    RTC_DS1307();
    
    /**
     * @brief Initialize the RTC module
     * @return true if successful, false otherwise
     */
    bool begin();
    
    /**
     * @brief Check if RTC is running
     * @return true if running, false if stopped
     */
    bool isrunning();
    
    /**
     * @brief Adjust (set) the RTC time
     * @param dt DateTime object with the time to set
     * @return true if successful, false otherwise
     */
    bool adjust(const DateTime& dt);
    
    /**
     * @brief Get current time from RTC
     * @return DateTime object with current time
     */
    DateTime now();
    
    /**
     * @brief Start the RTC oscillator
     * @return true if successful, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop the RTC oscillator
     * @return true if successful, false otherwise
     */
    bool stop();
    
    /**
     * @brief Check if RTC lost power
     * @return true if power was lost, false otherwise
     */
    bool lostPower();
    
    /**
     * @brief Clear the power lost flag
     * @return true if successful, false otherwise
     */
    bool clearPowerLostFlag();
    
    /**
     * @brief Read temperature from RTC (if supported)
     * @return Temperature in Celsius, or NaN if not supported
     */
    float getTemperature();
    
    /**
     * @brief Set 12-hour or 24-hour mode
     * @param is24Hour true for 24-hour mode, false for 12-hour mode
     * @return true if successful, false otherwise
     */
    bool set24HourMode(bool is24Hour = true);
    
    /**
     * @brief Check if in 24-hour mode
     * @return true if 24-hour mode, false if 12-hour mode
     */
    bool is24HourMode();
    
    /**
     * @brief Enable or disable square wave output
     * @param enable true to enable, false to disable
     * @param frequency Frequency (0=1Hz, 1=4096Hz, 2=8192Hz, 3=32768Hz)
     * @return true if successful, false otherwise
     */
    bool enableSquareWave(bool enable, uint8_t frequency = 0);
    
    /**
     * @brief Read raw register value
     * @param reg Register address (0-0x3F)
     * @return Register value (0-255)
     */
    uint8_t readRegister(uint8_t reg);
    
    /**
     * @brief Write raw register value
     * @param reg Register address (0-0x3F)
     * @param value Value to write (0-255)
     * @return true if successful, false otherwise
     */
    bool writeRegister(uint8_t reg, uint8_t value);
    
    /**
     * @brief Get last error code
     * @return Error code (0 = no error)
     */
    uint8_t getLastError() const { return _lastError; }
    
    /**
     * @brief Get error description
     * @param errorCode Error code
     * @return Error description string
     */
    String getErrorDescription(uint8_t errorCode);
    
    /**
     * @brief Perform RTC diagnostics
     * @return true if all tests pass, false otherwise
     */
    bool performDiagnostics();
    
    /**
     * @brief Get RTC status information
     * @return Status information string
     */
    String getStatusInfo();

private:
    /**
     * @brief Convert binary to BCD (Binary Coded Decimal)
     * @param val Binary value
     * @return BCD value
     */
    uint8_t bin2bcd(uint8_t val);
    
    /**
     * @brief Convert BCD to binary
     * @param val BCD value
     * @return Binary value
     */
    uint8_t bcd2bin(uint8_t val);
    
    /**
     * @brief Read multiple bytes from RTC
     * @param reg Starting register address
     * @param buffer Buffer to store read data
     * @param length Number of bytes to read
     * @return Number of bytes actually read
     */
    uint8_t readBytes(uint8_t reg, uint8_t* buffer, uint8_t length);
    
    /**
     * @brief Write multiple bytes to RTC
     * @param reg Starting register address
     * @param buffer Buffer containing data to write
     * @param length Number of bytes to write
     * @return true if successful, false otherwise
     */
    bool writeBytes(uint8_t reg, const uint8_t* buffer, uint8_t length);
    
    /**
     * @brief Validate I2C communication
     * @return true if communication is working, false otherwise
     */
    bool validateCommunication();
    
    /**
     * @brief Set error code
     * @param errorCode Error code to set
     */
    void setError(uint8_t errorCode);
};

// ========== Error Codes ==========
#define RTC_ERROR_NONE              0x00  ///< No error
#define RTC_ERROR_I2C_TIMEOUT       0x01  ///< I2C communication timeout
#define RTC_ERROR_I2C_DATA          0x02  ///< I2C data error
#define RTC_ERROR_INVALID_TIME      0x03  ///< Invalid time data
#define RTC_ERROR_OSCILLATOR_STOP   0x04  ///< Oscillator stopped
#define RTC_ERROR_POWER_LOST        0x05  ///< Power was lost
#define RTC_ERROR_REGISTER_ACCESS   0x06  ///< Register access error
#define RTC_ERROR_INITIALIZATION    0x07  ///< Initialization failed

// ========== Utility Functions ==========
/**
 * @brief Create DateTime from compile date and time
 * @param date Compile date string (__DATE__)
 * @param time Compile time string (__TIME__)
 * @return DateTime object
 */
DateTime compileDateTime(const char* date, const char* time);

/**
 * @brief Create DateTime from Unix timestamp
 * @param unixTime Unix timestamp
 * @return DateTime object
 */
DateTime unixTimestampToDateTime(uint32_t unixTime);

/**
 * @brief Get current Unix timestamp
 * @return Current Unix timestamp
 */
uint32_t getCurrentUnixTimestamp();

/**
 * @brief Check if date is valid
 * @param year Year
 * @param month Month (1-12)
 * @param day Day (1-31)
 * @return true if valid, false otherwise
 */
bool isValidDate(uint16_t year, uint8_t month, uint8_t day);

/**
 * @brief Check if time is valid
 * @param hour Hour (0-23)
 * @param minute Minute (0-59)
 * @param second Second (0-59)
 * @return true if valid, false otherwise
 */
bool isValidTime(uint8_t hour, uint8_t minute, uint8_t second);

/**
 * @brief Calculate Easter date for given year
 * @param year Year
 * @return DateTime object for Easter Sunday
 */
DateTime calculateEaster(uint16_t year);

/**
 * @brief Get timezone offset in seconds
 * @param timezoneName Timezone name (e.g., "UTC", "EST", "PST")
 * @return Offset in seconds from UTC
 */
int32_t getTimezoneOffset(const String& timezoneName);

/**
 * @brief Convert between timezones
 * @param dt DateTime in source timezone
 * @param fromOffset Source timezone offset (seconds)
 * @param toOffset Target timezone offset (seconds)
 * @return DateTime in target timezone
 */
DateTime convertTimezone(const DateTime& dt, int32_t fromOffset, int32_t toOffset);

// ========== Convenient Macros ==========
/**
 * @brief Create DateTime from compile time
 */
#define COMPILE_DATETIME() compileDateTime(__DATE__, __TIME__)

/**
 * @brief Create DateTime for current time (must be implemented by user)
 */
#define NOW() rtc.now()

/**
 * @brief Check if it's currently daytime (6 AM - 6 PM)
 */
#define IS_DAYTIME(dt) ((dt).hour() >= 6 && (dt).hour() < 18)

/**
 * @brief Check if it's currently nighttime
 */
#define IS_NIGHTTIME(dt) ((dt).hour() < 6 || (dt).hour() >= 18)

/**
 * @brief Check if it's weekend (Saturday or Sunday)
 */
#define IS_WEEKEND(dt) ((dt).dayOfTheWeek() == 0 || (dt).dayOfTheWeek() == 6)

/**
 * @brief Check if it's weekday (Monday through Friday)
 */
#define IS_WEEKDAY(dt) ((dt).dayOfTheWeek() >= 1 && (dt).dayOfTheWeek() <= 5)

#endif // RTCLIB_H