#include "Error.h"
#include <system_error>
#include <format>
using std::format;
using std::to_string;
using std::error_code;
using std::system_category;

namespace WXE
{
    Error::Error() noexcept : hrCode{}, lineNum{-1}
    {
    }

    Error::Error(const int32 hr, const string_view func, const string_view file, const int32 line) noexcept
        : hrCode(hr), funcName(func), lineNum(line)
    {
        size_t pos = file.find_last_of('\\');
        
        if (pos != string::npos) 
            fileName = file.substr(pos + 1);
    }

    string Error::ToString() const
    {
        error_code err(hrCode, system_category());

        return format("{} failed in {}, line {} :\n", 
                        funcName, fileName, to_string(lineNum), err.message());
    }
}