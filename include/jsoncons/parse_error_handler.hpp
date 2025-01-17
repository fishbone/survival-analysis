/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_PARSE_ERROR_HANDLER_HPP
#define JSONCONS_PARSE_ERROR_HANDLER_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons {

class json_parse_exception : public json_exception
{
public:
    json_parse_exception(std::error_code ec,
                         unsigned long line,
                         unsigned long column)
        : error_code_(ec),
          line_number_(line),
          column_number_(column)
    {
    }
    json_parse_exception(const json_parse_exception& other)
        : error_code_(other.error_code_),
          line_number_(other.line_number_),
          column_number_(other.column_number_)
    {
    }
    const char* what() const JSONCONS_NOEXCEPT
    {
        std::ostringstream os;
        os << error_code_.message() << " at line " << line_number_ << " and column " << column_number_;
        const_cast<std::string&>(buffer_) = os.str();
        return buffer_.c_str();
    }

    const std::error_code code() const
    {
        return error_code_;
    }

    unsigned long line_number() const
    {
        return line_number_;
    }

    unsigned long column_number() const
    {
        return column_number_;
    }
private:
    std::error_code error_code_;
    std::string buffer_;
    unsigned long line_number_;
    unsigned long column_number_;
};

template<typename Char>
class basic_parsing_context
{
public:
    virtual ~basic_parsing_context() {}

    unsigned long line_number() const
    {
        return do_line_number();
    }
    unsigned long column_number() const 
    {
        return do_column_number();
    }
    Char last_char() const
    {
        return do_last_char();
    }

private:
    virtual unsigned long do_line_number() const = 0;
    virtual unsigned long do_column_number() const = 0;
    virtual Char do_last_char() const = 0;
};

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

template <typename Char>
class basic_parse_error_handler
{
public:
    virtual ~basic_parse_error_handler()
    {
    }

    void warning(std::error_code ec,
                 const basic_parsing_context<Char>& context) throw (json_parse_exception) 
    {
        do_warning(ec,context);
    }

    void error(std::error_code ec,
               const basic_parsing_context<Char>& context) throw (json_parse_exception) 
    {
        do_error(ec,context);
    }

private:
    virtual void do_warning(std::error_code,
                            const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;

    virtual void do_error(std::error_code,
                          const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;
};

template <typename Char>
class default_basic_parse_error_handler : public basic_parse_error_handler<Char>
{
public:
    static basic_parse_error_handler<Char>& instance()
    {
        static default_basic_parse_error_handler<Char> instance;
        return instance;
    }
private:
    virtual void do_warning(std::error_code,
                            const basic_parsing_context<Char>& context) throw (json_parse_exception) 
    {
    }

    virtual void do_error(std::error_code ec,
                          const basic_parsing_context<Char>& context) throw (json_parse_exception)
    {
        throw json_parse_exception(ec,context.line_number(),context.column_number());
    }
};

typedef basic_parse_error_handler<char> parse_error_handler;
typedef basic_parse_error_handler<wchar_t> wparse_error_handler;

typedef default_basic_parse_error_handler<char> default_parse_error_handler;
typedef default_basic_parse_error_handler<wchar_t> wdefault_parse_error_handler;

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

namespace json_parser_errc 
{
    enum json_parser_errc_t 
    {
        unexpected_eof,
        invalid_json_text,
        extra_character,
        max_depth_exceeded,
        mismatched_parentheses_or_brackets,
        single_quote,
        illegal_character_in_string,
        expected_comma_or_end,
        extra_comma,
        unexpected_end_of_object,
        unexpected_end_of_array,
        expected_name,
        expected_value,
        expected_colon,
        illegal_control_character,
        illegal_escaped_character,
        expected_codepoint_surrogate_pair,
        invalid_hex_escape_sequence,
        invalid_unicode_escape_sequence,
        leading_zero,
        invalid_number
    };
}

class json_parser_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "json_input";
    }
    virtual std::string message(int ev) const
    {
        switch (ev)
        {
        case json_parser_errc::unexpected_eof:
            return "Unexpected end of file";
        case json_parser_errc::invalid_json_text:
            return "A jSON text must be an object or array";
        case json_parser_errc::extra_character:
            return "Unexpected non-whitespace character after JSON text";
        case json_parser_errc::max_depth_exceeded:
            return "Maximum JSON depth exceeded";
        case json_parser_errc::mismatched_parentheses_or_brackets:
            return "Mismatched parentheses or brackets";
        case json_parser_errc::single_quote:
            return "JSON strings cannot be quoted with single quotes";
        case json_parser_errc::illegal_character_in_string:
            return "Illegal character in string";
        case json_parser_errc::extra_comma:
            return "Extra comma";
        case json_parser_errc::expected_comma_or_end:
            return "Expected comma or end";
        case json_parser_errc::unexpected_end_of_object:
            return "Unexpected end of object '}'";
        case json_parser_errc::unexpected_end_of_array:
            return "Unexpected end of array ']'";
        case json_parser_errc::expected_name:
            return "Expected object member name";
        case json_parser_errc::expected_value:
            return "Expected value";
        case json_parser_errc::expected_colon:
            return "Expected name separator ':'";
        case json_parser_errc::illegal_control_character:
            return "Illegal control character in string";
        case json_parser_errc::illegal_escaped_character:
            return "Illegal escaped character in string";
        case json_parser_errc::expected_codepoint_surrogate_pair:
            return "Invalid codepoint, expected another \\u token to begin the second half of a codepoint surrogate pair.";
        case json_parser_errc::invalid_hex_escape_sequence:
            return "Invalid codepoint, expected hexadecimal digit.";
        case json_parser_errc::invalid_unicode_escape_sequence:
            return "Invalid codepoint, expected four hexadecimal digits.";
        case json_parser_errc::invalid_number:
            return "Invalid number";
        case json_parser_errc::leading_zero:
            return "A number cannot have a leading zero";
        default:
            return "Unknown JSON parser error";
        }
    }
};

inline
const std::error_category& json_parser_category()
{
  static json_parser_category_impl instance;
  return instance;
}

inline
std::error_code make_error_code(json_parser_errc::json_parser_errc_t e)
{
  return std::error_code(
      static_cast<int>(e),
      json_parser_category());
}

}
#endif
