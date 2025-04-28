#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
// #include "qgram_tree.h"
// #include "include/debug/debug.hpp"
#include "stool/include/stool.hpp"
#include "../include/old_implementations/btree_include.hpp"
#include "../include/drfmi.hpp"
#include "../include/dynamic_fm_index/packed_spsi_wrapper.hpp"


#include <filesystem>



void WB_test(uint64_t len, uint64_t seed, uint8_t degree)
{
    std::mt19937_64 mt64(seed);
    std::vector<uint8_t> chars;
    if(degree == 0){
        chars.push_back('A');
        chars.push_back('C');
        chars.push_back('G');
        chars.push_back('T');
    }else{
        for(uint64_t i = 1; i <= degree;i++){
            chars.push_back(i);
        }

    }
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, len);

    DynRankS wb;
    uint64_t n = chars.size();
    float *f;
    f = DynRankS::createCharDistribution(chars, (n > 10000000) ? n + 1 : 10000000, 1);
    wb.initEmptyDynRankS(f);

    std::chrono::system_clock::time_point st1, st2, st3, st4, st5, st6;
    st1 = std::chrono::system_clock::now();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t c = chars[m % chars.size()];
        uint64_t p = m % (i + 1);
        stool::DynRankSWrapper::insert(wb, p, c);
    }
    st2 = std::chrono::system_clock::now();

    uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t c = chars[m % chars.size()];
        uint64_t p = m % len;
        stool::DynRankSWrapper::remove(wb, p);
        stool::DynRankSWrapper::insert(wb, len - p - 1, c);
    }
    st3 = std::chrono::system_clock::now();
    uint64_t time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();

    uint64_t tmp_value = 0;

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t c = chars[m % chars.size()];
        uint64_t p = m % len;
        tmp_value += stool::DynRankSWrapper::rank(wb, c, p);
    }
    st4 = std::chrono::system_clock::now();
    uint64_t time3 = std::chrono::duration_cast<std::chrono::nanoseconds>(st4 - st3).count();

    std::vector<uint64_t> rank_vec;
    for (uint64_t i = 0; i < chars.size(); i++)
    {
        uint8_t c = chars[i];
        uint64_t count = stool::DynRankSWrapper::rank(wb, c, len - 1);
        rank_vec.push_back(count);
    }

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t c = chars[m % chars.size()];
        uint64_t p = m % rank_vec[m % chars.size()];
        tmp_value += stool::DynRankSWrapper::select(wb, c, p + 1);
    }

    st5 = std::chrono::system_clock::now();
    uint64_t time4 = std::chrono::duration_cast<std::chrono::nanoseconds>(st5 - st4).count();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t p = m % len;
        tmp_value += stool::DynRankSWrapper::access(wb, p);
    }
    st6 = std::chrono::system_clock::now();
    uint64_t time5 = std::chrono::duration_cast<std::chrono::nanoseconds>(st6 - st5).count();

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Test [WB], len = " << len << std::endl;
    std::cout << "sigma = " << chars.size() << std::endl;
    std::cout << "Hash = " << tmp_value << std::endl;

    std::cout << "Construction Time: " << (time1 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / len) << "[ns])" << std::endl;
    std::cout << "Update Time      : " << (time2 / (1000 * 1000)) << "[ms] (Avg: " << (time2 / len) << "[ns])" << std::endl;
    std::cout << "Rank Query       : " << (time3 / (1000 * 1000)) << "[ms] (Avg: " << (time3 / len) << "[ns])" << std::endl;
    std::cout << "Select Query     : " << (time4 / (1000 * 1000)) << "[ms] (Avg: " << (time4 / len) << "[ns])" << std::endl;
    std::cout << "Access Query     : " << (time5 / (1000 * 1000)) << "[ms] (Avg: " << (time5 / len) << "[ns])" << std::endl;
    std::cout << "Size: " << wb.getSize() << std::endl;
    stool::print_memory_usage();
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

