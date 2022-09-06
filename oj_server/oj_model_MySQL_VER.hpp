#pragma once
// **************************************** MySQL version ****************************************
#include "../comm/log.hpp"
#include "../comm/util.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include "include/mysql.h"

// load every problem's infor into memory, by "questions.list"
// model: use to interact with data, provide APIs to access data

namespace ns_model
{
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        std::string number; // number for quesions, unique
        std::string title;
        std::string difficulty; // easy/medium/hard
        std::string desc;       // desc of question
        std::string head;     // frame-code provide for user
        std::string tail;       // test cases. need to put together with head
        int cpu_limit;          // time limit (s)
        int mem_limit;          // memory limit (KB)
    };

    const std::string oj_questions = "oj_questions";
    const std::string host = "127.0.0.1";
    const std::string user = "oj_client";
    const std::string passwd = "123456";
    const std::string db = "oj";
    const int port = 3306;

    class Model
    {
    public:
        Model() {}

        ~Model() {}

        bool QueryMySQL(const std::string &sql, std::vector<Question> *out)
        {
            //create mysql root
            MYSQL *my = mysql_init(nullptr);    

            // connect MySQL
            if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0))
            {
                LOG(FAtAL) << " connect to MySQL failed!!! " << std::endl;
                return false;
            }
            LOG(INFO) << " connect MySQL success " << std::endl;

            // set charset
            mysql_set_character_set(my,"utf8");

            // execute sql
            if(0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARING) << " " << sql << " execute error" << std::endl;
                return false;
            }

            // take result
            MYSQL_RES *res = mysql_store_result(my);

            // assess result
            int rows = mysql_num_rows(res); //get num of rows
            int cols = mysql_num_fields(res);   // get num of colomns

            for(int i=0; i<rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                Question q;
                q.number = row[0];
                q.title = row[1];
                q.difficulty = row[2];
                q.desc = row[3];
                q.head = row[4];
                q.tail = row[5];
                q.cpu_limit = atoi(row[6]);
                q.mem_limit = atoi(row[7]);

                out->push_back(q);
            }

            // free memory
            free(res);

            //close mysql
            mysql_close(my);    
        }

        bool GetAllQuestions(std::vector<Question> *out)
        {
            std::string sql = "select * from ";
            sql += oj_questions;
            return QueryMySQL(sql, out);
        }

        bool GetOneQuestion(const std::string &number, Question *q)
        {
            bool res = false;
            std::string sql = "select * from ";
            sql += oj_questions;
            sql += " where number=";
            sql += number;
            std::vector<Question> result;
            if(QueryMySQL(sql, &result))
            {
                if(result.size() == 1)
                {
                    *q = result[0]; //这里不是浅拷贝吗，出函数后q对象不就随着result销毁了？？？要给question类加个拷贝赋值重吧
                    res = true;
                }
            }
            return res;
        }
    };
}