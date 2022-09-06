#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../comm/util.hpp"
#include "../comm/log.hpp"

// only for the compile of code

namespace ns_compiler
{
    // get functions from hutil.hpp
    using namespace ns_util;
    using namespace ns_log;

    class Compiler
    {
    public:
        Compiler()
        {}
        ~Compiler()
        {}

        // return value: true for compile success, or false
        // parameter: name of file waits to be compiled
        // file_name: 1234
        // 1234 ->  ./temp/1234.cpp  
        // 1234 ->  ./temp/1234.exe
        // 1234 ->  ./temp/1234.stderr
        static bool Compile(const std::string &file_name)
        {
            pid_t pid = fork();
            if(pid < 0)
            {
                LOG(ERROR) << "inner error, failed to create child process" << std::endl;
                return false;
            }
            else if(pid == 0)
            {
                umask(0);
                int _stderr = open(PathUtil::CompileError(file_name).c_str(), O_CREAT | O_WRONLY, 0644);
                if(_stderr < 0)
                {
                    LOG(WARNING) << "failed to create stderr file" << std::endl;
                    exit(1);
                }
                // re-direct: stderr -> _stderr
                dup2(_stderr, 2);

                // child process: execute g++, compile
                // g++ -o target src -std=c++11
                execlp("g++", "g++", "-o", PathUtil::Exe(file_name).c_str(), PathUtil::Src(file_name).c_str(), "-std=c++11", "-D", "COMPILER_ONLINE", nullptr);
                LOG(ERROR) << "failed to execute g++" << std::endl;
                exit(2);
            }
            else
            {
                waitpid(pid, nullptr, 0);
                // if compile succeed -> if file_name.exe exists
                if(FileUtil::IsFileExists(PathUtil::Exe(file_name)))
                {
                    LOG(INFO) << PathUtil::Src(file_name) << " compile succeed" << std::endl;
                    return true;
                }
            }
            LOG(ERROR) << "compile failed, no .exe file was created" << std::endl;
            return false;
        }
    };
}