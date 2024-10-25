#include "HeaderPassThrough.h"
#include "Util.h"
#include <charconv>

using namespace ThorsAnvil::Nisse::HTTP;

HeaderPassThrough::HeaderPassThrough(std::istream& stream)
    : src(stream)
    , encoding(static_cast<std::streamsize>(0))
{}

bool HeaderPassThrough::isContentLength(std::string_view line) const
{
    return std::ranges::equal(getValue(line), std::string_view("content-length"), ichar_equals);
}

bool HeaderPassThrough::isTransferEncoding(std::string_view line) const
{
    return std::ranges::equal(getValue(line), std::string_view("transfer-encoding"), ichar_equals);
}

std::string_view HeaderPassThrough::getValue(std::string_view input) const
{
    // Remove leading and trailing white space.
    input.remove_prefix(std::min(input.size(), input.find_first_not_of(" \r\t\v")));
    auto find = std::min(input.size(), input.find(':'));
    input.remove_suffix(input.size() - find);
    return input;
}

std::streamsize HeaderPassThrough::getContentLength(std::string_view line) const
{
    auto find = std::min(line.size(), line.find(':'));
    find = line.find_first_not_of(" \r\t\v", find + 1);
    line.remove_prefix(find);
    line.remove_suffix(1);
    int result = 0;
    std::from_chars(line.data(), line.data() + line.size(), result);
    return result;
}

void HeaderPassThrough::print(std::ostream& stream)
{
    std::string line;
    while (std::getline(src, line))
    {
        if (line == "\r") {
            break;
        }
        stream << line << "\n";

        if (isContentLength(line))
        {
            encoding.emplace<std::streamsize>(getContentLength(line));
            continue;
        }
        if (isTransferEncoding(line))
        {
            encoding.emplace<Encoding>(Encoding::Chunked);
            continue;
        }
    }
}
