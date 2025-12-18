#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include "./parser.hpp"

int parseJson(const char *name, JsonObject& object, ParserExitCode& code) {
    std::ifstream fileStream(name);
    char ch;
   
    int line = 1;
    bool writingArray = false;
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
        }
        switch (parserState) {
            // BEGINNING OF THE OBJECT PARSING
            case WAITING_FOR_OBJECT: {
                if(isWhiteSpace(ch)) continue;
                if(ch == '{') {   
                    parserState = BEGIN_KEY;
                }
                else {
                    code.returnCode = PARSE_ERR_INCORRECT_OBJECT_START;
                    code.message = "Incorrect beginning of the object";
                    code.lineNumber = line;
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
                else if(ch == '}') {
                    currentObject = currentObject->parent;
                    parserState = VALUE_WRITTEN;
                }
                else if(isWhiteSpace(ch)) 
                    continue;
                else { 
                    code.returnCode = PARSE_ERR_INCORRECT_KEY_DECLARATION;
                    code.message = "Incorrect key declaration";
                    code.lineNumber = line;
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
                } else if(isWhiteSpace(ch)) {
                    continue;
                } else {
                    code.returnCode = PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR;
                    code.message = "Incorrect key-value separator";
                    code.lineNumber = line;
                    fileStream.close();
                    return 0;
                }
                break;
            }
            // CHECKING VALUE TYPE
            case BEGIN_VALUE: {
                if(isWhiteSpace(ch)) 
                    continue;
                int retCode = detectValueType(ch, type);
                if(retCode == -1) {
                    std::cout << ch << std::endl;
                    code.returnCode = PARSE_ERR_INCORRECT_VALUE_TYPE;
                    code.message = "Incorrect type of the value";
                    code.lineNumber = line;
                    return 0;
                }
                if(type == JSON_NUMBER) tmpVal += ch;
                else if(type == JSON_BOOL) tmpVal += ch;
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
                            if(writingArray) {
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
                        if(isWhiteSpace(ch) || ch == ',') {
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
                            // Writing the value in the specific data structure
                            if(writingArray) {
                                currentArray->data.push_back(std::move(v));
                            } else { 
                                currentObject->data.emplace(key, std::move(v));
                            }
                            // Deciding on what to do with parser state when data is written
                            if(writingArray) {
                                if(ch == ',') parserState = BEGIN_VALUE;
                                if(isWhiteSpace(ch)) continue;
                            } else {
                                if(ch == ',') parserState = BEGIN_KEY;
                                if(isWhiteSpace(ch)) parserState = VALUE_WRITTEN;
                            }
                            key.clear();
                            tmpVal.clear();
                        } else {
                            if(!isNumber(ch)) {
                                code.returnCode = PARSE_ERR_INCORRECT_NUMBER_DEFINITION;
                                code.message  = "Number definition is incorrect";
                                code.lineNumber = line;
                                fileStream.close(); 
                                return 0;
                            }
                            tmpVal += ch;                           
                        }
                        break;
                    }
                    // --- Writing boolean variable ---
                    case JSON_BOOL: {
                        if(isWhiteSpace(ch) || ch == ',') {
                            JsonValue v;
                            v.type = type;
                            if(tmpVal == std::string("true")) {
                                v.value = true;
                            } 
                            else if(tmpVal == std::string("false")) {
                                v.value = false;
                            } else {
                                code.returnCode = PARSE_ERR_INCORRECT_BOOL_DEFINITION;
                                code.message = "Invalid definition of the boolean value";
                                code.lineNumber = line;
                                fileStream.close();
                                return 0;
                            }
                            if(writingArray) {
                                currentArray->data.push_back(std::move(v));
                                if(isWhiteSpace(ch)) continue;
                                if(ch == ',') parserState = BEGIN_VALUE;
                            } else {
                                currentObject->data.emplace(key, std::move(v));
                                if(isWhiteSpace(ch)) parserState = VALUE_WRITTEN;
                                if(ch == ',') parserState = BEGIN_KEY;                           
                            }
                            tmpVal.clear();
                            key.clear();
                        } else {
                            tmpVal += ch;
                        } 
                        break;
                    }
                    // --- Setting context to the child (object type) ---
                    case JSON_OBJECT: {
                        JsonValue v;
                        v.type = type;
                        v.value = std::make_unique<JsonObject>();
                        if(writingArray) {
                            auto &insertResult = currentArray->data.emplace_back(std::move(v));
                            auto &objUPtr = std::get<std::unique_ptr<JsonObject>>(insertResult.value);
                            JsonObject* objPtr = objUPtr.get();
                            objPtr->addParentArray(currentArray);
                            currentObject = objPtr;
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
                        break;
                    }
                    // --- Writing into array ---
                    case JSON_ARRAY: {
                        JsonValue v;
                        v.type = type;
                        v.value = std::make_unique<JsonArray>();
                        if(writingArray) {
                            auto &insertionResult = currentArray->data.emplace_back(std::move(v));
                            auto &arrayUptr = std::get<std::unique_ptr<JsonArray>>(insertionResult.value);
                            JsonArray* arrayPtr = arrayUptr.get();
                            arrayPtr->addParentArray(currentArray);
                            currentArray = arrayPtr;
                            parserState = BEGIN_VALUE;
                        } else {
                            auto insertionResult = currentObject->data.emplace(key, std::move(v));
                            auto &insertedValue = insertionResult.first->second;
                            auto &arrayUPtr = std::get<std::unique_ptr<JsonArray>>(insertedValue.value);
                            JsonArray* arrayPtr = arrayUPtr.get();
                            arrayPtr->addParentObject(currentObject);
                            if(currentArray != nullptr) {
                                arrayPtr->addParentArray(currentArray);
                            }
                            currentArray = arrayPtr;
                            writingArray = true;
                            parserState = BEGIN_VALUE;
                        }
                        break; 
                    }
                    // --- What to do when the type with unknown case is spotted ---
                    default:
                        std::cout << "Not implemented yet\n";
                        key.clear();
                        break;
                }
                break;
            }

            // What to do when the value is written
            case VALUE_WRITTEN: {
                if(isWhiteSpace(ch)) continue;
                else if(ch == ',' && !writingArray) {
                    parserState = BEGIN_KEY;
                }
                else if(ch == ',' && writingArray) {
                    parserState = BEGIN_VALUE;
                }
                else if(ch == ']' && currentArray->parentArray != nullptr) {
                    currentArray = currentArray->parentArray;
                }
                else if(ch == ']' && currentArray->parentArray == nullptr) {
                    currentArray = nullptr;
                    writingArray = false;
                    parserState = BEGIN_KEY;
                }
                else if(ch == '}') {
                    if(currentObject->parent == nullptr && currentObject->parentArray == nullptr) {
                        code.returnCode = PARSE_SUCCESS;
                        code.message = "Parse success";
                        std::cout << "Total line count = " << line << "\n----------------------\n";
                        fileStream.close();
                        return 1;
                    } else if (currentObject->parentArray != nullptr) {
                        currentArray = currentObject->parentArray;
                        currentObject = nullptr;
                    } else {
                        currentObject = currentObject->parent;
                    }
                }
                else {

                    code.returnCode = PARSE_ERR_INCORRECT_VALUE_ENDING;
                    code.message = "Unknown ending of the value line";
                    code.lineNumber = line;
                    fileStream.close();
                    return 0;
                }
                break;
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
       default:
           return -1;
    }
    return PARSE_SUCCESS;
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
