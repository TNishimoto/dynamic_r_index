#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "../../include/dynamic_r_index.hpp"
#include "stool/include/stool.hpp"
namespace stool
{
    namespace r_index_test
    {
        using namespace r_index;
        class WindowTest
        {
            static void widnow_test(uint64_t text_size, bool is_back_remove)
            {
                std::vector<std::string> max_strs;
                uint64_t max_edit_count = 0;

                std::vector<uint8_t> chars;
                chars.push_back('a');
                chars.push_back('b');

                std::cout << "Create Strings" << std::endl;
                std::vector<std::string> strs = stool::StringFunctions::get_all_strings(text_size, chars);

                uint64_t counter = 0;
                for (auto str : strs)
                {
                    // std::cout << str << std::endl;
                    if (counter % 50000 == 0)
                    {
                        std::cout << counter << "/" << strs.size() << "/" << max_edit_count << std::endl;
                    }
                    counter++;

                    std::vector<uint8_t> text;
                    for (auto c : str)
                    {
                        text.push_back(c);
                    }
                    text.push_back('$');
                    std::vector<uint64_t> sa = stool::construct_suffix_array(text);
                    std::vector<uint8_t> bwt = stool::construct_BWT(text, sa);

                    DynamicRIndex drfmi;
                    drfmi.initialize(chars, 8, '$');
                    drfmi.build(bwt);

                    uint64_t edit_count = 0;
                    while (drfmi.size() > 1)
                    {
                        if (is_back_remove)
                        {
                            edit_count += drfmi.delete_char(drfmi.size() - 2);
                        }
                        else
                        {
                            edit_count += drfmi.delete_char(0);
                        }
                    }

                    if (edit_count > max_edit_count)
                    {
                        max_edit_count = edit_count;
                        max_strs.clear();
                        max_strs.push_back(str);
                    }
                    else if (edit_count == max_edit_count)
                    {
                        max_strs.push_back(str);
                    }
                }

                // std::cout << "Finished." << std::endl;
                std::cout << "Len: " << text_size << ", MaxEditCount: " << max_edit_count << std::endl;
                std::cout << "[";
                for (auto s : max_strs)
                {
                    std::cout << s << ", ";
                }
                std::cout << "]" << std::endl;
            }

            static void widnow_test2(std::vector<uint8_t> &text, std::vector<uint8_t> &chars)
            {
                std::vector<uint64_t> sa = stool::construct_suffix_array(text);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa);

                DynamicRIndex drfmi;
                drfmi.initialize(chars, 8, '$');
                drfmi.build(bwt);

                std::string s = drfmi.get_text_str();

                uint64_t edit_count = 0;
                while (drfmi.size() > 1)
                {
                    edit_count += drfmi.delete_char(drfmi.size() - 2);
                }

                double p = edit_count / text.size();

                std::cout << "Len: " << text.size() << ", Text: " << s << ", EditCount: " << edit_count << ", per = " << p << std::endl;
            }
            static void widnow_test3(uint64_t text_size)
            {
                std::string s = generate_worst_string(text_size);
                std::vector<uint8_t> text;
                for (auto c : s)
                {
                    text.push_back(c);
                }
                text.push_back('$');

                std::vector<uint8_t> chars;
                chars.push_back('a');
                chars.push_back('b');

                widnow_test2(text, chars);
            }
        };

    }
}
