#pragma once

#include <cstdint>
#include <cstring>
#include <string>

/*!
Simple serializer/deserializer with optional bounds checking
*/
class StreamBufWriter {
public:
    StreamBufWriter(uint8_t* ptr, size_t len) : _ptr(ptr), _begin(ptr), _end(ptr + len + 1) {}
    StreamBufWriter(uint8_t* ptr, uint8_t* end) : _ptr(ptr), _begin(ptr), _end(end) {}
public:
    StreamBufWriter reader() { return StreamBufWriter(_begin, _ptr + 1); }

    void reset() { _ptr = _begin; }
    bool is_empty() const { return _ptr == _begin; }
    bool is_full() const { return _ptr + 1 >= _end; }
    const uint8_t* ptr() const { return _ptr; }
    const uint8_t* begin() const { return _begin; }
    const uint8_t* end() const { return _end; }

    /*!
    when writing - return available space
    when reading - return the number of bytes remaining in the buffer
    */
    ptrdiff_t bytes_remaining() const { return _end - _ptr - 1; }
    /*!
    when writing - return the number of bytes written to the buffer
    when reading - return the number of bytes read
    */
    ptrdiff_t bytes_written() const { return _ptr - _begin; }

    /*! Advance _ptr
    when reading - this skips data
    when writing - this effectively commits the written data
    */
    void advance(size_t size) { if (_ptr + size < _end) { _ptr += size; } }
     //! modifies internal pointers so that data can be read
    const uint8_t* switch_to_reader() {
        const uint8_t* end_previous = _end;
        _end = _ptr + 1;
        _ptr = _begin;
        return end_previous;
    }
//
// Read functions
//
    uint8_t read_u8() { return *_ptr++; }
    uint16_t read_u16() { return read_u8() | static_cast<uint16_t>(read_u8() << 8); }
    uint32_t read_u32() {
        uint32_t ret = read_u8();
        ret |= static_cast<uint32_t>(read_u8() <<  8);
        ret |= static_cast<uint32_t>(read_u8() << 16);
        ret |= static_cast<uint32_t>(read_u8() << 24);
        return ret;
    }
    uint16_t read_u16_big_endian() { return static_cast<uint16_t>(read_u8() << 8) | read_u8(); }
    uint32_t read_u32_big_endian() {
        uint32_t ret = static_cast<uint32_t>(read_u8() << 24);
        ret |= static_cast<uint32_t>(read_u8() << 16);
        ret |= static_cast<uint32_t>(read_u8() <<  8);
        ret |= read_u8();
        return ret;
    }
    /*float read_f32() {
        const uint32_t value = read_u32();
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        return *reinterpret_cast<const float*>(&value); // cppcheck-suppress invalidPointerCast
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }*/

    uint8_t read_u8_checked() { if (_ptr < _end) { return *_ptr++; } return 0; }
    uint16_t read_u16_checked() {
        if (_ptr < _end - sizeof(uint16_t)) {
            return read_u16();
        }
        return 0;
    }
    uint32_t read_u32_checked() {
        if (_ptr < _end - sizeof(uint32_t)) {
            return read_u32();
        }
        return 0;
    }
    uint16_t read_u16_big_endian_checked() {
        if (_ptr < _end - sizeof(uint16_t)) {
            return read_u16_big_endian();
        }
        return 0;
    }
    uint32_t read_u32_big_endian_checked() {
        if (_ptr < _end - sizeof(uint32_t)) {
            return read_u32_big_endian();
        }
        return 0;
    }
    /*float read_f32_checked() {
        if (_ptr < _end - sizeof(float)) {
            return read_f32();
        }
        return 0.0F;
    }*/

    void read_data(void *data, size_t len) { if (_ptr + len < _end) { memcpy(data, _ptr, len); _ptr += len; } }
//
// Write functions
//
    void write_u8(uint8_t value) { *_ptr++ = value; }
    void write_u16(uint16_t value) {
        write_u8(static_cast<uint8_t>(value));
        write_u8(static_cast<uint8_t>(value >> 8));
    }
    void write_u32(uint32_t value) {
        write_u8(static_cast<uint8_t>(value));
        write_u8(static_cast<uint8_t>(value >> 8));
        write_u8(static_cast<uint8_t>(value >> 16));
        write_u8(static_cast<uint8_t>(value >> 24));
    }
    void write_u16_big_endian(uint16_t value) {
        write_u8(static_cast<uint8_t>(value >> 8));
        write_u8(static_cast<uint8_t>(value));
    }
    void write_u32_big_endian(uint32_t value) {
        write_u8(static_cast<uint8_t>(value >> 24));
        write_u8(static_cast<uint8_t>(value >> 16));
        write_u8(static_cast<uint8_t>(value >> 8));
        write_u8(static_cast<uint8_t>(value));
    }
    void write_f32(float value) {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        const uint32_t u = *reinterpret_cast<const uint32_t*>(&value); // cppcheck-suppress invalidPointerCast
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
        write_u32(u);
    }

    void write_u8_checked(uint8_t value) { if (_ptr < _end) { *_ptr++ = value; } }
    void write_u16_checked(uint16_t value) {
        if (_ptr < _end - sizeof(uint16_t)) {
            write_u16(value);
        }
    }
    void write_u32_checked(uint32_t value) {
        if (_ptr < _end - sizeof(uint32_t)) {
            write_u32(value);
        }
    }
    void write_u16_big_endian_checked(uint16_t value) {
        if (_ptr < _end - sizeof(uint16_t)) {
            write_u16(value);
        }
    }
    void write_u32_big_endian_checked(uint32_t value) {
        if (_ptr < _end - sizeof(uint32_t)) {
            write_u32_big_endian(value);
        }
    }
    void write_f32Checked(float value) {
        if (_ptr < _end - sizeof(float)) {
            write_f32(value);
        }
    }

    // all bulk write operations are bounds checked
    void write_data(const void* data, size_t len) { if (_ptr + len < _end) { memcpy(_ptr, data, len); _ptr += len; } }
    void write_string(const char* str) { write_data(str, strlen(str)); }
    void write_string(const std::string& str) { write_data(str.c_str(), str.size()); }
    void write_string_with_zero_terminator(const char* string) { write_data(string, strlen(string) + 1); }
    void write_string_with_zero_terminator(const std::string& str) { write_data(str.c_str(), str.size() + 1); }

    void fill(uint8_t data, size_t len) { if (_ptr + len < _end) { memset(_ptr, data, len); _ptr += len; } }
    void fill_without_advancing(uint8_t data, size_t len) { if (_ptr + len < _end) { memset(_ptr, data, len); } }

protected:
    uint8_t* _ptr; // data pointer must be first
    uint8_t* const _begin;
    uint8_t* _end; // points to byte after the end of the buffer, as is conventional
};
