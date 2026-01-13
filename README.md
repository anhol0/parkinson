# parkinson

### A simple C++ library for parsing and manipulating JSON data

## Building
### Dependences
1. ICU 
2. GNU Make

### Building the library
To build the library:

`make lib` in the project directory. `libparkinson.a` can be found in the build

To test the built library:

`make test`, all test should pass

You can also build the demo app:

`make demo`. Demo program 

#### Demo usage:

`build/demo <any json file> <any other json file>`

## Getting started

The library provides an interface that allows user to interact with JSON objects 
All the functions and structures are in *json* namespace

#### Parsing the object

First, include `<parkinson.h>` create the empty object and exitCode objects:

``` c++
#include <parkinson.h>

int main() {
    json::object object;
    json::exitCode code;
}
```

Then, to parse the JSON you will have to provide the stream for parser to read from.\
In this example file stream will be used:
``` c++
// Code before

// Creating a stream
std::ifstream in("file.json");

// Parsing the file with the parkinson
int retCode = json::parse(in, object, code);
```
The line above called the parse function which wrote data to the *object*

It also wtote the data to the *code* variable that is:
```c++
struct ParserExitCode {
   json::parseRetVal returnCode;
   std::string message;
   int lineNumber; 
   int characterNumber;
};
```
If parsing finished successfully, `retCode`  will be 1 and fields of the `code` will be the following:
```
json::parseRetVal returnCode = PARSE_SUCCESS;
std::string message = "PARSE_SUCCESS";
int lineNumber = 0; 
int characterNumber = 0;
```
Otherwise, there occured a problem while parsing the file

It will fill `returnCode` and `message` fields according to the error that happened:
``` c++
PARSE_SUCCESS
PARSE_ERR_INCORRECT_OBJECT_START
PARSE_ERR_INCORRECT_KEY_DECLARATION
PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR
PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY
PARSE_ERR_INCORRECT_VALUE_TYPE
PARSE_ERR_INCORRECT_UNICODE_DECLARATION
PARSE_ERR_INCORRECT_NUMBER_DEFINITION
PARSE_ERR_INCORRECT_BOOL_DEFINITION
PARSE_ERR_INCORRECT_NULL_VALUE_DEFINITION
PARSE_ERR_INCORRECT_VALUE_ENDING
PARSE_ERR_INCORRECT_OBJECT_ENDING
PARSE_ERR_INCORRECT_ARRAY_ENDING
PARSE_ERR_DUPLICATE_ELEMENTS
PARSE_UNHANDLED_ERROR
PARSE_ERR_NULLPTR_PARENT
PARSE_ERR_NUMBER_OVERFLOW_OR_UNDERFLOW
PARSER_ERR_COMMA_AFTER_LAST_ELEMENT
```
Fields `lineNumber` and `characterNumber` will be filled with the line and character numbers of where the error happened

If parsing was completed successfully, user can get the data from the object using getter functions.
Their structure is the following:

Objects:

`bool object.get(std::string &key, <data type> &out)`

Arrays:

`bool array.get(size_t index, <data type> &out)`

So to get integer from the object/array you can use:
``` c++
// Code before
long long example;

// Objects 
bool ok = object.get("key", example);

// Arrays 
bool ok = array.get(0, example);

```
True will be returned and *example* will be filled with value if function succeeded in getting the data 

Otherwise, false will be returned and *example* will stay untouched

Also, you can check whether value for key exists with:

`bool ok = object.exists("key")`

True/False will be returned respectively to the existance of the element

You can also check type of the element if it exists:

``` c++
json::types type;

// Object 
bool ok = object.getType("key", type);

// Array 
bool ok = object.getType(0, type);

```
If function succeeds, type will be filled with one of the types:
```c++
JSON_NUMBER
JSON_STRING
JSON_BOOL
JSON_OBJECT
JSON_ARRAY
JSON_NULL
```
If not, `false` will be returned and `type` will remain untouched

Data from the objects can be cleared with `object.clear()`

Basic values in the parsed objects/arrays can be set with:

``` c++
// Object 
// If element exists, its value will be replaced 
// If not - new key-value pair will be added 
object.setValue("key", 111LL);
object.setValue("key", 111.34);
object.setValue("key", "string");
object.setValue("key", std::string("string"));
object.setValue("key", true);
object.setNull("key");

// Array
// Value will be added to the end of the array 
array.push(1110LL);
array.push(111.34);
array.push("value");
// etc...
array.pushNull();
```

