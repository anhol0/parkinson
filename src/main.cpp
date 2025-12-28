#include "./parkinson.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <variant>

void printValue(const JsonValue& value, int indent = 0);

void printIndent(int indentationLvl) {
    for(int i = 0; i < indentationLvl; i++) std::cout << "    ";
}

void printObject(const JsonObject& object, int indent = 0) {
    std::cout << "{\n";
    for(auto it = object.data.begin(); it != object.data.end(); ++it) {
        printIndent(indent + 1);
        std::cout << "\"" << it->first << "\": ";
        printValue(it->second, indent + 1);
        if(std::next(it) != object.data.end()) std::cout << ",";
        std::cout << "\n";
    }
    printIndent(indent);
    std::cout << "}";
}

void printArray(const JsonArray& array, int indent = 0) {
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

void printValue(const JsonValue& value, int indent) {
    switch(value.type) {
        case JSON_STRING:
            std::cout << "\"" << std::get<std::string>(value.value) << "\""; 
            break;  
        case JSON_BOOL:
            std::cout << std::get<bool>(value.value);
            break;  
        case JSON_NUMBER:
            if(const int* pInt = std::get_if<int>(&value.value)) {
                std::cout << *pInt;
            } else {
                std::cout << std::get<double>(value.value);
            }
            break;  
        case JSON_NULL:
            std::cout << "null"; 
            break;  
        case JSON_OBJECT:
            printObject(*std::get<std::unique_ptr<JsonObject>>(value.value), indent); 
            break;  
        case JSON_ARRAY:
            printArray(*std::get<std::unique_ptr<JsonArray>>(value.value), indent); 
            break;  
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        std::cout << "Usage: parkinson <filename.json>" << std::endl;
        return 1;
    }
    const char* name = argv[1];

    JsonObject object;
    
    ParserExitCode code;
    std::ifstream fileStream;
    fileStream.open(name);
    int retCode = parseJson(fileStream, object, code);
    fileStream.close();
    if (!retCode) {
        std::cout << "JSON_PARSE_ERROR_CODE: " << code.message << " on " << code.lineNumber << ":" << code.characterNumber << "\nExit code: " << code.returnCode << std::endl;
        object.clear();
        return 1;
    } else if (retCode) {
        std::cout << code.message << ": " << code.returnCode << std::endl;
    }

    printObject(object);
    std::cout << "\n----------------------\n";

    JsonTypes type;
    JsonObject *obj;
    std::string key = "var1";
    if(object.exists(key)) {
        if(object.getType(key, type)) {
            if(type == JSON_OBJECT) {
                if(object.getObject(key, obj)) {
                    JsonArray *array;
                    obj->getArray("var2", array);
                    printArray(*array);
                }
            } else {
                std::cout << "var1 doesn't contain an object\n";
            }
        }
    }
    return 0;
}
