#include "conversion/cast.hpp"

// Google test library headers
#include <gtest/gtest.h>

using ::testing::Values;

enum Foo
{
    First = 0,
    Second = 1
};

std::ostream& operator << (std::ostream& s, const Foo t)
{
    s << "fail";
    return s;
}
std::istream& operator >> (std::istream& s, Foo&)
{
    return s;
}


struct Base64Test : public ::testing::TestWithParam<std::pair<std::string, std::string>>
{
};

TEST_P(Base64Test, Conversion)
{
    const std::string valid(GetParam().first);

    const auto expectedLen = stlencoders::base64<char>::max_encode_size(valid.size());

    const std::string base64 = conv::cast<conv::Base64>(valid);
    EXPECT_EQ(base64, GetParam().second);
    EXPECT_EQ(expectedLen, base64.size());

    std::vector<char> binary = conv::cast<std::vector<char>, conv::Base64>(base64);
    std::string result(binary.begin(), binary.end());
    EXPECT_EQ(result, valid);
}

INSTANTIATE_TEST_CASE_P(ConversionTest,
                        Base64Test,
                        Values(std::make_pair("", ""),
                               std::make_pair("M", "TQ=="),
                               std::make_pair("Ma", "TWE="),
                               std::make_pair("Man", "TWFu"),
                               std::make_pair("pleasure.", "cGxlYXN1cmUu"),
                               std::make_pair("leasure.", "bGVhc3VyZS4="),
                               std::make_pair("easure.", "ZWFzdXJlLg=="),
                               std::make_pair("asure.", "YXN1cmUu"),
                               std::make_pair("sure.", "c3VyZS4="))
);

TEST(Conversion, Enum)
{
    EXPECT_EQ(conv::cast<std::string>(First), "0");
    EXPECT_EQ(conv::cast<Foo>(1), Second);
    EXPECT_EQ(conv::cast<Foo>("1"), Second);
}

TEST(Conversion, Unicode)
{
	const std::wstring wide = L"Unicode wide";
	const std::string utf8 = conv::cast<std::string>(wide);

	EXPECT_EQ(wide, conv::cast<std::wstring>(utf8));

	const std::string ansi = conv::cast<conv::Ansi>(wide);

 	EXPECT_EQ(ansi, "Unicode wide");
	const std::wstring testWide = conv::cast<std::wstring, conv::Ansi>(ansi);
	EXPECT_EQ(testWide, wide);

    const std::string utfFromAnsi = conv::cast<std::string, conv::Ansi>(ansi);
    EXPECT_EQ(utf8, utfFromAnsi);

	static const char testConstNarrowString[] = "And ansi now";
	const std::wstring testWideFromAnsi = conv::cast<std::wstring, conv::Ansi>(testConstNarrowString);
	EXPECT_EQ(testWideFromAnsi, L"And ansi now");
}

TEST(Conversion, Types)
{
	const int value = 1234567890;
	const std::wstring stringValue = conv::cast<std::wstring>(value);

	EXPECT_EQ(stringValue, L"1234567890");
	EXPECT_EQ(value, conv::cast<int>(L"1234567890"));
	EXPECT_THROW(conv::cast<int>(L"not_an_int"), conv::CastException);

	const std::string boolAsString = conv::cast<std::string>(false);
	EXPECT_EQ(boolAsString, "false");

	EXPECT_EQ(conv::cast<bool>(L"true"), true);
	EXPECT_EQ(conv::cast<bool>(L"1"), true);
	EXPECT_THROW(conv::cast<bool>(L"not_a_bool"), conv::CastException);

	const std::string floatAsString = conv::cast<std::string>(0.1234567891f);
	EXPECT_EQ(floatAsString, "0.123456791");

	EXPECT_EQ(conv::cast<float>(L"0.123456791"), 0.1234567891f);
	EXPECT_THROW(conv::cast<float>(L"not_a_float"), conv::CastException);

    const unsigned char charValue = 33;
    const std::wstring convertedChar = conv::cast<std::wstring>(charValue);
    EXPECT_EQ(convertedChar, L"33");

    const unsigned char charConverted = conv::cast<unsigned char>(convertedChar);
    EXPECT_EQ(charValue, charConverted);
}

TEST(Conversion, Bits)
{
    unsigned result = 13925428;

    const std::vector<unsigned> toStore = conv::cast<std::vector<unsigned>>(result);
    const unsigned restored = conv::cast<unsigned>(toStore);

    EXPECT_EQ(result, restored);
}

TEST(Conversion, DefaultBinaryToString)
{
    const std::vector<char> source = {0, 1, 2, 3, 4, 5};
    
    std::string str;
    stlencoders::base64<char>::encode(source.begin(), source.end(), std::back_inserter(str));

    const std::vector<char> bin = conv::cast<std::vector<char>>(str);

    const std::string result = conv::cast<std::string>(bin);
    EXPECT_EQ(str, result);
};

TEST(Conversion, Hex)
{
    const std::vector<char> source = {char(255), 0, 1, 2, 3, 4, 5, 25, 64, char(255), 100, -100, -20, -10};
    const std::string hex = conv::cast<conv::Hex>(source);

    // back to binary
    std::vector<char> binary = conv::cast<std::vector<char>, conv::Hex>(hex);
    EXPECT_EQ(source, binary);
};


TEST(Conversion, Time)
{
    {
        const auto now = boost::posix_time::microsec_clock::local_time();
        const std::string str = conv::cast<std::string>(now);

        const auto test = conv::cast<boost::posix_time::ptime>(str);
        EXPECT_EQ(now, test);
    }
    {
        const auto test = conv::cast<boost::posix_time::ptime>("2014-11-12T06:34:20Z");
        const std::string str = conv::cast<std::string>(test);
        EXPECT_EQ(str, "2014-11-12T06:34:20");
    }
    
    {
        const std::string src("2014-10-15T17:41:52.724658");

        boost::posix_time::ptime pt;
        std::istringstream is(src);
        is.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet("%Y-%m-%dT%H:%M:%S%f")));
        is >> pt;

        const auto str = conv::cast<std::string>(pt);

        EXPECT_EQ(src, str);

        const auto time = conv::cast<boost::posix_time::ptime>(str);

        EXPECT_EQ(pt, time);
    }
}



GTEST_API_ int main(int argc, char **argv) 
{
  std::cout << "Running main() from gtest_main.cc\n";

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
