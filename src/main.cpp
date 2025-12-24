#include "./parser.hpp"
#include <iostream>

int main(int argc, char **argv) {
    if(argc < 2) {
        std::cout << "Usage: parkinson <filename.json>" << std::endl;
        return 1;
    }
    const char* name = argv[1];

    JsonObject object;
    
    ParserExitCode code;
    int retCode = parseJson(name, object, code);

    if (!retCode) {
        std::cout << "JSON_PARSE_ERROR_CODE: " << code.message << " on " << code.lineNumber << ":" << code.characterNumber << "\nExit code: " << code.returnCode << std::endl;
        object.clear();
    } else if (retCode) {
        std::cout << code.message << ": " << code.returnCode << std::endl;
    }

    if(object.data.size() > 0) {
        std::cout << "Data is correct\n";
    } else {
        std::cout << "Error is occured during the parsing\n";
    }

    for(auto &x : object.data) {
        std::cout << x.first << ": ";
        int type = x.second.type;
        auto& valueVariant = x.second.value;
        switch (type) {
            case JSON_NUMBER: {
                const int* pIntVal;
                if((pIntVal = std::get_if<int>(&valueVariant))) {
                    std::cout << *pIntVal << std::endl;
                } else {
                    const double pDoubleVal = std::get<double>(valueVariant);
                    std::cout << pDoubleVal << std::endl;
                }
                break;
            }
            case JSON_STRING: {
                std::cout << std::get<std::string>(valueVariant) << std::endl;
                break;
            }
            case JSON_BOOL: {
                std::cout << std::get<bool>(valueVariant) << std::endl;
                break;
            }
            case JSON_OBJECT: {
                if((*(std::get<std::unique_ptr<JsonObject>>(valueVariant).get())).data.size() > 0) {
                    std::cout << "OBJECT\n";
                    break;
                } else {
                    std::cout << "Error/Empty object\n";
                }
            }
        }
    } 

    return 0;
}
