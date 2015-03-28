#include "conversion/cast.h"

// Google test library headers
#include <gtest/gtest.h>

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

TEST(Conversion, Base64)
{
    std::string valid("Zm9vYmFy");

    std::vector<char> data;
    stlencoders::base64<char>::decode(valid.begin(), valid.end(), std::back_inserter(data));

    {
        const std::string base64 = conv::cast<conv::Base64>(data);

        EXPECT_EQ(base64, valid);

        // back to binary
        std::vector<char> binary = conv::cast<std::vector<char>, conv::Base64>(base64);
        EXPECT_EQ(data, binary);
    }

    {
        const std::string src(data.begin(), data.end());
        const std::string base64 = conv::cast<conv::Base64>(src);
        EXPECT_EQ(base64, valid);

        // back to binary
        std::vector<char> base64Vector(base64.begin(), base64.end());

        std::vector<char> binary = conv::cast<std::vector<char>, conv::Base64>(base64Vector);
        EXPECT_EQ(data, binary);
    }

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
