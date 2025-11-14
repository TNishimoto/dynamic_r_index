#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/lib.hpp"
#include "../include/all.hpp"
#include "libdivsufsort/sa.hpp"

void create_all_permutations(uint64_t len, const std::vector<uint8_t> &alphabet, const std::string &prefix, std::vector<std::string> &output)
{
    if (prefix.size() > len)
    {
        for (uint8_t c : alphabet)
        {
            std::string new_prefix = prefix + (std::string(1, c));
            create_all_permutations(len, alphabet, new_prefix, output);
        }
    }
    else
    {
        output.push_back(prefix);
    }
}
std::vector<std::string> create_all_permutations(uint64_t len, const std::vector<uint8_t> &alphabet)
{
    std::string prefix = "";
    std::vector<std::string> output;
    create_all_permutations(len, alphabet, prefix, output);
    return output;
}


uint64_t get_rle_size(const std::vector<uint8_t> &text)
{
    uint64_t size = 0;
    for (uint64_t i = 0; i < text.size(); i++)
    {
        if (text[i] != text[i + 1])
        {
            size++;
        }
    }
    return size + 1;
}

std::vector<uint64_t> construct_reversed_perfect_PLCP_array(const std::string &text_str)
{
    std::vector<uint8_t> text;
    text.resize(text_str.size());
    for (uint64_t i = 0; i < text_str.size(); i++)
    {
        text[i] = text_str[text_str.size() - 1 - i];
    }
    std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
    std::vector<uint64_t> lcp = stool::ArrayConstructor::construct_LCP_array(text, sa, stool::Message::NO_MESSAGE);
    std::vector<uint64_t> plcp;
    plcp.resize(text.size());
    for (uint64_t i = 0; i < text.size(); i++)
    {
        uint64_t x = text.size() - 1 - sa[i];
        if (i + 1 < text.size())
        {
            plcp[x] = lcp[i] < lcp[i + 1] ? lcp[i + 1] : lcp[i];
        }
        else
        {
            plcp[x] = lcp[i];
        }
    }
    return plcp;
}

std::vector<uint64_t> construct_perfect_PLCP_array(const std::vector<uint8_t> &text, const std::vector<uint64_t> &sa, const std::vector<uint64_t> &lcp)
{
    std::vector<uint64_t> plcp;
    plcp.resize(text.size());
    for (uint64_t i = 0; i < text.size(); i++)
    {
        if (i + 1 < text.size())
        {
            plcp[sa[i]] = lcp[i] < lcp[i + 1] ? lcp[i + 1] : lcp[i];
        }
        else
        {
            plcp[sa[i]] = lcp[i];
        }
    }
    return plcp;
}

std::vector<uint64_t> construct_PLCP_array(const std::vector<uint8_t> &text, const std::vector<uint64_t> &sa, const std::vector<uint64_t> &lcp)
{
    std::vector<uint64_t> plcp;
    plcp.resize(text.size());
    for (uint64_t i = 0; i < text.size(); i++)
    {
        plcp[sa[i]] = lcp[i];
    }
    return plcp;
}

bool check_lex(const std::vector<uint8_t> &text, std::string &pattern, uint64_t i)
{
    std::string s1;
    std::string s2;
    s2 += pattern;
    for (uint64_t j = i; j < text.size(); j++)
    {
        s1.push_back(text[j]);
        s2.push_back(text[j]);
    }
    return s1 < s2;
}

uint64_t find_bounded_distance(const std::vector<uint64_t> &plcp, uint64_t i)
{
    if (i == 0)
    {
        return 0;
    }
    else
    {
        int64_t j = i - 1;
        while (plcp[j] >= i - j)
        {
            j--;
            if (j == -1)
            {
                return i;
            }
        }
        return (i - 1 - j);
    }
}

