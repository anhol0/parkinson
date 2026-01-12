#include "./parkinson.hpp"
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

void getObject(const char* name, json::object &object, json::exitCode &code);
void parseCheck(int exitCode, json::object &object, json::exitCode code);

using namespace json;

int main(int argc, char **argv) {
    if(argc < 3) {
        std::cout << "Usage: parkinson <filename.json>" << std::endl;
        return 1;
    }
    const char* name = argv[1];
    const char* name2 = argv[2];
    // Setting all values and parsing JSON with error checking
    json::object object;
    json::object object2;
    json::exitCode code;

    // Starting first object
    getObject(name, object, code);    
    // Starting second object
    getObject(name2, object2, code);

    std::ofstream stream;
    stream.open("test.json");

    // Adding second object to first object
    object.setValue("somekey", std::make_unique<json::object>(std::move(object2)));
    std::unique_ptr<json::object> copy = json::object::copy(object);
    object.setValue("keytocheck", "Hello world");
    json::outputObject(stream, *copy);

    return 0;
}

// Misc helper functions

void parseCheck(int exitCode, json::object &object, json::exitCode code) {
    if (!exitCode) {
        std::cout << "JSON_PARSE_ERROR_CODE: " << code.message << " on " << code.lineNumber << ":" << code.characterNumber << "\nExit code: " << code.returnCode << std::endl;
        object.clear();
        exit(1);
    } 
}

void getObject(const char* name, json::object &object, json::exitCode &code) {
    std::ifstream stream;
    stream.open(name);
    int retCode = json::parse(stream, object, code);
    parseCheck(retCode, object, code);
    stream.close();
    code.reset();
}
