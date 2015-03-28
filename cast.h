#ifndef Conversion_h__
#define Conversion_h__

#include <string>
#include <utility>

#include "stlencoders/base64.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/mpl/or.hpp>
#pragma warning(push)
#pragma warning(disable:4244) // 'argument' : conversion from 'boost::locale::utf::code_point' to 'const wchar_t', possible loss of data
#include <boost/locale/encoding.hpp>
#pragma warning(pop)

#include <boost/exception/errinfo_nested_exception.hpp>
#include <boost/exception/errinfo_type_info_name.hpp>
#include <boost/exception/detail/exception_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/algorithm/hex.hpp>

namespace conv
{
    struct CastException : virtual boost::exception, std::exception { };

    struct Ansi {};
    struct Base64 {};
    struct Hex {};

	namespace details
	{
        //! Type traits
        template<typename T>
        struct TypeTraits
        {
            typedef T Type;
        };

        template<>
        struct TypeTraits<Ansi>
        {
            typedef std::string Type;
        };

        template<>
        struct TypeTraits<Base64>
        {
            typedef std::string Type;
        };

        template<>
        struct TypeTraits<Hex>
        {
            typedef std::string Type;
        };

		//! Customized stream type for boolean
		struct Boolean 
		{
			bool m_Data;
			Boolean() {}
			Boolean(bool data) : m_Data(data) {}
			operator bool() const { return m_Data; }

			template<typename T>
			friend std::basic_ostream<T>& operator << (std::basic_ostream<T>& out, const Boolean& b) 
			{
				out << std::boolalpha << b.m_Data;
				return out;
			}

			template<typename T>
			friend std::basic_istream<T> & operator >> (std::basic_istream<T>& in, Boolean& b) 
			{
				in >> std::boolalpha >> b.m_Data;
				if (in.fail())
				{
					in.unsetf(std::ios_base::boolalpha);
					in.clear();
					in >> b.m_Data;
				}
				return in;
			}
		};

		template<typename Target, typename Source>
		typename boost::disable_if
		<
			boost::mpl::or_<boost::is_enum<Target>, boost::is_enum<Source> >,
			Target
		>::type CastImpl(const Source& src)
		{
			try 
			{
				return boost::lexical_cast<typename boost::remove_cv<Target>::type>(src);
			}
            catch (const boost::bad_lexical_cast&)
            {
                BOOST_THROW_EXCEPTION(CastException()
                    << boost::errinfo_nested_exception(boost::current_exception())
                    << boost::errinfo_type_info_name(typeid(Source).name())
                );
            }
		}

		template<typename Target, typename Source>
		typename boost::enable_if
		<
			boost::is_enum<Target>,
			Target
		>::type CastImpl(const Source& src)
		{
			try 
			{
				const int value = boost::lexical_cast<int>(src);
				return static_cast<typename boost::remove_cv<Target>::type>(value);
			}
            catch (const boost::bad_lexical_cast&)
            {
                BOOST_THROW_EXCEPTION(CastException() 
                    << boost::errinfo_nested_exception(boost::current_exception())
                    << boost::errinfo_type_info_name(typeid(Source).name())
                );
            }
        }

        template<typename Target, typename Source>
		typename boost::enable_if
		<
			 boost::is_enum<Source>,
			 Target
		>::type CastImpl(const Source& src)
		{
			try 
			{
				return boost::lexical_cast<typename boost::remove_cv<Target>::type>(static_cast<int>(src));
			}
            catch (const boost::bad_lexical_cast&)
            {
                BOOST_THROW_EXCEPTION(CastException()
                    << boost::errinfo_nested_exception(boost::current_exception())
                    << boost::errinfo_type_info_name(typeid(Source).name())
                );
            }
        }

		//! Help template struct
		template<typename Target, typename Source>
		struct Caster
		{
            Target operator () (const Source& src)
			{
				return CastImpl<Target, Source>(src);
			}
		};

		//! Specialized bool help struct
		template<typename Source>
		struct Caster<bool, Source>
		{
            bool operator () (const Source& src)
			{
				return CastImpl<Boolean, Source>(src);
			}
		};

		//! Specialized bool help struct
		template<typename Target>
		struct Caster<Target, bool>
		{
            Target operator () (const bool src)
			{
				return CastImpl<Target, Boolean>(src);
			}
		};

        //! Specialized byte help struct
        template<typename Source>
        struct Caster<unsigned char, Source>
        {
            unsigned char operator () (const Source& src)
            {
                return static_cast<unsigned char>(CastImpl<unsigned, Source>(src));
            }
        };

