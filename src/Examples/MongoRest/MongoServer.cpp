#include "MongoServer.h"
#include "ThorsMongo/MongoUtil.h"
#include "NisseServer/AsyncStream.h"
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"
#include <ranges>


namespace TAJson    = ThorsAnvil::Serialize;
namespace TAMongo   = ThorsAnvil::DB::Mongo;
namespace NisServer = ThorsAnvil::Nisse::Server;

using namespace ThorsAnvil::Nisse::Examples::MongoRest;

struct Address
{
    std::string         street1;
    std::string         street2;
    std::string         city;
    std::string         state;
    std::string         zip;
};

struct Telephone
{
    std::string         type;
    std::string         number;
};

struct ContactInfo
{
    Address             address;
    Telephone           telephone;
};

struct Name
{
    std::string         first;
    std::string         last;
};

struct Person
{
    TAMongo::ObjectID   _id;
    Name                name;
    int                 age;
    ContactInfo         contactInfo;
};
struct NewPerson
{
    Name                name;
    int                 age;
    ContactInfo         contactInfo;
};

struct FindResult
{
    TAMongo::ObjectID   _id;
};

ThorsAnvil_MakeTrait(Address,       street1, street2, city, state, zip);
ThorsAnvil_MakeTrait(Telephone,     type, number);
ThorsAnvil_MakeTrait(ContactInfo,   address, telephone);
ThorsAnvil_MakeTrait(Name,          first, last);
ThorsAnvil_MakeTrait(Person,        _id, name, age, contactInfo);
ThorsAnvil_MakeTrait(NewPerson,     name, age, contactInfo);
ThorsAnvil_MakeTrait(FindResult,    _id);

ThorsMongo_CreateFieldAccess(Person, _id);
ThorsMongo_CreateFieldAccess(Person, name, first);
ThorsMongo_CreateFieldAccess(Person, name, last);
ThorsMongo_CreateFieldAccess(Person, contactInfo, telephone, number);
ThorsMongo_CreateFieldAccess(Person, contactInfo, address, zip);

using FindById      = ThorsMongo_FilterFromAccess(Eq, Person, _id);
using FindByFName   = ThorsMongo_FilterFromAccess(Eq, Person, name, first);
using FindByLName   = ThorsMongo_FilterFromAccess(Eq, Person, name, last);
using FindByName    = TAMongo::QueryOp::And<FindByFName, FindByLName>;
using FindByTel     = ThorsMongo_FilterFromAccess(Eq, Person, contactInfo, telephone, number);
using FindByZip     = ThorsMongo_FilterFromAccess(Eq, Person, contactInfo, address, zip);

void MongoServer::requestFailed(NisHttp::Response& response, std::initializer_list<std::string> messages)
{
    response.setStatus(404);

    NisHttp::HeaderResponse   headers;
    for (auto const& message: messages) {
        headers.add("Error", message);
    }
    response.addHeaders(headers);
}

void requestStreamRange(NisHttp::Response& response, TAMongo::FindRange<FindResult>& result)
{
    std::ostream& output = response.body(NisHttp::Encoding::Chunked);
    output << '[';
    std::string sep = "";
    for (auto const& p: result)
    {
        output << sep << TAJson::jsonExporter(p);
        sep = ", ";
    }
    output << ']';
}

MongoServer::MongoServer(NisServer::NisseServer& server, std::size_t poolSize, std::string_view host, int port, std::string_view user, std::string_view password, std::string_view db)
    : mongoPool(server, poolSize, host, port, user, password, db)
{}

TAMongo::ObjectID MongoServer::getIdFromRequest(NisHttp::Request& request)
{
    TAMongo::ObjectID   result;

    std::stringstream   ss{std::move(request.variables()["id"])};
    ss >> result;

    return result;
}

void MongoServer::personCreate(NisHttp::Request& request, NisHttp::Response& response)
{
    LeaseConnection         lease(mongoPool, request.getContext());
    TAMongo::ThorsMongo&    mongo =lease.connection();
    NisServer::AsyncStream  async(mongo.getStream().getSocket(), request.getContext(), NisServer::EventType::Write);
    NewPerson               person;
    request.body() >> TAJson::jsonImporter(person);

    auto result = mongo["test"]["AddressBook"].insert(std::tie(person));
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    response.body(NisHttp::Encoding::Chunked) << TAJson::jsonExporter(result.inserted);
}

