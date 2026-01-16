#pragma once

#include <cstdint>
#include <cstring>
#include <string>

/*!
Simple serializer/deserializer with optional bounds checking
*/
class StreamBuf {
public:
    StreamBuf(uint8_t* ptr, size_t len) : _ptr(ptr), _begin(ptr), _end(ptr + len + 1) {}
    StreamBuf(uint8_t* ptr, uint8_t* end) : _ptr(ptr), _begin(ptr), _end(end) {}
public:
    StreamBuf reader() { return StreamBuf(_begin, _ptr + 1); }

    void reset() { _ptr = _begin; }
    bool isEmpty() const { return _ptr == _begin; }
    bool isFull() const { return _ptr + 1 >= _end; }
    const uint8_t* ptr() const { return _ptr; }
    const uint8_t* begin() const { return _begin; }
    const uint8_t* end() const { return _end; }

    /*! 
    when writing - return available space
    when reading - return the number of bytes remaining in the buffer
    */
    size_t bytesRemaining() const { return _end - _ptr - 1; }
    /*! 
    when writing - return the number of bytes written to the buffer
    when reading - return the number of bytes read
    */
    size_t bytesWritten() const { return _ptr - _begin; }

    /*! Advance _ptr
    when reading - this skips data
    when writing - this effectively commits the written data
    */
    void advance(size_t size) { if (_ptr + size < _end) { _ptr += size; } }
     //! modifies internal pointers so that data can be read
    const uint8_t* switchToReader() {
        const uint8_t* endPrevious = _end;
        _end = _ptr + 1;
        _ptr = _begin;
        return endPrevious;
    }
//
// Read functions
//
    uint8_t readU8() { return *_ptr++; }
    uint16_t readU16() { return readU8() | static_cast<uint16_t>(readU8() << 8); }
    uint32_t readU32() {
        uint32_t ret = readU8();
        ret |= static_cast<uint32_t>(readU8() <<  8);
        ret |= static_cast<uint32_t>(readU8() << 16);
        ret |= static_cast<uint32_t>(readU8() << 24);
        return ret;
    }
    uint16_t readU16_BigEndian() { return static_cast<uint16_t>(readU8() << 8) | readU8(); }
    uint32_t readU32_BigEndian() {
        uint32_t ret = static_cast<uint32_t>(readU8() << 24);
        ret |= static_cast<uint32_t>(readU8() << 16);
        ret |= static_cast<uint32_t>(readU8() <<  8);
        ret |= readU8();
        return ret;
    }
    float readFloat() {
        const uint32_t value = readU32();
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        return *reinterpret_cast<const float*>(&value); // cppcheck-suppress invalidPointerCast
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    uint8_t readU8_Checked() { if (_ptr < _end) { return *_ptr++; } return 0; }
    uint16_t readU16_Checked() {
        if (_ptr < _end - sizeof(uint16_t)) {
            return readU16();
        }
        return 0;
    }
    uint32_t readU32_Checked() {
        if (_ptr < _end - sizeof(uint32_t)) {
            return readU32();
        }
        return 0;
    }
    uint16_t readU16_BigEndianChecked() {
        if (_ptr < _end - sizeof(uint16_t)) {
            return readU16_BigEndian();
        }
        return 0;
    }
    uint32_t readU32_BigEndianChecked() {
        if (_ptr < _end - sizeof(uint32_t)) {
            return readU32_BigEndian();
        }
        return 0;
    }
    float readFloat_Checked() {
        if (_ptr < _end - sizeof(float)) {
            return readFloat();
        }
        return 0.0F;
    }

    void readData(void *data, size_t len) { if (_ptr + len < _end) { memcpy(data, _ptr, len); _ptr += len; } }
//
// Write functions
//
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
    void writeU16_BigEndian(uint16_t value) {
        writeU8(static_cast<uint8_t>(value >> 8));
        writeU8(static_cast<uint8_t>(value));
    }
    void writeU32_BigEndian(uint32_t value) {
        writeU8(static_cast<uint8_t>(value >> 24));
        writeU8(static_cast<uint8_t>(value >> 16));
        writeU8(static_cast<uint8_t>(value >> 8));
        writeU8(static_cast<uint8_t>(value));
    }
    void writeFloat(float value) {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        const uint32_t u = *reinterpret_cast<const uint32_t*>(&value); // cppcheck-suppress invalidPointerCast
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
        writeU32(u);
    }

    void writeU8_Checked(uint8_t value) { if (_ptr < _end) { *_ptr++ = value; } }
    void writeU16_Checked(uint16_t value) {
        if (_ptr < _end - sizeof(uint16_t)) {
            writeU16(value);
        }
    }
    void writeU32_Checked(uint32_t value) {
        if (_ptr < _end - sizeof(uint32_t)) {
            writeU32(value);
        }
    }
    void writeU16_BigEndianChecked(uint16_t value) {
        if (_ptr < _end - sizeof(uint16_t)) {
            writeU16(value);
        }
    }
    void writeU32_BigEndianChecked(uint32_t value) {
        if (_ptr < _end - sizeof(uint32_t)) {
            writeU32_BigEndian(value);
        }
    }
    void writeFloatChecked(float value) {
        if (_ptr < _end - sizeof(float)) {
            writeFloat(value);
        }
    }

    // all bulk write operations are bounds checked
    void writeData(const void* data, size_t len) { if (_ptr + len < _end) { memcpy(_ptr, data, len); _ptr += len; } }
    void writeString(const char* str) { writeData(str, strlen(str)); }
    void writeString(const std::string& str) { writeData(str.c_str(), str.size()); }
    void writeStringWithZeroTerminator(const char* string) { writeData(string, strlen(string) + 1); }
    void writeStringWithZeroTerminator(const std::string& str) { writeData(str.c_str(), str.size() + 1); }

    void fill(uint8_t data, size_t len) { if (_ptr + len < _end) { memset(_ptr, data, len); _ptr += len; } }
    void fillWithoutAdvancing(uint8_t data, size_t len) { if (_ptr + len < _end) { memset(_ptr, data, len); } }

protected:
    uint8_t* _ptr; // data pointer must be first
    uint8_t* const _begin;
    uint8_t* _end; // points to byte after the end of the buffer, as is conventional
};
