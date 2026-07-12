#pragma noce

#include <string>

/*
Logger 简单日志工具类

*/

class Logger
{
    public:
        static void debug(const std::string& module, const std::string& message);
        static void info(const std::string& module, const std::string& message);
        static void warn(const std::string& module, const std::string& message);
        static void error(const std::string& module, const std::string& message);
    
    private:
        static void log(
            const std::string& level,
            const std::string& module,
            const std::string& message);
        
        static std::string currentTime();

};
