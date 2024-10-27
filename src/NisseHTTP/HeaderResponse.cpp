#include "HeaderResponse.h"
#include "Util.h"

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
    for (auto const& [headerKey, headerValue] : headers)
    {
        if (std::ranges::equal(std::string_view(headerKey), std::string_view("content-length"), ichar_equals)) {
            continue;
        }
        if (std::ranges::equal(std::string_view(headerKey), std::string_view("transfer-encoding"), ichar_equals)) {
            continue;
        }
        stream << headerKey << ": " << headerValue << "\r\n";
    }
}
