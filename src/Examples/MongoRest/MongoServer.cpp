#include "MongoServer.h"
#include "ThorsMongo/MongoUtil.h"
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"
#include <ranges>


using namespace ThorsAnvil::Nisse::Examples::MongoRest;
namespace TAMongo  = ThorsAnvil::DB::Mongo;
namespace TAJson   = ThorsAnvil::Serialize;

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

struct Person
{
    TAMongo::ObjectID   _id;
    std::string         name;
    int                 age;
    ContactInfo         contactInfo;
};

ThorsAnvil_MakeTrait(Address,       street1, street2, city, state, zip);
ThorsAnvil_MakeTrait(Telephone,     type, number);
ThorsAnvil_MakeTrait(ContactInfo,   address, telephone);
ThorsAnvil_MakeTrait(Person,        _id, name, age, contactInfo);

MongoServer::MongoServer(std::string const& host, int port, std::string const& user, std::string const& password, std::string const& db)
    : mongo(TAMongo::MongoURL{host, port}, TAMongo::Auth::UserNamePassword{user, password, db})
{}

ThorsMongo_CreateFieldAccess(Person, _id);
using FindById = ThorsMongo_FilterFromAccess(Eq, Person, _id);

TAMongo::ObjectID MongoServer::getIdFromRequest(NHTTP::Request& request)
{
    TAMongo::ObjectID   result;

    std::stringstream   ss(std::move(request.variables()["id"]));
    ss >> TAJson::jsonImporter(result);

    return result;
}

void MongoServer::requestFailed(NHTTP::Response& response, std::initializer_list<std::string> messages)
{
    response.setStatus(404);

    NHTTP::HeaderResponse   headers;
    for (auto const& message: messages) {
        headers.add("Error", message);
    }
    response.addHeaders(headers);
}

void MongoServer::personCreate(NHTTP::Request& request, NHTTP::Response& response)
{
    Person  person;
    request.body() >> TAJson::jsonImporter(person);

    auto result = mongo["test"]["Person"].insert(std::tie(person));
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    response.body(NHTTP::Encoding::Chunked) << TAJson::jsonExporter(result);
}

void MongoServer::personGet(NHTTP::Request& request, NHTTP::Response& response)
{
    TAMongo::ObjectID       id  = getIdFromRequest(request);

    auto range = mongo["test"]["Person"].find<Person>(FindById{id});
    if (!range) {
        return requestFailed(response, {range.getHRErrorMessage()});
    }

    for (auto const& person: range)
    {
        response.body(NHTTP::Encoding::Chunked) << TAJson::jsonExporter(person);
        return;
    }

    // Only get here if there are no items in the range.
    requestFailed(response, {"No Value Found"});
}

void MongoServer::personUpdate(NHTTP::Request& request, NHTTP::Response& response)
{
    TAMongo::ObjectID   id  = getIdFromRequest(request);
    Person              person;
    request.body() >> TAJson::jsonImporter(person);

    auto result = mongo["test"]["Person"].findAndReplaceOne<Person>(FindById{id}, person);
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    if (!result.value)
    {
        // No original value.
        response.body(NHTTP::Encoding::Chunked) << "{}";
    }
    else
    {
        // This was the value we replaced.
        response.body(NHTTP::Encoding::Chunked) << TAJson::jsonExporter(*result.value);
    }
}

void MongoServer::personDelete(NHTTP::Request& request, NHTTP::Response& response)
{
    TAMongo::ObjectID   id  = getIdFromRequest(request);
    auto result = mongo["test"]["Person"].findAndRemoveOne<Person>(FindById{id});
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    if (!result.value)
    {
        // No original value.
        response.body(NHTTP::Encoding::Chunked) << "{}";
    }
    else
    {
        // This was the value we removed.
        response.body(NHTTP::Encoding::Chunked) << TAJson::jsonExporter(*result.value);
    }
}

void MongoServer::personFind(NHTTP::Request& request, NHTTP::Response& response)
{
    //mongo["test"]["Person"].find()
}
