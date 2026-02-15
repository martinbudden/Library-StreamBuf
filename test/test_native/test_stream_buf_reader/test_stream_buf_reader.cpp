#include "stream_buf_reader.h"
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
    StreamBufWriter sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytes_remaining());

    sbuf.write_u8(1);
    TEST_ASSERT_EQUAL(1, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(sizeof(buf) - 1, sbuf.bytes_remaining());

    sbuf.write_u16(2);
    TEST_ASSERT_EQUAL(3, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(sizeof(buf) - 3, sbuf.bytes_remaining());

    sbuf.write_u32(3);
    TEST_ASSERT_EQUAL(7, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(sizeof(buf) - 7, sbuf.bytes_remaining());

    StreamBufReader sbufReader(sbuf.reader());

    TEST_ASSERT_EQUAL(0, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(7, sbufReader.bytes_remaining());

    const uint8_t v1 = sbufReader.read_u8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(6, sbufReader.bytes_remaining());

    const uint16_t v2 = sbufReader.read_u16();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(4, sbufReader.bytes_remaining());

    const uint32_t v3 = sbufReader.read_u32();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(0, sbufReader.bytes_remaining());
}

void test_stream_buf_reader_strings()
{
    std::array<uint8_t, 256> buf;
    buf.fill(0xFF);
    StreamBufWriter sbuf(&buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytes_remaining());
    sbuf.fill_without_advancing(0xFF, sizeof(buf));
    TEST_ASSERT_EQUAL(sizeof(buf), sbuf.bytes_remaining());

    sbuf.write_string("Hello");
    TEST_ASSERT_EQUAL(5, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(sizeof(buf) - 5, sbuf.bytes_remaining());
    const uint8_t* ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL('o', *(ptr-1));
    TEST_ASSERT_EQUAL('l', *(ptr-2));
    TEST_ASSERT_EQUAL('l', *(ptr-3));
    TEST_ASSERT_EQUAL('e', *(ptr-4));
    TEST_ASSERT_EQUAL('H', *(ptr-5));

    const StreamBufReader sbufReader(sbuf.reader());
    ptr = sbufReader.ptr();
    TEST_ASSERT_EQUAL('H', ptr[0]);
    TEST_ASSERT_EQUAL('e', ptr[1]);
    TEST_ASSERT_EQUAL('l', ptr[2]);
    TEST_ASSERT_EQUAL('l', ptr[3]);
    TEST_ASSERT_EQUAL('o', ptr[4]);
    TEST_ASSERT_EQUAL(0xFF, ptr[5]);

    sbuf.switch_to_reader();
    ptr = sbuf.ptr();
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
