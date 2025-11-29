#pragma once

#include "StreamBuf.h"

/*!
Simple read only deserializer with no bounds checking
*/
class StreamBufReader {
public:
    StreamBufReader(const uint8_t* ptr, size_t len) : _ptr(ptr), _begin(ptr), _end(ptr + len) {}
    StreamBufReader(const uint8_t* ptr, const uint8_t* end) : _ptr(ptr), _begin(ptr), _end(end) {}
    explicit StreamBufReader(const StreamBuf& streamBuf) : _ptr(streamBuf.ptr()), _begin(streamBuf.begin()), _end(streamBuf.end()) {}
    StreamBufReader& operator=(const StreamBuf& streamBuf) {
        _ptr = streamBuf.ptr();
        _begin = streamBuf.begin();
        _end =streamBuf.end();
        return *this;
    }
public:
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
    //! return the number of bytes remaining in the buffer
    size_t bytesRemaining() const { return static_cast<size_t>(_end - _ptr); }
    //! return the number of bytes read
    size_t bytesWritten() const { return static_cast<size_t>(_ptr - _begin); }

    const uint8_t* ptr() const { return _ptr; }
    const uint8_t* begin() const { return _begin; }
    const uint8_t* end() const { return _end; }

    //! Advance _ptr, this skips data
    void advance(size_t size) { _ptr += size; }
    void reset() { _ptr = _begin; }
     //! modifies internal pointers so that data can be read
    const uint8_t* switchToReader() {
        const uint8_t* endPrevious = _end;
        _end = _ptr;
        _ptr = _begin;
        return endPrevious;
    }
protected:
    const uint8_t* _ptr; // data pointer must be first
    const uint8_t* _begin;
    const uint8_t* _end;
};
