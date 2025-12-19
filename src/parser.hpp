#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <map>
#include <variant>
#include <vector>

// --- JSON DATA STRUCTURES ---

enum JsonTypes {
    JSON_NUMBER = 0, // DONE
    JSON_STRING = 1, // DONE 
    JSON_BOOL = 2,   // DONE
    JSON_OBJECT = 3, // IN_PROGRESS
    JSON_ARRAY = 4   // NOT_STARTED
};

enum JsonParseRetVal {
    PARSE_SUCCESS,
    PARSE_ERR_INCORRECT_OBJECT_START,
    PARSE_ERR_INCORRECT_KEY_DECLARATION,
    PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR,
    PARSE_ERR_INCORRECT_VALUE_TYPE,
    PARSE_ERR_INCORRECT_NUMBER_DEFINITION,
    PARSE_ERR_INCORRECT_BOOL_DEFINITION,
    PARSE_ERR_INCORRECT_VALUE_ENDING,
    PARSE_ERR_INCORRECT_OBJECT_ENDING,
    PARSE_UNHANDLED_ERROR,
};

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


struct JsonValue;

struct JsonObject;

struct JsonArray;

struct JsonObject {
    JsonObject() = default;
    std::map<std::string, JsonValue> data;
    // Parents and children declaration
    JsonObject* parent = nullptr;
    JsonArray* parentArray = nullptr;
    // JSON object related functions
    void clear() {
        data.clear();
    }
    void addParentObject(JsonObject* parent) {
        this->parent = parent;
    }
    void addParentArray(JsonArray* parentArray) {
        this->parentArray = parentArray;
    }
};

struct JsonArray {
    std::vector<JsonValue> data;
    JsonArray* parentArray = nullptr;
    JsonObject* parentObject = nullptr;
    void addParentObject(JsonObject* parent) {
        this->parentObject = parent;
    }
    void addParentArray(JsonArray* parent) {
        this->parentArray = parent;
    }
};

struct JsonValue {
    JsonValue() = default;
    std::variant<
        int,
        double,
        bool,
        std::string,
        std::unique_ptr<JsonObject>,
        std::unique_ptr<JsonArray>
    > value;
    JsonTypes type;
};


struct ParserExitCode {
    JsonParseRetVal returnCode;
    std::string message;
    int lineNumber, characterNumber;
};

// --- END JSON DATA STRUCTURES --- 

int parseJson(const char* name, JsonObject& object, ParserExitCode& code);

// --- Parser itself and misc functions ---

// --- Declaration of misc functions ---
void throwErrSyntax(const char *err); 
int detectValueType(char ch, JsonTypes& type);
int isWhiteSpace(char ch);
int checkStringEnd(char ch);
int isNumber(char ch);
void constructExitCode(ParserExitCode& exitStruct, JsonParseRetVal code, std::string message, int lineNumber, int characterNumber);