        //! Specialized byte help struct
        template<typename Target>
        struct Caster<Target, unsigned char>
        {
            Target operator () (const unsigned char src)
            {
                return CastImpl<Target, unsigned>(src);
            }
        };

		//! Specialized unicode to utf8 struct
		template<>
		struct Caster<std::string, std::wstring>
		{
            std::string operator () (const std::wstring& src)
			{
				return boost::locale::conv::from_utf<wchar_t>(src, "utf8");
			}
		};

		//! Specialized utf8 to unicode struct
		template<>
		struct Caster<std::wstring, std::string>
		{
            std::wstring operator () (const std::string& src)
			{
				return boost::locale::conv::utf_to_utf<wchar_t, char>(src);
			}
		};

		//! Specialized unicode to utf8 help struct
		template<>
		struct Caster<std::string, const wchar_t*>
		{
            std::string operator () (const wchar_t* src)
			{
				return src ? boost::locale::conv::from_utf<wchar_t>(src, "utf8") : std::string();
			}
		};

		//! Specialized utf8 to unicode help struct
		template<>
		struct Caster<std::wstring, const char*>
		{
            std::wstring operator () (const char* src)
			{
				return src ? boost::locale::conv::utf_to_utf<wchar_t, char>(src) : std::wstring();
			}
		};

		//! Specialized ansi to utf8 help struct
		template<>
		struct Caster<std::string, Ansi>
		{
            std::string operator () (const std::string& src)
			{
				return boost::locale::conv::to_utf<char>(src, "cp1251");
			}
		};

		//! Specialized ansi to unicode help struct
		template<>
		struct Caster<std::wstring, Ansi>
		{
            std::wstring operator () (const std::string& src)
			{
				return boost::locale::conv::to_utf<wchar_t>(src, "cp1251");
			}
		};

		//! Specialized unicode to ansi help struct
		template<>
		struct Caster<Ansi, std::wstring>
		{
            std::string operator () (const std::wstring& src)
			{
				return boost::locale::conv::from_utf(src, "cp1251");
			}
		};

        //! Specialized help struct - conversion bits from integer to vector
        template<>
        struct Caster<std::vector<unsigned>, unsigned>
        {
            std::vector<unsigned> operator () (const unsigned src)
            {
                std::vector<unsigned> result;

                if (!src)
                    return result;

                unsigned mask = 1;
                unsigned counter = 0;
                for (; mask; mask <<= 1, ++counter)
                {
                    if (src & mask)
                        result.push_back(counter);
                }

                return result;
            }
        };

        //! Specialized help struct - conversion vector of values to bits
        template<>
        struct Caster<unsigned, std::vector<unsigned>>
        {
            unsigned operator () (const std::vector<unsigned>& src)
            {
                unsigned result = 0;

                if (src.empty())
                    return result;

                for (const unsigned bit : src)
                    result |= 1 << bit;

                return result;
            }
        };

        //! Specialized help struct - conversion posix time to 
        template<>
        struct Caster<boost::uint64_t, boost::posix_time::ptime>
        {
            boost::uint64_t operator () (const boost::posix_time::ptime& src)
            {
                using namespace boost::posix_time;
                static const ptime epoch(boost::gregorian::date(1970, 1, 1));
                time_duration diff(src - epoch);
                return diff.total_milliseconds();
            }
        };

        //! Specialized help struct - conversion int64 to posix time
        template<>
        struct Caster<boost::posix_time::ptime, boost::uint64_t>
        {
            boost::posix_time::ptime operator () (const boost::uint64_t& src)
            {
                using namespace boost::posix_time;
                static const ptime epoch(boost::gregorian::date(1970, 1, 1));
                epoch + boost::posix_time::milliseconds(src);
                return epoch + boost::posix_time::milliseconds(src);
            }
        };

        //! Specialized help struct - conversion posix time to string
        template<>
        struct Caster<std::string, boost::posix_time::ptime>
        {
            std::string operator () (const boost::posix_time::ptime& src)
            {
                return boost::posix_time::to_iso_extended_string(src);
            }
        };

