#include "parkinson.hpp"
#include <cassert>
#include <sstream>
#include <iostream>

int main(void) {
    // Test quite function
    auto runTest = [](const char* json, int shouldPass) {
        std::istringstream in(json);
        JsonObject obj;
        ParserExitCode code;
        int ok = parseJson(in, obj, code);
        std::cout << code.message << std::endl << ok << std::endl;
        assert(ok == shouldPass);
        std::cout << "\x1B[92mTest success\033[0m\n";
    };

    // Test data
    const char* json_empty_object = R"({})";
    const char* json_primitives = R"({
        "int": 42,
        "double": 3.14,
        "bool_true": true,
        "bool_false": false,
        "string": "hello",
        "null_value": null
    })";
    const char* json_mixed_array = R"({
        "array": [true, "hello", 1234, 5.67, null]
    })";
    const char* json_nested_object = R"({
        "level1": {
            "level2": {
                "value": 10
            },
        "flag": false
        }
    })";
    const char* json_missing_brace = R"({
        "a": 1
    )";
    const char* json_invalid_number = R"({
        "num": 12.3.14
    })";
    const char* json_invalid_number_alt = R"({
        "num": --
    })";
    const char* json_invalid_bool = R"({
        "flag": tru
    })";
    const char* json_invalid_bool_alt = R"({
        "flag": fals
    })";
    const char* json_invalid_array_missing_comma = R"({
        "arr": [1 2, 3]
    })";
    const char* json_invalid_array_trailing_comma = R"({
        "arr": [1, 2, 3,]
    })";
    const char* json_duplicate_keys = R"({
        "a": 1,
        "a": 2
    })";
    
    // Valid JSONs
    runTest(json_empty_object, 1);
    runTest(json_primitives, 1);
    runTest(json_mixed_array, 1);
    runTest(json_nested_object, 1);
    
    // Invalid JSONs
    runTest(json_missing_brace, 0);
    runTest(json_invalid_number, 0);
    runTest(json_invalid_number_alt, 0);
    runTest(json_invalid_bool, 0);
    runTest(json_invalid_bool_alt, 0);
    runTest(json_invalid_array_missing_comma, 0);
    runTest(json_invalid_array_trailing_comma, 0);
    runTest(json_duplicate_keys, 0);
}

