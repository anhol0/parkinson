#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include "./parser.hpp"

int parseJson(const char *name, JsonObject& object, ParserExitCode& code) {
    std::ifstream fileStream(name);
    char ch;
   
    int line = 1;
    int character = 1;
    parserState parserState = WAITING_FOR_OBJECT;
    JsonObject* currentObject = &object;
    JsonArray* currentArray = nullptr;
    JsonTypes type; 
    //Key temp string
    std::string key;
    // Temp values of the pair
    std::string tmpVal;

    // Reading the stream of data and 
    while(fileStream.get(ch)) {
        if(ch == '\n') {
            line++;
            character = 1;
        }
        character++;
    
        if((parserState != WRITE_VALUE) && isWhiteSpace(ch)) {
           continue; 
        }

        switch (parserState) {
            // BEGINNING OF THE OBJECT PARSING
            case WAITING_FOR_OBJECT: {
                if(ch == '{') {   
                    parserState = BEGIN_KEY;
                }
                else {
                    constructExitCode(code, PARSE_ERR_INCORRECT_OBJECT_START, "PARSE_ERR_INCORRECT_OBJECT_START", line, character);
                    fileStream.close();
                    return 0;
                }
                break;
            }
            // BEGINNING OF KEY WRITING
            case BEGIN_KEY: {
                if(ch == '"') {
                    parserState = WRITE_KEY;
                }
                else {
                    constructExitCode(code, PARSE_ERR_INCORRECT_KEY_DECLARATION, "PARSE_ERR_INCORRECT_KEY_DECLARATION", line, character);
                    fileStream.close();
                    return 0;
                }
                break;
            }
            // WRITING THE KEY
            case WRITE_KEY: {
                if(ch == '"') {
                    parserState = KEY_WRITTEN;
                }
                else { 
                    key += ch;
                }
                break;
            }
            // CHECKING SEPARATOR BETWEEN KEY AND VALUE
            case KEY_WRITTEN: {
                if(ch == ':') {
                    parserState = BEGIN_VALUE;
                } else {
                    constructExitCode(code, PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR, "PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR", line, character);
                    fileStream.close();
                    return 0;
                }
                break;
            }
            // CHECKING VALUE TYPE
            case BEGIN_VALUE: {
                int retCode = detectValueType(ch, type);
                if(retCode == -1) { 
                    constructExitCode(code, PARSE_ERR_INCORRECT_VALUE_TYPE, "PARSE_ERR_INCORRECT_VALUE_TYPE", line, character);
                    return 0;
                }
                if(type == JSON_NUMBER || type == JSON_BOOL || type == JSON_NULL) {
                    tmpVal += ch;
                } else if(type == JSON_ARRAY) {
                    JsonValue v;
                    v.type = type;
                    v.value = std::make_unique<JsonArray>();
                    if(isContextArray(currentArray, currentObject)) {
                        auto &insertionResult = currentArray->data.emplace_back(std::move(v));
                        auto &arrayUptr = std::get<std::unique_ptr<JsonArray>>(insertionResult.value);
                        JsonArray* arrayPtr = arrayUptr.get();
                        arrayPtr->addParentArray(currentArray);
                        currentArray = arrayPtr;
                        currentObject = nullptr;
                        parserState = BEGIN_VALUE;
                    } else {
                        auto insertionResult = currentObject->data.emplace(key, std::move(v));
                        auto &insertedValue = insertionResult.first->second;
                        auto &arrayUPtr = std::get<std::unique_ptr<JsonArray>>(insertedValue.value);
                        JsonArray* arrayPtr = arrayUPtr.get();
                        arrayPtr->addParentObject(currentObject);
                        currentArray = arrayPtr;
                        currentObject = nullptr;
                        parserState = BEGIN_VALUE;
                    }
                    tmpVal.clear();
                    key.clear();
                    break;
                } else if(type == JSON_OBJECT) {
                    JsonValue v;
                    v.type = type;
                    v.value = std::make_unique<JsonObject>();
                    if(isContextArray(currentArray, currentObject)) {
                        auto &insertResult = currentArray->data.emplace_back(std::move(v));
                        auto &objUPtr = std::get<std::unique_ptr<JsonObject>>(insertResult.value);
                        JsonObject* objPtr = objUPtr.get();
                        objPtr->addParentArray(currentArray);
                        currentObject = objPtr;
                        currentArray = nullptr;
                        parserState = BEGIN_KEY;
                    } else {
                        auto insertResult = currentObject->data.emplace(key, std::move(v));
                        auto &insertedValue = insertResult.first->second;
                        auto &objUPtr = std::get<std::unique_ptr<JsonObject>>(insertedValue.value);  
                        JsonObject* objPtr = objUPtr.get(); 
                        objPtr->addParentObject(currentObject);
                        currentObject = objPtr;
                        parserState = BEGIN_KEY; 
                    }
                    tmpVal.clear();
                    key.clear();
                    break;
                }
                parserState = WRITE_VALUE;
                break;               
            }
            // WRITING VALUE DEPENDING ON THE VALUE TYPE
            case WRITE_VALUE: {
                switch (type) {
                    // --- Writing string values ---
                    case JSON_STRING: {
                        if(checkStringEnd(ch)) {
                            JsonValue v;
                            v.type = type;
                            v.value = tmpVal;
                            if(isContextArray(currentArray, currentObject)) {
                                currentArray->data.push_back(std::move(v));
                            } else {
                                currentObject->data.emplace(key, std::move(v));
                            }
                            parserState = VALUE_WRITTEN;
                            key.clear();
                            tmpVal.clear();
                            continue;
                        } else {
                            tmpVal += ch;
                        }
                        break;
                    }
                    // --- Writing number (int/float) values ---
                    case JSON_NUMBER: {
                        if(isWhiteSpace(ch) || ch == ',' || ch == ']' || ch == '}') {
                            double numValue = std::stod(tmpVal.c_str());
                            JsonValue v;
                            v.type = type;
                            // Deciding if number will be double or int
                            if(static_cast<double>(static_cast<int>(numValue)) == numValue) {
                                int intNumVal = static_cast<int>(numValue);
                                v.value = intNumVal;
                            } else {
                                v.value = numValue;    
                            }
                            // Deciding whether to write data to the 
                            // array context or object context
                            if(isContextArray(currentArray, currentObject)) {
                                currentArray->data.push_back(std::move(v));
                            } else { 
                                currentObject->data.emplace(key, std::move(v));
                            }
                            parserState = VALUE_WRITTEN;
                            key.clear();
                            tmpVal.clear();
                        } else {
                            if(!isNumber(ch)) {
                                constructExitCode(code, PARSE_ERR_INCORRECT_NUMBER_DEFINITION, "PARSE_ERR_INCORRECT_NUMBER_DEFINITION", line, character);
                                fileStream.close(); 
                                return 0;
                            }
                            tmpVal += ch;                           
                        }
                        break;
                    }
                    // --- Writing boolean variable ---
                    case JSON_BOOL: {
                        if(isWhiteSpace(ch) || ch == ',' || ch == ']' || ch == '}') {
                            JsonValue v;
                            v.type = type;
                            // If value in the buffer is "true" make true
                            if(tmpVal == std::string("true")) {
                                v.value = true;
                            }
                            // If value in buffer is "false" make false
                            else if(tmpVal == std::string("false")) {
                                v.value = false;
                            }
                            // If neither - throw an error
                            else {
                                constructExitCode(code, PARSE_ERR_INCORRECT_BOOL_DEFINITION, "PARSE_ERR_INCORRECT_BOOL_DEFINITION", line, character);
                                fileStream.close();
                                return 0;
                            }
                            // Deciding where to write - array of the object
                            if(isContextArray(currentArray, currentObject)) {         
                                currentArray->data.push_back(std::move(v));           
                            } else {
                                currentObject->data.emplace(key, std::move(v));       
                            }
                            parserState = VALUE_WRITTEN;
                            tmpVal.clear();
                            key.clear();
                        } else {
                            tmpVal += ch;
                        } 
                        break;
                    }
                    case JSON_NULL: {
                        if(isWhiteSpace(ch) || ch == ',') {
                            JsonValue v;
                            v.type = JSON_NULL;
                            if(tmpVal == std::string("null")) {
                                v.value = false;
                            } else {
                                constructExitCode(code, PARSE_ERR_INCORRECT_NULL_VALUE_DEFINITION, "PARSE_ERR_INCORRECT_NULL_VALUE_DEFINITION", line, character);
                                fileStream.close();
                                return 0;
                            }
                            if(isContextArray(currentArray, currentObject)) {
                                currentArray->data.push_back(std::move(v));
                            } else {
                                currentObject->data.emplace(key, std::move(v));
                            }
                            parserState = VALUE_WRITTEN;
                            tmpVal.clear();
                            key.clear();
                        } else {
                            tmpVal += ch;
                        }
                        break;
                    } 
                    // --- What to do when the type with unknown case is spotted ---
                    default:
                        std::cout << "Not implemented yet\n";
                        key.clear();
                        tmpVal.clear();
                        break;
                }
                break;
            }
            // What to do when the value is written 
            case VALUE_WRITTEN:
                break;    
        }
        // If value is written, outside of the switch case it will be checked
        if (parserState == VALUE_WRITTEN) {
            if(isWhiteSpace(ch)) continue;
            else if(ch == ',' && !isContextArray(currentArray, currentObject)) {
                parserState = BEGIN_KEY;
            }
            else if(ch == ',' && isContextArray(currentArray, currentObject)) {
                parserState = BEGIN_VALUE;
            }
            else if(ch == ']' && currentArray->parentArray != nullptr) {
                currentArray = currentArray->parentArray;
            }
            else if(ch == ']' && currentArray->parentArray == nullptr) {
                currentObject = currentArray->parentObject;
                currentArray = nullptr; 
            }
            else if(ch == '}') {
                if(currentObject->parent == nullptr && currentObject->parentArray == nullptr) {
                    constructExitCode(code, PARSE_SUCCESS, "PARSE_SUCCESS", 0, 0);
                    std::cout << "Total line count = " << line << "\n----------------------\n";
                    fileStream.close();
                    return 1;
                } else if (currentObject->parentArray != nullptr) {
                    currentArray = currentObject->parentArray;
                    currentObject = nullptr;
                } else {
                    currentObject = currentObject->parent;
                    currentArray = nullptr;
                }
            }
            else {
                constructExitCode(code, PARSE_ERR_INCORRECT_VALUE_ENDING, "PARSE_ERR_INCORRECT_VALUE_ENDING", line, character);
                fileStream.close();
                return 0;
            }
        }
    }
    fileStream.close();
    return PARSE_UNHANDLED_ERROR;
}

