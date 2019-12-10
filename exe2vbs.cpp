/* exe2vbs.cpp by katahiromz
   Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
   License: MIT
 */

#include <cstdio>
#include <string>
using namespace std;

void show_version(void)
{
    puts("exe2vbs by katahiromz version 0.2");
}

void show_help(void)
{
    puts("Usage: exe2vbs [options] your-file.exe [output-file.vbs]");
    puts("Options:");
    puts("--help        Show this help.");
    puts("--version     Show version info.");
    puts("--auto-start  Make VBS file auto-start.");
}

/***********************************************
Sub WriteBinary(FileName, Buf)
	Dim I, B, SZ, BS
	SZ = UBound(Buf): ReDim B(SZ \ 2)
	For I = 0 To SZ - 1 Step 2
		B(I \ 2) = ChrW(Buf(I + 1) * 256 + Buf(I))
	Next
	If I = SZ Then B(I \ 2) = ChrW(Buf(I))
	B = Join(B, "")
	Set BS = CreateObject("ADODB.Stream")
	BS.Type = 1: BS.Open
	With CreateObject("ADODB.Stream")
		.Type = 2 : .Open: .WriteText B
		.Position = 2: .CopyTo BS: .Close
	End With
	BS.SaveToFile FileName, 2: BS.Close
	Set BS = Nothing
End Sub

Sub ArrayFromHex(A, S)
	Dim I, C1, C2, K1, K2, L, M
	L = Len(S)
	M = 0
	For I = 1 To L Step 2
		C1 = Mid(S, I, 1)
		C2 = Mid(S, I + 1, 1)
		K1 = InStr("0123456789ABCDEF", C1) - 1
		K2 = InStr("0123456789ABCDEF", C2) - 1
		A(M) = K1 * 16 + K2
		M = M + 1
	Next
End Sub

Dim str
str = ""
***********************************************/

static const char s_header[] =
"Sub WriteBinary(FileName, Buf)\r\n"
"	Dim I, B, SZ, BS\r\n"
"	SZ = UBound(Buf): ReDim B(SZ \\ 2)\r\n"
"	For I = 0 To SZ - 1 Step 2\r\n"
"		B(I \\ 2) = ChrW(Buf(I + 1) * 256 + Buf(I))\r\n"
"	Next\r\n"
"	If I = SZ Then B(I \\ 2) = ChrW(Buf(I))\r\n"
"	B = Join(B, \"\")\r\n"
"	Set BS = CreateObject(\"ADODB.Stream\")\r\n"
"	BS.Type = 1: BS.Open\r\n"
"	With CreateObject(\"ADODB.Stream\")\r\n"
"		.Type = 2 : .Open: .WriteText B\r\n"
"		.Position = 2: .CopyTo BS: .Close\r\n"
"	End With\r\n"
"	BS.SaveToFile FileName, 2: BS.Close\r\n"
"	Set BS = Nothing\r\n"
"End Sub\r\n"
"\r\n"
"Sub ArrayFromHex(A, S)\r\n"
"	Dim I, C1, C2, K1, K2, L, M\r\n"
"	L = Len(S)\r\n"
"	M = 0\r\n"
"	For I = 1 To L Step 2\r\n"
"		C1 = Mid(S, I, 1)\r\n"
"		C2 = Mid(S, I + 1, 1)\r\n"
"		K1 = InStr(\"0123456789ABCDEF\", C1) - 1\r\n"
"		K2 = InStr(\"0123456789ABCDEF\", C2) - 1\r\n"
"		A(M) = K1 * 16 + K2\r\n"
"		M = M + 1\r\n"
"	Next\r\n"
"End Sub\r\n"
"\r\n"
"Dim str\r\n"
"str = \"\"\r\n";

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

    fputs(s_header, outf);

    unsigned char buf[64];
    for (;;)
    {
        size_t count = fread(buf, 1, sizeof(buf), inf);
        if (count == 0)
            break;

        fprintf(outf, "str = str & \"");
        for (size_t i = 0; i < count; ++i)
        {
            fprintf(outf, "%02X", buf[i]);
        }
        fprintf(outf, "\"\r\n");
    }

    const char *filename = strrchr(input, '\\');
    if (filename == NULL)
    {
        filename = strrchr(input, '/');
        if (filename == NULL)
        {
            filename = input;
        }
    }

    fprintf(outf, "ReDim Data((Len(str) + 1) \\ 2 - 1)\r\n");
    fprintf(outf, "ArrayFromHex Data, str\r\n");
    fprintf(outf, "WriteBinary \"%s\", Data\r\n", filename);
    if (auto_start)
    {
        fprintf(outf, "Dim wsh\r\n");
        fprintf(outf, "Set wsh = WScript.CreateObject(\"WScript.Shell\")\r\n");
        fprintf(outf, "wsh.Run \"%s\"\r\n", filename);
        fprintf(outf, "Set wsh = Nothing\r\n");
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
