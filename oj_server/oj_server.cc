#include <iostream>
#include <signal.h>
#include "../comm/httplib.h"
#include "oj_controller.hpp"

using namespace httplib;
using namespace ns_controller;

static Controller *ctrl_ptr = nullptr;

void Recovery(int signo)
{
    ctrl_ptr->BackOnline();
}

int main()
{
    signal(SIGQUIT, Recovery);

    // service requested by client
    Server svr;
    Controller ctrl;
    ctrl_ptr = &ctrl;

    // get problems list
    svr.Get("/all_questions", [&ctrl](const Request &req, Response &resp){
        // return a .html includes all questions
        std::string html;
        ctrl.AllQuestions(&html);

        resp.set_content(html, "text/html; charset=utf-8"); 
        });

    // user can get the content of prolem by it's problem-number
    //  /question/100 -> regular expression
    svr.Get(R"(/questions/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string num = req.matches[1];
        std::string html;
        ctrl.Question(num, &html);
        
        resp.set_content(html, "text/html; charset=utf-8"); 
        });

    // user submit code, use judge function(1. test cases for each problems, 2. compile_server)
    svr.Post(R"(/judge/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string num = req.matches[1];
        std::string result_json;
        ctrl.Judge(num, req.body, &result_json);
        resp.set_content(result_json, "application/json; charset=utf-8");
        // resp.set_content("the judge a specific problem: " + num, "text/plain; charset=utf-8"); 
        });

    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8079);
    return 0;
}