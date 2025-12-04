#include "StreamBuf.h"
#include "StreamBufReader.h"
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
    enum { BUF_SIZE = 32 };
    std::array<uint8_t, BUF_SIZE> buf;
    StreamBufTest sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(0, sbuf.ptrOffset());
}

void test_stream_buf()
{
    enum { BUF_SIZE = 256 };
    std::array<uint8_t, BUF_SIZE> buf;
    StreamBuf sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());

    sbuf.writeU8(1);
    TEST_ASSERT_EQUAL(1, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 1, sbuf.bytesRemaining());

    sbuf.writeU16(2);
    TEST_ASSERT_EQUAL(3, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 3, sbuf.bytesRemaining());

    sbuf.writeU32(3);
    TEST_ASSERT_EQUAL(7, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 7, sbuf.bytesRemaining());

    StreamBufReader sbufReader(sbuf.reader());
    TEST_ASSERT_EQUAL(0, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(7, sbufReader.bytesRemaining());

    uint8_t v1 = sbufReader.readU8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(6, sbufReader.bytesRemaining());

    uint16_t v2 = sbufReader.readU16();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(4, sbufReader.bytesRemaining());

    uint32_t v3 = sbufReader.readU32();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(0, sbufReader.bytesRemaining());


    sbuf.switchToReader();
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(7, sbuf.bytesRemaining());

    v1 = sbuf.readU8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(6, sbuf.bytesRemaining());

    v2 = sbuf.readU16();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(4, sbuf.bytesRemaining());

    v3 = sbuf.readU32();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(0, sbuf.bytesRemaining());
}

void test_stream_buf_big_endian()
{
    enum { BUF_SIZE = 256 };
    std::array<uint8_t, BUF_SIZE> buf;
    StreamBuf sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());

    sbuf.writeU8(1);
    TEST_ASSERT_EQUAL(BUF_SIZE - 1, sbuf.bytesRemaining());

    sbuf.writeU16_BigEndian(2);
    TEST_ASSERT_EQUAL(BUF_SIZE - 3, sbuf.bytesRemaining());

    sbuf.writeU32_BigEndian(3);
    TEST_ASSERT_EQUAL(BUF_SIZE - 7, sbuf.bytesRemaining());

    StreamBufReader sbufReader(sbuf.reader());
    TEST_ASSERT_EQUAL(0, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(7, sbufReader.bytesRemaining());

    uint8_t v1 = sbufReader.readU8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(6, sbufReader.bytesRemaining());

    uint16_t v2 = sbufReader.readU16_BigEndian();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(4, sbufReader.bytesRemaining());

    uint32_t v3 = sbufReader.readU32_BigEndian();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(0, sbufReader.bytesRemaining());


    sbuf.switchToReader();
    TEST_ASSERT_EQUAL(7, sbuf.bytesRemaining());

    v1 = sbuf.readU8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(6, sbuf.bytesRemaining());

    v2 = sbuf.readU16_BigEndian();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(4, sbuf.bytesRemaining());

    v3 = sbuf.readU32_BigEndian();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(0, sbuf.bytesRemaining());
}

void test_stream_buf_size()
{
    enum { BUF_SIZE = 2 };
    std::array<uint8_t, BUF_SIZE + 1> buf;
    buf.fill(0xFF);
    TEST_ASSERT_EQUAL(0xFF, buf[0]);
    TEST_ASSERT_EQUAL(0xFF, buf[1]);
    TEST_ASSERT_EQUAL(0xFF, buf[2]); // cppcheck-suppress containerOutOfBounds

    StreamBuf sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(true, sbuf.isEmpty());
    TEST_ASSERT_EQUAL(false, sbuf.isFull());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());

    sbuf.writeU16_Checked(0xABCD);

    TEST_ASSERT_EQUAL(0, sbuf.bytesRemaining());
    TEST_ASSERT_EQUAL(2, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(false, sbuf.isEmpty());
    TEST_ASSERT_EQUAL(true, sbuf.isFull());

    TEST_ASSERT_EQUAL(0xCD, buf[0]);
    TEST_ASSERT_EQUAL(0xAB, buf[1]);
    TEST_ASSERT_EQUAL(0xFF, buf[2]); // cppcheck-suppress containerOutOfBounds

    sbuf.reset();
    TEST_ASSERT_EQUAL(true, sbuf.isEmpty());
    TEST_ASSERT_EQUAL(false, sbuf.isFull());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());

    sbuf.writeU32_Checked(0xABCD1234);
    TEST_ASSERT_EQUAL(true, sbuf.isEmpty());
    TEST_ASSERT_EQUAL(false, sbuf.isFull());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
}

