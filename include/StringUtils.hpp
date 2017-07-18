#ifndef SIMPLE_STRINGUTILS_HPP
#define SIMPLE_STRINGUTILS_HPP
#include <algorithm>
#include <string>
#include <vector>

namespace Simple {
namespace StringUtils {

using namespace std::string_literals;

template <typename CharT>
using tstring = std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

template <typename CharT>
using tstringstream = std::basic_stringstream<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

/**
 * @brief Converts std::basic_string to upper case
 *
 * @tparam CharT type of character
 * @param text input text
 *
 * @return std::basic_string with all upper case characters
 */
template <typename CharT>
inline tstring<CharT> to_upper(tstring<CharT> text)
{
    std::transform(std::begin(text), std::end(text), std::begin(text), std::toupper);
    return text;
}

/**
 * @brief Converts std::basic_string to lower case
 *
 * @tparam CharT type of character
 * @param text input text
 *
 * @return std::basic_string with all lower case characters
 */
template <typename CharT>
inline tstring<CharT> to_lower(tstring<CharT> text)
{
    std::transform(std::begin(text), std::end(text), std::begin(text), std::tolower);
    return text;
}

/**
 * @brief Reverse std:basic_string
 *
 * @tparam CharT type of character
 * @param text input text
 *
 * @return std::basic_string with all characters reversed
 */
template <typename CharT>
inline tstring<CharT> reverse(tstring<CharT> text)
{
    std::reverse(std::begin(text), std::end(text));
    return text;
}

/**
 * @brief Trim all spaces from start and end of std::basic_string
 *
 * @tparam CharT type of character
 * @param text input text
 *
 * @return std::basic_string trimmed from beginning and end
 */
template <typename CharT>
inline tstring<CharT> trim(tstring<CharT> const& text)
{
    auto first{text.find_first_not_of(' ')};
    auto last{text.find_last_not_of(' ')};
    return text.substr(first, (last - first + 1));
}

/**
 * @brief Trim all spaces from start of std::basic_string
 *
 * @tparam CharT type of character
 * @param text input text
 *
 * @return std::basic_string trimmed from beginning
 */
template <typename CharT>
inline tstring<CharT> trimleft(tstring<CharT> const& text)
{
    auto first{text.find_first_not_of(' ')};
    return text.substr(first, text.size() - first);
}

/**
 * @brief Trim all spaces from end of std::basic_string
 *
 * @tparam CharT type of character
 * @param text input text
 *
 * @return std::basic_string trimmed from end
 */
template <typename CharT>
inline tstring<CharT> trimright(tstring<CharT> const& text)
{
    auto last{text.find_last_not_of(' ')};
    return text.substr(0, last + 1);
}

/**
 * @brief Trim all defined characters from begining and end of std::basic_string
 *
 * @tparam CharT type of character
 * @param text input text
 * @param chars character that should be trimmed
 *
 * @return std::basic_string trimmed from beginning and end
 */
template <typename CharT>
inline tstring<CharT> trim(tstring<CharT> const& text, tstring<CharT> const& chars)
{
    auto first{text.find_first_not_of(chars)};
    auto last{text.find_last_not_of(chars)};
    return text.substr(first, (last - first + 1));
}

/**
 * @brief Trim all defined characters from begining of std::basic_string
 *
 * @tparam CharT type of character
 * @param text input text
 * @param chars character that should be trimmed
 *
 * @return std::basic_string trimmed from beginning
 */
template <typename CharT>
inline tstring<CharT> trimleft(tstring<CharT> const& text, tstring<CharT> const& chars)
{
    auto first{text.find_first_not_of(chars)};
    return text.substr(first, text.size() - first);
}

/**
 * @brief Trim all defined characters from end of std::basic_string
 *
 * @tparam CharT type of character
 * @param text input text
 * @param chars character that should be trimmed
 *
 * @return std::basic_string trimmed from end
 */
template <typename CharT>
inline tstring<CharT> trimright(tstring<CharT> const& text, tstring<CharT> const& chars)
{
    auto last{text.find_last_not_of(chars)};
    return text.substr(0, last + 1);
}

/**
 * @brief Remove all specified characters from std::basic_string
 *
 * @tparam CharT type of character
 * @param text input text
 * @param ch Character that should be removed from string
 *
 * @return  std::basic_string with removed characters
 */
template <typename CharT>
inline tstring<CharT> remove(tstring<CharT> text, CharT const ch)
{
    auto start = std::remove_if(std::begin(text), std::end(text), [=](CharT const c) { return c == ch; });
    text.erase(start, std::end(text));
    return text;
}

/**
 * @brief Split std::basic_string by specified delimiter
 *
 * @tparam CharT type of characters
 * @param text input text
 * @param delimiter Character by with input text should be splitted
 *
 * @return Vector of substrings gathered by splitting input text
 */
template <typename CharT>
inline std::vector<tstring<CharT>> split(tstring<CharT> text, CharT const delimiter)
{
    tstringstream<CharT> sstr{text};
    std::vector<tstring<CharT>> tokens;
    tstring<CharT> token;
    while (std::getline(sstr, token, delimiter)) {
        if (!token.empty())
            tokens.push_back(token);
    }
    return tokens;
}
}
}

#endif
