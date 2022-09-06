#pragma once
// **************************************** File version ****************************************
#include "../comm/log.hpp"
#include "../comm/util.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cassert>

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
        int cpu_limit;          // time limit (s)
        int mem_limit;          // memory limit (KB)
        std::string desc;       // desc of question
        std::string head;     // frame-code provide for user
        std::string tail;       // test cases. need to put together with head
    };

    const std::string question_list = "./questions/questions.list";
    const std::string question_path = "./questions/";

    class Model
    {
    private:
        // K:V = num:details
        std::unordered_map<std::string, Question> questions;

    public:
        Model()
        {
            assert(LoadQuestionList(question_list));
        }

        ~Model() {}

        bool LoadQuestionList(const std::string &question_list)
        {
            // load file question/question.list + num of qustion file
            std::ifstream in(question_list);
            if (!in.is_open())
            {
                LOG(FATAL) << " load questions list failed, check if the list exists" << std::endl;
                return false;
            }

            std::string line;
            while (getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, " ");
                if (tokens.size() != 5)
                {
                    LOG(WARNING) << " load one question failed, check file's format" << std::endl;
                    continue;
                }
                Question q;
                q.number = tokens[0];
                q.title = tokens[1];
                q.difficulty = tokens[2];
                q.cpu_limit = atoi(tokens[3].c_str());
                q.mem_limit = atoi(tokens[4].c_str());

                std::string path = question_path;
                path += q.number;
                path += "/";

                FileUtil::ReadFile(path + "desc.txt", &(q.desc), true);
                FileUtil::ReadFile(path + "head.cpp", &(q.head), true);
                FileUtil::ReadFile(path + "tail.cpp", &(q.tail), true);

                questions.insert({q.number, q});
            }
            LOG(INFO) << " load qustions success..." << std::endl;

            in.close();

            return true;
        }

        bool GetAllQuestions(std::vector<Question> *out)
        {
            if (questions.empty())
            {
                LOG(ERROR) << " user failed to get questions list" << std::endl;
                return false;
            }
            for (const auto &q : questions)
            {
                out->push_back(q.second);
            }
            return true;
        }

        bool GetOneQuestion(const std::string &number, Question *q)
        {
            const auto &iter = questions.find(number);
            if (iter == questions.end())
            {
                LOG(ERROR) << " user failed to get question, number of question: " << number << std::endl;
                return false;
            }
            (*q) = iter->second;
            return true;
        }
    };
}