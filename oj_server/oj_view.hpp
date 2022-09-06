#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <ctemplate/template.h>

// #include "oj_model_File_VER.hpp"    //File version
#include "oj_model_MySQL_VER.hpp"

namespace ns_view
{
    using namespace ns_model;

    const std::string template_path = "./template_html/";

    class View
    {
    public:
        View() {}
        ~View() {}

    public:
        void AllExpandHtml(const std::vector<Question> &questions, std::string *html)
        {
            // num + title + difficulty
            // use table to show
            // 1.create path
            std::string src_html = template_path + "all_questions.html";

            // 2.create template_dict
            ctemplate::TemplateDictionary root("all_questions");
            for(const auto &q: questions)
            {
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("questions_list");
                sub->SetValue("number", q.number);
                sub->SetValue("title", q.title);
                sub->SetValue("star", q.difficulty);
            }

            // 3.get .html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4.get result .html
            tpl->Expand(html, &root);
        }

        void OneExpandHtml(const Question &q, std::string *html)
        {
            // 1.create path
            std::string src_html = template_path + "one_question.html";
            
            // 2.create template_dict
            ctemplate::TemplateDictionary root("one_question");
            root.SetValue("number", q.number);
            root.SetValue("title", q.title);
            root.SetValue("star", q.difficulty);
            root.SetValue("desc", q.desc);
            root.SetValue("pre_code", q.head);

            // 3.get .html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4.get result .html
            tpl->Expand(html, &root);
        }
    };
}