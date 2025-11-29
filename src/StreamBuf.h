#pragma once

#include <cstdint>
#include <cstring>
#include <string>

/*!
Simple serializer/deserializer with no bounds checking
*/
class StreamBuf {
public:
    StreamBuf(uint8_t* ptr, size_t len) : _ptr(ptr), _begin(ptr), _end(ptr + len) {}
    StreamBuf(uint8_t* ptr, uint8_t* end) : _ptr(ptr), _begin(ptr), _end(end) {}
public:
    void writeU8(uint8_t value) { *_ptr++ = value; }
    void writeU16(uint16_t value) {
        writeU8(static_cast<uint8_t>(value));
        writeU8(static_cast<uint8_t>(value >> 8));
    }
    void writeU32(uint32_t value) {
        writeU8(static_cast<uint8_t>(value));
        writeU8(static_cast<uint8_t>(value >> 8));
        writeU8(static_cast<uint8_t>(value >> 16));
        writeU8(static_cast<uint8_t>(value >> 24));
    }
    void writeU16BigEndian(uint16_t value) {
        writeU8(static_cast<uint8_t>(value >> 8));
        writeU8(static_cast<uint8_t>(value));
    }
    void writeU32BigEndian(uint32_t value) {
        writeU8(static_cast<uint8_t>(value >> 24));
        writeU8(static_cast<uint8_t>(value >> 16));
        writeU8(static_cast<uint8_t>(value >> 8));
        writeU8(static_cast<uint8_t>(value));
    }
    void writeFloat(float value) {
        union { float f; uint32_t i; } u { .f = value }; writeU32(u.i); // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    void writeData(const void* data, size_t len) { memcpy(_ptr, data, len); _ptr += len; }
    void writeString(const char* str) { writeData(str, strlen(str)); }
    void writeString(const std::string& str) { writeData(str.c_str(), str.size()); }
    void writeStringWithZeroTerminator(const char* string) { writeData(string, strlen(string) + 1); }
    void writeStringWithZeroTerminator(const std::string& str) { writeData(str.c_str(), str.size() + 1); }

    void fill(uint8_t data, size_t len) { memset(_ptr, data, len); _ptr += len; }
    void fillWithoutAdvancing(uint8_t data, size_t len) { memset(_ptr, data, len); }

    uint8_t readU8() { return *_ptr++; }
    uint16_t readU16() { return readU8() | static_cast<uint16_t>(readU8() << 8); }
    uint32_t readU32() {
        uint32_t ret = readU8();
        ret |= static_cast<uint32_t>(readU8() <<  8);
        ret |= static_cast<uint32_t>(readU8() << 16);
        ret |= static_cast<uint32_t>(readU8() << 24);
        return ret;
    }
    uint16_t readU16BigEndian() { return static_cast<uint16_t>(readU8() << 8) | readU8(); }
    uint32_t readU32BigEndian() {
        uint32_t ret = static_cast<uint32_t>(readU8() << 24);
        ret |= static_cast<uint32_t>(readU8() << 16);
        ret |= static_cast<uint32_t>(readU8() <<  8);
        ret |= readU8();
        return ret;
    }
    float readFloat() {
        union { float f; uint32_t i; } u { .i = readU32() }; return u.f; // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    void readData(void *data, size_t len) { memcpy(data, _ptr, len); _ptr += len; }
    /*! 
        when writing - return available space
        when reading - return the number of bytes remaining in the buffer
    */
    size_t bytesRemaining() const { return static_cast<size_t>(_end - _ptr); }
    /*! 
        when writing - return the number of bytes written to the buffer
        when reading - return the number of bytes read
    */
    size_t bytesWritten() const { return static_cast<size_t>(_ptr - _begin); }

    uint8_t* ptr() { return _ptr; }
    const uint8_t* ptr() const { return _ptr; }
    const uint8_t* begin() const { return _begin; }
    const uint8_t* end() const { return _end; }
    /*! Advance _ptr
        when reading - this skips data
        when writing - this effectively commits the written data
    */
    void advance(size_t size) { _ptr += size; }
    void reset() { _ptr = _begin; }
     //! modifies internal pointers so that data can be read
    uint8_t* switchToReader() {
        uint8_t* endPrevious = _end;
        _end = _ptr;
        _ptr = _begin;
        return endPrevious;
    }
    /*! Modifies StreamBuf so that it can be used for writing again.
        Any data in StreamBuf is effectively lost.
    */
    void switchToWriter(uint8_t* base) {
        _end = _ptr;
        _ptr = base;
        _begin = base;
    }
protected:
    uint8_t* _ptr; // data pointer must be first
    uint8_t* _begin;
    uint8_t* _end;
};
