#include "stream_buf_writer.h"
#include "stream_buf_reader.h"
#include <array>
#include <unity.h>

void setUp()
{
}

void tearDown()
{
}

class StreamBufTest : public StreamBufWriter {
public:
    StreamBufTest(uint8_t* ptr, size_t len) : StreamBufWriter (ptr, len) {}
    StreamBufTest(uint8_t* ptr, uint8_t* end) : StreamBufWriter (ptr, end) {}
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
    StreamBufWriter sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());

    sbuf.write_u8(1);
    TEST_ASSERT_EQUAL(1, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 1, sbuf.bytes_remaining());

    sbuf.write_u16(2);
    TEST_ASSERT_EQUAL(3, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 3, sbuf.bytes_remaining());

    sbuf.write_u32(3);
    TEST_ASSERT_EQUAL(7, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 7, sbuf.bytes_remaining());

    StreamBufReader sbufReader(sbuf.reader());
    TEST_ASSERT_EQUAL(0, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(7, sbufReader.bytes_remaining());

    uint8_t v1 = sbufReader.read_u8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(6, sbufReader.bytes_remaining());

    uint16_t v2 = sbufReader.read_u16();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(4, sbufReader.bytes_remaining());

    uint32_t v3 = sbufReader.read_u32();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(0, sbufReader.bytes_remaining());


    sbuf.switch_to_reader();
    TEST_ASSERT_EQUAL(0, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(7, sbuf.bytes_remaining());

    v1 = sbuf.read_u8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(6, sbuf.bytes_remaining());

    v2 = sbuf.read_u16();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(4, sbuf.bytes_remaining());

    v3 = sbuf.read_u32();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(0, sbuf.bytes_remaining());
}

void test_stream_buf_big_endian()
{
    enum { BUF_SIZE = 256 };
    std::array<uint8_t, BUF_SIZE> buf;
    StreamBufWriter sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());

    sbuf.write_u8(1);
    TEST_ASSERT_EQUAL(BUF_SIZE - 1, sbuf.bytes_remaining());

    sbuf.write_u16_big_endian(2);
    TEST_ASSERT_EQUAL(BUF_SIZE - 3, sbuf.bytes_remaining());

    sbuf.write_u32_big_endian(3);
    TEST_ASSERT_EQUAL(BUF_SIZE - 7, sbuf.bytes_remaining());

    StreamBufReader sbufReader(sbuf.reader());
    TEST_ASSERT_EQUAL(0, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(7, sbufReader.bytes_remaining());

    uint8_t v1 = sbufReader.read_u8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(1, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(6, sbufReader.bytes_remaining());

    uint16_t v2 = sbufReader.read_u16_big_endian();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(3, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(4, sbufReader.bytes_remaining());

    uint32_t v3 = sbufReader.read_u32_big_endian();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(7, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(0, sbufReader.bytes_remaining());


    sbuf.switch_to_reader();
    TEST_ASSERT_EQUAL(7, sbuf.bytes_remaining());

    v1 = sbuf.read_u8();
    TEST_ASSERT_EQUAL(1, v1);
    TEST_ASSERT_EQUAL(6, sbuf.bytes_remaining());

    v2 = sbuf.read_u16_big_endian();
    TEST_ASSERT_EQUAL(2, v2);
    TEST_ASSERT_EQUAL(4, sbuf.bytes_remaining());

    v3 = sbuf.read_u32_big_endian();
    TEST_ASSERT_EQUAL(3, v3);
    TEST_ASSERT_EQUAL(0, sbuf.bytes_remaining());
}

void test_stream_buf_size()
{
    enum { BUF_SIZE = 2 };
    std::array<uint8_t, BUF_SIZE + 1> buf;
    buf.fill(0xFF);
    TEST_ASSERT_EQUAL(0xFF, buf[0]);
    TEST_ASSERT_EQUAL(0xFF, buf[1]);
    TEST_ASSERT_EQUAL(0xFF, buf[2]); // cppcheck-suppress containerOutOfBounds

    StreamBufWriter sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(true, sbuf.is_empty());
    TEST_ASSERT_EQUAL(false, sbuf.is_full());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());
    TEST_ASSERT_EQUAL(0, sbuf.bytes_written());

    sbuf.write_u16_checked(0xABCD);

    TEST_ASSERT_EQUAL(0, sbuf.bytes_remaining());
    TEST_ASSERT_EQUAL(2, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(false, sbuf.is_empty());
    TEST_ASSERT_EQUAL(true, sbuf.is_full());

    TEST_ASSERT_EQUAL(0xCD, buf[0]);
    TEST_ASSERT_EQUAL(0xAB, buf[1]);
    TEST_ASSERT_EQUAL(0xFF, buf[2]); // cppcheck-suppress containerOutOfBounds

    sbuf.reset();
    TEST_ASSERT_EQUAL(true, sbuf.is_empty());
    TEST_ASSERT_EQUAL(false, sbuf.is_full());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());
    TEST_ASSERT_EQUAL(0, sbuf.bytes_written());

    sbuf.write_u32_checked(0xABCD1234);
    TEST_ASSERT_EQUAL(true, sbuf.is_empty());
    TEST_ASSERT_EQUAL(false, sbuf.is_full());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());
    TEST_ASSERT_EQUAL(0, sbuf.bytes_written());
}

void test_stream_buf_strings()
{
    enum { BUF_SIZE = 6 };
    std::array<uint8_t, BUF_SIZE + 1> buf;
    StreamBufWriter sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());
    sbuf.fill_without_advancing(0xFF, BUF_SIZE);
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());
    const uint8_t* ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, ptr[0]);
    TEST_ASSERT_EQUAL(0xFF, ptr[1]);
    TEST_ASSERT_EQUAL(0xFF, ptr[2]);
    TEST_ASSERT_EQUAL(0xFF, ptr[3]);
    TEST_ASSERT_EQUAL(0xFF, ptr[4]);
    TEST_ASSERT_EQUAL(0xFF, ptr[5]);

    sbuf.write_string("Hello");
    TEST_ASSERT_EQUAL(5, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 5, sbuf.bytes_remaining());
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(sbuf.end() - 2, ptr);
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL('o', *(ptr-1));
    TEST_ASSERT_EQUAL('l', *(ptr-2));
    TEST_ASSERT_EQUAL('l', *(ptr-3));
    TEST_ASSERT_EQUAL('e', *(ptr-4));
    TEST_ASSERT_EQUAL('H', *(ptr-5));

    sbuf.reset();
    TEST_ASSERT_EQUAL(0, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());
    sbuf.fill_without_advancing(0xFF, BUF_SIZE);

    sbuf.write_string_with_zero_terminator("Hello");
    TEST_ASSERT_EQUAL(6, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 6, sbuf.bytes_remaining());
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
    sbuf.fill_without_advancing(0xFF, BUF_SIZE);
    sbuf.write_string(hello);
    TEST_ASSERT_EQUAL(5, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 5, sbuf.bytes_remaining());
    ptr = sbuf.ptr();
    TEST_ASSERT_EQUAL(0xFF, *ptr);
    TEST_ASSERT_EQUAL('o', *(ptr-1));
    TEST_ASSERT_EQUAL('l', *(ptr-2));
    TEST_ASSERT_EQUAL('l', *(ptr-3));
    TEST_ASSERT_EQUAL('e', *(ptr-4));
    TEST_ASSERT_EQUAL('H', *(ptr-5));


    sbuf.reset();
    sbuf.fill_without_advancing(0xFF, BUF_SIZE);
    sbuf.write_string_with_zero_terminator(hello);
    TEST_ASSERT_EQUAL(6, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 6, sbuf.bytes_remaining());
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
    /*enum { BUF_SIZE = 256 };
    std::array<uint8_t, BUF_SIZE> buf;
    StreamBufWriter sbuf(&buf[0], BUF_SIZE);

    TEST_ASSERT_EQUAL(0, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());
    sbuf.fill_without_advancing(0xFF, BUF_SIZE);
    TEST_ASSERT_EQUAL(0, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE, sbuf.bytes_remaining());

    sbuf.write_f32(18.9F);
    TEST_ASSERT_EQUAL(4, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 4, sbuf.bytes_remaining());

    sbuf.write_u8(7);

    sbuf.write_u16(19);

    sbuf.write_f32(3.14159F);
    TEST_ASSERT_EQUAL(11, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(BUF_SIZE - 11, sbuf.bytes_remaining());

    StreamBufReader sbufReader(sbuf.reader());
    TEST_ASSERT_EQUAL(0, sbufReader.bytes_read());
    TEST_ASSERT_EQUAL(11, sbufReader.bytes_remaining());

    float v1 = sbufReader.read_f32();
    TEST_ASSERT_EQUAL_FLOAT(18.9F, v1);
    TEST_ASSERT_EQUAL(7, sbufReader.bytes_remaining());

    uint16_t v2 = sbufReader.read_u8();
    TEST_ASSERT_EQUAL(7, v2);
    TEST_ASSERT_EQUAL(6, sbufReader.bytes_remaining());

    uint32_t v3 = sbufReader.read_u16();
    TEST_ASSERT_EQUAL(19, v3);
    TEST_ASSERT_EQUAL(4, sbufReader.bytes_remaining());

    float v4 = sbufReader.read_f32();
    TEST_ASSERT_EQUAL_FLOAT(3.14159F, v4);
    TEST_ASSERT_EQUAL(0, sbufReader.bytes_remaining());


    sbuf.switch_to_reader();
    TEST_ASSERT_EQUAL(0, sbuf.bytes_written());
    TEST_ASSERT_EQUAL(11, sbuf.bytes_remaining());

    v1 = sbuf.read_f32();
    TEST_ASSERT_EQUAL_FLOAT(18.9F, v1);
    TEST_ASSERT_EQUAL(7, sbuf.bytes_remaining());

    v2 = sbuf.read_u8();
    TEST_ASSERT_EQUAL(7, v2);
    TEST_ASSERT_EQUAL(6, sbuf.bytes_remaining());

    v3 = sbuf.read_u16();
    TEST_ASSERT_EQUAL(19, v3);
    TEST_ASSERT_EQUAL(4, sbuf.bytes_remaining());

    v4 = sbuf.read_f32();
    TEST_ASSERT_EQUAL_FLOAT(3.14159F, v4);
    TEST_ASSERT_EQUAL(0, sbuf.bytes_remaining());*/
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
    //RUN_TEST(test_stream_buf_float);

    UNITY_END();
}