void find_example()
{
    std::vector<std::string> r = create_all_permutations(8, {'a', 'b'});
    std::cout << r.size() << std::endl;

    std::vector<uint8_t> alphabet_with_end_marker = {'$', 'a', 'b'};

    for (auto &str : r)
    {
        std::vector<uint8_t> text;
        for (auto &c : str)
        {
            text.push_back(c);
        }
        text.push_back('$');
        std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
        std::vector<uint64_t> lcp = stool::ArrayConstructor::construct_LCP_array(text, sa, stool::Message::NO_MESSAGE);
        std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

        uint64_t rle_size = get_rle_size(bwt);
        // std::cout << str << ", " << rle_size << std::endl;
        if (rle_size == 5)
        {

            stool::dynamic_r_index::DynamicFMIndex dyn_index1 = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, 4, stool::Message::NO_MESSAGE);
            stool::dynamic_r_index::DynamicFMIndex dyn_index2 = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, 4, stool::Message::NO_MESSAGE);

            std::string _bwt = dyn_index1.get_bwt_str();

            stool::dynamic_r_index::FMIndexEditHistory output_history1;
            stool::dynamic_r_index::FMIndexEditHistory output_history2;

            std::vector<uint8_t> pattern1 = {'a', 'b'};
            std::vector<uint8_t> pattern2 = {'b', 'a'};

            dyn_index1.insert_string(4, pattern1, output_history1);
            dyn_index2.insert_string(4, pattern2, output_history2);

            bool b1 = output_history1.move_history.size() == 2;
            bool d1 = false;
            for (auto &m : output_history1.move_history)
            {
                int i1 = m.first < m.second ? m.first : m.second;
                int i2 = m.second < m.first ? m.first : m.second;
                if (i2 - i1 != 1)
                {
                    d1 = true;
                }
            }

            bool b2 = output_history2.move_history.size() == 2;
            bool d2 = false;
            for (auto &m : output_history2.move_history)
            {
                int i1 = m.first < m.second ? m.first : m.second;
                int i2 = m.second < m.first ? m.first : m.second;
                if (i2 - i1 != 1)
                {
                    d2 = true;
                }
            }

            if (b1 && d1)
            {
                std::cout << "text: " << str << "$, pattern:" << "ab" << ", RLE size: " << rle_size << ", bwt: " << _bwt << ", bwt2:" << dyn_index1.get_bwt_str() << std::endl;
            }

            if (b2 && d2)
            {
                std::cout << "text: " << str << "$, pattern:" << "ba" << ", RLE size: " << rle_size << ", bwt: " << _bwt << ", bwt2:" << dyn_index2.get_bwt_str() << std::endl;
            }
        }
    }
}

class CounterExampleInfo
{
public:
    uint64_t i = UINT64_MAX;
    uint64_t dist = 0;
    uint64_t d = 0;
    std::string str;
    std::string pattern;

    uint64_t get_diff() const
    {
        return this->d - this->dist;
    }
    double get_ratio() const
    {
        uint64_t k = (this->dist * 2) + 1;
        return double(this->d) / double(k);
    }
    void swap(CounterExampleInfo &other)
    {
        std::swap(this->i, other.i);
        std::swap(this->dist, other.dist);
        std::swap(this->d, other.d);
        std::swap(this->str, other.str);
        std::swap(this->pattern, other.pattern);
    }

    CounterExampleInfo() {}
};

void print_PLCP(std::string str)
{
    str.push_back('$');
    auto plcp = construct_reversed_perfect_PLCP_array(str);
    stool::DebugPrinter::print_integers(plcp, "rPLCP");
}

void print_counter_example(const CounterExampleInfo &info)
{
    std::vector<uint8_t> alphabet = {'a', 'b'};
    std::vector<uint8_t> alphabet_with_end_marker = {'$', 'a', 'b'};
    std::vector<uint8_t> text;
    for (auto &c : info.str)
    {
        text.push_back(c);
    }
    text.push_back('$');
    std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
    std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::NO_MESSAGE);
    std::vector<uint64_t> lcp = stool::ArrayConstructor::construct_LCP_array(text, sa, stool::Message::NO_MESSAGE);
    std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

    std::vector<uint64_t> plcp = construct_PLCP_array(text, sa, lcp);

    uint8_t c = info.pattern[0];
    uint64_t i = info.i;
    uint64_t dist = info.dist;
    uint64_t d = info.d;

    stool::dynamic_r_index::DynamicFMIndex dyn_index = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, 4, stool::Message::NO_MESSAGE);
    stool::dynamic_r_index::FMIndexEditHistory output_history;
    dyn_index.insert_char(i, c, output_history);

    std::cout << "text: " << info.str << "$, pattern:" << c << ", i = " << i << ", dist = " << dist << ", d = " << d << std::endl;
    std::string new_str = dyn_index.get_text_str().substr(0, text.size());
    print_PLCP(info.str);
    print_PLCP(new_str);

    output_history.print();
}

