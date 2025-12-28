#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <map>
#include <variant>
#include <vector>

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
    PARSE_ERR_INCORRECT_VALUE_TYPE,
    PARSE_ERR_INCORRECT_NUMBER_DEFINITION,
    PARSE_ERR_INCORRECT_BOOL_DEFINITION,
    PARSE_ERR_INCORRECT_NULL_VALUE_DEFINITION,
    PARSE_ERR_INCORRECT_VALUE_ENDING,
    PARSE_ERR_INCORRECT_OBJECT_ENDING,
    PARSE_ERR_DUPLICATE_ELEMENTS,
    PARSE_UNHANDLED_ERROR,
    PARSE_ERR_NULLPTR_PARENT
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
    bool getString(const std::string &key, std::string &out);
    bool getInt(const std::string &key, int &out);
    bool getDouble(const std::string &key, double &out);
    bool getBool(const std::string &key, bool &out);
    bool isNull(const std::string &key);
    bool getObject(const std::string &key, JsonObject *&out);
    bool getArray(const std::string &key, JsonArray *&out);
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
    bool getString(size_t index, std::string &out);
    bool getInt(size_t index, int &out);
    bool getDouble(size_t index, double &out);
    bool getBool(size_t index, bool &out);
    bool isNull(size_t index);
    bool getObject(size_t index, JsonObject *&out);
    bool getArray(size_t index, JsonArray *&out);
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

// --- END JSON DATA STRUCTURES --- 

// --- Parser itself and misc functions ---
int parseJson(std::ifstream &fileStream, JsonObject& object, ParserExitCode& code);


