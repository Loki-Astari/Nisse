#include "HeaderResponse.h"

using namespace ThorsAnvil::Nisse::HTTP;

bool HeaderResponse::empty() const
{
    return headers.empty();
}

void HeaderResponse::add(std::string_view header, std::string_view value)
{
    headers.emplace(header, value);
}

void HeaderResponse::print(std::ostream& stream) const
{
    for (auto const& header: headers)
    {
        if (std::ranges::equal(std::string_view(header.first), std::string_view("content-length"), ichar_equals)) {
            continue;
        }
        if (std::ranges::equal(std::string_view(header.first), std::string_view("transfer-encoding"), ichar_equals)) {
            continue;
        }
        stream << header.first << ": " << header.second << "\r\n";
    }
}
