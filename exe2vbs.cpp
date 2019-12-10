
/* exe2vbs.cpp by katahiromz
   Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
   License: MIT
 */

#include <string>
#include <cstdio>
#include <cstring>
#include "base64.hpp"
using namespace std;

void show_version(void)
{
    puts("exe2vbs by katahiromz version 1.0");
}

void show_help(void)
{
    puts("Usage: exe2vbs [options] your-file.exe [output-file.vbs]");
    puts("Options:");
    puts("--help        Show this help.");
    puts("--version     Show version info.");
    puts("--auto-start  Make VBS file auto-start.");
}

const char *get_filename(const char *input)
{
    const char *ret;
    const char *pch1 = strrchr(input, '\\');
    const char *pch2 = strrchr(input, '/');
    if (pch1 && pch2)
    {
        if (pch1 < pch2)
            ret = pch2 + 1;
        else
            ret = pch1 + 1;
    }
    else if (pch1)
    {
        ret = pch1 + 1;
    }
    else if (pch2)
    {
        ret = pch2 + 1;
    }
    else
    {
        ret = input;
    }
    return ret;
}

int just_do_it(const char *input, const char *output, bool auto_start)
{
    FILE *inf = fopen(input, "rb");
    if (!inf)
    {
        printf("ERROR: Cannot open '%s'\n", input);
        return EXIT_FAILURE;
    }

    FILE *outf = fopen(output, "wb");
    if (!outf)
    {
        printf("ERROR: Cannot open '%s'\n", output);
        fclose(inf);
        return EXIT_FAILURE;
    }

    fputs("Dim T\r\n"
          "T = \"\"\r\n",
          outf);

    std::string str;
    char buf[64];
    for (;;)
    {
        size_t count = fread(buf, 1, sizeof(buf), inf);
        if (count == 0)
            break;

        str += std::string(buf, count);
    }

    std::string base64 = base64_encode(str.c_str(), str.size(), 0);

#define WIDTH 78

    size_t i;
    for (i = 0; i + WIDTH < base64.size(); i += WIDTH)
    {
        fprintf(outf, "T = T & \"");
        fwrite(&base64[i], WIDTH, 1, outf);
        fprintf(outf, "\"\r\n");
    }
    if (i < base64.size())
    {
        fprintf(outf, "T = T & \"");
        fwrite(&base64[i], base64.size() - i, 1, outf);
        fprintf(outf, "\"\r\n");
    }

#undef WIDTH

    const char *filename = get_filename(input);

    fprintf(outf,
"Dim dom, elem, bin, stream\r\n"
"Set dom = CreateObject(\"Microsoft.XMLDOM\")\r\n"
"Set elem = dom.createElement(\"elem\")\r\n"
"elem.DataType = \"bin.base64\"\r\n"
"elem.Text = T\r\n"
"bin = elem.NodeTypedValue\r\n"
"\r\n"
"Set stream = CreateObject(\"ADODB.Stream\")\r\n"
"stream.Open\r\n"
"stream.Type = 1\r\n"
"stream.Write bin\r\n"
"stream.SaveToFile \"%s\", 2\r\n"
"stream.Close\r\n"
"\r\n"
"Set dom = Nothing\r\n"
"Set elem = Nothing\r\n"
"Set stream = Nothing\r\n"
        , filename);

    if (auto_start)
    {
        fprintf(outf,
            "Dim wsh\r\n"
            "Set wsh = WScript.CreateObject(\"WScript.Shell\")\r\n"
            "wsh.Run \"%s\"\r\n"
            "Set wsh = Nothing\r\n", filename);
    }

    fclose(inf);
    fclose(outf);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        show_help();
        return EXIT_SUCCESS;
    }

    std::string arg, file1, file2;
    bool auto_start = false;
    for (int i = 1; i < argc; ++i)
    {
        arg = argv[i];
        if (arg == "--help")
        {
            show_help();
            return EXIT_SUCCESS;
        }
        if (arg == "--version")
        {
            show_version();
            return EXIT_SUCCESS;
        }
        if (arg == "--auto-start")
        {
            auto_start = true;
            continue;
        }
        if (file1.empty())
        {
            file1 = arg;
        }
        else if (file2.empty())
        {
            file2 = arg;
        }
        else
        {
            puts("ERROR: Too many arguments");
            show_help();
            return EXIT_FAILURE;
        }
    }

    if (file2.empty())
    {
        file2 = file1;
        file2 += ".vbs";
    }

    return just_do_it(file1.c_str(), file2.c_str(), auto_start);
}
