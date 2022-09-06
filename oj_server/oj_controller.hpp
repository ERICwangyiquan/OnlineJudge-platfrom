#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>
#include <cassert>
#include <json/json.h>


#include "../comm/httplib.h"
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "oj_model_MySQL_VER.hpp"   //MySQL version
// #include "oj_model_File_VER.hpp"    //File version
#include "oj_view.hpp"

namespace ns_controller
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;
    using namespace httplib;

    // service machines
    class Machine
    {
    public:
        std::string ip;  // service ip
        int port;        // service port
        uint64_t load;   // service load condition
        std::mutex *mtx; // mutex itself cant be copied, use ptr

    public:
        Machine() : ip(""), port(0), load(0), mtx(nullptr)
        {
        }
        ~Machine()
        {
        }

    public:
        void IncreaseLoad()
        {
            if(mtx) mtx->lock();
            load++;
            if(mtx) mtx->unlock();            
        }

        void DecreaseLoad()
        {
            if(mtx) mtx->lock();
            load--;
            if(mtx) mtx->unlock();    
        }

        void ResetLoad()
        {
            if(mtx) mtx->lock();
            load = 0;
            if(mtx) mtx->unlock();
        }

        // get load, useless but for united APIs
        uint64_t Load()
        {
            uint64_t _load = 0;
            if(mtx) mtx->lock();
            _load = load;
            if(mtx) mtx->unlock();

            return load;
        }
    };

    const std::string service_machine = "./conf/service_machine.conf";

    // load balancing
    class LoadBalance
    {
    private:
        // all machines can be used
        //  every machine has its index in vector for machine id
        std::vector<Machine> machines;
        // online machines
        std::vector<int> online;
        // offline machines;
        std::vector<int> offline;
        // assure LoadBalance's data safe
        std::mutex mtx;

    public:
        LoadBalance()
        {
            assert(LoadConf(service_machine));
            LOG(INFO) << " load " << service_machine << " success " << std::endl;
        }
        
        ~LoadBalance()
        {
        }

    public:
        bool LoadConf(const std::string &machine_conf)
        {
            std::ifstream in(machine_conf);
            if(!in.is_open())
            {
                LOG(FATAL) << " load "<< machine_conf << " failed " << std::endl;
                return false;
            }

            std::string line;
            while(std::getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, ":");
                if(tokens.size() != 2)
                {
                    LOG(WARNING) << " Split" << line << " failed " << std::endl;
                    continue;
                }
                Machine m;
                m.ip = tokens[0];
                m.port = atoi(tokens[1].c_str());
                m.load = 0;
                m.mtx = new std::mutex();

                online.push_back(machines.size());
                machines.push_back(m);
            }

            in.close();
            return true;
        }

        // id : output parameter
        // m : output parameter
        bool AutoChoose(int *id, Machine **m)
        {
            // 1. use chosen machine (update machine_name.load)
            // 2. may offline this machine
            mtx.lock();
            // algorithm for load balancing
            // polling + hash
            int online_num = online.size();
            if(online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << " there's no online machine now, repair immediately!! " << std::endl;
                return false;
            }
            // find the machine with least load by polling
            *id = online[0];
            *m = &machines[online[0]];
            uint64_t min_load = machines[online[0]].Load();
            for(int i = 1; i < online_num; i++)
            {
                uint64_t cur_load = machines[online[i]].Load();
                if(min_load > cur_load)
                {
                    min_load = cur_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }
            mtx.unlock();
            return true;
        }

        void OffineMachine(int which)
        {
            mtx.lock();
            for(auto iter = online.begin(); iter != online.end(); iter++)
            {
                if(*iter == which)
                {
                    // found it, now offline it
                    machines[which].ResetLoad();

                    online.erase(iter);
                    offline.push_back(which);
                    break;  //because of break, no need to worry invalid iterator
                }
            }
            mtx.unlock();
        }

        void OnlineMachine()
        {
            // when all the machines r offline, then put every machines online again 
            mtx.lock();
            online = offline;
            // online.insert(online.end(), offline.begin(), offline.end());
            offline.erase(offline.begin(), offline.end());
            mtx.unlock();

            LOG(INFO) << " all machines are back to online now! " << std::endl;
        }

        // for test
        void ShowMachines()
        {
            mtx.lock();
            std::cout << "current online machines list: ";
            for(auto &id : online)
            {
                std::cout << id << ' ';
            }
            std::cout << std::endl;
            
            std::cout << "current offline machines list: ";
            for(auto &id : offline)
            {
                std::cout << id << ' ';
            }
            std::cout << std::endl;
            mtx.unlock();
        }
    };

    class Controller
    {
    private:
        Model model_;
        View view_; // provide .html
        LoadBalance load_balance_;  //load_balancer

    public:
        Controller() {}

        ~Controller() {}

    public:
        void BackOnline()
        {
            load_balance_.OnlineMachine();
        }

        // use questions data to build web page
        bool AllQuestions(std::string *html)
        {
            bool ret = true;
            std::vector<ns_model::Question> all;
            if (model_.GetAllQuestions(&all))
            {
                std::sort(all.begin(), all.end(), [](const ns_model::Question &q1, const ns_model::Question &q2){
                    return atoi(q1.number.c_str()) < atoi(q2.number.c_str());
                });
                // get list success, build a .html
                view_.AllExpandHtml(all, html);
            }
            else
            {
                *html = "failed to get questions list";
                ret = false;
            }
            return ret;
        }

        bool Question(const std::string &num, std::string *html)
        {
            bool ret = true;
            ns_model::Question q;
            if (model_.GetOneQuestion(num, &q))
            {
                // get one question success, build .html
                view_.OneExpandHtml(q, html);
            }
            else
            {
                *html = "question: " + num + "doesn't exists ";
                ret = false;
            }
            return ret;
        }

        // code: #include...
        // input: ""
        void Judge(const std::string &number, const std::string &in_json, std::string *out_json)
        {
            // LOG(DEBUG) << in_json << "\nnumber: " << number << std::endl;
            
            // 0. get the desc of question by number
            ns_model::Question q;
            model_.GetOneQuestion(number, &q);

            // 1. deserialize in_json, get id, code, input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();

            // 2. catenate user's code with tail.cpp
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + "\n" + q.tail;  //***
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;
            Json::FastWriter writer;
            std::string compile_string = writer.write(compile_value);

            // 3. choose machine has the lowest load balancing（handle errors)
            // rule: keep choosing until find a usable machine, or all unusable
            while(true)
            {
                int id = 0;
                Machine *m = nullptr;
                if(!load_balance_.AutoChoose(&id, &m))
                {
                    break;
                }

                // 4. send HTTP request, get result
                Client cli(m->ip, m->port);
                m->IncreaseLoad();
                LOG(INFO) << " choose machine success, machine id: " << id << " details: " << m->ip << ":" << m->port << " Current Load is: " << m->Load() << std::endl;
                if(auto res = cli.Post("/compile_and_run", compile_string, "application/json; charset=utf-8"))
                {
                    // 5. return result to out_json
                    if(res->status == 200)
                    {
                        *out_json = res->body; 
                        m->DecreaseLoad();
                        LOG(INFO) << " request compile and run service success" << std::endl;
                        break;
                    }
                    m->DecreaseLoad();
                }
                else
                {
                    // request failed
                    LOG(ERROR) << " requesting machine, id: " << id << " details: " << m->ip << ":" << m->port << " is not working"<< std::endl;
                    load_balance_.OffineMachine(id);
                    load_balance_.ShowMachines();   //for debug
                }
            }
        }
    };
}