CounterExampleInfo find_counter_example(const std::string &str, std::string pattern, bool use_d = false)
{
    std::vector<uint8_t> alphabet = {'a', 'b'};
    std::vector<uint8_t> alphabet_with_end_marker = {'$', 'a', 'b'};
    std::vector<uint8_t> text;
    for (auto &c : str)
    {
        text.push_back(c);
    }
    text.push_back('$');
    std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
    std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::NO_MESSAGE);
    std::vector<uint64_t> lcp = stool::ArrayConstructor::construct_LCP_array(text, sa, stool::Message::NO_MESSAGE);
    std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

    std::vector<uint64_t> plcp = construct_PLCP_array(text, sa, lcp);

    CounterExampleInfo info;

    std::vector<uint8_t> pattern2;
    for (auto &c : pattern)
    {
        pattern2.push_back(c);
    }

    for (uint64_t i = 0; i < str.size(); i++)
    {

        uint64_t dist = find_bounded_distance(plcp, i);
        bool b = check_lex(text, pattern, i);
        if (!b)
        {
            stool::dynamic_r_index::DynamicFMIndex dyn_index = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, 4, stool::Message::NO_MESSAGE);
            stool::dynamic_r_index::FMIndexEditHistory output_history;
            dyn_index.insert_string(i, pattern2, output_history);

            uint64_t d = output_history.move_history.size();

            if (d > dist)
            {
                CounterExampleInfo tmp;
                tmp.i = i;
                tmp.dist = dist;
                tmp.d = d;
                tmp.str = str;
                tmp.pattern = pattern;

                if (use_d)
                {
                    if (tmp.get_diff() > info.get_diff())
                    {
                        info.swap(tmp);
                    }
                }
                else
                {
                    if (tmp.get_ratio() > info.get_ratio())
                    {
                        info.swap(tmp);
                    }
                }
            }
        }
    }
    return info;
}
/*
void find_counter_example1()
{
    std::vector<std::string> r = create_all_permutations(12, {'a', 'b'});
    std::cout << r.size() << std::endl;

    std::vector<uint8_t> alphabet_with_end_marker = {'$', 'a', 'b'};

    uint64_t k = 0;
    for (auto &str : r)
    {
        k++;
        if (k % 100 == 0)
        {
            std::cout << "k = " << k << std::endl;
        }

        find_counter_example(str, "a");
        find_counter_example(str, "b");
    }
}
*/

void find_counter_example2()
{
    for (uint64_t i = 2; i < 10; i++)
    {

        std::string str;
        uint64_t pref_len = i;
        uint64_t suf_len = i;
        double threshold = 0.9;

        for (uint64_t j = 0; j < pref_len; j++)
        {
            str.push_back('b');
        }
        str += "a";
        for (uint64_t j = 0; j < suf_len; j++)
        {
            str.push_back('b');
        }
        CounterExampleInfo info = find_counter_example(str, "a", true);
        if (info.get_ratio() > threshold)
        {
            std::cout << "len: " << str << ", i: " << info.i << ", d: " << info.d << ", dist : " << info.dist << ", ratio: " << info.get_ratio() << std::endl;
            print_counter_example(info);
            threshold = info.get_ratio();
        }
    }
}

void find_counter_example3(uint64_t max_text_len, uint64_t max_pattern_len)
{
    std::vector<std::string> r = create_all_permutations(max_text_len, {'a', 'b'});
    std::vector<std::string> patterns = create_all_permutations(max_pattern_len, {'a', 'b'});

    uint64_t k = 0;
    double threshold = 0.9;
    for (auto &str : r)
    {
        for (auto &pattern : patterns)
        {
            k++;
            if (k % 100 == 0)
            {
                std::cout << "k = " << k << std::endl;
            }
            CounterExampleInfo info = find_counter_example(str, pattern);
            if (info.get_ratio() > threshold)
            {
                std::cout << "len: " << str << ", pattern: " << pattern << ", i ;" << info.i << ", d: " << info.d << ", dist : " << info.dist << ", ratio: " << info.get_ratio() << std::endl;
                threshold = info.get_ratio();
            }
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef DEBUG
    std::cout << "\033[41m";
    std::cout << "DEBUG MODE!" << std::endl;
    std::cout << "\e[m" << std::endl;
    // std::cout << "\033[30m" << std::endl;
#endif
#ifdef SLOWDEBUG
    std::cout << "\033[41m";
    std::cout << "SLOWDEBUG MODE!" << std::endl;
    std::cout << "\e[m" << std::endl;
    // std::cout << "\033[30m" << std::endl;
#endif

    cmdline::parser p;

    p.add<uint>("mode", 'm', "mode", true, 1);

    p.parse_check(argc, argv);
    uint mode = p.get<uint>("mode");

    if (mode == 0)
    {
        find_example();
    }
    else if (mode == 1)
    {
        find_counter_example3(12, 1);
    }
    else if (mode == 2)
    {
        find_counter_example2();
    }
    else if (mode == 3)
    {
    }
}
