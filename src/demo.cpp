#include "./parkinson.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <variant>

using namespace json;

void printValue(const json::value& value, int indent = 0);

void printIndent(int indentationLvl) {
    for(int i = 0; i < indentationLvl; i++) std::cout << "    ";
}

void printObject(const json::object& object, int indent = 0) {
    std::cout << "{\n";
    for(auto it = object.data.begin(); it != object.data.end(); ++it) {
        printIndent(indent + 1);
        std::cout << "\"" << it->first << "\": ";
        printValue(it->second, indent + 1);
        if(std::next(it) != object.data.end()) std::cout << ",";
        std::cout << "\n";
    }
    printIndent(indent);
    indent != 0 ? std::cout << "}" : std::cout << "}\n";
}

void printArray(const json::array& array, int indent = 0) {
    std::cout << "[\n";
    for(size_t i = 0; i < array.data.size(); i++) {
       printIndent(indent+1);
       printValue(array.data[i], indent + 1);
       if(i + 1 != array.data.size()) std::cout << ",";
       std::cout << "\n";
    }
    printIndent(indent);
    std::cout << "]";
}

void printValue(const json::value& value, int indent) {
    switch(value.type) {
        case JSON_STRING:
            std::cout << "\"" << std::get<std::string>(value.value) << "\""; 
            break;  
        case JSON_BOOL:
            std::cout << std::get<bool>(value.value);
            break;  
        case JSON_NUMBER:
            if(const long long* pInt = std::get_if<long long>(&value.value)) {
                std::cout << *pInt;
            } else {
                std::cout << std::get<double>(value.value);
            }
            break;  
        case JSON_NULL:
            std::cout << "null"; 
            break;  
        case JSON_OBJECT:
            printObject(*std::get<std::unique_ptr<json::object>>(value.value), indent); 
            break;  
        case JSON_ARRAY:
            printArray(*std::get<std::unique_ptr<json::array>>(value.value), indent); 
            break;  
    }
}

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
    int retCode = json::parseJson(stream, object, code);
    parseCheck(retCode, object, code);
    stream.close();
    code.reset();
}

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

    // Adding second object to first object
    object.setValue("somekey", std::make_unique<json::object>(std::move(object2)));
    std::unique_ptr<json::object> copy = json::object::copy(object);
    object.setValue("keytocheck", "Hello world");
    printObject(*copy);
    std::cout << "----------------------\n";
    printObject(object);
    return 0;
}
