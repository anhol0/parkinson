#include "parkinson.hpp"

void outputValue(std::ostream &stream, const json::value& value, int indent = 0);

void outputIndent(std::ostream &stream, int indentationLvl) {
    for(int i = 0; i < indentationLvl; i++) stream << "    ";
}

void json::outputObject(std::ostream &stream, const json::object& object, int indent) {
    stream << "{\n";
    for(auto it = object.data.begin(); it != object.data.end(); ++it) {
        outputIndent(stream, indent + 1);
        stream << "\"" << it->first << "\": ";
        outputValue(stream, it->second, indent + 1);
        if(std::next(it) != object.data.end()) stream << ",";
        stream << "\n";
    }
    outputIndent(stream, indent);
    indent != 0 ? stream << "}" : stream << "}\n";
}

void outputArray(std::ostream &stream, const json::array& array, int indent = 0) {
    stream << "[\n";
    for(size_t i = 0; i < array.data.size(); i++) {
       outputIndent(stream, indent+1);
       outputValue(stream, array.data[i], indent + 1);
       if(i + 1 != array.data.size()) stream << ",";
       stream << "\n";
    }
    outputIndent(stream, indent);
    stream << "]";
}

void outputValue(std::ostream &stream, const json::value& value, int indent) {
    switch(value.type) {
        case json::JSON_STRING:
            stream << "\"" << std::get<std::string>(value.value) << "\""; 
            break;  
        case json::JSON_BOOL:
            stream << std::get<bool>(value.value);
            break;  
        case json::JSON_NUMBER:
            if(const long long* pInt = std::get_if<long long>(&value.value)) {
                stream << *pInt;
            } else {
                stream << std::get<double>(value.value);
            }
            break;  
        case json::JSON_NULL:
            stream << "null"; 
            break;  
        case json::JSON_OBJECT:
            outputObject(stream, *std::get<std::unique_ptr<json::object>>(value.value), indent); 
            break;  
        case json::JSON_ARRAY:
            outputArray(stream, *std::get<std::unique_ptr<json::array>>(value.value), indent); 
            break;  
    }
}
