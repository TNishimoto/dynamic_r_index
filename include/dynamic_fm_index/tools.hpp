#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
// #include "./dynamic_rank_select/gRankS.h"
#include <cassert>
namespace stool
{
    namespace dynamic_r_index
    {
        using TextIndex = uint64_t;
        using SAIndex = uint64_t;
        using SAMove = std::pair<SAIndex, SAIndex>;
        using RunIndex = uint64_t;
        using PositionInRun = uint64_t;
        using SampledSAIndex = uint64_t;
        using SampledISAIndex = uint64_t;
        using SAValue = uint64_t;
        using ISAValue = uint64_t;
        using SAInterval = std::pair<int64_t, int64_t>;

        struct BackwardSearchResult
        {
        public:
            int64_t b;
            int64_t e;
            int64_t sa_b_;

            BackwardSearchResult() 
            {
            }
            BackwardSearchResult(int64_t _b, int64_t _e, int64_t _sa_b_) : b(_b), e(_e), sa_b_(_sa_b_)
            {
            }
            BackwardSearchResult(int64_t _b, int64_t _e) : b(_b), e(_e), sa_b_(-1)
            {
            }


            static BackwardSearchResult create_empty_result()
            {
                return BackwardSearchResult(-1, -1, -1);
            }
            SAInterval get_sa_interval() const {
                return SAInterval(this->b, this->e);
            }
            uint64_t get_sa_interval_size() const {
                if(this->is_empty()){
                    return 0;
                }else{
                    return this->e - this->b + 1;
                }
            }
            bool is_empty() const
            {
                return this->b == -1;
            }
        };

        ////////////////////////////////////////////////////////////////////////////////
        /// @class      RunPosition
        /// @brief      A representation of a position in RLBWT.
        ///
        ////////////////////////////////////////////////////////////////////////////////
        struct RunPosition
        {
        public:
            uint64_t run_index;
            uint64_t position_in_run;

            RunPosition(uint64_t _run_index, uint64_t _position_in_run) : run_index(_run_index), position_in_run(_position_in_run)
            {
            }
            static RunPosition create_null_value()
            {
                return RunPosition(UINT64_MAX, UINT64_MAX);
            }
            bool is_null() const
            {
                return this->run_index == UINT64_MAX;
            }

            static int compare(RunPosition p1, RunPosition p2)
            {
                if (p1.run_index == p2.run_index)
                {
                    if (p1.position_in_run < p2.position_in_run)
                    {
                        return -1;
                    }
                    else if (p1.position_in_run > p2.position_in_run)
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    if (p1.run_index < p2.run_index)
                    {
                        return -1;
                    }
                    else
                    {
                        return 1;
                    }
                }
            }
            std::string to_string() const
            {
                return "[RunIndex: " + std::to_string(this->run_index) + ", pos = " + std::to_string(this->position_in_run) + "]";
            }
        };
        enum RunReplacementType
        {
            FirstCharReplacement,
            LastCharReplacement,
            CenterCharReplacement,
            RunReplacement,
            NoReplacement
        };
        struct DetailedReplacement
        {
        public:
            bool no_replancement;
            bool is_first;
            bool is_last;
            bool upper_merge;
            bool lower_merge;

            DetailedReplacement()
            {
            }
            DetailedReplacement(bool _no_replancement, bool _is_first, bool _is_last, bool _upper_merge, bool _lower_merge) : no_replancement(_no_replancement), is_first(_is_first), is_last(_is_last), upper_merge(_upper_merge), lower_merge(_lower_merge)
            {
            }

            void print() const
            {
                std::cout << "DetailedReplacement[" << this->no_replancement << ", " << this->is_first << ", " << this->is_last << ", " << this->upper_merge << ", " << this->lower_merge << "]" << std::endl;
            }
        };

        enum RunInsertionType
        {
            FirstCharInsertion,
            LastCharInsertion,
            CenterCharInsertion,
            NewRunInsertionWithoutSplit,
            NewRunInsertionWithSplit
        };

        enum RunRemovalType
        {
            FirstCharRemoval,
            LastCharRemoval,
            CenterCharRemoval,
            RunRemovalWithMerge,
            RunRemovalWithoutMerge,
            RunRemovalDummy
        };

    }
}
