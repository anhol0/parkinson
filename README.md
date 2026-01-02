# parkinson

### A simple C++ library for parsing and manipulating JSON data

## Building
### Dependences
1. ICU (will be removed in future for no-depencency build)
2. GNU Make

### Building the library
To build the library:

`make lib` in the project directory. `libparkinson.a` can be found in the build

To test the built library:

`make test`, all test should pass

You can also build the demo app:

`make demo`. it will print the JSON object that was parsed.

#### Demo usage:

`build/demo <any json file>`

## Getting started

The library provides an interface that allows user to interact with JSON objects:

#### Parsing the object

First, include `<parkinson.h>` create the empty JsonObject and ParserExitCode objects:

``` c++
#include <parkinson.h>

int main() {
    JsonOnject object;
    ParserExitCode code;
}
```

Then, to parse the JSON you will have to provide the stream for parser to read from.\
In this example file stream will be used:
``` c++
// Code before
// Creating a stream

std::ifstream in("file.json");

// Parsing the file with the parkinson

int retCode = parseJson(in, object, code);
```
The line above called the parse function which wrote data to the object

It also wtote the data to the 'code' variable that is:
```c++
struct ParserExitCode {
   JsonParseRetVal returnCode;
   std::string message;
   int lineNumber; 
   int characterNumber;
};
```
If parsing finished successfully, `retCode`  will be 1 and fields of the `code` will be the following:
```
JsonParseRetVal returnCode = PARSE_SUCCESS;
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

`bool object.get(std::string &key, <data type> &out)`

So to get integer from the object you can use:
``` c++
// Code before
long long example;
bool ok = object.get("key", i);
// Operations with got result
```
True will be returned and `i` will be filled with value if function succeeded in getting the data, false will be returned and `i` will stay untouched otherwise

Also, you can check whether value for key exists with:

`bool ex = object.exists("key")`

True/False will be returned respectively to the existance of the element

You can also check type of the element if it exists:

``` c++
JsonTypes type;
bool ok = getType("key", type);
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

Also, data from the objects can be cleared with `object.clear()`

#### All JSON features, including basic escaping, Unicode codepoints (UTF-8 and UTF-16) and exponents in numbers are supported.
## TO DO

1. Adding data to the objects
2. Jumping to parents of the objects (arrays and objects)
3. Not depending on ICU and utilizing own interfaces to encode/decode UTF-8 and check Unicode codepoints

## Credits:
1. [ICU from the Unicode team](https://github.com/unicode-org/icu)
2. [GNU Make as a build system](https://www.gnu.org/home.en.html)
