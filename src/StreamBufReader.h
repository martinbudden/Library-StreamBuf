#pragma once

#include "StreamBufWriter.h"

/*!
Simple read only deserializer with optional bounds checking
*/
class StreamBufReader {
public:
    StreamBufReader(const uint8_t* ptr, size_t len) : _ptr(ptr), _begin(ptr), _end(ptr + len + 1) {}
    StreamBufReader(const uint8_t* ptr, const uint8_t* end) : _ptr(ptr), _begin(ptr), _end(end) {}
    explicit StreamBufReader(const StreamBufWriter& stream_buf) : _ptr(stream_buf.ptr()), _begin(stream_buf.begin()), _end(stream_buf.end()) {}
public:
    void reset() { _ptr = _begin; }
    bool is_empty() const { return _ptr == _begin; }
    bool is_full() const { return _ptr + 1 >= _end; }
    const uint8_t* ptr() const { return _ptr; }
    const uint8_t* begin() const { return _begin; }
    const uint8_t* end() const { return _end; }

    //! return the number of bytes remaining in the buffer
    size_t bytes_remaining() const { return _end - _ptr - 1; }
    size_t bytes_read() const { return _ptr - _begin; }

    //! Advance _ptr, this skips data
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
    float read_f32() {
        const uint32_t value = read_u32();
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        return *reinterpret_cast<const float*>(&value); // cppcheck-suppress invalidPointerCast
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

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
    float read_f32_checked() {
        if (_ptr < _end - sizeof(float)) {
            return read_f32();
        }
        return 0.0F;
    }

    void read_data(void *data, size_t len) { if (_ptr + len < _end) { memcpy(data, _ptr, len); _ptr += len; } }
protected:
    const uint8_t* _ptr; // data pointer must be first
    const uint8_t* const _begin;
    const uint8_t* _end;
};