void WB2_test(uint64_t len, uint64_t seed)
{
    std::mt19937_64 mt64(seed);
    std::vector<uint8_t> chars;
    chars.push_back('A');
    chars.push_back('C');
    chars.push_back('G');
    chars.push_back('T');
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, len);

    dyn::wt_string<dyn::suc_bv> wb;

    std::chrono::system_clock::time_point st1, st2, st3, st4, st5, st6;
    st1 = std::chrono::system_clock::now();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t c = chars[m % chars.size()];
        wb.push_back(c);
    }
    st2 = std::chrono::system_clock::now();

    uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t c = chars[m % chars.size()];
        uint64_t p = m % len;
        wb.remove(p);
        wb.insert(len - p - 1, c);
    }
    st3 = std::chrono::system_clock::now();
    uint64_t time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();

    uint64_t tmp_value = 0;

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t c = chars[m % chars.size()];
        uint64_t p = m % len;
        tmp_value += wb.rank(p, c);
    }
    st4 = std::chrono::system_clock::now();
    uint64_t time3 = std::chrono::duration_cast<std::chrono::nanoseconds>(st4 - st3).count();

    std::vector<uint64_t> rank_vec;
    for (uint64_t i = 0; i < chars.size(); i++)
    {
        uint8_t c = chars[i];
        uint64_t count = wb.rank(len - 1, c);
        rank_vec.push_back(count);
    }

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t c = chars[m % chars.size()];
        uint64_t p = m % rank_vec[m % chars.size()];
        tmp_value += wb.select(p, c);
    }

    st5 = std::chrono::system_clock::now();
    uint64_t time4 = std::chrono::duration_cast<std::chrono::nanoseconds>(st5 - st4).count();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t p = m % len;
        tmp_value += wb[p];
    }
    st6 = std::chrono::system_clock::now();
    uint64_t time5 = std::chrono::duration_cast<std::chrono::nanoseconds>(st6 - st5).count();

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Test [WB2], len = " << len << std::endl;
    std::cout << "Hash = " << tmp_value << std::endl;

    std::cout << "Construction Time: " << (time1 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / len) << "[ns])" << std::endl;
    std::cout << "Update Time      : " << (time2 / (1000 * 1000)) << "[ms] (Avg: " << (time2 / len) << "[ns])" << std::endl;
    std::cout << "Rank Query       : " << (time3 / (1000 * 1000)) << "[ms] (Avg: " << (time3 / len) << "[ns])" << std::endl;
    std::cout << "Select Query     : " << (time4 / (1000 * 1000)) << "[ms] (Avg: " << (time4 / len) << "[ns])" << std::endl;
    std::cout << "Access Query     : " << (time5 / (1000 * 1000)) << "[ms] (Avg: " << (time5 / len) << "[ns])" << std::endl;
    std::cout << "Size: " << wb.size() << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

void SPSI_test(uint64_t len, uint64_t seed)
{
    dyn::packed_spsi spsi;
    uint64_t max_value = 500;
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, len * max_value);
    std::mt19937_64 mt64(seed);

    std::chrono::system_clock::time_point st1, st2, st3, st4, st5;
    st1 = std::chrono::system_clock::now();
    std::cout << "Build Data Structure..." << std::endl;

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t value = m % max_value;
        spsi.push_back(value);
    }
    st2 = std::chrono::system_clock::now();
    uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

    std::cout << "Insert and delete random elements..." << std::endl;
    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t p = m % spsi.size();
        uint64_t value = m % max_value;
        stool::PackedSPSIWrapper::remove(spsi, p);
        spsi.insert(len - p - 1, value);
    }
    st3 = std::chrono::system_clock::now();
    uint64_t time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();

    std::cout << "Random psum..." << std::endl;
    uint64_t tmp_value = 0;
    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t p = m % spsi.size();
        uint64_t value = spsi.psum(p);
        tmp_value += value;
    }
    std::cout << tmp_value << std::endl;
    st4 = std::chrono::system_clock::now();
    uint64_t time3 = std::chrono::duration_cast<std::chrono::nanoseconds>(st4 - st3).count();

    std::cout << "Random search..." << std::endl;
    uint64_t sum = spsi.psum();
    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t p = m % sum;
        uint64_t value = spsi.search(p);
        tmp_value += value;
    }
    st5 = std::chrono::system_clock::now();
    uint64_t time4 = std::chrono::duration_cast<std::chrono::nanoseconds>(st5 - st4).count();

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Test [SPSI], len = " << len << std::endl;
    std::cout << "Checksum = " << tmp_value << std::endl;
    std::cout << "Construction Time  : " << (time1 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / len) << "[ns])" << std::endl;
    std::cout << "Update Time        : " << (time2 / (1000 * 1000)) << "[ms] (Avg: " << ((time2 * 2) / len) << "[ns])" << std::endl;
    std::cout << "PSUM Time          : " << (time3 / (1000 * 1000)) << "[ms] (Avg: " << (time3 / len) << "[ns])" << std::endl;
    std::cout << "Search Time        : " << (time4 / (1000 * 1000)) << "[ms] (Avg: " << (time4 / len) << "[ns])" << std::endl;
    std::cout << "Total Running Time : " << (time1 + time2 + time3 + time4) / (1000 * 1000) << "[ms] (Avg: " << ((time1 + time2 + time3 + time4) / len) << "[ns])" << std::endl;

    std::cout << "Size: " << spsi.size() << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

