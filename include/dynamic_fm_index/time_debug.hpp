#pragma once
#include <cstdint>
#include <vector>
#include <string>


namespace stool
{
    inline uint64_t wavelet_tree_update_time = 0;
    inline uint64_t wavelet_tree_update_count = 0;
    inline uint64_t isa_update_time = 0;
    inline uint64_t isa_update_count = 0;
    inline uint64_t C_update_time = 0;
    inline uint64_t C_update_count = 0;
    inline uint64_t LF_time = 0;
    inline uint64_t LF_count = 0;
    inline uint64_t pom_access_time = 0;
    inline uint64_t pom_access_count = 0;
    inline uint64_t increment_run_time = 0;
    inline uint64_t increment_run_count = 0;

    inline std::vector<std::string> messages;



    class TimeDebug
    {
    public:
        static void clear()
        {
            wavelet_tree_update_time = 0;
            wavelet_tree_update_count = 0;
            isa_update_time = 0;
            isa_update_count = 0;
            C_update_time = 0;
            C_update_count = 0;
            LF_time = 0;
            LF_count = 0;
            pom_access_time = 0;
            pom_access_count = 0;
            increment_run_time = 0;
            increment_run_count = 0;
        }
    };

}
