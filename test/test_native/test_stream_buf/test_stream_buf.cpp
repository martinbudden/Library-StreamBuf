#include "StreamBuf.h"
#include <array>
#include <unity.h>

void setUp()
{
}

void tearDown()
{
}

class StreamBufTest : public StreamBuf {
public:
    StreamBufTest(uint8_t* ptr, size_t len) : StreamBuf(ptr, len) {}
    StreamBufTest(uint8_t* ptr, uint8_t* end) : StreamBuf(ptr, end) {}
public:
    size_t ptrOffset() { return offsetof(StreamBufTest, _ptr); }
};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-magic-numbers)
void test_stream_buf_offset()
{
    std::array<uint8_t, 32> buf;
    StreamBufTest sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(0, sbuf.ptrOffset());
}

void test_stream_buf()
{
    std::array<uint8_t, 256> buf;
    StreamBuf sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());

    sbuf.writeU8(1);
    TEST_ASSERT_EQUAL(1, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 1, sbuf.bytesRemaining());

    sbuf.writeU16(2);
    TEST_ASSERT_EQUAL(3, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 3, sbuf.bytesRemaining());

    sbuf.writeU32(3);
    TEST_ASSERT_EQUAL(7, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 7, sbuf.bytesRemaining());

    sbuf.switchToReader();
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(7, sbuf.bytesRemaining());

    const uint8_t v1 = sbuf.readU8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(6, sbuf.bytesRemaining());

    const uint16_t v2 = sbuf.readU16();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(4, sbuf.bytesRemaining());

    const uint32_t v3 = sbuf.readU32();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(0, sbuf.bytesRemaining());
}

void test_stream_buf_big_endian()
{
    std::array<uint8_t, 256> buf;
    StreamBuf sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());

    sbuf.writeU8(1);
    TEST_ASSERT_EQUAL(sizeof(buf) - 1, sbuf.bytesRemaining());

    sbuf.writeU16BigEndian(2);
    TEST_ASSERT_EQUAL(sizeof(buf) - 3, sbuf.bytesRemaining());

    sbuf.writeU32BigEndian(3);
    TEST_ASSERT_EQUAL(sizeof(buf) - 7, sbuf.bytesRemaining());

    sbuf.switchToReader();
    TEST_ASSERT_EQUAL(7, sbuf.bytesRemaining());

    const uint8_t v1 = sbuf.readU8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(6, sbuf.bytesRemaining());

    const uint16_t v2 = sbuf.readU16BigEndian();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(4, sbuf.bytesRemaining());

    const uint32_t v3 = sbuf.readU32BigEndian();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(0, sbuf.bytesRemaining());
}

void test_stream_buf_strings()
{
    std::array<uint8_t, 256> buf;
    StreamBuf sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());
    sbuf.fillWithoutAdvancing(0xFF, sizeof(buf));
    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());

    sbuf.writeString("Hello");
    TEST_ASSERT_EQUAL(5, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 5, sbuf.bytesRemaining());
    uint8_t* ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL('o', *(ptr-1));

    sbuf.reset();
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());
    sbuf.fillWithoutAdvancing(0xFF, sizeof(buf));

    sbuf.writeStringWithZeroTerminator("Hello");
    TEST_ASSERT_EQUAL(6, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 6, sbuf.bytesRemaining());
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL(0, *(ptr-1));
    TEST_ASSERT_EQUAL('o', *(ptr-2));

    const std::string hello("Hello");

    sbuf.reset();
    sbuf.fillWithoutAdvancing(0xFF, sizeof(buf));
    sbuf.writeString(hello);
    TEST_ASSERT_EQUAL(5, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 5, sbuf.bytesRemaining());
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL('o', *(ptr-1));

    sbuf.reset();
    sbuf.fillWithoutAdvancing(0xFF, sizeof(buf));
    sbuf.writeStringWithZeroTerminator(hello);
    TEST_ASSERT_EQUAL(6, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 6, sbuf.bytesRemaining());
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL(0, *(ptr-1));
    TEST_ASSERT_EQUAL('o', *(ptr-2));
}

void test_stream_buf_float()
{
    std::array<uint8_t, 256> buf;
    StreamBuf sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());
    sbuf.fillWithoutAdvancing(0xFF, sizeof(buf));
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());

    sbuf.writeFloat(18.9F);
    TEST_ASSERT_EQUAL(4, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 4, sbuf.bytesRemaining());

    sbuf.writeU8(7);

    sbuf.writeU16(19);

    sbuf.writeFloat(3.14159F);
    TEST_ASSERT_EQUAL(11, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 11, sbuf.bytesRemaining());

    sbuf.switchToReader();
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(11, sbuf.bytesRemaining());

    const float v1 = sbuf.readFloat();
    TEST_ASSERT_EQUAL_FLOAT(18.9F, v1);
    TEST_ASSERT_EQUAL(7, sbuf.bytesRemaining());

    const uint16_t v2 = sbuf.readU8();
    TEST_ASSERT_EQUAL(7, v2);
    TEST_ASSERT_EQUAL(6, sbuf.bytesRemaining());

    const uint32_t v3 = sbuf.readU16();
    TEST_ASSERT_EQUAL(19, v3);
    TEST_ASSERT_EQUAL(4, sbuf.bytesRemaining());

    const float v4 = sbuf.readFloat();
    TEST_ASSERT_EQUAL_FLOAT(3.14159F, v4);
    TEST_ASSERT_EQUAL(0, sbuf.bytesRemaining());
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-magic-numbers)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_stream_buf_offset);
    RUN_TEST(test_stream_buf);
    RUN_TEST(test_stream_buf_big_endian);
    RUN_TEST(test_stream_buf_strings);
    RUN_TEST(test_stream_buf_float);

    UNITY_END();
}
