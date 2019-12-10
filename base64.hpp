// base64.hpp -- Base64 encoder/decoder
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
////////////////////////////////////////////////////////////////////////////
/* 
   base64.cpp and base64.h

   base64 encoding and decoding with C++.

   Version: 1.01.00

   Copyright (C) 2004-2017 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#ifndef KATAHIROMZ_BASE64_HPP_
#define KATAHIROMZ_BASE64_HPP_      4   // Version 4

////////////////////////////////////////////////////////////////////////////
// std::string base64_encode(const void *data, size_t size,
//                           size_t line_len = 76);
// std::string base64_encode(const std::string& src, size_t line_len = 76);
// std::string base64_decode(const char *data, size_t size);
// std::string base64_decode(const std::string& src);
////////////////////////////////////////////////////////////////////////////

#include <string>       // for std::string
#include <algorithm>    // for std::find
#include <cctype>       // for std::isspace

// NOTE: You can change this table (64 characters):
#ifndef BASE64_TABLE
    #define BASE64_TABLE \
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#endif

// NOTE: You can change the newline code:
#ifndef BASE64_NEWLINE
    #define BASE64_NEWLINE "\n"
#endif

#ifndef BASE64_PADDING
    #define BASE64_PADDING '='
#endif

////////////////////////////////////////////////////////////////////////////

inline const char *base64_table(void)
{
    return BASE64_TABLE;
}

inline unsigned char base64_index(char ch)
{
    const char *table = base64_table();
    const char *pch = std::find(table, table + 64, ch);
    if (pch == table + 64)
        return 0xFF;
    return (unsigned char)(size_t)(pch - table);
}

inline std::string
base64_encode(const void *data, size_t size, size_t line_len = 76)
{
    const char *table = base64_table();
    const unsigned char *src =
        reinterpret_cast<const unsigned char *>(data);

    std::string ret;
    ret.reserve((size * 140) / 100);

    unsigned char a3[3], a4[4];

    size_t i = 0;
    while (size--)
    {
        a3[i++] = *src;
        ++src;
        if (i != 3)
            continue;

        a4[0] = (a3[0] & 0xFC) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xF0) >> 4);
        a4[2] = ((a3[1] & 0x0F) << 2) + ((a3[2] & 0xC0) >> 6);
        a4[3] = a3[2] & 0x3F;

        for (i = 0; i < 4; ++i)
            ret += table[a4[i]];

        i = 0;
    }

    if (i)
    {
        for (size_t j = i; j < 3; ++j)
            a3[j] = 0;

        a4[0] = (a3[0] & 0xFC) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xF0) >> 4);
        a4[2] = ((a3[1] & 0x0F) << 2) + ((a3[2] & 0xC0) >> 6);

        for (size_t j = 0; j < i + 1; ++j)
            ret += table[a4[j]];

        while (i++ < 3)
            ret += BASE64_PADDING;
    }

    if (line_len)
    {
        const std::string newline = BASE64_NEWLINE;
        for (size_t k = line_len;
             k < ret.size();
             k += line_len + newline.size())
        {
            ret.insert(k, newline);
        }
        ret += newline;
    }

    return ret;
}

inline std::string
base64_decode(const char *data, size_t size)
{
    std::string ret;
    ret.reserve((size * 100) / 140);

    unsigned char ka[4], m;
    size_t n = 0;
    for (size_t i = 0; i < size; ++i)
    {
        const char ch = data[i];
        if (std::isspace(ch))
            continue;

        m = base64_index(ch);
        if (m == 0xFF)
            break;

        ka[n & 3] = m;
        ++n;

        if ((n & 3) == 0)
        {
            ret.push_back(((ka[0] & 0x3F) << 2) | ((ka[1] & 0x30) >> 4));
            ret.push_back(((ka[1] & 0x0F) << 4) | ((ka[2] & 0x3C) >> 2));
            ret.push_back(((ka[2] & 0x03) << 6) | (ka[3] & 0x3F));
        }
    }

    switch (n & 3)
    {
    case 2:
        ret.push_back(((ka[0] & 0x3F) << 2) | ((ka[1] & 0x30) >> 4));
        break;
    case 3:
        ret.push_back(((ka[0] & 0x3F) << 2) | ((ka[1] & 0x30) >> 4));
        ret.push_back(((ka[1] & 0x0F) << 4) | ((ka[2] & 0x3C) >> 2));
        break;
    }

    return ret;
}

inline std::string
base64_encode(const std::string& src, size_t line_len = 76)
{
    return base64_encode(src.c_str(), src.size(), line_len);
}

inline std::string
base64_decode(const std::string& src)
{
    return base64_decode(&src[0], src.size());
}

////////////////////////////////////////////////////////////////////////////

#endif  // ndef KATAHIROMZ_BASE64_HPP_
