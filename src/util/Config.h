#ifndef __medleap_Config__
#define __medleap_Config__

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

/** Stores/loads configuration values in a file. */
namespace gl{
        
class Config
{
public:
    
    virtual ~Config();
    

    
    template <typename T>
    void putValue(const std::string& name, const T& value)
    {
        std::stringstream ss;
        ss << value;
        values[name] = ss.str();
    }
    
    template <typename T>
    T getValue(const std::string& name, const T& default_value)
    {
        T result = default_value;
        std::unordered_map<std::string, std::string>::iterator it = values.find(name);
        if (it != values.end()) {
            std::stringstream ss(it->second);
            ss >> result;
        }
        return result;
    }
    
    void clear();
    bool load(const std::string& fileName);
    void save(const std::string& fileName);
    
    
private:
    std::unordered_map<std::string, std::string> values;
};
}

#endif // __medleap_Config__