#ifndef OBJECT_CPP
#define OBJECT_CPP
#include <memory>
#endif

#ifdef OBJECT_CPP
#include "parkinson.hpp"
// JSON OBJECT get functions

bool json::object::exists(const std::string key) {
    auto it = data.find(key);
    if(it == data.end()) return false;
    return true;
}

bool json::object::getType(const std::string key, json::types &out) {
    auto it = data.find(key);
    if(it == data.end()) return false;
    out = it->second.type;
    return true;
}

bool json::object::get(const std::string key, std::string &out) {
    auto it = data.find(key);
    if(it == data.end()) return false;
    if(it->second.type != JSON_STRING) return false;
    out = std::get<std::string>(it->second.value);
    return true;
}

bool json::object::get(const std::string key, long long &out) {
    auto it = data.find(key);
    if(it == data.end()) return false;
    if(it->second.type != JSON_NUMBER) return false;
    if(long long* x = std::get_if<long long>(&it->second.value)) {
        out = *x;
        return true;
    }
    return false;
} 
bool json::object::get(const std::string key, double& out) {
    auto it = data.find(key);
    if(it == data.end()) return false;
    if(it->second.type != JSON_NUMBER) return false;
    if(double* x = std::get_if<double>(&it->second.value)) {
        out = *x;
        return true;
    }
    return false;   
} 
bool json::object::get(const std::string key, bool &out) {
   auto it = data.find(key);
   if(it == data.end()) return false;
   if(it->second.type != JSON_BOOL) return false;
   out = std::get<bool>(it->second.value);
   return true;
} 
bool json::object::isNull(const std::string key) {
    auto it = data.find(key);
    if(it == data.end()) return false;
    if(it->second.type == JSON_NULL) return true;
    return false;
} 
bool json::object::get(const std::string key, json::object *&out) {
    auto it = data.find(key);
    if(it == data.end()) return false;
    if(it->second.type != JSON_OBJECT) return false;
    out = std::get<std::unique_ptr<json::object>>(it->second.value).get();
    return true;
}

bool json::object::get(const std::string key, json::array *&out) {    
    auto it = data.find(key);
    if(it == data.end()) return false;
    if(it->second.type != JSON_ARRAY) return false;
    out = std::get<std::unique_ptr<json::array>>(it->second.value).get();
    return true;
}

void json::object::setValue(const std::string key, const char* value) {
    data[key] = json::value{ std::string(value), JSON_STRING };
}

void json::object::setValue(const std::string key, std::string value) {
    data[key] = json::value{ std::string(value), JSON_STRING };
}

void json::object::setValue(const std::string key, long long value) {
    data[key] = json::value{ value, JSON_NUMBER };
}

void json::object::setValue(const std::string key, double value) {
    data[key] = json::value{ value, JSON_NUMBER };
}

void json::object::setValue(const std::string key, bool value) {
    data[key] = json::value{ value, JSON_BOOL };
}

void json::object::setValue(const std::string key, std::unique_ptr<json::object> value) {
    data[key] = json::value{ std::move(value), JSON_OBJECT };
}

void json::object::setValue(const std::string key, std::unique_ptr<json::array> value) {
    data[key] = json::value{ std::move(value), JSON_ARRAY };
}

json::object& json::object::emplaceObject(const std::string key) {
    auto obj = std::make_unique<json::object>();
    json::object &ref = *obj;
    setValue(key, std::move(obj));
    return ref;
}

json::array& json::object::emplaceArray(const std::string key) {
    auto arr = std::make_unique<json::array>();
    json::array &ref = *arr;
    setValue(key, std::move(arr));
    return ref;
}

void json::object::setNull(const std::string key) {
    data[key] = json::value{ std::monostate{}, JSON_NULL }; 
}

std::unique_ptr<json::object> json::object::copy(json::object &original) {
    std::unique_ptr<json::object> object = std::make_unique<json::object>();
    for(auto& [key, val]: original.data) {
        json::value copyVal;
        copyVal.type = val.type;

        switch(val.type) {
            case JSON_STRING: {
                copyVal.value = std::get<std::string>(val.value);
                break;  
            }
            case JSON_NUMBER: {
                long long *t;
                if((t = std::get_if<long long>(&val.value))) copyVal.value = *t;
                else copyVal.value = std::get<double>(val.value);
                break;  
            }
            case JSON_BOOL: {
                copyVal.value = std::get<bool>(val.value);
                break;  
            }
            case JSON_NULL: { 
                copyVal.value = std::monostate{};
                break;  
            }
            case JSON_OBJECT: {
                auto &origObj = *std::get<std::unique_ptr<json::object>>(val.value);
                copyVal.value = json::object::copy(origObj);
                break;
            }
            case JSON_ARRAY: {
                auto &origArr = *std::get<std::unique_ptr<json::array>>(val.value);
                copyVal.value = json::array::copy(origArr);
                break;
            }
        } 
        object->data.emplace(key, std::move(copyVal));
    }
    return object;
}

json::object* json::object::goToParentObject() {
    return parent;
}

json::array* json::object::goToParentArray() {
    return parentArray; 
}

bool json::object::remove(const std::string key) {
    if(data.erase(key) > 0) return true;
    return false;
}

#endif
