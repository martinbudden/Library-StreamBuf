#include "StreamBufReader.h"
#include <array>
#include <unity.h>

void setUp()
{
}

void tearDown()
{
}

class StreamBufReaderTest : public StreamBufReader {
public:
    StreamBufReaderTest(const uint8_t* ptr, size_t len) : StreamBufReader(ptr, len) {}
    StreamBufReaderTest(const uint8_t* ptr, const uint8_t* end) : StreamBufReader(ptr, end) {}
public:
    size_t ptrOffset() { return offsetof(StreamBufReaderTest, _ptr); }
};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-magic-numbers)
void test_stream_buf_reader_offset()
{
    std::array<uint8_t, 32> buf;
    StreamBufReaderTest sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(0, sbuf.ptrOffset());
}

void test_stream_buf_reader()
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

    StreamBufReader sbufReader(sbuf);
    sbufReader.switchToReader();

    TEST_ASSERT_EQUAL(0, sbufReader.bytesWritten());
    TEST_ASSERT_EQUAL(7, sbufReader.bytesRemaining());

    const uint8_t v1 = sbufReader.readU8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbufReader.bytesWritten());
    TEST_ASSERT_EQUAL(6, sbufReader.bytesRemaining());

    const uint16_t v2 = sbufReader.readU16();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbufReader.bytesWritten());
    TEST_ASSERT_EQUAL(4, sbufReader.bytesRemaining());

    const uint32_t v3 = sbufReader.readU32();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbufReader.bytesWritten());
    TEST_ASSERT_EQUAL(0, sbufReader.bytesRemaining());
}

void test_stream_buf_reader_strings()
{
    std::array<uint8_t, 256> buf;
    buf.fill(0xFF);
    StreamBuf sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());
    sbuf.fillWithoutAdvancing(0xFF, sizeof(buf));
    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytesRemaining());

    sbuf.writeString("Hello");
    TEST_ASSERT_EQUAL(5, sbuf.bytesWritten());
    TEST_ASSERT_EQUAL(sizeof(buf) - 5, sbuf.bytesRemaining());
    const uint8_t* ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL('o', *(ptr-1));
    TEST_ASSERT_EQUAL('l', *(ptr-2));
    TEST_ASSERT_EQUAL('l', *(ptr-3));

    sbuf.switchToReader();
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL('H', ptr[0]);
    TEST_ASSERT_EQUAL('e', ptr[1]);
    TEST_ASSERT_EQUAL('l', ptr[2]);
    TEST_ASSERT_EQUAL('l', ptr[3]);
    TEST_ASSERT_EQUAL('o', ptr[4]);
    TEST_ASSERT_EQUAL(0xFF, ptr[5]);

    const StreamBufReader sbufReader(sbuf);
    ptr = sbufReader.ptr();
    TEST_ASSERT_EQUAL('H', ptr[0]);
    TEST_ASSERT_EQUAL('e', ptr[1]);
    TEST_ASSERT_EQUAL('l', ptr[2]);
    TEST_ASSERT_EQUAL('l', ptr[3]);
    TEST_ASSERT_EQUAL('o', ptr[4]);
    TEST_ASSERT_EQUAL(0xFF, ptr[5]);

    std::array<uint8_t, 256> buf2;
    StreamBuf sbuf2(&buf2[0], sizeof(buf2));
    StreamBufReader sbufReader2(sbuf2);
    sbufReader2 = sbuf;
    ptr = sbufReader2.ptr();
    TEST_ASSERT_EQUAL('H', ptr[0]);
    TEST_ASSERT_EQUAL('e', ptr[1]);
    TEST_ASSERT_EQUAL('l', ptr[2]);
    TEST_ASSERT_EQUAL('l', ptr[3]);
    TEST_ASSERT_EQUAL('o', ptr[4]);
    TEST_ASSERT_EQUAL(0xFF, ptr[5]);
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-magic-numbers)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_stream_buf_reader_offset);
    RUN_TEST(test_stream_buf_reader);
    RUN_TEST(test_stream_buf_reader_strings);

    UNITY_END();
}