        //! Specialized help struct - conversion string to posix time
        template<>
        struct Caster<boost::posix_time::ptime, std::string>
        {
            boost::posix_time::ptime operator () (const std::string& src)
            {
                boost::posix_time::ptime pt;
                std::istringstream is((!src.empty() && src.back() == 'Z') ? src.substr(0, src.size() - 1) : src);
                is.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet("%Y-%m-%dT%H:%M:%S%f")));
                is >> pt;
                return pt;
            }
        };

        template<typename T>
        struct CharTraits
        {
            typedef T type;
        };

        template<>
        struct CharTraits<char>
        {
            typedef char type;
        };
        template<>
        struct CharTraits<unsigned char>
        {
            typedef char type;
        };

        //! Bin to base64 help struct
        template<typename Source>
        struct Caster<Base64, Source>
        {
            std::string operator () (const Source& src)
            {
                if (src.empty())
                    return std::string();

                std::string result;
                stlencoders::base64<typename CharTraits<typename Source::value_type>::type>::encode(src.begin(), src.end(), std::back_inserter(result));
                return result;
            }
        };

        template<typename Target>
        struct Caster<Target, Base64>
        {
            template<typename T>
            Target operator () (const T& src)
            {
                if (src.empty())
                    return Target();

                Target result;
                stlencoders::base64<typename CharTraits<typename T::value_type>::type>::decode(src.begin(), src.end(), std::back_inserter(result));
                return result;
            }
        };

        //! Base64 to binary help struct
        template<>
        struct Caster<std::vector<char>, std::string>
        {
            std::vector<char> operator () (const std::string& src)
            {
                return Caster<std::vector<char>, Base64>()(src);
            }
        };

        //! Binary to base64 help struct
        template<>
        struct Caster<std::string, std::vector<char> >
        {
            std::string operator () (const std::vector<char>& src)
            {
                return Caster<Base64, std::vector<char> >()(src);
            }
        };

        //! Base64 to binary help struct
        template<>
        struct Caster<std::vector<unsigned char>, std::string>
        {
            std::vector<unsigned char> operator () (const std::string& src)
            {
                return Caster<std::vector<unsigned char>, Base64>()(src);
            }
        };

        //! Binary to base64 help struct
        template<>
        struct Caster<std::string, std::vector<unsigned char> >
        {
            std::string operator () (const std::vector<unsigned char>& src)
            {
                return Caster<Base64, std::vector<unsigned char> >()(src);
            }
        };

        //! Bin to hex help struct
        template<>
        struct Caster<Hex, std::vector<char> >
        {
            std::string operator () (const std::vector<char>& src)
            {
                if (src.empty())
                    return std::string();

                std::string result;
                result.reserve(src.size() * 2);

                boost::algorithm::hex(src.begin(), src.end(), std::back_inserter(result));
                return result;
            }
        };


        //! Hex to binary help struct
        template<>
        struct Caster<std::vector<char>, Hex>
        {
            std::vector<char> operator () (const std::string& src)
            {
                std::vector<char> data;
                data.reserve(src.size() / 2);

                boost::algorithm::unhex(src.begin(), src.end(), std::back_inserter(data));
                return data;
            }
        };

	} // namespace details

    //! Cast function
    template<typename Target, typename Source>
    inline typename details::TypeTraits<Target>::Type cast(const Source& value)
    {
        return details::Caster<Target, Source>()(value);
    }

    //! Cast function
    template<typename Target, typename From, typename Source>
    inline typename details::TypeTraits<Target>::Type cast(const Source& value)
    {
        return details::Caster<Target, From>()(value);
    }

    //! Cast function for const strings
    template<typename Target, typename Source, size_t N>
    inline typename details::TypeTraits<Target>::Type cast(const Source (&value)[N])
    {
        typedef std::basic_string<Source> SourceString;
        return details::Caster<Target, SourceString>()(SourceString(value, N - 1));
    }

    //! Cast function
    template<typename Target, typename Source>
    inline typename details::TypeTraits<Target>::Type cast(const Source& value, const Target& def)
    {
        try
        {
            return details::Caster<Target, Source>()(value);
        }
        catch (const CastException&)
        {
            return def;
        }
    }

    //! Cast function
    template<typename Target, typename From, typename Source>
    inline typename details::TypeTraits<Target>::Type cast(const Source& value, const Target& def)
    {
        try
        {
            return details::Caster<Target, From>()(value);
        }
        catch (const CastException&)
        {
            return def;
        }

    }

    //! Cast function for const strings
    template<typename Target, typename Source, size_t N>
    inline typename details::TypeTraits<Target>::Type cast(const Source(&value)[N], const Target& def)
    {
        try
        {
            typedef std::basic_string<Source> SourceString;
            return details::Caster<Target, SourceString>()(SourceString(value, N - 1));
        }
        catch (const CastException&)
        {
            return def;
        }
    }
}


#endif // Conversion_h__