void MongoServer::personGet(NisHttp::Request& request, NisHttp::Response& response)
{
    LeaseConnection         lease(mongoPool, request.getContext());
    TAMongo::ThorsMongo&    mongo =lease.connection();
    NisServer::AsyncStream  async(mongo.getStream().getSocket(), request.getContext(), NisServer::EventType::Write);
    TAMongo::ObjectID       id  = getIdFromRequest(request);

    auto range = mongo["test"]["AddressBook"].find<Person>(FindById{id});
    if (!range) {
        return requestFailed(response, {range.getHRErrorMessage()});
    }

    for (auto const& person: range)
    {
        response.body(NisHttp::Encoding::Chunked) << TAJson::jsonExporter(person);
        return;
    }

    // Only get here if there are no items in the range.
    requestFailed(response, {"No Value Found"});
}

void MongoServer::personUpdate(NisHttp::Request& request, NisHttp::Response& response)
{
    LeaseConnection         lease(mongoPool, request.getContext());
    TAMongo::ThorsMongo&    mongo =lease.connection();
    NisServer::AsyncStream  async(mongo.getStream().getSocket(), request.getContext(), NisServer::EventType::Write);
    TAMongo::ObjectID       id  = getIdFromRequest(request);
    NewPerson               person;
    std::istream& stream = request.body();
    stream >> TAJson::jsonImporter(person);

    auto result = mongo["test"]["AddressBook"].findAndReplaceOne<NewPerson>(FindById{id}, person);
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    if (!result.value)
    {
        // No original value.
        response.body(NisHttp::Encoding::Chunked) << "{}";
    }
    else
    {
        // This was the value we replaced.
        response.body(NisHttp::Encoding::Chunked) << TAJson::jsonExporter(*result.value);
    }
}

void MongoServer::personDelete(NisHttp::Request& request, NisHttp::Response& response)
{
    LeaseConnection         lease(mongoPool, request.getContext());
    TAMongo::ThorsMongo&    mongo =lease.connection();
    NisServer::AsyncStream  async(mongo.getStream().getSocket(), request.getContext(), NisServer::EventType::Write);
    TAMongo::ObjectID       id  = getIdFromRequest(request);

    auto result = mongo["test"]["AddressBook"].findAndRemoveOne<Person>(FindById{id});
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    if (!result.value)
    {
        // No original value.
        response.body(NisHttp::Encoding::Chunked) << "{}";
    }
    else
    {
        // This was the value we removed.
        response.body(NisHttp::Encoding::Chunked) << TAJson::jsonExporter(*result.value);
    }
}

void MongoServer::personFindByName(NisHttp::Request& request, NisHttp::Response& response)
{
    LeaseConnection         lease(mongoPool, request.getContext());
    TAMongo::ThorsMongo&    mongo =lease.connection();
    NisServer::AsyncStream  async(mongo.getStream().getSocket(), request.getContext(), NisServer::EventType::Write);
    std::string             first = request.variables()["first"];
    std::string             last  = request.variables()["last"];

    TAMongo::FindRange<FindResult> result;

    if (first == "" && last == "")
    {
        response.setStatus(400);
    }
    if (first == "")
    {
        result = mongo["test"]["AddressBook"].find<FindResult>(FindByLName{last});
    }
    else if (last == "")
    {
        result = mongo["test"]["AddressBook"].find<FindResult>(FindByFName{first});
    }
    else
    {
        result = mongo["test"]["AddressBook"].find<FindResult>(FindByName{first, last});
    }

    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    requestStreamRange(response, result);
}

void MongoServer::personFindByTel(NisHttp::Request& request, NisHttp::Response& response)
{
    LeaseConnection         lease(mongoPool, request.getContext());
    TAMongo::ThorsMongo&    mongo =lease.connection();
    NisServer::AsyncStream  async(mongo.getStream().getSocket(), request.getContext(), NisServer::EventType::Write);
    std::string             tel = request.variables()["tel"];

    auto result = mongo["test"]["AddressBook"].find<FindResult>(FindByTel{tel});
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    requestStreamRange(response, result);
}

void MongoServer::personFindByZip(NisHttp::Request& request, NisHttp::Response& response)
{
    LeaseConnection         lease(mongoPool, request.getContext());
    TAMongo::ThorsMongo&    mongo =lease.connection();
    NisServer::AsyncStream  async(mongo.getStream().getSocket(), request.getContext(), NisServer::EventType::Write);
    std::string             zip = request.variables()["zip"];

    auto result = mongo["test"]["AddressBook"].find<FindResult>(FindByZip{zip});
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    requestStreamRange(response, result);
}
