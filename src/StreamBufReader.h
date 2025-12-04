#pragma once

#include "StreamBuf.h"

/*!
Simple read only deserializer with optional bounds checking
*/
class StreamBufReader {
public:
    StreamBufReader(const uint8_t* ptr, size_t len) : _ptr(ptr), _begin(ptr), _end(ptr + len + 1) {}
    StreamBufReader(const uint8_t* ptr, const uint8_t* end) : _ptr(ptr), _begin(ptr), _end(end) {}
    explicit StreamBufReader(const StreamBuf& streamBuf) : _ptr(streamBuf.ptr()), _begin(streamBuf.begin()), _end(streamBuf.end()) {}
public:
    void reset() { _ptr = _begin; }
    bool isEmpty() const { return _ptr == _begin; }
    bool isFull() const { return _ptr + 1 == _end; }
    const uint8_t* ptr() const { return _ptr; }
    const uint8_t* begin() const { return _begin; }
    const uint8_t* end() const { return _end; }

    //! return the number of bytes remaining in the buffer
    size_t bytesRemaining() const { return static_cast<size_t>(_end - _ptr - 1); }
    size_t bytesRead() const { return static_cast<size_t>(_ptr - _begin); }

    //! Advance _ptr, this skips data
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
    uint8_t readU8() const { return *_ptr++; }
    uint16_t readU16() const { return readU8() | static_cast<uint16_t>(readU8() << 8); }
    uint32_t readU32() const {
        uint32_t ret = readU8();
        ret |= static_cast<uint32_t>(readU8() <<  8);
        ret |= static_cast<uint32_t>(readU8() << 16);
        ret |= static_cast<uint32_t>(readU8() << 24);
        return ret;
    }
    uint16_t readU16_BigEndian() const { return static_cast<uint16_t>(readU8() << 8) | readU8(); }
    uint32_t readU32_BigEndian() const {
        uint32_t ret = static_cast<uint32_t>(readU8() << 24);
        ret |= static_cast<uint32_t>(readU8() << 16);
        ret |= static_cast<uint32_t>(readU8() <<  8);
        ret |= readU8();
        return ret;
    }
    float readFloat() const {
        union { float f; uint32_t i; } u { .i = readU32() }; return u.f; // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    uint8_t readU8_Checked() const { if (_ptr < _end) { return *_ptr++; } return 0; }
    uint16_t readU16_Checked() const {
        if (_ptr < _end - sizeof(uint16_t)) {
            return readU16();
        }
        return 0;
    }
    uint32_t readU32_Checked() const {
        if (_ptr < _end - sizeof(uint32_t)) {
            return readU32();
        }
        return 0;
    }
    uint16_t readU16_BigEndianChecked() const {
        if (_ptr < _end - sizeof(uint16_t)) {
            return readU16_BigEndian();
        }
        return 0;
    }
    uint32_t readU32_BigEndianChecked() const {
        if (_ptr < _end - sizeof(uint32_t)) {
            return readU32_BigEndian();
        }
        return 0;
    }
    float readFloat_Checked() const {
        if (_ptr < _end - sizeof(float)) {
            union { float f; uint32_t i; } u { .i = readU32() }; return u.f; // NOLINT(cppcoreguidelines-pro-type-union-access)
        }
        return 0.0F;
    }

    void readData(void *data, size_t len) const { if (_ptr + len < _end) { memcpy(data, _ptr, len); _ptr += len; } }
protected:
    mutable const uint8_t* _ptr; // data pointer must be first
    const uint8_t* const _begin;
    const uint8_t* const _end;
};
