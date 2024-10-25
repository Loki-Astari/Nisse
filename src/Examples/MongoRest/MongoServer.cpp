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

ThorsAnvil_MakeTrait(Address,       street1, street2, city, state, zip);
ThorsAnvil_MakeTrait(Telephone,     type, number);
ThorsAnvil_MakeTrait(ContactInfo,   address, telephone);
ThorsAnvil_MakeTrait(Name,          first, last);
ThorsAnvil_MakeTrait(Person,        _id, name, age, contactInfo);

MongoServer::MongoServer(std::string const& host, int port, std::string const& user, std::string const& password, std::string const& db)
    : mongo(TAMongo::MongoURL{host, port}, TAMongo::Auth::UserNamePassword{user, password, db})
{}

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

void requestStreamRange(NHTTP::Response& response, TAMongo::FindRange<Person>& result)
{

    std::ostream& output = response.body(NHTTP::Encoding::Chunked);
    output << '[';
    std::string sep = "";
    for (auto const& p: result)
    {
        output << sep << TAJson::jsonExporter(p);
        sep = ", ";
    }
    output << ']';
}

void MongoServer::personFindByName(NHTTP::Request& request, NHTTP::Response& response)
{
    std::string first = request.variables()["first"];
    std::string last  = request.variables()["last"];

    TAMongo::FindRange<Person> result;

    if (first == "" && last == "")
    {
        response.setStatus(400);
    }
    if (first == "")
    {
        result = mongo["test"]["Person"].find<Person>(FindByLName{last});
    }
    else if (last == "")
    {
        result = mongo["test"]["Person"].find<Person>(FindByFName{first});
    }
    else
    {
        result = mongo["test"]["Person"].find<Person>(FindByName{first, last});
    }

    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    requestStreamRange(response, result);
}

void MongoServer::personFindByTel(NHTTP::Request& request, NHTTP::Response& response)
{
    std::string tel = request.variables()["tel"];

    auto result = mongo["test"]["Person"].find<Person>(FindByTel{tel});
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    requestStreamRange(response, result);
}

void MongoServer::personFindByZip(NHTTP::Request& request, NHTTP::Response& response)
{
    std::string tel = request.variables()["zip"];

    auto result = mongo["test"]["Person"].find<Person>(FindByZip{tel});
    if (!result) {
        return requestFailed(response, {result.getHRErrorMessage()});
    }

    requestStreamRange(response, result);
}