void POM_test(uint64_t len, uint64_t degree, uint64_t seed)
{
    stool::old_implementations::VPomPermutation vpt;
    vpt.initialize(degree);

    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, len);
    std::mt19937_64 mt64(seed);

    std::chrono::system_clock::time_point st1, st2, st3, st4;
    st1 = std::chrono::system_clock::now();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m1 = get_rand_uni_int(mt64);
        uint64_t m2 = get_rand_uni_int(mt64);
        uint64_t pi = m1 % (i + 1);
        uint64_t inv_pi = m2 % (i + 1);
        vpt.insert(pi, inv_pi);
    }
    st2 = std::chrono::system_clock::now();

    vpt.pi_tree.defragmentation();
    vpt.inverse_pi_tree.defragmentation();
    vpt.print_memory();

    uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m1 = get_rand_uni_int(mt64);
        uint64_t m2 = get_rand_uni_int(mt64);
        uint64_t m3 = get_rand_uni_int(mt64);

        uint64_t pi = m1 % len;
        uint64_t inv_pi = m2 % len;
        uint64_t rmv = m3 % len;

        vpt.insert(pi, inv_pi);
        vpt.erase(rmv);
    }
    st3 = std::chrono::system_clock::now();
    uint64_t time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();

    uint64_t tmp_value = 0;
    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t pi = m % len;
        tmp_value += vpt.access(pi);
    }
    std::cout << tmp_value << std::endl;
    st4 = std::chrono::system_clock::now();
    uint64_t time3 = std::chrono::duration_cast<std::chrono::nanoseconds>(st4 - st3).count();

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Test [POM], len = " << len << std::endl;
    std::cout << "Degree: " << degree << std::endl;
    std::cout << "Construction Time: " << (time1 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / len) << "[ns])" << std::endl;
    std::cout << "Update Time      : " << (time2 / (1000 * 1000)) << "[ms] (Avg: " << (time2 / len) << "[ns])" << std::endl;
    std::cout << "Access Query     : " << (time3 / (1000 * 1000)) << "[ms] (Avg: " << (time3 / len) << "[ns])" << std::endl;
    std::cout << "Size: " << vpt.size() << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}
void NPOM_test(uint64_t len, uint64_t degree, uint64_t seed)
{
    stool::old_implementations::PermutationOrderMaintenance vpt;
    vpt.initialize(degree);

    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, len);
    std::mt19937_64 mt64(seed);

    std::chrono::system_clock::time_point st1, st2, st3, st4;
    st1 = std::chrono::system_clock::now();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m1 = get_rand_uni_int(mt64);
        uint64_t m2 = get_rand_uni_int(mt64);
        uint64_t pi = m1 % (i + 1);
        uint64_t inv_pi = m2 % (i + 1);
        vpt.insert(pi, inv_pi);
    }
    st2 = std::chrono::system_clock::now();
    uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m1 = get_rand_uni_int(mt64);
        uint64_t m2 = get_rand_uni_int(mt64);
        uint64_t m3 = get_rand_uni_int(mt64);

        uint64_t pi = m1 % len;
        uint64_t inv_pi = m2 % len;
        uint64_t rmv = m3 % len;

        vpt.insert(pi, inv_pi);
        vpt.erase(rmv);
    }
    st3 = std::chrono::system_clock::now();
    uint64_t time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();

    uint64_t tmp_value = 0;
    for (uint64_t i = 0; i < len; i++)
    {
        uint64_t m = get_rand_uni_int(mt64);
        uint64_t pi = m % len;
        tmp_value += vpt.access(pi);
    }
    std::cout << tmp_value << std::endl;
    st4 = std::chrono::system_clock::now();
    uint64_t time3 = std::chrono::duration_cast<std::chrono::nanoseconds>(st4 - st3).count();

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Test [Pointer POM], len = " << len << std::endl;
    std::cout << "Degree: " << degree << std::endl;
    std::cout << "Construction Time: " << (time1 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / len) << "[ns])" << std::endl;
    std::cout << "Update Time      : " << (time2 / (1000 * 1000)) << "[ms] (Avg: " << (time2 / len) << "[ns])" << std::endl;
    std::cout << "Access Query     : " << (time3 / (1000 * 1000)) << "[ms] (Avg: " << (time3 / len) << "[ns])" << std::endl;
    std::cout << "Size: " << vpt.size() << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
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

    // p.add<std::string>("input_file", 'i', "input file name", true);
    p.add<uint>("mode", 'm', "mode", true);
    p.add<uint>("length", 'l', "length", true);
    p.add<uint>("degree", 'd', "degree", false, 4);

    p.parse_check(argc, argv);
    uint64_t mode = p.get<uint>("mode");
    uint64_t length = p.get<uint>("length");
    uint64_t degree = p.get<uint>("degree");

    if (mode == 0)
    {
        WB_test(length, 0, degree);
    }
    else if (mode == 1)
    {
        SPSI_test(length, 0);
    }
    else if (mode == 2)
    {
        WB2_test(length, 0);
    }
    else if (mode == 3)
    {
        POM_test(length, degree, 0);
    }
    else if (mode == 4)
    {
        NPOM_test(length, degree, 0);
    }

    stool::print_memory_usage();
}
