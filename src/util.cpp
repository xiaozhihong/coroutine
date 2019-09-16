#include "util.h"

using namespace std;

string BinToHex(const std::string& str, const size_t& one_line_count, const bool& print_ascii)
{
    return BinToHex((const uint8_t*)str.data(), str.size(), one_line_count, print_ascii);
}

string BinToHex(const uint8_t* buf, const size_t& len, const size_t& one_line_count, const bool& print_ascii)
{
    string hex = "\n";
    string line_hex = "";
    string line_ascii = "    ";

    line_hex.reserve(256);
    line_ascii.reserve(80);

    for (size_t i = 0; i < len; ++i)
    {
        static char hex_tmp[4] = {0, 0, 0, 0};
        snprintf(hex_tmp, sizeof(hex_tmp), "%02X ", buf[i]);

        if (i % one_line_count == 0)
        {
            line_ascii = "    ";
            static char offset_tmp[] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
            int nbytes = snprintf(offset_tmp, sizeof(offset_tmp), "%06zu: ", i);
            line_hex.assign(offset_tmp, nbytes);
        }

        line_hex.append(hex_tmp, 3);

        if (print_ascii)
        {
            if (isprint(buf[i]) && buf[i] != '\r' && buf[i] != '\n')
            {
                line_ascii += buf[i];
            }
            else
            {
                line_ascii += '.';
            }
        }

        if (i % one_line_count == (one_line_count - 1))
        {
            hex += line_hex;

            if (print_ascii)
            {
                hex += line_ascii;
            }

            hex += '\n';
        }
    }

    if (len % one_line_count != 0)
    {
        hex += line_hex;

        if (print_ascii)
        {
            size_t space = (one_line_count - (len % one_line_count)) * 3;

            hex.append(space, ' ');

            hex += line_ascii;
        }

        hex += '\n';
    }

    return hex;
}

vector<string> SplitStr(const string& input, const string& sep)
{
    vector<string> ret;

    size_t pre_pos = 0;
    while (true)
    {   
        auto pos = input.find(sep, pre_pos);

        if (pos == string::npos)
        {   
            ret.push_back(input.substr(pre_pos));
            break;
        }   

        string tmp = input.substr(pre_pos, pos - pre_pos);

        if (tmp == sep)
        {   
        }   
        else
        {   
            if (! tmp.empty())
            {
                ret.push_back(tmp);
            }
        }   

        pre_pos = pos + sep.size();
    }   

    return ret;
}
