#pragma once

#include "compiler.hpp"
#include "runner.hpp"
#include "../comm/log.hpp"
#include "../comm/util.hpp"

#include <json/json.h>
#include <signal.h>
#include <unistd.h>

namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;

    class CompileAndRun
    {
    public:
        // code > 0: process recerived a signal and stop
        // code < 0: non-runtime error (code is empty or this program's fault)
        // code = 0: all done
        static std::string CodeToDesc(int code, const std::string &file_name)
        {
            std::string desc;
            switch (code)
            {
            case 0:
                desc = "compile and run success";
                break;
            case -1:
                desc = "code submitted is empty";
                break;
            case -2:
                desc = "unknown error";
                break;
            case -3:
                // desc = "compile failed";
                FileUtil::ReadFile(PathUtil::CompileError(file_name), &desc, true);
                break;
            case SIGABRT: // signal 6
                desc = "memory used is over the limit";
                break;
            case SIGXCPU: // signal 24
                desc = "Time Limit Exceeded";
                break;
            case SIGFPE: // signal 8
                desc = "Floating point exception";
                break;
            default:
                desc = "unknown: " + std::to_string(code);
                break;
            }
            return desc;
        }

        static void RemoveTempFiles(const std::string &file_name)
        {
            std::string _src = PathUtil::Src(file_name);
            if (FileUtil::IsFileExists(_src))
                unlink(_src.c_str());

            std::string _compile_err = PathUtil::CompileError(file_name);
            if(FileUtil::IsFileExists(_compile_err))
                unlink(_compile_err.c_str());

            std::string _execute = PathUtil::Exe(file_name);
            if(FileUtil::IsFileExists(_execute))
                unlink(_execute.c_str());

            std::string _stdin = PathUtil::Stdin(file_name);
            if(FileUtil::IsFileExists(_stdin))
                unlink(_stdin.c_str());

            std::string _stdout = PathUtil::Stdout(file_name);
            if(FileUtil::IsFileExists(_stdout))
                unlink(_stdout.c_str());

            std::string _stderr = PathUtil::Stderr(file_name);
            if(FileUtil::IsFileExists(_stderr))
                unlink(_stderr.c_str());
        }

        /*********************************************************************
         * INPUT:
         * code: code submitted from user
         * input：user's input for his code, (not handled in this program)
         * cpu_limit: run time limit
         * mem_limit: max memory use
         *
         * OUTPUT:
         * MUST
         * status: condition code
         * reason: request result
         * CHOOSE
         * stdout: the result after execute code
         * stderr: error result if an error happened
         *
         * PARAMETER:
         * in_json: {"code": "#include...", "input": "", "cpu_limit": "1", "mem_limit": "10240(KB)"}
         * out_json: {"status": "0", "reason": "", "stdout": "", "stderr": ""}
         * ********************************************************************/
        static void Start(const std::string &in_json, std::string *out_json)
        {
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json, in_value);

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            int status_code = 0;
            Json::Value out_value;
            int run_result = 0;    // can't initialize between goto sentences
            std::string file_name; // can't initialize between goto sentences

            if (code.size() == 0)
            {
                status_code = -1; // user's code is empty
                goto END;
            }

            file_name = FileUtil::UniqFileName();

            if (!FileUtil::WriteFile(PathUtil::Src(file_name), code)) // create src file
            {
                status_code = -2; // unknown error
                goto END;
            }

            if (!Compiler::Compile(file_name))
            {
                // compile failed
                status_code = -3;
                goto END;
            }

            run_result = Runner::Run(file_name, cpu_limit, mem_limit);
            if (run_result < 0)
            {
                // unknown error
                status_code = -2;
            }
            else if (run_result > 0)
            {
                // runtime error, (received signal)
                status_code = run_result;
            }
            else
            {
                // run success, dont care if the result is right
                status_code = 0;
            }

        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeToDesc(status_code, file_name);
            if (status_code == 0)
            {
                // here dont know/care if the result is right
                std::string _stdout;
                FileUtil::ReadFile(PathUtil::Stdout(file_name), &_stdout, true);
                out_value["stdout"] = _stdout;

                std::string _stderr;
                FileUtil::ReadFile(PathUtil::Stderr(file_name), &_stderr, true);
                out_value["stderr"] = _stderr;
            }

            Json::FastWriter writer;
            *out_json = writer.write(out_value);

            RemoveTempFiles(file_name);
        }
    };
}