// --- Definitions of the misc functions ---

void throwErrSyntax(const char *err) {
    std::cout << "JSON PARSE ERR: " << err << std::endl;
    exit(1);
}

int isWhiteSpace(char ch) {
    if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
        return 1;
    else 
        return 0;
}

int detectValueType(char ch, JsonTypes& type) {
    switch (ch) {
        case '"':
            type = JSON_STRING;
            break;
        case '[':
            type = JSON_ARRAY;
            break;
        case '{':
            type = JSON_OBJECT;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':               
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
           type = JSON_NUMBER;
           break;
        case 't':
        case 'f':  
            type = JSON_BOOL;
            break;
        case 'n':
            type = JSON_NULL;
            break;
        default:
            return -1;
    }
    return PARSE_SUCCESS;
}

int isContextArray(JsonArray *aCtx, JsonObject *oCtx) {
    int isctxarr = aCtx != nullptr && oCtx == nullptr;
    // if(isctxarr) {
    //     std::cout << "Context is array\n";
    // } else {
    //     std::cout << "Context is object\n";
    // }
    return isctxarr;
}

void constructExitCode(ParserExitCode &exitStruct, 
                       JsonParseRetVal code, std::string message,
                       int lineNumber, int characterNumber)
{
    exitStruct.returnCode = code;
    exitStruct.message = message;
    exitStruct.lineNumber = lineNumber;
    exitStruct.characterNumber = characterNumber;
    return;
}

int checkStringEnd(char ch) {
     if(ch == '"') {
        return 1;
     } else {
        return 0;
     }
}

int isNumber(char ch) {
//    std::cout << ch << std::endl;
    if((ch >= 0x30 && ch <= 0x39) || ch == 0x2d || ch == 0x2e) return 1;
    else return 0;
}
