#include "RTClib.h"

// Month names arrays
static const char* monthNamesShort[] = {
    "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char* monthNamesFull[] = {
    "", "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

// Day names arrays
static const char* dayNamesShort[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char* dayNamesFull[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

// Days in each month (non-leap year)
static const uint8_t daysInMonthArray[] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

// ========== DateTime Implementation ==========

DateTime::DateTime() {
    _year = 2000;
    _month = 1;
    _day = 1;
    _hour = 0;
    _minute = 0;
    _second = 0;
    _dayOfWeek = 6; // Saturday
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, 
                   uint8_t hour, uint8_t minute, uint8_t second) {
    _year = year;
    _month = month;
    _day = day;
    _hour = hour;
    _minute = minute;
    _second = second;
    _dayOfWeek = calculateDayOfWeek(year, month, day);
}

DateTime::DateTime(uint32_t unixTime) {
    uint32_t days = unixTime / 86400UL;
    uint32_t seconds = unixTime % 86400UL;
    
    _hour = seconds / 3600;
    _minute = (seconds % 3600) / 60;
    _second = seconds % 60;
    
    uint16_t year = 1970;
    while (days >= (isLeapYear(year) ? 366 : 365)) {
        days -= (isLeapYear(year) ? 366 : 365);
        year++;
    }
    _year = year;
    
    uint8_t month = 1;
    while (days >= getDaysInMonth(month, year)) {
        days -= getDaysInMonth(month, year);
        month++;
    }
    _month = month;
    _day = days + 1;
    _dayOfWeek = calculateDayOfWeek(_year, _month, _day);
}

DateTime::DateTime(const char* date, const char* time) {
    parseDate(date);
    parseTime(time);
    _dayOfWeek = calculateDayOfWeek(_year, _month, _day);
}

uint32_t DateTime::unixtime() const {
    uint32_t days = 0;
    
    for (uint16_t y = 1970; y < _year; y++) {
        days += isLeapYear(y) ? 366 : 365;
    }
    
    for (uint8_t m = 1; m < _month; m++) {
        days += getDaysInMonth(m, _year);
    }
    
    days += _day - 1;
    return days * 86400UL + _hour * 3600UL + _minute * 60UL + _second;
}

String DateTime::formatTime(const String& format) const {
    String result = format;
    
    result.replace("HH", _hour < 10 ? "0" + String(_hour) : String(_hour));
    result.replace("H", String(_hour));
    result.replace("mm", _minute < 10 ? "0" + String(_minute) : String(_minute));
    result.replace("m", String(_minute));
    result.replace("ss", _second < 10 ? "0" + String(_second) : String(_second));
    result.replace("s", String(_second));
    
    return result;
}

String DateTime::formatDate(const String& format) const {
    String result = format;
    
    result.replace("YYYY", String(_year));
    result.replace("YY", String(_year % 100));
    result.replace("MM", _month < 10 ? "0" + String(_month) : String(_month));
    result.replace("M", String(_month));
    result.replace("DD", _day < 10 ? "0" + String(_day) : String(_day));
    result.replace("D", String(_day));
    
    return result;
}

bool DateTime::operator==(const DateTime& other) const {
    return (_year == other._year && _month == other._month && _day == other._day &&
            _hour == other._hour && _minute == other._minute && _second == other._second);
}

bool DateTime::operator<(const DateTime& other) const {
    return unixtime() < other.unixtime();
}

uint8_t DateTime::calculateDayOfWeek(uint16_t year, uint8_t month, uint8_t day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    
    uint16_t k = year % 100;
    uint16_t j = year / 100;
    uint16_t h = (day + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    
    return (h + 5) % 7;
}

void DateTime::parseDate(const char* date) {
    char monthStr[4];
    int day, year;
    sscanf(date, "%3s %d %d", monthStr, &day, &year);
    
    for (uint8_t i = 1; i <= 12; i++) {
        if (strcmp(monthStr, monthNamesShort[i]) == 0) {
            _month = i;
            break;
        }
    }
    _day = day;
    _year = year;
}

void DateTime::parseTime(const char* time) {
    int hour, minute, second;
    sscanf(time, "%d:%d:%d", &hour, &minute, &second);
    _hour = hour;
    _minute = minute;
    _second = second;
}

uint8_t DateTime::getDaysInMonth(uint8_t month, uint16_t year) {
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return daysInMonthArray[month];
}

bool DateTime::isLeapYear(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// ========== RTC_DS1307 Implementation ==========

RTC_DS1307::RTC_DS1307() : _isInitialized(false), _isRunning(false), _lastError(0) {
}

bool RTC_DS1307::begin() {
    Wire.begin();
    
    Wire.beginTransmission(DS1307_ADDRESS);
    uint8_t result = Wire.endTransmission();
    if (result != 0) {
        return false;
    }
    
    uint8_t secondsReg = readRegister(DS1307_SECONDS_REG);
    _isRunning = !(secondsReg & 0x80);
    _isInitialized = true;
    
    return true;
}

bool RTC_DS1307::isrunning() {
    if (!_isInitialized) return false;
    
    uint8_t secondsReg = readRegister(DS1307_SECONDS_REG);
    _isRunning = !(secondsReg & 0x80);
    return _isRunning;
}

bool RTC_DS1307::adjust(const DateTime& dt) {
    if (!_isInitialized) return false;
    
    uint8_t buffer[7];
    buffer[0] = bin2bcd(dt.second());
    buffer[1] = bin2bcd(dt.minute());
    buffer[2] = bin2bcd(dt.hour());
    buffer[3] = bin2bcd(dt.dayOfTheWeek() + 1);
    buffer[4] = bin2bcd(dt.day());
    buffer[5] = bin2bcd(dt.month());
    buffer[6] = bin2bcd(dt.year() % 100);
    
    buffer[0] &= 0x7F; // Start oscillator
    
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(DS1307_SECONDS_REG);
    for (int i = 0; i < 7; i++) {
        Wire.write(buffer[i]);
    }
    
    return (Wire.endTransmission() == 0);
}

DateTime RTC_DS1307::now() {
    if (!_isInitialized) return DateTime();
    
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(DS1307_SECONDS_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(DS1307_ADDRESS, 7);
    
    if (Wire.available() < 7) return DateTime();
    
    uint8_t second = bcd2bin(Wire.read() & 0x7F);
    uint8_t minute = bcd2bin(Wire.read());
    uint8_t hour = bcd2bin(Wire.read());
    Wire.read(); // Skip day of week
    uint8_t day = bcd2bin(Wire.read());
    uint8_t month = bcd2bin(Wire.read());
    uint16_t year = bcd2bin(Wire.read()) + 2000;
    
    return DateTime(year, month, day, hour, minute, second);
}

uint8_t RTC_DS1307::readRegister(uint8_t reg) {
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    
    Wire.requestFrom(DS1307_ADDRESS, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}

uint8_t RTC_DS1307::bin2bcd(uint8_t val) {
    return val + 6 * (val / 10);
}

uint8_t RTC_DS1307::bcd2bin(uint8_t val) {
    return val - 6 * (val >> 4);
}