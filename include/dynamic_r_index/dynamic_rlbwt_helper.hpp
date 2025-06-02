#pragma once
#include "dynamic_rlbwt.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        ////////////////////////////////////////////////////////////////////////////////
        /// @class      DynamicRLBWTHelper
        /// @brief      Helper functions for dynamic RLBWT
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class DynamicRLBWTHelper
        {
        private:
            static uint64_t get_upper_character(const DynamicRLBWT &dbwt, uint64_t run_index, uint64_t pos_in_run, uint8_t c)
            {
                if (pos_in_run > 0)
                {
                    return c;
                }
                else if (run_index > 0)
                {
                    return dbwt.get_char(run_index - 1);
                }
                else
                {
                    return UINT64_MAX;
                }
            }

            static uint64_t get_lower_character(const DynamicRLBWT &dbwt, uint64_t run_index, uint64_t pos_in_run, uint8_t c)
            {
                uint64_t run_count = dbwt.run_count();
                uint64_t len = run_index < run_count ? dbwt.get_run_length(run_index) : UINT64_MAX;
                if (run_index < run_count)
                {
                    if (pos_in_run + 1 < len)
                    {
                        return c;
                    }
                    else if (run_index + 1 < run_count)
                    {
                        return dbwt.get_char(run_index + 1);
                    }
                    else
                    {
                        return UINT64_MAX;
                    }
                }
                else
                {
                    return UINT64_MAX;
                }
            }

            static DetailedReplacement get_replacement_type2(const DynamicRLBWT &dbwt, uint64_t run_index, uint64_t pos_in_run, uint8_t new_char)
            {
                uint64_t len = dbwt.get_run_length(run_index);
                uint8_t old_c = dbwt.get_char(run_index);
                uint64_t prev_c = get_upper_character(dbwt, run_index, pos_in_run, old_c);
                uint64_t next_c = get_lower_character(dbwt, run_index, pos_in_run, old_c);

                bool _is_first = pos_in_run == 0;
                bool _is_last = pos_in_run + 1 == len;
                bool rep = old_c == new_char;
                bool _upper_merge = new_char == prev_c && _is_first;
                bool _lower_merge = new_char == next_c && _is_last;
                return DetailedReplacement(rep, _is_first, _is_last, _upper_merge, _lower_merge);
            }

            static RunReplacementType get_replacement_type(const DynamicRLBWT &dbwt, uint64_t run_index, uint64_t pos_in_run, uint8_t new_char)
            {
                uint64_t len = dbwt.get_run_length(run_index);
                uint8_t c = dbwt.get_char(run_index);
                if (len == 1)
                {
                    return c == new_char ? RunReplacementType::NoReplacement : RunReplacementType::RunReplacement;
                }
                else
                {
                    if (pos_in_run == 0)
                    {
                        return RunReplacementType::FirstCharReplacement;
                    }
                    else if (pos_in_run + 1 == len)
                    {
                        return RunReplacementType::LastCharReplacement;
                    }
                    else
                    {
                        return RunReplacementType::CenterCharReplacement;
                    }
                }
            }

        public:
            static RunPosition proper_successor_on_F(const DynamicRLBWT &dbwt, RunPosition x, uint8_t c)
            {
                uint8_t xc = dbwt.get_char(x.run_index);
                uint64_t run_length = dbwt.get_run_length(x.run_index);
                if (xc == c && x.position_in_run + 1 < run_length)
                {
                    return RunPosition(x.run_index, x.position_in_run + 1);
                }
                else
                {
                    int64_t c_rank1 = dbwt.rank_on_first_characters_of_RLBWT(c, x.run_index);
                    int64_t c_rank2 = dbwt.rank_on_first_characters_of_RLBWT(c, dbwt.run_count() - 1);
                    if (c_rank1 < c_rank2)
                    {
                        int64_t idx = dbwt.select_on_first_characters_of_RLBWT(c_rank1, c);
                        return RunPosition(idx, 0);
                    }
                    else
                    {
                        int64_t z = dbwt.get_c_array().successor_on_effective_alphabet((uint8_t)(c + 1));
                        if (z == -1)
                        {
                            z = 0;
                        }
                        uint8_t c2 = dbwt.get_c_array().get_character_in_effective_alphabet(z);
                        int64_t idx2 = dbwt.select_on_first_characters_of_RLBWT(0, c2);
                        return RunPosition(idx2, 0);
                    }
                }
            }

            static RunPosition proper_successor_on_F(const DynamicRLBWT &dbwt, RunPosition x)
            {
                uint8_t xc = dbwt.get_char(x.run_index);
                return proper_successor_on_F(dbwt, x, xc);
            }

            /*
            static RunPosition proper_predecessor_on_F_for_deletion(const DynamicRLBWT &dbwt, RunPosition x, uint8_t c, uint8_t old_char, uint64_t positioin_to_replace)
            {
                uint8_t xc = dbwt.get_char(x.run_index);
                if (x.position_in_run > 0 && xc == c)
                {
                    return RunPosition(x.run_index, x.position_in_run - 1);
                }
                else
                {
                    int64_t c_rank1 = x.run_index > 0 ? dbwt.rank_on_first_characters_of_RLBWT(c, x.run_index - 1) : 0;
                    if (c_rank1 > 0)
                    {
                        int64_t idx = dbwt.select_on_first_characters_of_RLBWT(c_rank1 - 1, c);
                        int64_t run_length = dbwt.get_run_length(idx);
                        return RunPosition(idx, run_length - 1);
                    }
                    else
                    {
                        int64_t z = dbwt.get_c_array().predecessor_on_effective_alphabet((uint8_t)(c - 1));
                        if (z == -1)
                        {
                            z = dbwt.get_c_array().effective_alphabet_size() - 1;
                        }

                        uint8_t c2 = dbwt.get_c_array().get_character_in_effective_alphabet(z);
                        int64_t occ = dbwt.rank_on_first_characters_of_RLBWT(c2, dbwt.run_count() - 1);
                        assert(occ > 0);

                        int64_t idx = dbwt.select_on_first_characters_of_RLBWT(occ - 1, c2);
                        int64_t run_length = dbwt.get_run_length(idx);

                        return RunPosition(idx, run_length - 1);
                    }
                }
            }
            */

            static RunPosition proper_predecessor_on_F(const DynamicRLBWT &dbwt, RunPosition x, uint8_t c)
            {
                uint8_t xc = dbwt.get_char(x.run_index);
                if (x.position_in_run > 0 && xc == c)
                {
                    return RunPosition(x.run_index, x.position_in_run - 1);
                }
                else
                {
                    int64_t c_rank1 = x.run_index > 0 ? dbwt.rank_on_first_characters_of_RLBWT(c, x.run_index - 1) : 0;
                    if (c_rank1 > 0)
                    {
                        int64_t idx = dbwt.select_on_first_characters_of_RLBWT(c_rank1 - 1, c);
                        int64_t run_length = dbwt.get_run_length(idx);
                        return RunPosition(idx, run_length - 1);
                    }
                    else
                    {
                        int64_t z = dbwt.get_c_array().predecessor_on_effective_alphabet((uint8_t)(c - 1));
                        if (z == -1)
                        {
                            z = dbwt.get_c_array().effective_alphabet_size() - 1;
                        }

                        uint8_t c2 = dbwt.get_c_array().get_character_in_effective_alphabet(z);
                        int64_t occ = dbwt.rank_on_first_characters_of_RLBWT(c2, dbwt.run_count() - 1);
                        assert(occ > 0);

                        int64_t idx = dbwt.select_on_first_characters_of_RLBWT(occ - 1, c2);
                        int64_t run_length = dbwt.get_run_length(idx);

                        return RunPosition(idx, run_length - 1);
                    }
                }
            }
            static RunPosition proper_predecessor_on_F(const DynamicRLBWT &dbwt, RunPosition x)
            {
                uint8_t xc = dbwt.get_char(x.run_index);
                return proper_predecessor_on_F(dbwt, x, xc);
            }

            static RunRemovalType remove_char(DynamicRLBWT &dbwt, RunPosition i, [[maybe_unused]] uint8_t c)
            {

                uint64_t run_length = dbwt.get_run_length(i.run_index);
                if (run_length == 1)
                {
                    uint64_t c1 = i.run_index > 0 ? dbwt.get_char(i.run_index - 1) : UINT64_MAX;
                    uint64_t c2 = i.run_index + 1 < dbwt.run_count() ? dbwt.get_char(i.run_index + 1) : UINT64_MAX - 1;

                    if (c1 == c2)
                    {
                        uint64_t plen = dbwt.get_run_length(i.run_index + 1);
                        dbwt.increment_run(i.run_index - 1, plen);
                        dbwt.remove_BWT_run(i.run_index + 1);
                        dbwt.remove_BWT_run(i.run_index);
                        return RunRemovalType::RunRemovalWithMerge;
                    }
                    else
                    {
                        dbwt.remove_BWT_run(i.run_index);
                        return RunRemovalType::RunRemovalWithoutMerge;
                    }
                }
                else
                {
                    dbwt.decrement_run(i.run_index, 1);

                    if (i.position_in_run + 1 == run_length)
                    {
                        return RunRemovalType::LastCharRemoval;
                    }
                    else if (i.position_in_run == 0)
                    {
                        return RunRemovalType::FirstCharRemoval;
                    }
                    else
                    {
                        return RunRemovalType::CenterCharRemoval;
                    }
                }
            }

            static bool try_to_merge_runs(DynamicRLBWT &dbwt, uint64_t run_index)
            {
                assert(run_index < (uint64_t)dbwt.run_count());
                uint64_t size = dbwt.run_count();
                if (run_index + 1 < size)
                {
                    uint8_t c1 = dbwt.get_char(run_index);
                    uint8_t c2 = dbwt.get_char(run_index + 1);
                    if (c1 == c2)
                    {
                        uint64_t len = dbwt.get_run_length(run_index + 1);
                        dbwt.increment_run(run_index, len);

                        assert(run_index + 1 < (uint64_t)dbwt.run_count());

                        dbwt.remove_BWT_run(run_index + 1);

                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            

            static RunPosition insert_char_for_insertion(DynamicRLBWT &dbwt, RunPosition i, uint8_t new_char, RunInsertionType type)
            {
                if (type == RunInsertionType::NewRunInsertionWithSplit)
                {
                    int64_t run_length = dbwt.get_run_length(i.run_index);
                    uint8_t c1 = dbwt.access_character_by_run_index(i.run_index);
                    int64_t diff1 = i.position_in_run;
                    int64_t diff2 = run_length - i.position_in_run;
                    dbwt.decrement_run(i.run_index, run_length - diff1);
                    dbwt.insert_new_run(i.run_index + 1, new_char, 1);
                    dbwt.insert_new_run(i.run_index + 2, c1, diff2);
                    return RunPosition(i.run_index + 1, 0);
                }
                else if (type == RunInsertionType::NewRunInsertionWithoutSplit)
                {
                    dbwt.insert_new_run(i.run_index, new_char, 1);
                    return RunPosition(i.run_index, 0);
                }
                else
                {
                    dbwt.increment_run(i.run_index, 1);
                    return i;
                }
            }

            static std::pair<RunInsertionType, RunPosition> compute_insertion_type_and_position(const DynamicRLBWT &dbwt, RunPosition i, uint8_t c)
            {
                // RunPosition i = dbwt.to_run_position(insertion_position, true);

                uint64_t run_count = dbwt.run_count();
                uint64_t old_c = i.run_index < run_count ? dbwt.get_char(i.run_index) : (UINT64_MAX - 1);
                uint64_t c_prev = get_upper_character(dbwt, i.run_index, i.position_in_run, old_c);
                uint64_t c_next = old_c;
                bool is_fst = i.position_in_run == 0;

                if (is_fst)
                {
                    if (i.run_index < run_count)
                    {
                        if (c_prev == c)
                        {
                            if (c == c_next)
                            {
                                throw std::logic_error("Error: Invalid RLBWT!");
                            }
                            else
                            {
                                uint64_t len2 = dbwt.get_run_length(i.run_index - 1);
                                return std::pair<RunInsertionType, RunPosition>(RunInsertionType::LastCharInsertion, RunPosition(i.run_index - 1, len2));
                            }
                        }
                        else
                        {
                            if (c == c_next)
                            {
                                return std::pair<RunInsertionType, RunPosition>(RunInsertionType::FirstCharInsertion, i);
                            }
                            else
                            {
                                return std::pair<RunInsertionType, RunPosition>(RunInsertionType::NewRunInsertionWithoutSplit, i);
                            }
                        }
                    }
                    else
                    {
                        if (c_prev == c)
                        {
                            uint64_t len2 = dbwt.get_run_length(i.run_index - 1);
                            return std::pair<RunInsertionType, RunPosition>(RunInsertionType::LastCharInsertion, RunPosition(i.run_index - 1, len2));
                        }
                        else
                        {
                            return std::pair<RunInsertionType, RunPosition>(RunInsertionType::NewRunInsertionWithoutSplit, i);
                        }
                    }
                }
                else
                {
                    if (c == c_next)
                    {
                        return std::pair<RunInsertionType, RunPosition>(RunInsertionType::CenterCharInsertion, i);
                    }
                    else
                    {
                        return std::pair<RunInsertionType, RunPosition>(RunInsertionType::NewRunInsertionWithSplit, i);
                    }
                }
            }

            static DetailedReplacement replace_char(DynamicRLBWT &dbwt, RunPosition i, uint8_t new_char)
            {
                DetailedReplacement r = get_replacement_type2(dbwt, i.run_index, i.position_in_run, new_char);

                if (!r.no_replancement)
                {
                    if (r.is_first)
                    {
                        if (r.is_last)
                        {
                            if (r.upper_merge)
                            {
                                if (r.lower_merge)
                                {
                                    uint64_t p = 1 + dbwt.get_run_length(i.run_index + 1);
                                    dbwt.increment_run(i.run_index - 1, p);
                                    dbwt.remove_BWT_run(i.run_index + 1);
                                    dbwt.remove_BWT_run(i.run_index);
                                }
                                else
                                {
                                    dbwt.increment_run(i.run_index - 1, 1);
                                    dbwt.remove_BWT_run(i.run_index);
                                }
                            }
                            else
                            {
                                if (r.lower_merge)
                                {
                                    dbwt.increment_run(i.run_index + 1, 1);
                                    dbwt.remove_BWT_run(i.run_index);
                                }
                                else
                                {
                                    dbwt.insert_new_run(i.run_index, new_char, 1);
                                    dbwt.remove_BWT_run(i.run_index + 1);
                                }
                            }
                        }
                        else
                        {
                            if (r.upper_merge)
                            {
                                dbwt.increment_run(i.run_index - 1, 1);
                                dbwt.decrement_run(i.run_index, 1);
                            }
                            else
                            {
                                dbwt.insert_new_run(i.run_index, new_char, 1);
                                dbwt.decrement_run(i.run_index + 1, 1);
                            }
                        }
                    }
                    else
                    {
                        if (r.is_last)
                        {
                            if (r.lower_merge)
                            {
                                dbwt.increment_run(i.run_index + 1, 1);
                                dbwt.decrement_run(i.run_index, 1);
                            }
                            else
                            {
                                dbwt.insert_new_run(i.run_index + 1, new_char, 1);
                                dbwt.decrement_run(i.run_index, 1);
                            }
                        }
                        else
                        {
                            int64_t run_length = dbwt.get_run_length(i.run_index);
                            uint8_t c1 = dbwt.access_character_by_run_index(i.run_index);
                            int64_t diff1 = i.position_in_run;
                            int64_t diff2 = run_length - 1 - i.position_in_run;
                            dbwt.decrement_run(i.run_index, run_length - diff1);
                            dbwt.insert_new_run(i.run_index + 1, new_char, 1);
                            dbwt.insert_new_run(i.run_index + 2, c1, diff2);
                        }
                    }
                }
                return r;
            }

            
        };

    }
}
