#include <string>
#include <sstream>
namespace Hazel{
namespace Utils{
template<typename T1, typename T2, typename T3>
struct triplet{
    T1 first;
    T2 second;
    T3 third;
    triplet(T1 first, T2 second, T3 third): first(first), second(second), third(third){}
};
template<typename T>
T to_numeric(const std::string& str, bool allow_hex = false, bool allow_binary = false){
    //The decoding for when allow_hex/allow_binary is true is done based on if T is float or int. If T is float then we treat the hex/bin as float of that size. Else we treat it as int of that size
    //TODO:Write it properly to handle hex and stuff in future. And also give error if we get float in integer and all that
    std::istringstream iss(str);
    T num;
    iss >> num;
    if(iss.fail()){
        throw std::invalid_argument("Invalid numeric string: " + str);
    }
    return num;
}
}
}