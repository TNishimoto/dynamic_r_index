#pragma once
#include "./dynamic_bwt.hpp"
#include "./dynamic_isa.hpp"
#include "./dynamic_sampled_sa.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        ////////////////////////////////////////////////////////////////////////////////
        /// @class      DynamicFMIndex
        /// @brief      A dynamic data structure of FM-index. This implementation requires $O(n log n)$ words for the input string of length $n$.
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class DynamicFMIndex
        {
            DynamicBWT dbwt;
            // DynamicISA disa;
            DynamicSampledSA dsa;

        public:
            static inline constexpr uint LOAD_KEY = 99999999;

            using Interval = std::pair<int64_t, int64_t>;
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Initializers and Builders
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            DynamicFMIndex()
            {
                this->dsa.set_BWT(&this->dbwt);
            }
            DynamicFMIndex &operator=(const DynamicFMIndex &) = delete;
            DynamicFMIndex(DynamicFMIndex &&other) noexcept
            {
                this->dbwt = std::move(other.dbwt);
                this->dsa = std::move(other.dsa);
                this->dsa.set_BWT(&this->dbwt);
            };
            DynamicFMIndex &operator=(DynamicFMIndex &&other) noexcept
            {
                if (this != &other)
                {
                    this->dbwt = std::move(other.dbwt);
                    this->dsa = std::move(other.dsa);
                    this->dsa.set_BWT(&this->dbwt);
                }
                return *this;
            };

            DynamicBWT *_get_dbwt_pointer()
            {
                return &this->dbwt;
            }
            DynamicSampledSA *_get_dsa_pointer()
            {
                return &this->dsa;
            }

            uint64_t get_alphabet_size() const
            {
                return this->dbwt.get_alphabet_size();
            }
            std::vector<uint8_t> get_alphabet() const
            {
                return this->dbwt.get_alphabet();
            }

            uint64_t get_end_marker() const
            {
                return this->dbwt.get_end_marker();
            }

            uint64_t get_samling_interval() const
            {
                return this->dsa.get_sampling_interval();
            }

            uint64_t get_sampled_suffix_array_values_count() const
            {
                return this->dsa.get_sampled_suffix_array_values_count();
            }

            uint64_t size_in_bytes() const
            {
                return this->dbwt.size_in_bytes() + this->dsa.size_in_bytes();
            }
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log1 = this->dbwt.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->dsa.get_memory_usage_info(message_paragraph + 1);

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicFMIndex: " + std::to_string(this->size_in_bytes()) + " bytes =");
                for (std::string &s : log1)
                {
                    r.push_back(s);
                }
                for (std::string &s : log2)
                {
                    r.push_back(s);
                }

                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");
                return r;
            }
            void print_memory_usage(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log = this->get_memory_usage_info(message_paragraph);
                for (std::string &s : log)
                {
                    std::cout << s << std::endl;
                }
            }
            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicFMIndex):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: " << this->size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet size: " << this->dbwt.get_alphabet_size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Sampling interval: " << this->dsa.get_sampling_interval() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The number of sampled sa-values: " << this->dsa.get_sampled_suffix_array_values_count() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The density of sampled sa-values: " << (this->size() / this->dsa.get_sampled_suffix_array_values_count()) << std::endl;

                this->dbwt.print_statistics(message_paragraph + 1);
                this->dsa.print_statistics(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            void print_light_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicFMIndex):" << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: \t\t\t\t\t" << this->size() << std::endl;
                if (this->text_size() < 1000)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text: \t\t\t\t\t" << stool::DebugPrinter::to_visible_string(this->get_text_str()) << std::endl;
                }
                else
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text: \t\t\t\t\t" << "[Omitted]" << std::endl;
                }
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet size: \t\t\t\t" << this->get_alphabet_size() << std::endl;
                auto alphabet = this->get_alphabet();
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet: \t\t\t\t\t" << stool::DebugPrinter::to_integer_string_with_characters(alphabet) << std::endl;

                if (this->text_size() < 1000)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "BWT: \t\t\t\t\t\t" << stool::DebugPrinter::to_visible_string(this->get_bwt_str()) << std::endl;
                }
                else
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "BWT: \t\t\t\t\t\t" << "[Omitted]" << std::endl;
                }

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Sampling interval for Sampled suffix array: \t" << this->get_samling_interval() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The number of sampled sa-values: \t\t" << this->get_sampled_suffix_array_values_count() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Average sampling interval: \t\t\t" << (this->size() / this->get_sampled_suffix_array_values_count()) << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            void print_content(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Content(DynamicFMIndex):" << std::endl;
                this->dbwt.print_content(message_paragraph + 1);
                this->dsa.print_content(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            uint8_t access(int64_t i) const
            {
                return this->dbwt.access(i);
            }
            int64_t rank(uint8_t c, int64_t i) const
            {
                return this->dbwt.rank(c, i);
            }
            int64_t select(uint8_t c, int64_t ith) const
            {
                return this->dbwt.select(c, ith);
            }

            static void save(DynamicFMIndex &item, std::ofstream &os, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Writing Dynamic FM-index..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t key = DynamicFMIndex::LOAD_KEY;
                os.write(reinterpret_cast<const char *>(&key), sizeof(uint64_t));

                DynamicBWT::save(item.dbwt, os);
                DynamicSampledSA::save(item.dsa, os);

                uint64_t text_size = item.size();
                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }
            }
            uint64_t text_size() const
            {
                return this->dbwt.size();
            }

            static DynamicFMIndex build_from_data(std::ifstream &ifs, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic FM-index from Data..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t _key = 0;
                ifs.read(reinterpret_cast<char *>(&_key), sizeof(uint64_t));
                if (_key != DynamicFMIndex::LOAD_KEY)
                {
                    throw std::runtime_error("This data is not Dynamic FM-index!");
                }

                auto tmp1 = DynamicBWT::build_from_data(ifs);
                DynamicFMIndex r;
                r.dbwt.swap(tmp1);

                auto tmp2 = DynamicSampledSA::build_from_data(ifs, &r.dbwt);
                r.dsa.swap(tmp2);

                uint64_t text_size = r.size();

                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }

            /*
            static DynamicFMIndex build(const std::vector<uint8_t> &bwt, const std::vector<uint8_t> &alphabet, const std::vector<uint64_t> &isa)
            {
                return DynamicFMIndex::build(bwt, alphabet, isa, DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL);
            }
            */

            static DynamicFMIndex build(const std::vector<uint8_t> &bwt, const std::vector<uint8_t> &alphabet, const std::vector<uint64_t> &isa, uint64_t sampling_interval_of_SA = DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                if (message_paragraph >= 0 && bwt.size() > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic FM-index from BWT and ISA..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicFMIndex r;
                DynamicBWT _dbwt = DynamicBWT::build(bwt, alphabet, stool::Message::add_message_paragraph(message_paragraph));
                r.dbwt.swap(_dbwt);

                DynamicSampledSA _dsa = DynamicSampledSA::build(isa, &r.dbwt, sampling_interval_of_SA, stool::Message::add_message_paragraph(message_paragraph));
                r.dsa.swap(_dsa);

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && bwt.size() > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)bwt.size()) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }

            static DynamicFMIndex build(const std::vector<uint8_t> &bwt, const std::vector<uint8_t> &alphabet, uint64_t sampling_interval_of_SA = DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0 && bwt.size() > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic FM-index from BWT..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                stool::bwt::LFDataStructure lfds = stool::bwt::LFDataStructure::build(bwt, stool::Message::add_message_paragraph(message_paragraph));
                stool::bwt::BackwardISA<stool::bwt::LFDataStructure> bisa;
                bisa.set(&lfds, lfds.get_end_marker_position(), lfds.get_text_size());

                DynamicFMIndex r;
                DynamicBWT _dbwt = DynamicBWT::build(bwt, alphabet, stool::Message::add_message_paragraph(message_paragraph));
                r.dbwt.swap(_dbwt);

                DynamicSampledSA _dsa = DynamicSampledSA::build(bisa, &r.dbwt, sampling_interval_of_SA, stool::Message::add_message_paragraph(message_paragraph));
                r.dsa.swap(_dsa);

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && bwt.size() > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)bwt.size()) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }
            /*
            static DynamicFMIndex build(const std::vector<uint8_t> &bwt, const std::vector<uint8_t> &alphabet)
            {
                return DynamicFMIndex::build(bwt, alphabet, DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL);
            }
            */

            void swap(DynamicFMIndex &item)
            {
                this->dbwt.swap(item.dbwt);
                this->dsa.swap(item.dsa);
                item.dsa.set_BWT(&item.dbwt);
                this->dsa.set_BWT(&this->dbwt);
            }
            void clear(int message_paragraph = stool::Message::NO_MESSAGE)
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Clearing Dynamic FM-index..." << std::endl;
                }
                this->dbwt.clear(stool::Message::add_message_paragraph(message_paragraph));
                // this->disa.clear();
                this->dsa.clear();
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Finished. [Dynamic FM-index]" << std::endl;
                }
            }

            void set_degree(uint64_t degree)
            {
                this->dbwt.clear();
                // this->disa.clear();
                this->dsa.clear();
                // this->disa.set_degree(degree);
                this->dsa.set_degree(degree);
            }

            void initialize(std::vector<uint8_t> &alphabet)
            {
                this->dbwt.initialize(alphabet);
            }
            //@}
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Properties
            ///   The properties of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            uint64_t size() const
            {
                return this->dbwt.size();
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Const Methods
            ///   The const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            BackwardSearchResult backward_search(const Interval &intv, uint8_t c) const
            {
                uint64_t num1 = this->dbwt.rank(c, intv.first - 1);
                uint64_t c_count = this->dbwt.get_c_array().get_c_count(c);
                if (num1 < c_count)
                {
                    uint64_t fst_c_pos = this->dbwt.select(c, num1 + 1);

                    if ((int64_t)fst_c_pos <= intv.second)
                    {
                        uint64_t last_ith = this->dbwt.rank(c, intv.second);

                        uint64_t lst_c_pos = this->dbwt.select(c, last_ith);

                        return BackwardSearchResult(this->dbwt.LF(fst_c_pos), this->dbwt.LF(lst_c_pos));
                    }
                    else
                    {
                        return BackwardSearchResult::create_empty_result();
                    }
                }
                else
                {
                    return BackwardSearchResult::create_empty_result();
                }
            }
            BackwardSearchResult backward_search(const std::vector<uint8_t> &pattern) const
            {
                BackwardSearchResult bsr(0, this->size() - 1);
                // Interval intv(0, this->size() - 1);
                for (int64_t i = pattern.size() - 1; i >= 0; i--)
                {
                    bsr = this->backward_search(bsr.get_sa_interval(), pattern[i]);
                    if (bsr.is_empty())
                    {
                        break;
                    }
                }
                return bsr;
            }
            std::vector<uint64_t> compute_sa_values(const Interval &intv) const
            {
                std::vector<uint64_t> r;
                for (int64_t i = intv.first; i <= intv.second; i++)
                {
                    r.push_back(this->dsa.sa(i));
                }
                return r;
            }
            std::vector<uint64_t> compute_sa_values(const BackwardSearchResult &bsr) const
            {
                if (bsr.is_empty())
                {
                    return std::vector<uint64_t>();
                }
                else
                {
                    return this->compute_sa_values(bsr.get_sa_interval());
                }
            }

            uint64_t count_query(const std::vector<uint8_t> &pattern) const
            {
                BackwardSearchResult bsr = this->backward_search(pattern);
                if (bsr.is_empty())
                {
                    return 0;
                }
                else
                {
                    return bsr.get_sa_interval_size();
                }
            }
            std::vector<uint64_t> locate_query(const std::vector<uint8_t> &pattern) const
            {
                BackwardSearchResult bsr = this->backward_search(pattern);
                return this->compute_sa_values(bsr);
            }

            std::vector<uint8_t> get_bwt() const
            {
                return this->dbwt.get_bwt();
            }
            std::vector<uint8_t> get_text() const
            {
                return this->dbwt.get_text();
            }
            std::string get_bwt_str() const
            {
                return this->dbwt.get_bwt_str();
            }
            std::string get_text_str() const
            {
                return this->dbwt.get_text_str();
            }

            std::vector<uint64_t> get_sa() const
            {
                return this->dsa.get_sa();
            }
            std::vector<uint64_t> get_isa() const
            {
                return this->dsa.get_isa();
            }
            void print_bwt_table() const
            {
                std::vector<uint64_t> sa = this->get_sa();
                std::vector<uint8_t> bwt = this->get_bwt();
                stool::Printer::print_bwt_table(bwt, sa);
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Non-const Methods
            ///   The public non-const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            uint64_t insert_char(int64_t pos, uint8_t c, FMIndexEditHistory &output_history)
            {
                output_history.clear();
                return this->insert_char(pos, c, &output_history);
            }

            uint64_t insert_char(int64_t pos, uint8_t c, FMIndexEditHistory *output_history = nullptr)
            {
                SAIndex isa_of_insertionPosOnText = this->dsa.isa(pos);
                // assert(isa_of_insertionPosOnText == this->dsa.isa(pos));

                uint64_t positionToReplace = isa_of_insertionPosOnText;
                uint64_t prev_isa = this->dbwt.LF(isa_of_insertionPosOnText);
                uint64_t oldPositionToInsert = positionToReplace;

                if (output_history != nullptr)
                {
                    output_history->replaced_sa_index = positionToReplace;
                    output_history->type = EditType::InsertionOfChar;
                    output_history->inserted_string.push_back(c);
                }

                uint8_t oldChar = this->dbwt.access(positionToReplace);
                uint64_t j = prev_isa;
                uint8_t new_letter_L = c;

                this->dbwt.replace_BWT_character(positionToReplace, new_letter_L);
                // this->dbwt.update_C_for_deletion(oldChar);
                // this->dbwt.update_C_for_insertion(new_letter_L);

                uint64_t C_count = this->dbwt.get_c_array().at(new_letter_L);
                if (oldChar < new_letter_L)
                {
                    C_count++;
                }
                uint64_t rank_value = this->dbwt.rank(new_letter_L, positionToReplace);
                if (positionToReplace <= oldPositionToInsert && new_letter_L == oldChar)
                {
                    rank_value++;
                }

                uint64_t positionToInsert = C_count + rank_value - 1;

                if (output_history != nullptr)
                {
                    output_history->inserted_sa_index = positionToInsert;
                    output_history->inserted_sa_indexes.push_back(positionToInsert);
                }

                this->dbwt.insert_BWT_character(positionToInsert, oldChar);
                if (positionToInsert <= j)
                {
                    j++;
                }
                if (positionToInsert <= positionToReplace)
                {
                    positionToReplace++;
                }

                // this->disa.update_for_insertion(positionToInsert, pos);
                this->dsa.update_for_insertion(positionToInsert, pos);

                uint64_t j_prime = this->dbwt.LF(positionToInsert);

                // std::cout << "j = " << j << ", j' = " << j_prime << std::endl;
                if (output_history != nullptr)
                {
                    output_history->first_j = j;
                    output_history->first_j_prime = j_prime;
                }
                std::vector<SAMove> swap_history = reorder_BWT(j, j_prime);
                uint64_t sum = swap_history.size();

                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }

                this->dsa.update_sample_marks(pos + 1);

                return sum;
            }
            /*
            void check_LF() const
            {
                std::vector<uint64_t> sa1 = this->disa.get_sa();
                for (uint64_t i = 0; i < sa1.size(); i++)
                {
                    if(sa1[i] > 0){
                        assert(sa1[this->dbwt.LF(i)] == sa1[i]-1);
                    }else{
                        assert(sa1[this->dbwt.LF(i)] == sa1.size()-1);
                    }
                }
            }
            */
            /*
            void check_SA() const
            {
                //this->check_LF();
                //std::vector<uint64_t> sa1 = this->disa.get_sa();
                std::vector<uint64_t> sa2 = this->dsa.get_sa();
                try
                {
                    stool::equal_check("SA", sa1, sa2);
                }
                catch (std::logic_error e)
                {
                    stool::DebugPrinter::print_integers(sa1, "correct SA");
                    stool::DebugPrinter::print_integers(sa2, "test SA");

                    std::vector<uint64_t> LF_vec;
                    LF_vec.resize(this->dbwt.size());
                    for (uint64_t i = 0; i < LF_vec.size(); i++)
                    {
                        LF_vec[i] = this->dbwt.LF(i);
                    }
                    stool::DebugPrinter::print_integers(LF_vec, "LF");

                    this->dsa.print_info();

                    throw e;
                }
            }
            */

            uint64_t insert_string(int64_t pos, const std::vector<uint8_t> &pattern, FMIndexEditHistory &output_history)
            {
                return insert_string(pos, pattern, &output_history);
            }

            uint64_t insert_string(int64_t pos, const std::vector<uint8_t> &pattern, FMIndexEditHistory *output_history = nullptr)
            {

                SAIndex isa_of_insertionPosOnText = this->dsa.isa(pos);
                // assert(isa_of_insertionPosOnText == this->dsa.isa(pos));
                uint64_t positionToReplace = isa_of_insertionPosOnText;
                uint64_t prev_isa = this->dbwt.LF(isa_of_insertionPosOnText);
                uint64_t oldPositionToInsert = positionToReplace;

                if (output_history != nullptr)
                {
                    output_history->replaced_sa_index = positionToReplace;
                    output_history->type = EditType::InsertionOfString;

                    for (size_t i = 0; i < pattern.size(); i++)
                    {
                        output_history->inserted_string.push_back(pattern[i]);
                    }
                }

                uint8_t oldChar = this->dbwt.access(positionToReplace);
                uint64_t j = prev_isa;
                uint8_t new_letter_L = pattern[pattern.size() - 1];

                this->dbwt.replace_BWT_character(positionToReplace, pattern[pattern.size() - 1]);

                uint64_t C_count = this->dbwt.get_c_array().at(new_letter_L);
                if (oldChar < new_letter_L)
                {
                    C_count++;
                }
                uint64_t rank_value = this->dbwt.rank(new_letter_L, positionToReplace);
                if (positionToReplace <= oldPositionToInsert && new_letter_L == oldChar)
                {
                    rank_value++;
                }

                uint64_t positionToInsert = C_count + rank_value - 1;

                for (int64_t k = pattern.size() - 1; k > 0; k--)
                {
                    new_letter_L = pattern[k - 1];

                    this->dbwt.insert_BWT_character(positionToInsert, new_letter_L);
                    this->dsa.update_for_insertion(positionToInsert, pos);

                    if (output_history != nullptr)
                    {
                        output_history->inserted_sa_indexes.push_back(positionToInsert);
                    }

                    if (positionToInsert <= j)
                    {
                        j++;
                    }
                    if (positionToInsert <= positionToReplace)
                    {
                        positionToReplace++;
                    }

                    oldPositionToInsert = positionToInsert;
                    int64_t C_count = this->dbwt.get_c_array().at(new_letter_L);
                    if (oldChar < new_letter_L)
                    {
                        C_count++;
                    }
                    int64_t rank_value = this->dbwt.rank(new_letter_L, positionToInsert);
                    if (positionToReplace <= oldPositionToInsert && pattern[k - 1] == oldChar)
                        rank_value++;

                    positionToInsert = C_count + rank_value - 1;
                }

                new_letter_L = oldChar;

                if (output_history != nullptr)
                {
                    output_history->inserted_sa_indexes.push_back(positionToInsert);
                }

                this->dbwt.insert_BWT_character(positionToInsert, oldChar);
                // this->dbwt.update_C_for_insertion(oldChar);
                if (positionToInsert <= j)
                {
                    j++;
                }
                if (positionToInsert <= positionToReplace)
                {
                    positionToReplace++;
                }

                // this->disa.update_for_insertion(positionToInsert, pos);
                this->dsa.update_for_insertion(positionToInsert, pos);

                int64_t j_prime = this->dbwt.LF(positionToInsert);

                // std::cout << "j = " << j << ", j' = " << j_prime << std::endl;

                std::vector<SAMove> swap_history = reorder_BWT(j, j_prime);

                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }

                this->dsa.update_sample_marks(pos + pattern.size());

                return 0;
            }

            uint64_t delete_char(int64_t pos, FMIndexEditHistory &output_history)
            {
                output_history.clear();
                return this->delete_char(pos, &output_history);
            }

            uint64_t delete_char(int64_t pos, FMIndexEditHistory *output_history = nullptr)
            {

                SAIndex isa_pos = this->dsa.isa(pos);
                // assert(isa_pos = this->dsa.isa(pos));

                TextIndex pointer = pos + 1 < (int64_t)this->size() ? pos + 1 : 0;

                SAIndex isa_pos_succ = this->dsa.isa(pointer);
                // assert(isa_pos_succ == this->dsa.isa(pointer));
                uint8_t newChar = this->dbwt.access(isa_pos);

                SAIndex isa_pos_prev = this->dbwt.LF(isa_pos);
                // uint8_t oldChar = this->dbwt.access(isa_pos_succ);

                this->dbwt.replace_BWT_character(isa_pos_succ, newChar);
                this->dbwt.remove_BWT_character(isa_pos);
                // this->dbwt.update_C_for_deletion(oldChar);

                SAIndex new_isa_pos_succ = isa_pos_succ <= isa_pos ? isa_pos_succ : isa_pos_succ - 1;
                SAIndex new_isa_pos_prev = isa_pos_prev <= isa_pos ? isa_pos_prev : isa_pos_prev - 1;

                if (output_history != nullptr)
                {
                    output_history->replaced_sa_index = isa_pos_succ;
                    output_history->inserted_sa_index = isa_pos;
                }
                // this->disa.update_for_deletion(isa_pos);
                this->dsa.update_for_deletion(isa_pos, pos);

                /*
                auto _sa = this->disa.get_sa();
                stool::DebugPrinter::print_integers(_sa, "c SA");
                this->dsa.print_info();
                */

                std::vector<SAMove> swap_history = reorder_BWT(new_isa_pos_prev, this->dbwt.LF(new_isa_pos_succ));
                uint64_t sum = swap_history.size();

                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }

                this->dsa.update_sample_marks(pos);

                return sum;
            }
            uint64_t delete_string(const int64_t pos, int64_t len, FMIndexEditHistory &output_history)
            {
                return delete_string(pos, len, &output_history);
            }
            /*
            uint64_t delete_string(const int64_t pos, int64_t len, FMIndexEditHistory *output_history = nullptr)
            {

                TextIndex pointer = pos + len < (int64_t)this->size() ? pos + len : 0;

                uint64_t positionToReplace = this->dsa.isa(pointer);
                // assert(positionToReplace == this->dsa.isa(pointer));

                SAIndex isa_pos2 = this->dsa.isa(pos);
                // assert(isa_pos2 == this->dsa.isa(pos));

                uint8_t current_letter;
                uint64_t positionToDelete = this->dbwt.LF(positionToReplace);
                uint64_t positionToDeleteOnText = pointer - 1;
                uint64_t j = this->dbwt.LF(isa_pos2);
                uint8_t new_char = this->dbwt.access(isa_pos2);
                uint8_t old_char = this->dbwt.access(positionToReplace);

                if (output_history != nullptr)
                {
                    output_history->type = EditType::DeletionOfString;
                    output_history->replaced_sa_index = positionToReplace;
                }

                uint64_t tmp_rank;
                for (int64_t k = len - 1; k > 0; k--)
                {

                    current_letter = this->dbwt.access(positionToDelete);
                    // Computes the rank before we delete at this position!
                    tmp_rank = this->dbwt.rank(current_letter, positionToDelete);
                    if (positionToReplace <= positionToDelete && current_letter == old_char)
                    {
                        tmp_rank--;
                    }

                    // std::cout << "bwt: " << this->dbwt.get_bwt_str() <<  ", del_pos = " << positionToDelete << std::endl;

                    if (output_history != nullptr)
                    {
                        output_history->deleted_sa_indexes.push_back(positionToDelete);
                    }
                    std::cout << "positionToDelete = " << positionToDelete << std::endl;

                    this->dbwt.remove_BWT_character(positionToDelete);
                    // this->disa.update_for_deletion(positionToDelete);
                    this->dsa.update_for_deletion(positionToDelete, positionToDeleteOnText);

                    if (positionToDelete < j)
                    {
                        j--;
                    }
                    if (positionToDelete < positionToReplace)
                    {
                        positionToReplace--;
                    }

                    int64_t c_value = this->dbwt.get_c_array().at(current_letter);
                    if (old_char < current_letter)
                    {
                        c_value--;
                    }

                    positionToDelete = c_value + tmp_rank - 1;
                    positionToDeleteOnText--;
                }

                current_letter = this->dbwt.access(positionToDelete);

                std::cout << "positionToDelete = " << positionToDelete << std::endl;

                this->dbwt.remove_BWT_character(positionToDelete);
                this->dsa.update_for_deletion(positionToDelete, positionToDeleteOnText);

                if (positionToDelete < j)
                {
                    j--;
                }
                if (positionToDelete < positionToReplace)
                {
                    positionToReplace--;
                }

                this->dbwt.replace_BWT_character(positionToReplace, new_char);

                tmp_rank = this->dbwt.rank(new_char, positionToReplace);
                uint64_t j_prime = this->dbwt.get_c_array().at(new_char) + tmp_rank - 1;

                if (output_history != nullptr)
                {
                    output_history->deleted_sa_indexes.push_back(positionToDelete);

                    output_history->first_j = j;
                    output_history->first_j_prime = j_prime;
                }

                std::cout << "j = " << j << ", j' = " << j_prime << std::endl;
                std::vector<SAMove> swap_history = reorder_BWT(j, j_prime);
                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }
                this->dsa.update_sample_marks(pos);
                return 0;
            }
            */

            uint64_t delete_string(const int64_t pos, int64_t len, FMIndexEditHistory *output_history = nullptr)
            {

                TextIndex pointer = pos + len < (int64_t)this->size() ? pos + len : 0;

                uint64_t positionToReplace = this->dsa.isa(pointer);
                // assert(positionToReplace == this->dsa.isa(pointer));

                SAIndex isa_pos2 = this->dsa.isa(pos);
                // assert(isa_pos2 == this->dsa.isa(pos));

                uint8_t current_letter;
                uint64_t positionToDelete = this->dbwt.LF(positionToReplace);
                uint64_t positionToDeleteOnText = pointer - 1;
                //uint64_t j = this->dbwt.LF(isa_pos2);
                uint8_t new_char = this->dbwt.access(isa_pos2);
                uint8_t old_char = this->dbwt.access(positionToReplace);

                if (output_history != nullptr)
                {
                    output_history->type = EditType::DeletionOfString;
                    output_history->replaced_sa_index = positionToReplace;
                }

                this->dbwt.replace_BWT_character(positionToReplace, new_char);

                uint64_t tmp_rank;
                for (int64_t k = len - 1; k >= 0; k--)
                {
                    uint64_t next_position_to_delete = UINT64_MAX;
                    next_position_to_delete = this->dbwt.LF_for_deletion(positionToDelete, new_char, positionToReplace, positionToDelete);


                    /*
                    current_letter = this->dbwt.access(positionToDelete);
                    // Computes the rank before we delete at this position!
                    tmp_rank = this->dbwt.rank(current_letter, positionToDelete);
                    if (positionToReplace <= positionToDelete && current_letter == old_char)
                    {
                        tmp_rank--;
                    }
                    */

                    // std::cout << "bwt: " << this->dbwt.get_bwt_str() <<  ", del_pos = " << positionToDelete << std::endl;

                    if (output_history != nullptr)
                    {
                        output_history->deleted_sa_indexes.push_back(positionToDelete);
                    }
                    //std::cout << "positionToDelete = " << positionToDelete << std::endl;

                    this->dbwt.remove_BWT_character(positionToDelete);
                    // this->disa.update_for_deletion(positionToDelete);
                    this->dsa.update_for_deletion(positionToDelete, positionToDeleteOnText);

                    if (positionToDelete < positionToReplace)
                    {
                        positionToReplace--;
                    }
                    /*
                    int64_t c_value = this->dbwt.get_c_array().at(current_letter);
                    if (old_char < current_letter)
                    {
                        c_value--;
                    }

                    positionToDelete = c_value + tmp_rank - 1;
                    */
                    // std::cout << "P = " << positionToDelete << " -> " << next_position_to_delete << "/" << len << std::endl;
                    positionToDelete = next_position_to_delete;

                    positionToDeleteOnText--;
                }
                uint64_t j = positionToDelete;
                uint64_t j_prime = this->dbwt.LF(positionToReplace);

                /*

                current_letter = this->dbwt.access(positionToDelete);

                this->dbwt.remove_BWT_character(positionToDelete);
                this->dsa.update_for_deletion(positionToDelete, positionToDeleteOnText);

                if (positionToDelete < j)
                {
                    j--;
                }
                if (positionToDelete < positionToReplace)
                {
                    positionToReplace--;
                }

                //this->dbwt.replace_BWT_character(positionToReplace, new_char);

                tmp_rank = this->dbwt.rank(new_char, positionToReplace);
                uint64_t j_prime = this->dbwt.get_c_array().at(new_char) + tmp_rank - 1;
                */

                if (output_history != nullptr)
                {
                    output_history->deleted_sa_indexes.push_back(positionToDelete);

                    output_history->first_j = j;
                    output_history->first_j_prime = j_prime;
                }

                //std::cout << "j = " << j << ", j' = " << j_prime << std::endl;
                std::vector<SAMove> swap_history = reorder_BWT(j, j_prime);
                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }
                this->dsa.update_sample_marks(pos);
                return 0;
            }

            //@}
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Private Non-const Methods
            ///   The private non-const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
        private:
            void move_row(int64_t j, int64_t j_prime)
            {
                uint8_t j_char = this->dbwt.access(j);

                this->dbwt.remove_BWT_character(j);
                this->dbwt.insert_BWT_character(j_prime, j_char);
            }
            void single_reorder_BWT(int64_t &j, int64_t &j_prime)
            {
                int64_t new_j = this->dbwt.LF(j);

                this->move_row(j, j_prime);
                // this->disa.move_update(j, j_prime);
                this->dsa.move_update(j, j_prime);

                j = new_j;
                j_prime = this->dbwt.LF(j_prime);
            }

            std::vector<SAMove> reorder_BWT(int64_t j, int64_t j_prime)
            {

                std::vector<SAMove> swap_history;
                int64_t counter = 0;

                while (j != j_prime)
                {
                    // std::cout << "REORDER: j = " << j << ", j' = " << j_prime << std::endl;
                    swap_history.push_back(SAMove(j, j_prime));

                    this->single_reorder_BWT(j, j_prime);

                    counter++;

                    if (counter > (int64_t)this->size())
                    {
                        throw std::logic_error("Error: modify_SA_for_insertion");
                    }
                }
                // swap_history.push_back(SAMove(j, j_prime));

                return swap_history;
            }

            //@}
            /*
            uint64_t insert_char(int64_t pos, uint8_t c, FMIndexEditHistory *output_history = nullptr)
            {

                SAIndex isa_of_insertionPosOnText = this->disa.isa(pos);
                assert(c != this->dbwt.end_marker);
                // FMIndexEditHistory sa_edit_history;

                int64_t replacePositionOnBWT = isa_of_insertionPosOnText;
                int64_t prev_isa = this->dbwt.LF(isa_of_insertionPosOnText);

                uint8_t oldChar = this->dbwt.access(replacePositionOnBWT);
                int64_t insertionPosOnBWT = this->dbwt.C[c] + this->dbwt.rank(c, replacePositionOnBWT - 1);

                this->dbwt.replace_BWT_character(replacePositionOnBWT, c);
                this->dbwt.update_C_for_deletion(oldChar);
                this->dbwt.update_C_for_insertion(c);

                this->dbwt.insert_BWT_character(insertionPosOnBWT, oldChar);
                this->dbwt.update_C_for_insertion(oldChar);

                if (output_history != nullptr)
                {
                    output_history->inserted_string.push_back(c);
                    output_history->replaced_sa_index = replacePositionOnBWT;
                    output_history->inserted_sa_index = insertionPosOnBWT;
                }

                this->disa.update_for_insertion(insertionPosOnBWT, pos);

                // int64_t previousPos = insertionPosOnText - 1;
                int64_t j = prev_isa < insertionPosOnBWT ? prev_isa : prev_isa + 1;
                int64_t j_prime = this->dbwt.LF(insertionPosOnBWT);

                std::vector<SAMove> swap_history = reorder_BWT(j, j_prime);

                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }
                return 0;
            }
            */
        };
    }
}
