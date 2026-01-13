#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unicode/umachine.h>
#include <unicode/utf16.h>
#include <unicode/utf8.h>
#include <utility>
#include <variant>
#include "./parkinson.hpp"

// Internal structure that indicated state of the parser
enum parserState {
    WAITING_FOR_OBJECT,
    // KEY STATES
    BEGIN_KEY,
    WRITE_KEY,
    KEY_WRITTEN,
    // VALUE STATES
    BEGIN_VALUE,
    WRITE_VALUE,
    VALUE_WRITTEN,   
};

using namespace json;

// --- Declaration of misc functions ---
bool processString(std::string &in, std::string &out);
bool isWhole(const std::string& s);
bool processNumber(std::string &in);
void throwErrSyntax(const char *err); 
int detectValueType(char ch, json::types& type);
int isWhiteSpace(char ch);
int checkStringEnd(char ch);
int isNumber(char ch);
int isContextArray(json::array* aCtx, object* oCtx);
void constructExitCode(exitCode& exitStruct, json::parseRetVal code, std::string message, int lineNumber, int characterNumber);

int json::parse(std::istream &stream, object& object, exitCode& code) {
    char ch;
   
    int line = 1;
    int character = 1;
    parserState parserState = WAITING_FOR_OBJECT;
    json::object* currentObject = &object;
    json::array* currentArray = nullptr;
    json::types type; 
    //Key temp string
    std::string tmpKey;
    std::string key;
    // Temp values of the pair
    std::string tmpVal;
    // Reading the stream of data and
    bool prevBS = false;
    while(stream.get(ch)) {
        if(ch == '\n') {
            line++;
            character = 1;
        }
        else if(ch == '\t') {
            character += 4;
        } else {
            character++;
        }
    
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
                    return 0;
                } 
                break;
            }
            // BEGINNING OF KEY WRITING
            case BEGIN_KEY: {
                if(ch == '"') {
                    parserState = WRITE_KEY;
                    prevBS = false;
                } else if(ch == '}') {
                    parserState = VALUE_WRITTEN;
                } else {
                    constructExitCode(code, PARSE_ERR_INCORRECT_KEY_DECLARATION, "PARSE_ERR_INCORRECT_KEY_DECLARATION", line, character);
                    return 0;
                }
                break;
            }
            // WRITING THE KEY
            case WRITE_KEY: {
                if(ch == '"' && !prevBS) { 
                    parserState = KEY_WRITTEN;
                    if(!processString(tmpKey, key)) {
                        constructExitCode(code, PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY, "PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY", line, character);
                        return 0;
                    }
                    if(currentObject->data.contains(key)){
                        constructExitCode(code, PARSE_ERR_DUPLICATE_ELEMENTS, "PARSE_ERR_DUPLICATE_ELEMENTS", line, character);
                        return 0;
                    }
                    tmpKey.clear();
                } else {
                    tmpKey += ch;
                    prevBS = (ch == '\\' && !prevBS);
                }
                break;
            }
            // CHECKING SEPARATOR BETWEEN KEY AND VALUE
            case KEY_WRITTEN: {
                if(ch == ':') {
                    parserState = BEGIN_VALUE;
                } else {
                    constructExitCode(code, PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR, "PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR", line, character);
                    return 0;
                }
                break;
            }
            // CHECKING VALUE TYPE
            case BEGIN_VALUE: {
                prevBS = false;
                int retCode = detectValueType(ch, type);
                if(ch == '}') {
                    constructExitCode(code, PARSER_ERR_COMMA_AFTER_LAST_ELEMENT, "PARSER_ERR_COMMA_AFTER_LAST_ELEMENT", line, character);
                    return 0;
                } else if(ch == ']') {
                    if(type == JSON_ARRAY) {
                        parserState = VALUE_WRITTEN;
                        break;
                    } else {
                        constructExitCode(code, PARSER_ERR_COMMA_AFTER_LAST_ELEMENT, "PARSER_ERR_COMMA_AFTER_LAST_ELEMENT", line, character);   
                        return 0;
                    }
                }
                if(retCode == -1) {
                    constructExitCode(code, PARSE_ERR_INCORRECT_VALUE_TYPE, "PARSE_ERR_INCORRECT_VALUE_TYPE", line, character);
                    return 0;
                }
                if(type == JSON_NUMBER || type == JSON_BOOL || type == JSON_NULL) {
                    tmpVal += ch;
                } else if(type == JSON_ARRAY) {
                    json::value v;
                    v.type = type;
                    v.value = std::make_unique<json::array>();
                    if(isContextArray(currentArray, currentObject)) {
                        auto &insertionResult = currentArray->data.emplace_back(std::move(v));
                        auto &arrayUptr = std::get<std::unique_ptr<json::array>>(insertionResult.value);
                        json::array* arrayPtr = arrayUptr.get();
                        arrayPtr->addParentArray(currentArray);
                        currentArray = arrayPtr;
                        currentObject = nullptr;
                        parserState = BEGIN_VALUE;
                    } else {
                        auto insertionResult = currentObject->data.emplace(key, std::move(v));
                        auto &insertedValue = insertionResult.first->second;
                        auto &arrayUPtr = std::get<std::unique_ptr<json::array>>(insertedValue.value);
                        json::array* arrayPtr = arrayUPtr.get();
                        arrayPtr->addParentObject(currentObject);
                        currentArray = arrayPtr;
                        currentObject = nullptr;
                        parserState = BEGIN_VALUE;
                    }
                    tmpVal.clear();
                    key.clear();
                    break;
                } else if(type == JSON_OBJECT) {
                    json::value v;
                    v.type = type;
                    v.value = std::make_unique<json::object>();
                    if(isContextArray(currentArray, currentObject)) {
                        auto &insertResult = currentArray->data.emplace_back(std::move(v));
                        auto &objUPtr = std::get<std::unique_ptr<json::object>>(insertResult.value);
                        json::object* objPtr = objUPtr.get();
                        objPtr->addParentArray(currentArray);
                        currentObject = objPtr;
                        currentArray = nullptr;
                        parserState = BEGIN_KEY;
                    } else {
                        auto insertResult = currentObject->data.emplace(key, std::move(v));
                        auto &insertedValue = insertResult.first->second;
                        auto &objUPtr = std::get<std::unique_ptr<json::object>>(insertedValue.value);  
                        json::object* objPtr = objUPtr.get(); 
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
                        if(checkStringEnd(ch) && !prevBS) {
                            json::value v;
                            v.type = type;
                            std::string processed;
                            if(!processString(tmpVal, processed)) {
                                constructExitCode(code, PARSE_ERR_INCORRECT_UNICODE_DECLARATION, "PARSE_ERR_INCORRECT_UNICODE_DECLARATION", line, character);
                                return 0;
                            }
                            v.value = processed;
                            if(isContextArray(currentArray, currentObject)) {
                                currentArray->data.push_back(std::move(v));
                            } else {
                                currentObject->data.emplace(key, std::move(v));
                            }
                            parserState = VALUE_WRITTEN;
                            key.clear();
                            tmpVal.clear();
                            processed.clear();
                            continue;
                        } else {
                            tmpVal += ch;
                            prevBS = (ch == '\\' && !prevBS);
                        }
                        break;
                    }
                    // --- Writing number (int/float) values ---
                    case JSON_NUMBER: {
                        if(isWhiteSpace(ch) || ch == ',' || ch == ']' || ch == '}') {
                            bool success = processNumber(tmpVal);
                            if(!success) {
                                constructExitCode(code, PARSE_ERR_INCORRECT_NUMBER_DEFINITION, "PARSE_ERR_INCORRECT_NUMBER_DEFINITION", line, character);
                                return 0;
                            }
                            bool isInt = isWhole(tmpVal);
                            json::value v;
                            v.type = type;
                            if(isInt) {
                                errno = 0;
                                long long i = std::strtoll(tmpVal.c_str(), nullptr, 10); 
                                v.value = i; 
                            } else {
                                double d = std::strtod(tmpVal.c_str(), nullptr);
                                if(errno == ERANGE && !std::isfinite(d)) {
                                    constructExitCode(code, PARSE_ERR_NUMBER_OVERFLOW_OR_UNDERFLOW, "PARSE_ERR_NUMBER_OVERFLOW_OR_UNDERFLOW", line, character);
                                    return 0;
                                }
                                v.value = d;
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
                            tmpVal += ch;                           
                        }
                        break;
                    }
                    // --- Writing boolean variable ---
                    case JSON_BOOL: {
                        if(isWhiteSpace(ch) || ch == ',' || ch == ']' || ch == '}') {
                            json::value v;
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
                        if(isWhiteSpace(ch) || ch == ',' || ch == ']' || ch == '}') {
                            json::value v;
                            v.type = JSON_NULL;
                            if(tmpVal == std::string("null")) {
                                v.value = std::monostate{};
                            } else {
                                constructExitCode(code, PARSE_ERR_INCORRECT_NULL_VALUE_DEFINITION, "PARSE_ERR_INCORRECT_NULL_VALUE_DEFINITION", line, character);
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
            if(currentArray != nullptr) {
                if(ch == ',') {
                    parserState = BEGIN_VALUE;
                }
                else if(ch == ']') {
                    if(currentArray->parentArray != nullptr) {
                        currentArray = currentArray->parentArray;
                    } else if(currentArray->parentObject != nullptr) {
                        currentObject = currentArray->parentObject;
                        currentArray = nullptr;
                    } else {
                        currentArray = nullptr;
                    }
                    parserState = VALUE_WRITTEN;
                } else if(ch == '}') {
                    constructExitCode(code, PARSE_ERR_INCORRECT_ARRAY_ENDING, "PARSE_ERR_INCORRECT_ARRAY_ENDING", line, character);
                    return 0; 
                } else {
                    constructExitCode(code, PARSE_ERR_INCORRECT_VALUE_ENDING, "PARSE_ERR_INCORRECT_VALUE_ENDING", line, character);
                    return 0;
                }
            } else if(currentObject != nullptr) {
                if(ch == ',') parserState = BEGIN_KEY;
                else if(ch == '}') {
                    if(currentObject->parentArray != nullptr) {
                        currentArray = currentObject->parentArray;
                        currentObject = nullptr;
                    } else if(currentObject->parent != nullptr) {
                        currentObject = currentObject->parent;
                    } else {
                        constructExitCode(code, PARSE_SUCCESS, "PARSE_SUCCESS", 0, 0);
                        return 1;
                    }
                } else if(ch == ']') {
                    constructExitCode(code, PARSE_ERR_INCORRECT_OBJECT_ENDING, "PARSE_ERR_INCORRECT_OBJECT_ENDING", line, character);
                    return 0; 
                } else {
                    constructExitCode(code, PARSE_ERR_INCORRECT_VALUE_ENDING, "PARSE_ERR_INCORRECT_VALUE_ENDING", line, character);
                    return 0;
                } 
            } else {
                constructExitCode(code, PARSE_ERR_INCORRECT_VALUE_ENDING, "PARSE_ERR_INCORRECT_VALUE_ENDING", line, character);
                return 0;
            }
        }
    }
    constructExitCode(code, PARSE_ERR_INCORRECT_OBJECT_ENDING, "PARSE_ERR_INCORRECT_OBJECT_ENDING", line, character);
    return 0;
}

// --- Definitions of the misc functions ---

bool processNumber(std::string& in) {
    int i = 0;
    int size = in.size();

    if(size == 0) return false;

    if(in[i] == '-') {
        i++;
    }

    if(i >= size) return false;
    
    if(in[i] == '0') {
        i++;
        if(i < size && isdigit(in[i])) return false; 
    } else if(std::isdigit(in[i])) {
        while(i < size && std::isdigit(in[i])) i++;
    } else {
        return false;
    }
    
    if(i < size && in[i] == '.') {
        i++;
        if(i >= size || !std::isdigit(in[i])) return false;
        while(i < size && std::isdigit(in[i])) i++;
    }
    
    if(i < size && (in[i] == 'e' || in[i] == 'E')) {
        i++;
        if(i < size && (in[i] == '+' || in[i] == '-')) i++;
        if(i >= size || !std::isdigit(in[i])) return false;
        while(i < size && std::isdigit(in[i])) i++;
    }

    if(i != size) return false; 

    return true;
}

bool isWhole(const std::string& s) {
    for(char c: s) {
        if(c == '.' || c == 'e' || c == 'E') {
            return false;
        }
    }
    return true;
}

bool codepointToUTF8S(UChar32 cp, std::string &out) {
    char buf[5];
    int32_t i = 0;
    UBool error = false;
    U8_APPEND(buf, i, 5, cp, error);
    if(error) return false;
    out.append(buf, i);
    return true;
}

bool processString(std::string &in, std::string &out) {
    std::istringstream stream(in);
    char c;

    UChar32 lead;
    bool has_lead = false;
    while(stream.get(c)) {
        if(c == '\\' && stream.peek() == 'u') {
            stream.get();
            char hex[4];
            for(int i = 0; i < 4; i++) {
                if(!stream.get(hex[i]) || ! std::isxdigit(hex[i])) return false;
            }
            UChar32 cp = std::stoul(std::string(hex, 4), nullptr, 16);
            if(U16_IS_LEAD(cp)) {
                if(has_lead) return false;
                lead = cp;
                has_lead = true;
            } else if(U16_IS_TRAIL(cp)) {
                if(!has_lead) return false;
                UChar32 cp16 = U16_GET_SUPPLEMENTARY(lead, cp);
                if(!codepointToUTF8S(cp16, out)) return false;
                has_lead = false;
            } else {
                if(has_lead) return false;
                if(!codepointToUTF8S(cp, out)) return false;
            }
        } else if(c == '\\') {
            stream.get(c);
            switch (c) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case '/':  out += '/';  break;
                case 'b':  out += '\b'; break;
                case 'f':  out += '\f'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                default: return false;
            }
        } else {
            if(has_lead) return false;
            out += c;
        }
    }
    return !has_lead;
}

int isWhiteSpace(char ch) {
    if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
        return 1;
    else 
        return 0;
}

int detectValueType(char ch, json::types& type) {
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
        case '.':
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

int isContextArray(json::array *aCtx, object *oCtx) {
    int isctxarr = aCtx != nullptr && oCtx == nullptr;
    return isctxarr;
}

void constructExitCode(exitCode &exitStruct, 
                       json::parseRetVal code, std::string message,
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
    if((ch >= '0' && ch <= '9') || ch == '-' || ch == '.' || ch == 'e' || ch == '+') return 1;
    else return 0;
}
