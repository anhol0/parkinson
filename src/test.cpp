#include "parkinson.hpp"
#include <cassert>
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>
#include <type_traits>

template<typename T> bool runTestObjectGetters(const char* json, const char* name, const std::string& key, const T& expected) { 
    std::istringstream in(json);
    json::object obj;
    json::exitCode code;
    int ok = json::parse(in, obj, code);
    if(!ok) {
        std::cout << name << ": " << "\x1B[91mFAIL\033[0m\n";
        return false;
    }

    ok = false;

    if constexpr (std::is_same_v<T, std::string>) {
        std::string out;
        ok = obj.get(key, out) && out == expected;
    } else if constexpr (std::is_same_v<T, long long>) {
        long long out;
        ok = obj.get(key, out) && out == expected;
    } else if constexpr (std::is_same_v<T, double>) {
        double out;
        ok = obj.get(key, out) && out == expected;
    } else if constexpr (std::is_same_v<T, bool>) {
        bool out;
        ok = obj.get(key, out) && out == expected;
    } else if constexpr (std::is_same_v<T, json::object*>) {
        json::object* out;
        ok = obj.get(key, out) && out != nullptr;
    } else if constexpr (std::is_same_v<T, json::array*>) {
        json::array* out;
        ok = obj.get(key, out) && out != nullptr;
    }

    std::cout << name << ": " << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m") << "\n";
    return ok;

};

template<typename T> bool runTestArrayGetters(const char* json, const char* name, const int index, const std::string& key, const T& expected) {
    std::istringstream in(json);
    json::object obj;
    json::exitCode code;
    int ok = json::parse(in, obj, code);
    if(!ok) {
        std::cout << name << ": " << "\x1B[91mFAIL\033[0m\n";
        return false;
    }
    json::array* arr;
    if(!obj.get(key, arr)) {
        std::cout << name << ": failed to get array '" << key << "'\n";
        return false;
    }
    ok = false;
    if constexpr (std::is_same_v<T, std::string>) {
        std::string out;
        ok = arr->get(index, out) && out == expected;
    } else if constexpr (std::is_same_v<T, long long>) {
        long long out;
        ok = arr->get(index, out) && out == expected;
    } else if constexpr (std::is_same_v<T, double>) {
        double out;
        ok = arr->get(index, out) && out == expected;
    } else if constexpr (std::is_same_v<T, bool>) {
        bool out;
        ok = arr->get(index, out) && out == expected;
    }
    std::cout << name << ": " << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m") << "\n";
    return ok;
};

template<typename T> bool runTestObjectSetVal(const char* json, const char* name, const std::string& key, const T& value) {
    std::istringstream in(json);
    json::object obj;
    json::exitCode code;

    using U = std::decay_t<T>;

    if (!json::parse(in, obj, code)) {
        std::cout << name << ": \x1B[91mFAIL (parse)\033[0m\n";
        return false;
    }

    // ---- SET VALUE ----
    if constexpr (std::is_same_v<U, const char*> || std::is_same_v<U, char*>)
        obj.setValue(key, value);

    else if constexpr (std::is_same_v<U, std::string>)
        obj.setValue(key, value);

    else if constexpr (std::is_same_v<U, long long>)
        obj.setValue(key, value);

    else if constexpr (std::is_same_v<U, double>)
        obj.setValue(key, value);

    else if constexpr (std::is_same_v<U, bool>)
        obj.setValue(key, value);

    else if constexpr (std::is_same_v<U, std::unique_ptr<json::object>>) {
        obj.setValue(key, std::move(const_cast<T&>(value)));
    }

    else if constexpr (std::is_same_v<U, std::unique_ptr<json::array>>) {
        obj.setValue(key, std::move(const_cast<T&>(value)));
    }

    else if constexpr (std::is_same_v<U, std::nullptr_t>) {
        obj.setNull(key);
    }

    else {
        static_assert(sizeof(U) == 0, "Unsupported type in runTestObjectSetVal");
    }

    // ---- GET + VERIFY ----
    bool ok = false;

    if constexpr (std::is_same_v<U, const char*> || std::is_same_v<U, std::string> || std::is_same_v<U, char*>) {
        std::string out;
        ok = obj.get(key, out) && out == value;
    }
    else if constexpr (std::is_same_v<U, long long>) {
        long long out;
        ok = obj.get(key, out) && out == value;
    }
    else if constexpr (std::is_same_v<U, double>) {
        double out;
        ok = obj.get(key, out) && out == value;
    }
    else if constexpr (std::is_same_v<U, bool>) {
        bool out;
        ok = obj.get(key, out) && out == value;
    }
    else if constexpr (
        std::is_same_v<U, std::unique_ptr<json::object>> ||
        std::is_same_v<U, std::unique_ptr<json::array>>
    ) {
        // existence check only
        ok = obj.contains(key);
    }
    else if constexpr (std::is_same_v<U, std::nullptr_t>) {
        ok = obj.isNull(key);
    }

    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";

    return ok;
}


