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
