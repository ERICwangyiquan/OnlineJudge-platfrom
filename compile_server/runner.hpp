#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "../comm/log.hpp"
#include "../comm/util.hpp"

namespace ns_runner
{
    using namespace ns_log;
    using namespace ns_util;

    class Runner
    {
    public:
        Runner()
        {}
        ~Runner()
        {}

    public:
        // limit the total resource this process can use
        static void SetProcLimit(int _cpu_limit, int _mem_limit)
        {
            // set cpu time
            struct rlimit cpu_rlimit;
            cpu_rlimit.rlim_max = RLIM_INFINITY;
            cpu_rlimit.rlim_cur = _cpu_limit;
            setrlimit(RLIMIT_CPU, &cpu_rlimit);
            
            // set memory size
            struct rlimit mem_rlimit;
            mem_rlimit.rlim_max = RLIM_INFINITY;
            mem_rlimit.rlim_cur = _mem_limit * 1024;    //change to KB
            setrlimit(RLIMIT_AS, &mem_rlimit);
        }

        // only need file_name
        /********************************
         * return val > 0 : runtime error, received signal, return val is the code of signal
         * ret val == 0 : finished normally, result stored in file
         * ret val < 0 : other errors like create files failed
         * 
         * cpu_limit : the max execution time this process can use
         * mem_limit : the max memory size it can use (KB)
         *******************************/
        static int Run(const std::string &file_name, int cpu_limit, int mem_limit)
        {
            std::string _execute  = PathUtil::Exe(file_name);
            std::string _stdin   = PathUtil::Stdin(file_name);
            std::string _stdout  = PathUtil::Stdout(file_name);
            std::string _stderr  = PathUtil::Stderr(file_name);

            umask(0);
            int _stdin_fd = open(_stdin.c_str(), O_CREAT | O_RDONLY, 0644);
            int _stdout_fd = open(_stdout.c_str(), O_CREAT | O_WRONLY, 0644);
            int _stderr_fd = open(_stderr.c_str(), O_CREAT | O_WRONLY, 0644);\

            if(_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                LOG(ERROR) << "runtime open std files failed" << std::endl;
                return -1;  //open files failed
            }

            pid_t pid = fork();
            if(pid < 0)
            {
                LOG(ERROR) << "runtime create child process failed" << std::endl;
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                return -2;  //create child process failed
            }
            else if(pid == 0)
            {
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd, 1);
                dup2(_stderr_fd, 2);

                SetProcLimit(cpu_limit, mem_limit);
                
                execl(_execute.c_str(), _execute.c_str(), nullptr);
                exit(1);
            }
            else
            {
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);

                int status = 0;
                waitpid(pid, &status, 0);
                // if there's runtime error, must received signal in status
                LOG(INFO) << "execute done, info: " << (status & 0x7F) << std::endl;
                return status & 0x7F;
            }
        }
    };
} 