bool testEmplaceObject(const char* name) {
    json::object root;

    json::object& child = root.emplaceObject("child");

    // mutate returned reference
    child.setValue("x", 123LL);

    auto it = root.data.find("child");
    if (it == root.data.end()) return false;
    if (it->second.type != json::JSON_OBJECT) return false;

    auto& stored = *std::get<std::unique_ptr<json::object>>(it->second.value);
    bool ok = (stored.data.at("x").type == json::JSON_NUMBER);
    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";
    return ok;
}

bool testEmplaceArray(const char* name) {
    json::object root;

    json::array& arr = root.emplaceArray("arr");

    arr.push(1LL);
    arr.push(2LL);

    auto it = root.data.find("arr");
    if (it == root.data.end()) return false;
    if (it->second.type != json::JSON_ARRAY) return false;

    auto& stored = *std::get<std::unique_ptr<json::array>>(it->second.value);
    bool ok = (stored.data.size() == 2);
    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";
    return ok;
}

template<typename T>
bool runTestArrayPush(
    const char* json,
    const char* name,
    const T& value
) {
    std::istringstream in(json);
    json::object root;
    json::exitCode code;

    using U = std::decay_t<T>;

    if(!json::parse(in, root, code)) {
        std::cout << name << ": \x1B[91mFAIL\033[0m (parse)\n";
        return false;
    }

    json::array& arr = root.emplaceArray("arr");
    bool ok = true;

    if constexpr (std::is_same_v<U, std::string>) {
        arr.push(value);
        std::string out;
        ok = arr.get(0, out) && out == value;

    } else if constexpr (std::is_same_v<U, char*>) {
        arr.push(value);
        std::string out;
        ok = arr.get(0, out) && out == value;

    } else if constexpr (std::is_same_v<U, long long>) {
        arr.push(value);
        long long out;
        ok = arr.get(0, out) && out == value;

    } else if constexpr (std::is_same_v<U, double>) {
        arr.push(value);
        double out;
        ok = arr.get(0, out) && out == value;

    } else if constexpr (std::is_same_v<U, bool>) {
        arr.push(value);
        bool out;
        ok = arr.get(0, out) && out == value;

    } else if constexpr (std::is_same_v<U, std::unique_ptr<json::object>>) {
        arr.push(std::move(const_cast<U&>(value)));
        json::object* out;
        ok = arr.get(0, out) && out != nullptr;

    } else if constexpr (std::is_same_v<U, std::unique_ptr<json::array>>) {
        arr.push(std::move(const_cast<U&>(value)));
        json::array* out;
        ok = arr.get(0, out) && out != nullptr;

    } else if constexpr (std::is_same_v<U, std::nullptr_t>) {
        arr.pushNull();
        ok = arr.isNull(0);

    } else {
        static_assert(!sizeof(U), "Unsupported push() type");
    }

    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";
    return ok;
}

bool runTestArrayImplicitPush(const char* name, bool objectTest) {
    json::object root;
    json::array& arr = root.emplaceArray("arr");

    bool ok = false;

    if(objectTest) {
        json::object& obj = arr.pushObject();
        obj.setValue("x", 42LL);

        json::object* out;
        ok = arr.get(0, out) && out->contains("x");
    } else {
        json::array& a = arr.pushArray();
        a.push(1LL);

        json::array* out;
        ok = arr.get(0, out) && out->length() == 1;
    }

    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";
    return ok;
}

template<typename T>
bool runTestArraySet(
    const char* json,
    const char* name,
    uint index,
    const T& value
) {
    std::istringstream in(json);
    json::object root;
    json::exitCode code;

    if(!json::parse(in, root, code)) {
        std::cout << name << ": \x1B[91mFAIL\033[0m (parse)\n";
        return false;
    }

    json::array& arr = root.emplaceArray("arr");
    arr.pushNull(); // ensure index 0 exists

    bool ok = false;

    if constexpr (std::is_same_v<T, std::string>) {
        ok = arr.setValue(index, value);
        std::string out;
        ok &= arr.get(index, out) && out == value;

    } else if constexpr (std::is_same_v<T, const char*>) {
        ok = arr.setValue(index, value);
        std::string out;
        ok &= arr.get(index, out) && out == value;

    } else if constexpr (std::is_same_v<T, long long>) {
        ok = arr.setValue(index, value);
        long long out;
        ok &= arr.get(index, out) && out == value;

    } else if constexpr (std::is_same_v<T, double>) {
        ok = arr.setValue(index, value);
        double out;
        ok &= arr.get(index, out) && out == value;

    } else if constexpr (std::is_same_v<T, bool>) {
        ok = arr.setValue(index, value);
        bool out;
        ok &= arr.get(index, out) && out == value;

    } else if constexpr (std::is_same_v<T, std::unique_ptr<json::object>>) {
        ok = arr.setValue(index, std::move(const_cast<T&>(value)));
        json::object* out;
        ok &= arr.get(index, out) && out != nullptr;

    } else if constexpr (std::is_same_v<T, std::unique_ptr<json::array>>) {
        ok = arr.setValue(index, std::move(const_cast<T&>(value)));
        json::array* out;
        ok &= arr.get(index, out) && out != nullptr;

    } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
        ok = arr.setNull(index) && arr.isNull(index);

    } else {
        static_assert(!sizeof(T), "Unsupported setValue() type");
    }

    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";
    return ok;
}

