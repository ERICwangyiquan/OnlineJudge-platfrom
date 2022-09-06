#include "compile_run.hpp"
#include "../comm/httplib.h"

using namespace ns_compile_and_run;
using namespace httplib;

void Usage(std::string proc)
{
    std::cerr << "Usage: " << "\n\t" << proc << " port" << std::endl;
}

//./compile_server port
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        return 1;
    }
    Server svr;

    // svr.Get("/hello", [](const Request &req, Response &resp){
    //     resp.set_content("hello httplib!", "text/plain; charset=utf-8");
    // });

    svr.Post("/compile_and_run", [](const Request &req, Response &resp){
        // post's body contains json string we want
        std::string in_json = req.body;
        std::string out_json;
        if(!in_json.empty())
        {
            CompileAndRun::Start(in_json, &out_json);
            resp.set_content(out_json, "application/json; charset=utf-8");
        }
    });
    
    svr.listen("0.0.0.0", atoi(argv[1])); // start http
    // // in_json: {"code": "#include...", "input": "", "cpu_limit": "1", "mem_limit": "10240(KB)"}
    // // out_json: {"status": "0", "reason": "", "stdout": "", "stderr": ""}
    // //get json string from client through http
    // std::string in_json;

    // Json::Value in_value;
    // in_value["code"]= "";
    // in_value["input"] = "";
    // in_value["cpu_limit"] = 1;
    // in_value["mem_limit"] = 10240 * 3;

    // Json::FastWriter writer;
    // in_json = writer.write(in_value);
    // // CompileAndRun::Start();
    return 0;
}