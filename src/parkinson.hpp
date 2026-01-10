#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <istream>

namespace json {

// List of all the types in json objects
enum types {
    JSON_NUMBER, 
    JSON_STRING, 
    JSON_BOOL,   
    JSON_OBJECT, 
    JSON_ARRAY,
    JSON_NULL
};

// Return codes of the parser depending on the error
enum parseRetVal {
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

// See definition below
struct value;

// See definition below 
struct object;

// See definition below 
struct array;

// Structure that holds information about parse status
struct exitCode {
    parseRetVal returnCode;
    std::string message;
    int lineNumber = 0, characterNumber = 0;
    void reset() {
        message.clear();
        lineNumber = 0;
        characterNumber = 0;
    }
};

// Json object structure and its functions
struct object {
    std::map<std::string, value> data;
    // Parents and children declaration
    object* parent = nullptr;
    array* parentArray = nullptr;
    // JSON objectt function to add parent objects/arrays
    void addParentObject(object* parent) {
        this->parent = parent;
    }
    void addParentArray(array* parentArray) {
        this->parentArray = parentArray;
    }
    // Clearing data and parents of the object
    void clear() {
        data.clear();
        parent = nullptr;
        parentArray = nullptr;
    } 
    // Checking if element exists in the object
    bool exists(const std::string key);
    // Get type of the element of the object
    // If no such element exists, false is returned
    bool getType(const std::string key, types &out);
    // Check if element is a null 
    bool isNull(const std::string key);
    // Overloaded function to get data from the fields of the object 
    bool get(const std::string key, std::string &out); // Getting the string (JSON_STRING)
    bool get(const std::string key, long long &out); // Getting long long (JSON_NUMBER)
    bool get(const std::string key, double &out); // Getting double (JSON_NUMBER)
    bool get(const std::string key, bool &out); // Getting boolean (JSON_BOOL)
    bool get(const std::string key, object *&out); // Getting an object (pointer to the object) (JSON_OBJECT)
    bool get(const std::string key, array *&out); // Getting an array (Pointer to the array) (JSON_ARRAY)
    // Overloaded function to change value of the existing key 
    // or to add a new element with given key if element doesn't exist  
    void setValue(const std::string key, const char* value); // Setting string (1st method)  
    void setValue(const std::string key, std::string value); // Setting string (2nd method)
    void setValue(const std::string key, long long value); // Setting long long 
    void setValue(const std::string key, double value); // Setting double 
    void setValue(const std::string key, bool value); // Setting bool 
    void setValue(const std::string key, std::unique_ptr<object> value); // Setting object explicitly by moving it. To copy see "copy" method 
    object& emplaceObject(const std::string key); // Inexplicitly setting object (inserting empty object). Returns reference to an object inserted 
    void setValue(const std::string key, std::unique_ptr<array> value); // Setting array explicitly by moving it. To copy see "copy" method 
    array& emplaceArray(const std::string key); // Inexplicitly setting array (inserting empty array). Returns a reference to an array inserted 
    void setNull(const std::string key); // Setting value of a key to null (JSON_NULL). NULL type contains a std::monostate type 
    // Other object-specific functions 
    // Removing the element corresponding to the key. 
    // If element was removed, true is returned, otherwise false is returned 
    bool remove(const std::string key); 
    // Copying object. Returns a NON-COPYABLE pointer 
    static std::unique_ptr<object> copy(object &original);
    // Functions to jump to parent structures. 
    // Returns pointers to parent structure ot nullptr if parent doesn't exist. 
    // Only root object doesn't have any parents
    object* goToParentObject();
    array* goToParentArray();
};

// Array structure with it's functions 
struct array {
    // Data in the array
    // See value for more details on what it holds 
    std::vector<value> data;  
    // Parents of the array 
    // Has to be either one since array can't be standalone
    array* parentArray = nullptr;
    object* parentObject = nullptr;
    // Internal functions for adding parents 
    void addParentObject(object* parent) {
        this->parentObject = parent;
    }
    void addParentArray(array* parent) {
        this->parentArray = parent;
    }
    // Public functions to manipulate data in the array 
    // Returns length of the array 
    int length();
    // Gets the type of element (see types) by it's index 
    // If index is greater than length - 1 returns false  
    bool getType(size_t index, types &type);
    // Checks if element at an index is null or not
    bool isNull(size_t index);
    // Getter functions for the array.
    // Simmilar to object getters but thet get element by the index 
    bool get(size_t index, std::string &out); // Get string (JSON_STRING)
    bool get(size_t index, long long &out); // Get long long (JSON_NUMBER)
    bool get(size_t index, double &out); // Get double (JSON_NUMBER)
    bool get(size_t index, bool &out); // Get bool (JSON_BOOL)
    bool get(size_t index, object *&out); // Get an object (pointer to an object) 
    bool get(size_t index, array *&out); // Get an array (pointer to an array)
    // Overloaded function to push data into the array  
    void push(std::string val); // Push the string (1st method) 
    void push(const char *val); // Push the string (2nd method)
    void push(long long val); // Push the long long 
    void push(double val); // Push the double 
    void push(bool val); // Push bool 
    void push(std::unique_ptr<object> value); // Push the object explicitly (original structure will be moved) 
    void push(std::unique_ptr<array> value); // Push the array explicitly (original structure will be moved) 
    object& pushObject(); // Inexplicitly push empty object
    array& pushArray(); // Inexplicitly push empty array 
    void pushNull(); // Push null 
    // Overloaded function for setting elements at an index position
    bool setValue(uint index, std::string value); // Set string (method 1)
    bool setValue(uint index, const char* value); // Set string (method 2)
    bool setValue(uint index, long long value); // Set long long 
    bool setValue(uint index, double value); // Set double
    bool setValue(uint index, bool value); // Set bool 
    bool setValue(uint index, std::unique_ptr<object> value); // Set the object explicitly (original structure will be moved) 
    bool setValue(uint index, std::unique_ptr<array> value); // Set the array explicitly (original structure will be moved) 
    object* emplaceObject(uint index); // Inexplicitly set empty object
    array* emplaceArray(uint index); // Inexplicitly set empty array
    bool setNull(uint index); // Set null 
    // Other array-related functions 
    // Copying the array 
    // Returns a NON-COPYABLE pointer to the newly allocated copy 
    static std::unique_ptr<array> copy(array& original); 
    // Functions to jump to the parents of the current array
    // Array can have only one parent at a time
    // functions return pointer to the parent structure 
    object* goToParentObject();
    array* goToParentArray();
    // Removing the element with at the index position
    // If such element doesn't exist, returns false
    bool remove(uint index);
};

// Internal structure that allows to have a polymorphyc type 
// All the values in the objects/arrays are value typed to avoid having multiple types 
struct value {
    // Value variant. value van be got with std::get<type>(jv.value);
    std::variant<
        long long,
        double,
        bool,
        std::string,
        std::monostate,
        std::unique_ptr<object>,
        std::unique_ptr<array>
    > value;
    // Type corresponding to the value so it is clear 
    types type;
};

// --- END JSON DATA STRUCTURES --- 

// Parser function 
// Reads data from the stream 
// Parses JSON, writes data to object and writes exit information to code 
int parseJson(std::istream &stream, object& object, exitCode& code);

}