bool runTestArrayCopy(const char* name) {
    json::array original;

    original.push(123LL);
    original.push("hello");

    json::object& obj = original.pushObject();
    obj.setValue("x", 42LL);

    json::array& arr = original.pushArray();
    arr.push(true);

    std::unique_ptr<json::array> copy = json::array::copy(original);

    bool ok = true;

    // Different instances
    ok &= (copy.get() != &original);

    // Size preserved
    ok &= (copy->length() == original.length());

    // Primitive checks
    long long i;
    std::string s;
    ok &= copy->get(0, i) && i == 123;
    ok &= copy->get(1, s) && s == "hello";

    // Object deep copy
    json::object* objCopy;
    ok &= copy->get(2, objCopy);
    ok &= objCopy != &obj;

    long long x;
    ok &= objCopy->get("x", x) && x == 42;

    // Array deep copy
    json::array* arrCopy;
    ok &= copy->get(3, arrCopy);
    ok &= arrCopy != &arr;

    bool b;
    ok &= arrCopy->get(0, b) && b == true;

    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";

    return ok;
}

bool runTestObjectCopy(const char* name) {
    json::object original;

    original.setValue("a", 10LL);
    original.setValue("b", "text");
    original.setValue("c", true);

    json::object& child = original.emplaceObject("child");
    child.setValue("x", 999LL);

    json::array& arr = original.emplaceArray("arr");
    arr.push(3.14);

    std::unique_ptr<json::object> copy = json::object::copy(original);

    bool ok = true;

    // Different instances
    ok &= (copy.get() != &original);

    // Primitive values
    long long a;
    std::string b;
    bool c;

    ok &= copy->get("a", a) && a == 10;
    ok &= copy->get("b", b) && b == "text";
    ok &= copy->get("c", c) && c == true;

    // Object deep copy
    json::object* childCopy;
    ok &= copy->get("child", childCopy);
    ok &= childCopy != &child;

    long long x;
    ok &= childCopy->get("x", x) && x == 999;

    // Array deep copy
    json::array* arrCopy;
    ok &= copy->get("arr", arrCopy);
    ok &= arrCopy != &arr;

    double d;
    ok &= arrCopy->get(0, d) && d == 3.14;

    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";

    return ok;
}

bool outputTest(const char* json, const char* name) {
    std::istringstream in(json);
    json::object object;
    json::exitCode code;
    int ok = json::parse(in, object, code);
    if(!ok) {
        std::cout << name << ": "
            << "\x1B[91mFAIL\033[0m"
            << "\n";
        return false;
    }
    std::stringbuf str;
    std::ostream stream(&str);
    json::outputObject(stream, object);
    code.reset();
    object.clear();
    std::istringstream s(str.str());
    ok = json::parse(s, object, code);
    if(!ok) {
        std::cout << name << ": "
            << "\x1B[91mFAIL\033[0m"
            << "\n";
        return false;
    }
    std::cout << name << ": "
              << (ok ? "\x1B[92mPASS\033[0m" : "\x1B[91mFAIL\033[0m")
              << "\n";
    return ok;
}

