#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <istream>

// --- JSON DATA STRUCTURES ---
enum JsonTypes {
    JSON_NUMBER, 
    JSON_STRING, 
    JSON_BOOL,   
    JSON_OBJECT, 
    JSON_ARRAY,
    JSON_NULL
};

enum JsonParseRetVal {
    PARSE_SUCCESS,
    PARSE_ERR_INCORRECT_OBJECT_START,
    PARSE_ERR_INCORRECT_KEY_DECLARATION,
    PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR,
    PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY,
    PARSE_ERR_INCORRECT_VALUE_TYPE,
    PARSE_ERR_INCORRECT_UNICODE_DECLARATION,
    PARSE_ERR_INCORRECT_NUMBER_DEFINITION,
    PARSE_ERR_INCORRECT_BOOL_DEFINITION,
    PARSE_ERR_INCORRECT_NULL_VALUE_DEFINITION,
    PARSE_ERR_INCORRECT_VALUE_ENDING,
    PARSE_ERR_INCORRECT_OBJECT_ENDING,
    PARSE_ERR_INCORRECT_ARRAY_ENDING,
    PARSE_ERR_DUPLICATE_ELEMENTS,
    PARSE_UNHANDLED_ERROR,
    PARSE_ERR_NULLPTR_PARENT,
    PARSE_ERR_NUMBER_OVERFLOW_OR_UNDERFLOW,
    PARSER_ERR_COMMA_AFTER_LAST_ELEMENT
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

struct ParserExitCode {
    JsonParseRetVal returnCode;
    std::string message;
    int lineNumber = 0, characterNumber = 0;
};

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
    bool exists(const std::string &key);
    bool getType(const std::string &key, JsonTypes &out);
    bool isNull(const std::string &key);
    bool get(const std::string &key, std::string &out);
    bool get(const std::string &key, long long &out);
    bool get(const std::string &key, double &out);
    bool get(const std::string &key, bool &out);
    bool get(const std::string &key, JsonObject *&out);
    bool get(const std::string &key, JsonArray *&out);
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
    bool getType(size_t index, JsonTypes &type);
    bool isNull(size_t index);
    bool get(size_t index, std::string &out);
    bool get(size_t index, long long &out);
    bool get(size_t index, double &out);
    bool get(size_t index, bool &out);
    bool get(size_t index, JsonObject *&out);
    bool get(size_t index, JsonArray *&out);
};

struct JsonValue {
    JsonValue() = default;
    std::variant<
        long long,
        double,
        bool,
        std::string,
        std::unique_ptr<JsonObject>,
        std::unique_ptr<JsonArray>
    > value;
    JsonTypes type;
};

// --- END JSON DATA STRUCTURES --- 

// --- Parser itself and misc functions ---
int parseJson(std::istream &stream, JsonObject& object, ParserExitCode& code);