void test_stream_buf_strings()
{
    enum { BUF_SIZE = 6 };
    std::array<uint8_t, BUF_SIZE + 1> buf;
    StreamBuf sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());
    sbuf.fillWithoutAdvancing(0xFF, BUF_SIZE);
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());
    const uint8_t* ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, ptr[0]);
    TEST_ASSERT_EQUAL(0xFF, ptr[1]);
    TEST_ASSERT_EQUAL(0xFF, ptr[2]);
    TEST_ASSERT_EQUAL(0xFF, ptr[3]);
    TEST_ASSERT_EQUAL(0xFF, ptr[4]);
    TEST_ASSERT_EQUAL(0xFF, ptr[5]);

    sbuf.writeString("Hello");
    TEST_ASSERT_EQUAL(5, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 5, sbuf.bytesRemaining());
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(sbuf.end() - 2, ptr);
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL('o', *(ptr-1));
    TEST_ASSERT_EQUAL('l', *(ptr-2));
    TEST_ASSERT_EQUAL('l', *(ptr-3));
    TEST_ASSERT_EQUAL('e', *(ptr-4));
    TEST_ASSERT_EQUAL('H', *(ptr-5));

    sbuf.reset();
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());
    sbuf.fillWithoutAdvancing(0xFF, BUF_SIZE);

    sbuf.writeStringWithZeroTerminator("Hello");
    TEST_ASSERT_EQUAL(6, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 6, sbuf.bytesRemaining());
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(sbuf.end() - 1, ptr);
    TEST_ASSERT_EQUAL(0, *(ptr-1));
    TEST_ASSERT_EQUAL('o', *(ptr-2));
    TEST_ASSERT_EQUAL('l', *(ptr-3));
    TEST_ASSERT_EQUAL('l', *(ptr-4));
    TEST_ASSERT_EQUAL('e', *(ptr-5));
    TEST_ASSERT_EQUAL('H', *(ptr-6));

    const std::string hello("Hello");

    sbuf.reset();
    sbuf.fillWithoutAdvancing(0xFF, BUF_SIZE);
    sbuf.writeString(hello);
    TEST_ASSERT_EQUAL(5, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 5, sbuf.bytesRemaining());
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL('o', *(ptr-1));
    TEST_ASSERT_EQUAL('l', *(ptr-2));
    TEST_ASSERT_EQUAL('l', *(ptr-3));
    TEST_ASSERT_EQUAL('e', *(ptr-4));
    TEST_ASSERT_EQUAL('H', *(ptr-5));


    sbuf.reset();
    sbuf.fillWithoutAdvancing(0xFF, BUF_SIZE);
    sbuf.writeStringWithZeroTerminator(hello);
    TEST_ASSERT_EQUAL(6, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 6, sbuf.bytesRemaining());
    ptr = sbuf.ptr();
    //TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL(0, *(ptr-1));
    TEST_ASSERT_EQUAL('o', *(ptr-2));
    TEST_ASSERT_EQUAL('l', *(ptr-3));
    TEST_ASSERT_EQUAL('l', *(ptr-4));
    TEST_ASSERT_EQUAL('e', *(ptr-5));
    TEST_ASSERT_EQUAL('H', *(ptr-6));
}

void test_stream_buf_float()
{
    enum { BUF_SIZE = 256 };
    std::array<uint8_t, BUF_SIZE> buf;
    StreamBuf sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());
    sbuf.fillWithoutAdvancing(0xFF, BUF_SIZE);
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytesRemaining());

    sbuf.writeFloat(18.9F);
    TEST_ASSERT_EQUAL(4, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 4, sbuf.bytesRemaining());

    sbuf.writeU8(7);

    sbuf.writeU16(19);

    sbuf.writeFloat(3.14159F);
    TEST_ASSERT_EQUAL(11, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(BUF_SIZE - 11, sbuf.bytesRemaining());

    StreamBufReader sbufReader(sbuf.reader());
    TEST_ASSERT_EQUAL(0, sbufReader.bytesRead());
    TEST_ASSERT_EQUAL(11, sbufReader.bytesRemaining());

    float v1 = sbufReader.readFloat();
    TEST_ASSERT_EQUAL_FLOAT(18.9F, v1);
    TEST_ASSERT_EQUAL(7, sbufReader.bytesRemaining());

    uint16_t v2 = sbufReader.readU8();
    TEST_ASSERT_EQUAL(7, v2);
    TEST_ASSERT_EQUAL(6, sbufReader.bytesRemaining());

    uint32_t v3 = sbufReader.readU16();
    TEST_ASSERT_EQUAL(19, v3);
    TEST_ASSERT_EQUAL(4, sbufReader.bytesRemaining());

    float v4 = sbufReader.readFloat();
    TEST_ASSERT_EQUAL_FLOAT(3.14159F, v4);
    TEST_ASSERT_EQUAL(0, sbufReader.bytesRemaining());


    sbuf.switchToReader();
    TEST_ASSERT_EQUAL(0, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(11, sbuf.bytesRemaining());

    v1 = sbuf.readFloat();
    TEST_ASSERT_EQUAL_FLOAT(18.9F, v1);
    TEST_ASSERT_EQUAL(7, sbuf.bytesRemaining());

    v2 = sbuf.readU8();
    TEST_ASSERT_EQUAL(7, v2);
    TEST_ASSERT_EQUAL(6, sbuf.bytesRemaining());

    v3 = sbuf.readU16();
    TEST_ASSERT_EQUAL(19, v3);
    TEST_ASSERT_EQUAL(4, sbuf.bytesRemaining());

    v4 = sbuf.readFloat();
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
    RUN_TEST(test_stream_buf_size);
    RUN_TEST(test_stream_buf_strings);
    RUN_TEST(test_stream_buf_float);

    UNITY_END();
}