int main(void) {
    // Test quite function
    auto runTest = [](const char* json, const char* name, json::parseRetVal retCode, int shouldPass) {
        std::istringstream in(json);
        json::object obj;
        json::exitCode code;
        int ok = json::parse(in, obj, code);
        if(ok == shouldPass && code.returnCode == retCode) {
            std::cout << name << ": " <<"\x1B[92mPASS\033[0m\n";
            return true;
        } else {
            std::cout << code.message << std::endl;
            std::cout << name << ": " << "\x1B[91mFAIL\033[0m\n";
            return false;
        }
    };

    // Test data
    const char* json_sample_object_object = R"({})";
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
    const char* json_sample_object_array = R"({
        "arr": []
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
    const char* json_unicode_basic =
    R"({
        "text": "Привет мир"
    })";
    const char* json_unicode_latin =
    R"({
        "text": "Café naïve résumé"
    })";
    const char* json_unicode_cjk =
    R"({
        "text": "漢字 かな 한글"
    })";
    const char* json_unicode_emoji =
    R"({
        "emoji": "😀🚀🔥"
    })";
    const char* json_unicode_key =
    R"({
        "ключ": "значение"
    })";
    const char* json_unicode_array =
    R"({
        "arr": ["😀", "Привет", "漢字"]
    })";
    const char* json_obj_missing_outer =
    R"({
        "a": {
            "b": 1
        }
    )";
    const char* json_obj_missing_inner =
    R"({
        "a": {
            "b": 1
    )";
    const char* json_obj_extra_close =
    R"({
        "a": 1
    }}
    )";
    const char* json_obj_wrong_close =
    R"({
        "a": {
            "b": 1
        ]
    })";
    const char* json_obj_early_close =
    R"({
        "a": {
            "b": 1
    })";
    const char* json_arr_missing_outer =
    R"({
        "a": [1, 2, 3
    })";
    const char* json_arr_missing_inner =
    R"({
        "a": [1, [2, 3]
    })";
    const char* json_arr_extra_close =
    R"({
        "a": [1, 2]
    ]]
    )";
    const char* json_arr_wrong_close =
    R"({
        "a": [1, 2}
    })";
    const char* json_arr_early_close =
    R"({
        "a": [1, [2, 3]
    ]
    )";
    const char* json_mixed_obj_close_array_open =
    R"({
        "a": [
            { "b": 1 }
    })";
    const char* json_mixed_arr_close_obj_open =
    R"({
        "a": {
            "b": [1, 2]
        ]
    })";
    const char* json_mixed_multi_mismatch =
    R"({
        "a": [
            {
                "b": [1, 2}
            ]
        }
    })";
    const char* json_nested_valid =
    R"({
        "a": {
            "b": [1, { "c": 2 }, 3]
        }
    })";
    const char* json_num_int = R"({
        "a": 0,
        "b": 42,
        "c": -17
    })";
    
    const char* json_num_big_int = R"({
        "big": 9223372036854775807,
        "neg": -9223372036854775808
    })";
    
    const char* json_num_decimal = R"({
        "pi": 3.14159,
        "small": -0.001,
        "exact": 10.500
    })";
    
    const char* json_num_exp_basic = R"({
        "a": 1e0,
        "b": 1e10,
        "c": -2e3
    })";
    
    const char* json_num_exp_signed = R"({
        "a": 5e-1,
        "b": 9e+2,
        "c": -3e-2
    })";
    
    const char* json_num_exp_decimal = R"({
        "a": 1.5e2,
        "b": -3.14e-2,
        "c": 0.1e1
    })";
    
    const char* json_num_exp_large = R"({
        "min": 1e-308,
        "max": 1e308
    })";
    
    const char* json_num_mixed = R"({
        "i": 10,
        "d": 3.14,
        "e": 1e2,
        "mix": -0.01e-2
    })";
    const char* json_num_leading_zero = R"({
        "bad": 01
    })";
    
    const char* json_num_leading_zero_neg = R"({
        "bad": -01
    })";
    const char* json_num_trailing_dot = R"({
        "bad": 1.
    })";
    
    const char* json_num_missing_int = R"({
        "bad": .5
    })";
    const char* json_num_exp_missing = R"({
        "bad": 1e
    })";
    
    const char* json_num_exp_sign_only = R"({
        "bad": 1e+
    })";
    
    const char* json_num_exp_double = R"({
        "bad": 1ee10
    })";
    
    const char* json_num_exp_malformed = R"({
        "bad": 1e+-2
    })";
    const char* json_num_double_dot = R"({
        "bad": 1.2.3
    })";
    
    const char* json_num_double_sign = R"({
        "bad": --1
    })";
    const char* json_num_nan = R"({
        "bad": NaN
    })";
    
    const char* json_num_inf = R"({
        "bad": Infinity
    })";
    const char* json_num_exp_oob = R"({
        "badmin": 1e-325,
        "badmax": 1e325
    })";
    // Basic BMP escape
    const char* json_unicode_escape_basic = R"({
        "text": "\u0048\u0065\u006C\u006C\u006F"
    })";
    
    // Mixed raw UTF-8 + escapes
    const char* json_unicode_escape_mixed = R"({
        "text": "Hello \u0020 World"
    })";
    
    // Cyrillic via escapes
    const char* json_unicode_escape_cyrillic = R"({
        "text": "\u041F\u0440\u0438\u0432\u0435\u0442"
    })";
    
    // CJK via escapes
    const char* json_unicode_escape_cjk = R"({
        "text": "\u4F60\u597D"
    })";
    
    // Emoji (surrogate pair)
    const char* json_unicode_escape_emoji = R"({
        "text": "\uD83D\uDE03"
    })";
    
    // Unicode escape inside array
    const char* json_unicode_escape_array = R"({
        "arr": ["\u0041", "\u0042", "\u0043"]
    })";
    
    // Unicode escape as key
    const char* json_unicode_escape_key = R"({
        "\u006B\u0065\u0079": "value"
    })";
    // Incomplete escape
    const char* json_unicode_escape_short = R"({
        "text": "\u123"
    })";
    
    // Non-hex digit
    const char* json_unicode_escape_nonhex = R"({
        "text": "\u12X4"
    })";
    
    // Lone high surrogate
    const char* json_unicode_escape_lone_high = R"({
        "text": "\uD800"
    })";
    
    // Lone low surrogate
    const char* json_unicode_escape_lone_low = R"({
        "text": "\uDC00"
    })";
    
    // Reversed surrogate order
    const char* json_unicode_escape_reversed = R"({
        "text": "\uDE03\uD83D"
    })";
    
    // High surrogate not followed by escape
    const char* json_unicode_escape_high_no_low = R"({
        "text": "\uD83Dabc"
    })";
    
    // Escape cut off by EOF
    const char* json_unicode_escape_eof = R"({
        "text": "\uD83D\uDE0"
    })";
    // Simple escaped quote in key
    const char* json_key_escape_quote = R"({
        "ke\"y": "value"
    })";
    
    // Backslash in key
    const char* json_key_escape_backslash = R"({
        "ke\\y": "value"
    })";
    
    // Forward slash (allowed)
    const char* json_key_escape_slash = R"({
        "ke\/y": "value"
    })";
    
    // Control escapes
    const char* json_key_escape_control = R"({
        "line\nbreak": "value"
    })";
    
    // Unicode escape (BMP)
    const char* json_key_unicode_basic = R"({
        "\u006B\u0065\u0079": "value"
    })";
    
    // Mixed raw UTF-8 + escape
    const char* json_key_unicode_mixed = R"({
        "ключ\u0031": "value"
    })";
    
    // Emoji in key (surrogate pair)
    const char* json_key_unicode_emoji = R"({
        "\uD83D\uDE03": "smile"
    })";
    
    // Escaped whitespace in key
    const char* json_key_escape_tab = R"({
        "tab\tkey": "value"
    })";
    // Invalid escape character
    const char* json_key_escape_invalid = R"({
        "ke\y": "value"
    })";
    
    // Unicode escape too short
    const char* json_key_unicode_short = R"({
        "\u12": "value"
    })";
    
    // Unicode escape non-hex
    const char* json_key_unicode_nonhex = R"({
        "\u12X4": "value"
    })";
    
    // Lone high surrogate
    const char* json_key_unicode_lone_high = R"({
        "\uD800": "value"
    })";
    
    // Lone low surrogate
    const char* json_key_unicode_lone_low = R"({
        "\uDC00": "value"
    })";
    
    // Reversed surrogate pair
    const char* json_key_unicode_reversed = R"({
        "\uDE03\uD83D": "value"
    })";
    
    // Escape ends at EOF
    const char* json_key_escape_eof = R"({
        "key\uD83D\uDE0"
    })";
    
    // Missing closing quote in key
    const char* json_key_missing_quote = R"({
        "key: "value"
    })";
    // JSON with escaped backslash
    const char* json_backslash = R"({"key": "This is a backslash: \\"})";
    
    // JSON with escaped quote
    const char* json_quote = R"({"key":"She said: \"Hello!\""})";
    
    // JSON with newline
    const char* json_newline = R"({"key":"Line1\nLine2"})";
    
    // JSON with tab
    const char* json_tab = R"({"key":"Column1\tColumn2"})";
    
    // JSON with Unicode
    const char* json_unicode = R"({"key":"Unicode: \u0041\u03B1"})"; // 'A' + 'α'



    int success = 0;
    int fail = 0;

    runTest(json_sample_object_object, "empty object", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_primitives, "all primitive types", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_mixed_array, "array with mixed types", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_nested_object, "nested object", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_basic, "unicode basic", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_latin, "unicode latin", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_cjk, "unicode cjk", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_emoji, "unicode emoji", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_key, "unicode key", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_array, "unicode array", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_sample_object_array, "empty array", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    
    runTest(json_missing_brace, "missing closing brace", json::PARSE_ERR_INCORRECT_OBJECT_ENDING, 0) ? success++ : fail++;
    runTest(json_invalid_number, "invalid number", json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_invalid_number_alt, "invalid number alt", json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_invalid_bool, "invalid bool", json::PARSE_ERR_INCORRECT_BOOL_DEFINITION, 0) ? success++ : fail++;
    runTest(json_invalid_bool_alt, "invalid bool alt", json::PARSE_ERR_INCORRECT_BOOL_DEFINITION, 0) ? success++ : fail++;
    runTest(json_invalid_array_missing_comma, "array missing comma", json::PARSE_ERR_INCORRECT_VALUE_ENDING, 0) ? success++ : fail++;
    runTest(json_invalid_array_trailing_comma, "array trailing comma", json::PARSER_ERR_COMMA_AFTER_LAST_ELEMENT, 0) ? success++ : fail++;
    runTest(json_duplicate_keys, "duplicate keys", json::PARSE_ERR_DUPLICATE_ELEMENTS, 0) ? success++ : fail++;
    
    runTest(json_obj_missing_outer, "object missing outer brace", json::PARSE_ERR_INCORRECT_OBJECT_ENDING, 0) ? success++ : fail++;
    runTest(json_obj_missing_inner, "object missing inner brace", json::PARSE_ERR_INCORRECT_OBJECT_ENDING, 0) ? success++ : fail++;
    runTest(json_obj_extra_close, "object extra closing brace", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_obj_wrong_close, "object wrong closing type", json::PARSE_ERR_INCORRECT_OBJECT_ENDING, 0) ? success++ : fail++;
    runTest(json_obj_early_close, "object closed too early", json::PARSE_ERR_INCORRECT_OBJECT_ENDING, 0) ? success++ : fail++;
    
    runTest(json_arr_missing_outer, "array missing outer bracket", json::PARSE_ERR_INCORRECT_ARRAY_ENDING, 0) ? success++ : fail++;
    runTest(json_arr_missing_inner, "array missing inner bracket", json::PARSE_ERR_INCORRECT_ARRAY_ENDING, 0) ? success++ : fail++;
    runTest(json_arr_extra_close, "array extra closing bracket", json::PARSE_ERR_INCORRECT_OBJECT_ENDING, 0) ? success++ : fail++;
    runTest(json_arr_wrong_close, "array wrong closing type", json::PARSE_ERR_INCORRECT_ARRAY_ENDING, 0) ? success++ : fail++;
    runTest(json_arr_early_close, "array closed too early", json::PARSE_ERR_INCORRECT_OBJECT_ENDING, 0) ? success++ : fail++;
    
    runTest(json_mixed_obj_close_array_open, "object closed while array open", json::PARSE_ERR_INCORRECT_ARRAY_ENDING, 0) ? success++ : fail++;
    runTest(json_mixed_arr_close_obj_open, "array closed while object open", json::PARSE_ERR_INCORRECT_OBJECT_ENDING, 0) ? success++ : fail++;
    runTest(json_mixed_multi_mismatch, "multiple nesting mismatch", json::PARSE_ERR_INCORRECT_ARRAY_ENDING, 0) ? success++ : fail++;
    
    runTest(json_nested_valid, "proper nested object and array", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    
    // ---- VALID NUMBERS ----
    runTest(json_num_int,             "json_num_int",             json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_num_big_int,         "json_num_big_int",         json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_num_decimal,         "json_num_decimal",         json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_num_exp_basic,       "json_num_exp_basic",       json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_num_exp_signed,      "json_num_exp_signed",      json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_num_exp_decimal,     "json_num_exp_decimal",     json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_num_exp_large,       "json_num_exp_large",       json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_num_mixed,           "json_num_mixed",           json::PARSE_SUCCESS, 1) ? success++ : fail++;
    
    // ---- INVALID NUMBERS ----
    runTest(json_num_leading_zero,        "json_num_leading_zero",        json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_leading_zero_neg,    "json_num_leading_zero_neg",    json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_trailing_dot,        "json_num_trailing_dot",        json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_missing_int,         "json_num_missing_int",         json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_exp_missing,         "json_num_exp_missing",         json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_exp_sign_only,       "json_num_exp_sign_only",       json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_exp_double,          "json_num_exp_double",          json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_exp_malformed,       "json_num_exp_malformed",       json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_double_dot,          "json_num_double_dot",          json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_double_sign,         "json_num_double_sign",         json::PARSE_ERR_INCORRECT_NUMBER_DEFINITION, 0) ? success++ : fail++;
    runTest(json_num_nan,                 "json_num_nan",                 json::PARSE_ERR_INCORRECT_VALUE_TYPE, 0) ? success++ : fail++;
    runTest(json_num_inf,                 "json_num_inf",                 json::PARSE_ERR_INCORRECT_VALUE_TYPE, 0) ? success++ : fail++;
    runTest(json_num_exp_oob,             "json_num_exp_oob",             json::PARSE_ERR_NUMBER_OVERFLOW_OR_UNDERFLOW, 0) ? success++ : fail++;
    // ---- VALID UNICODE ----
    runTest(json_unicode_escape_basic,     "unicode escape basic",     json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_escape_mixed,     "unicode escape mixed",     json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_escape_cyrillic,  "unicode escape cyrillic",  json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_escape_cjk,       "unicode escape cjk",       json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_escape_emoji,     "unicode escape emoji",     json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_escape_array,     "unicode escape array",     json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode_escape_key,       "unicode escape key",       json::PARSE_SUCCESS, 1) ? success++ : fail++;
    
    // ---- INVALID UNICODE ----
    runTest(json_unicode_escape_short,         "unicode escape too short",        json::PARSE_ERR_INCORRECT_UNICODE_DECLARATION, 0) ? success++ : fail++;
    runTest(json_unicode_escape_nonhex,        "unicode escape non-hex",          json::PARSE_ERR_INCORRECT_UNICODE_DECLARATION, 0) ? success++ : fail++;
    runTest(json_unicode_escape_lone_high,     "unicode lone high surrogate",     json::PARSE_ERR_INCORRECT_UNICODE_DECLARATION, 0) ? success++ : fail++;
    runTest(json_unicode_escape_lone_low,      "unicode lone low surrogate",      json::PARSE_ERR_INCORRECT_UNICODE_DECLARATION, 0) ? success++ : fail++;
    runTest(json_unicode_escape_reversed,      "unicode reversed surrogate pair", json::PARSE_ERR_INCORRECT_UNICODE_DECLARATION, 0) ? success++ : fail++;
    runTest(json_unicode_escape_high_no_low,   "unicode high surrogate no low",   json::PARSE_ERR_INCORRECT_UNICODE_DECLARATION, 0) ? success++ : fail++;
    runTest(json_unicode_escape_eof,           "unicode escape cut by eof",       json::PARSE_ERR_INCORRECT_UNICODE_DECLARATION, 0) ? success++ : fail++;

    // ---- VALID KEY ESCAPES ----
    runTest(json_key_escape_quote,        "key escape quote",        json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_key_escape_backslash,    "key escape backslash",    json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_key_escape_slash,        "key escape slash",        json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_key_escape_control,      "key escape control",      json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_key_unicode_basic,       "key unicode basic",       json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_key_unicode_mixed,       "key unicode mixed",       json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_key_unicode_emoji,       "key unicode emoji",       json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_key_escape_tab,          "key escape tab",          json::PARSE_SUCCESS, 1) ? success++ : fail++;
    
    // ---- INVALID KEY ESCAPES ----
    runTest(json_key_escape_invalid,      "key invalid escape",      json::PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY, 0) ? success++ : fail++;
    runTest(json_key_unicode_short,       "key unicode too short",   json::PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY, 0) ? success++ : fail++;
    runTest(json_key_unicode_nonhex,      "key unicode non-hex",     json::PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY, 0) ? success++ : fail++;
    runTest(json_key_unicode_lone_high,   "key lone high surrogate", json::PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY, 0) ? success++ : fail++;
    runTest(json_key_unicode_lone_low,    "key lone low surrogate",  json::PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY, 0) ? success++ : fail++;
    runTest(json_key_unicode_reversed,    "key reversed surrogate",  json::PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY, 0) ? success++ : fail++;
    runTest(json_key_escape_eof,          "key escape eof",          json::PARSE_ERR_INCORRECT_UNICODE_ESC_IN_KEY, 0) ? success++ : fail++;
    runTest(json_key_missing_quote,       "key missing quote",       json::PARSE_ERR_INCORRECT_KEY_VALUE_SEPARATOR,    0) ? success++ : fail++;

    runTest(json_backslash, "escaped backslash", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_quote, "escaped quote", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_newline, "escaped newline", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_tab, "escaped tab", json::PARSE_SUCCESS, 1) ? success++ : fail++;
    runTest(json_unicode, "unicode escapes", json::PARSE_SUCCESS, 1) ? success++ : fail++;


    const char* sampleJson = R"({
        "str": "hello",
        "int": 42,
        "dbl": 3.14,
        "bool": true,
        "nullVal": null,
        "obj": { "nested": 1 },
        "arr": [1, 2, 3]
    })";

    const char* sampleUnicodeJson = R"({
        "str": "\udb80\udeb5"
    })";

    runTestObjectGetters(sampleJson, "String test", "str", std::string("hello")) ? success++ : fail++;
    runTestObjectGetters(sampleJson, "Int test", "int", 42LL) ? success++ : fail++;
    runTestObjectGetters(sampleJson, "Double test", "dbl", 3.14) ? success++ : fail++;
    runTestObjectGetters(sampleJson, "Bool test", "bool", true) ? success++ : fail++;
    runTestObjectGetters(sampleJson, "Object test", "obj", (json::object*)nullptr) ? success++ : fail++;
    runTestObjectGetters(sampleJson, "Array test", "arr", (json::array*)nullptr) ? success++ : fail++;

    runTestArrayGetters(sampleJson, "Array index test", 0, "arr", 1LL);
    runTestArrayGetters(sampleJson, "Array index test", 1, "arr", 2LL);
    runTestArrayGetters(sampleJson, "Array index test", 2, "arr", 3LL);

    runTestObjectGetters(sampleUnicodeJson, "Unicode escaping parsing", "str", std::string("󰊵")) ? success++ : fail++;

    // -------------------- TEST DATA --------------------
    static const char* json_sample_object = R"({})";
    
    {
        // STRING TESTS
        runTestObjectSetVal(
            json_sample_object,
            "string: const char*",
            "str1",
            "value1"
        ) ? success++ : fail++;
    
        runTestObjectSetVal(
            json_sample_object,
            "string: std::string",
            "str2",
            std::string("value2")
        ) ? success++ : fail++;
    
        // NUMBER TESTS
        runTestObjectSetVal(
            json_sample_object,
            "number: long long",
            "num1",
            123456LL
        ) ? success++ : fail++;
    
        runTestObjectSetVal(
            json_sample_object,
            "number: double",
            "num2",
            3.14159
        ) ? success++ : fail++;
    
        // BOOLEAN TEST
        runTestObjectSetVal(
            json_sample_object,
            "bool",
            "flag",
            true
        ) ? success++ : fail++;
    
        // NULL TEST
        runTestObjectSetVal(
            json_sample_object,
            "null",
            "nothing",
            nullptr
        ) ? success++ : fail++;
    
        // OBJECT TEST
        {
            auto obj = std::make_unique<json::object>();
            obj->setValue("inner", 42LL);
    
            runTestObjectSetVal(
                json_sample_object,
                "object",
                "obj",
                std::move(obj)
            ) ? success++ : fail++;
        }
    
        // ARRAY TEST
        {
            auto arr = std::make_unique<json::array>();
            arr->push(1LL);
            arr->push(2LL);
    
            runTestObjectSetVal(
                json_sample_object,
                "array",
                "arr",
                std::move(arr)
            ) ? success++ : fail++;
        }
    }
    testEmplaceObject("emplace object test") ? success++ : fail++;
    testEmplaceArray("emplace array test") ? success++ : fail++;


    runTestArrayPush(json_sample_object, "push string", std::string("abc")) ? success++ : fail++;
    runTestArrayPush(json_sample_object, "push c-string", "abc") ? success++ : fail++;
    runTestArrayPush(json_sample_object, "push int", 123LL) ? success++ : fail++;
    runTestArrayPush(json_sample_object, "push double", 3.14) ? success++ : fail++;
    runTestArrayPush(json_sample_object, "push bool", true) ? success++ : fail++;
    runTestArrayPush(json_sample_object, "push null", nullptr) ? success++ : fail++;
    
    runTestArrayPush(json_sample_object, "push object",
        std::make_unique<json::object>()) ? success++ : fail++;
    
    runTestArrayPush(json_sample_object, "push array",
        std::make_unique<json::array>()) ? success++ : fail++;
    
    runTestArrayImplicitPush("pushObject implicit", true) ? success++ : fail++;
    runTestArrayImplicitPush("pushArray implicit", false) ? success++ : fail++;
    
    runTestArraySet(json_sample_object, "setValue string", 0, std::string("xyz")) ? success++ : fail++;
    runTestArraySet(json_sample_object, "setValue int", 0, 999LL) ? success++ : fail++;
    runTestArraySet(json_sample_object, "setValue bool", 0, false) ? success++ : fail++;
    runTestArraySet(json_sample_object, "setNull", 0, nullptr) ? success++ : fail++;

    runTestArrayCopy("array::copy deep copy") ? success++ : fail++;
    runTestObjectCopy("object::copy deep copy") ? success++ : fail++;

    outputTest(json_nested_object, "object output test") ? success++ : fail++;

    // -------------------- RESULTS --------------------
    std::cout << "TEST RESULTS\n";
    std::cout << "\x1B[92mSuccess: " << success<< "\033[0m\n";
    std::cout << "\x1B[91mFail: " << fail << "\033[0m\n";
    if(fail > 0) {
        std::cout << "\x1B[91mTEST FAIL\033[0m\n"; 
    } else {
        std::cout << "\x1B[92mALL TESTS PASS\033[0m\n"; 
    } 
}


