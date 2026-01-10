#ifndef ARRAY_CPP
#include <memory>
#define ARRAY_CPP
#endif

#ifdef ARRAY_CPP
#include "parkinson.hpp"

bool json::array::getType(size_t index, json::types &type) {
    if(data.size() - 1 < index) return false;
    type = data[index].type;
    return true;
}

bool json::array::get(size_t index, std::string &out) {
    if(data.size() - 1 < index) return false; 
    if(data[index].type != JSON_STRING) return false;
    out = std::get<std::string>(data[index].value);
    return true;
}

bool json::array::get(size_t index, long long &out) {
    if(data.size() - 1 < index) return false;
    if(data[index].type != JSON_NUMBER) return false;
    if(long long *x = std::get_if<long long>(&data[index].value)) {
        out = *x;
        return true;
    }
    return false;
}

bool json::array::get(size_t index, double &out) {
    if(data.size() - 1 < index) return false;
    if(data[index].type != JSON_NUMBER) return false;
    if(double *x = std::get_if<double>(&data[index].value)) {
        out = *x;
        return true;
    }
    return false;   
}

bool json::array::get(size_t index, bool &out) {
    if(data.size() - 1 < index) return false;
    if(data[index].type != JSON_BOOL) return false;
    out = std::get<bool>(data[index].value);
    return true;
}

bool json::array::isNull(size_t index) {
   if(data.size() - 1 < index) return false;
   if(data[index].type == JSON_NULL) return true;
   return false;
}

bool json::array::get(size_t index, json::object *&out) {
    if(data.size() - 1 < index) return false;
    if(data[index].type != JSON_OBJECT) return false;
    out = std::get<std::unique_ptr<json::object>>(data[index].value).get();
    return true;
}

bool json::array::get(size_t index, json::array *&out) {
    if(data.size() - 1 < index) return false;
    if(data[index].type != JSON_ARRAY) return false;
    out = std::get<std::unique_ptr<json::array>>(data[index].value).get();
    return true;
}

void json::array::push(std::string value) {
    data.push_back(json::value{ value, JSON_STRING });
}

void json::array::push(const char* value) {
    data.push_back(json::value{ std::string(value), JSON_STRING });
}

void json::array::push(long long value) {
    data.push_back(json::value{ value, JSON_NUMBER });
}

void json::array::push(double value) {
    data.push_back(json::value{ value, JSON_NUMBER });
}

void json::array::push(bool value) {
    data.push_back(json::value{ value, JSON_BOOL });
}

void json::array::push(std::unique_ptr<json::object> value) {
    data.push_back(json::value{ std::move(value), JSON_OBJECT });
}

void json::array::push(std::unique_ptr<json::array> value) {
    data.push_back(json::value{ std::move(value), JSON_ARRAY });
}

json::object& json::array::pushObject() {
    auto obj = std::make_unique<json::object>();
    json::object &ref = *obj;
    push(std::move(obj));
    return ref;
}

json::array& json::array::pushArray() {
   auto arr = std::make_unique<json::array>();
   json::array& ref = *arr;
   push(std::move(arr));
   return ref;
}

void json::array::pushNull() {
    data.push_back(json::value{ std::monostate{}, JSON_NULL });
}

bool json::array::setValue(uint index, std::string value) {
    if(data.size() - 1 < index) return false;
    data[index] = json::value{ value, JSON_STRING };
    return true;
}

bool json::array::setValue(uint index, const char* value) {
    if(data.size() - 1 < index) return false;
    data[index] = json::value{ std::string(value), JSON_STRING };
    return true;
}

bool json::array::setValue(uint index, long long value) {
    if(data.size() - 1 < index) return false;
    data[index] = json::value{ value, JSON_NUMBER };
    return true;
}

bool json::array::setValue(uint index, double value) {
    if(data.size() - 1 < index) return false;
    data[index] = json::value{ value, JSON_NUMBER };
    return true;
}

bool json::array::setValue(uint index, bool value) {
    if(data.size() - 1 < index) return false;
    data[index] = json::value{ value, JSON_BOOL };
    return true;
}

bool json::array::setValue(uint index, std::unique_ptr<json::object> value) {
    if(data.size() - 1 < index) return false;
    data[index] = json::value{ std::move(value), JSON_OBJECT };
    return true;
}

bool json::array::setValue(uint index, std::unique_ptr<json::array> value) {
    if(data.size() - 1 < index) return false;
    data[index] = json::value{ std::move(value), JSON_ARRAY };
    return true;
}

bool json::array::setNull(uint index) {
    if(data.size() - 1 < index) return false;
    data[index] = json::value{ std::monostate{}, JSON_NULL };
    return true;
}

json::object* json::array::emplaceObject(uint index) {
    if(data.size() - 1 < index) return nullptr;
    data[index] = json::value{ std::make_unique<json::object>(), JSON_OBJECT };
    return std::get<std::unique_ptr<json::object>>(data[index].value).get();
}

json::array* json::array::emplaceArray(uint index) {
    if(data.size() - 1 < index) return nullptr;
    data[index] = json::value{ std::make_unique<json::array>(), JSON_OBJECT };
    return std::get<std::unique_ptr<json::array>>(data[index].value).get();
}

bool json::array::remove(uint index) {
   if(data.size() - 1 < index) return false;
   data.erase(data.begin() + index);
   return true;
}

std::unique_ptr<json::array> json::array::copy(json::array &original) {
    std::unique_ptr<json::array> array = std::make_unique<json::array>();
    for(auto& el: original.data) {
        json::value val;
        val.type = el.type;
        switch(el.type) {
            case JSON_STRING: {
                val.value = std::get<std::string>(el.value);
                break;  
            }
            case JSON_NUMBER: {
                long long *t;
                if((t = std::get_if<long long>(&el.value))) val.value = *t;
                else val.value = std::get<double>(el.value);
                break;  
            }
            case JSON_BOOL: {
                val.value = std::get<bool>(el.value);
                break;  
            }
            case JSON_NULL: { 
                val.value = std::monostate{};
                break;  
            }
            case JSON_OBJECT: {
                auto &origObj = *std::get<std::unique_ptr<json::object>>(el.value);
                val.value = json::object::copy(origObj);
                break;
            }
            case JSON_ARRAY: {
                auto &origArr = *std::get<std::unique_ptr<json::array>>(el.value);
                val.value = json::array::copy(origArr);
                break;
            }
        }
        array->data.push_back(std::move(val));
    }
    return array;
}

json::object* json::array::goToParentObject() {
    return parentObject;
}

json::array* json::array::goToParentArray() {
    return parentArray; 
}

int json::array::length() {
    return data.size();
} 

#endif