For complex types as *object* and *array*, a unique pointer has to be provided as such:

``` c++
json::object objectToMove;
// Add any data to objectToMove 
// Add objects 

// Object implementation
object.setValue("key", std::make_unique<json::object>(std::move(objectToMove)));

// Array implementation 

// Pushes object to the last position of the array 
object.push(std::make_unique<json::object>(std::move(objectToMove)));

// Adds object at the certain index of the array 
object.setValue(0, std::make_unique<json::object>(std::move(objectToMove)));


// Add arrays
json::array arrayToMove;
// Do something with arrayToMove, for example copy data from main object into it 

// Object implementation 
object.setValue("key", std::make_unique<json::array>(std::move(arrayToMove)));

// Array implementation 

// Pushes array to the last position of the array 
object.push(std::make_unique<json::array>(std::move(objectToMove)));

// Adds array at the certain index of the array 
object.setValue(0, std::make_unique<json::array>(std::move(objectToMove)));
```

If needed to insert empty object/array, then automated functions can be used

``` c++
// Object implementation 

// Adding empty object 
json::object &refObj = object.emplaceObject("key");

// Adding empty array 
json::array &refArr = object.emplaceArray("key");


// Array implementation

// Adding empty object at a specific index  
json::object &refObj = array.emplaceObject(index);

// Adding empty array ar a specific index 
json::array &refArr = array.emplaceArray(index);

// Pushing empty object to the end of array 
json::object &refObj = array.pushObject();

// Pushing empty array to the end of the array 
json::array &refArr = array.pushArray();
```

Reference to emplaced values will be returned so data could be added to them later

Data from the structures can be removed with

```c++
// Objects 
object.remove("key");
// Arrays 
bool ok = array.remove(index);
// true -> success 
// false -> fail 
```

Objects and arrays can be copied with

```c++
// Copying the object 
std::unique_ptr<json::object> copy = json::object::copy(origObject);

// Copying the array
std::unique_ptr<json::array> copyArr = json::array::copy(origArray);
```

After unique pointer is returned, it can be modified separately from the original object or array  

Parent objects/arrays can be accessed with

```c++
// If no parent of this type, nullptr will be returned 
json::object *po = object.goToParentObject();
Json::array *pa = object.goToParentArray();

// Same idea for arrays
// Only difference is that for arrays one of the parent is always present 
json::object *po = array.goToParentObject();
Json::array *pa = array.goToParentArray();
```

After all modifications to the object were made, it can be outputed into the output stream

``` c++
std::ostream stream(std::cout.rdbuf());
json::outputObject(stream, object);
```

### Sample program:

``` c++
int main(int argc, char **argv) {
    if(argc < 2) {
        std::cout << "Usage: parkinson <filename.json>" << std::endl;
        return 1;
    }
    const char* name = argv[1];

    // Setting all variables and objects 
    json::object object;
    json::exitCode code;
    
    // Opening a new input stream 
    std::ifstream stream;
    stream.open(name);
    
    // Parsing the json file into an object
    json::parse(stream, object, code);    
    
    stream.close();

    // Error checking
    if (!exitCode) {
        std::cout << 
        "JSON_PARSE_ERROR_CODE: " << code.message << 
        " on " << code.lineNumber << ":" << code.characterNumber << "\n" <<
        "Exit code: " << code.returnCode << std::endl;
        object.clear();
        exit(1);
    }

    // Creating a new output stream that outputs to the stdout
    std::ofstream stream(std::cout.rdbuf());

    // Adding data to the object parsed 
    object.setValue("somekey", 123LL);
    // Making a copy of an object 
    std::unique_ptr<json::object> copy = json::object::copy(object);
    // Adding new value to the object copy 
    copy->setValue("key", "Hello world");

    // Outputing both objects to stdout 
    stream << "Original object with extra key-value\n------\n";
    jsson::outputObject(stream, object);
    stream << "Copy object with extra added key in it\n------\n";
    json::outputObject(stream, *copy);

    return 0;
}
```

#### All JSON features, including basic escaping, Unicode codepoints (UTF-8 and UTF-16) and exponents in numbers are supported.

## Credits:
1. [ICU from the Unicode team](https://github.com/unicode-org/icu)
2. [GNU Make as a build system](https://www.gnu.org/home.en.html)
