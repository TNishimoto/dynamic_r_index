#pragma once
#include "dynamic_r_index/update/r_index_helper_for_update.hpp"
namespace stool
{
    namespace dynamic_r_index
    {

        /**
         * @brief Dynamic r-index supporting pattern matching queries and text updates
         * 
         * This class implements the dynamic r-index data structure, which provides:
         * - Pattern matching: Count and locate occurrences of patterns
         * - Dynamic updates: Insert and delete substrings
         * - Space efficiency: O(r log n) bytes where r is the number of BWT runs
         * 
         * The dynamic r-index is particularly efficient for highly repetitive texts
         * where the number of BWT runs r is much smaller than the text length n.
         * 
         * @note Time complexities:
         *   - Count query: O(m log σ log n) where m is pattern length
         *   - Locate query: O((m + occ) log σ log n) where occ is number of occurrences
         *   - Insert/Delete: Average O((m + L_avg) log σ log n) where L_avg is average LCP
         * 
         * \ingroup StringIndexes
         * \ingroup DynamicRIndexes
         */
        class DynamicRIndex
        {
        private:
            DynamicRLBWT dbwt;
            DynamicPhi disa;

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Properties
            ///   The properties of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{

        public:
            static inline constexpr uint32_t LOAD_KEY = 8888888;

            DynamicRIndex()
            {
            }
            DynamicRIndex &operator=(const DynamicRIndex &) = delete;
            DynamicRIndex(DynamicRIndex &&) noexcept = default;
            DynamicRIndex &operator=(DynamicRIndex &&) noexcept = default;

            /**
             * @brief Get the alphabet size (number of distinct characters)
             * @return The alphabet size including the end marker
             */
            uint64_t get_alphabet_size() const
            {
                return this->dbwt.get_alphabet_size();
            }

            /**
             * @brief Get the effective alphabet (distinct characters in the text)
             * @return Vector of distinct characters in lexicographic order
             */
            std::vector<uint8_t> get_effective_alphabet() const
            {
                return this->dbwt.get_effective_alphabet();
            }
            
            /**
             * @brief Get the end marker character
             * @return The end marker character value
             */
            uint64_t get_end_marker() const
            {
                return this->dbwt.get_end_marker();
            }

            /**
             * @brief Get the number of runs in the BWT
             * @return The number of runs in the current BWT
             */
            uint64_t run_count() const
            {
                return this->dbwt.run_count();
            }

            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicRIndex):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: " << this->size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet size: " << this->dbwt.get_alphabet_size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The number of runs in BWT: " << this->run_count() << std::endl;

                this->dbwt.print_statistics(message_paragraph + 1);
                this->disa.print_statistics(message_paragraph + 1);
            }
            void print_light_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicRIndex):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: \t\t\t\t\t" << this->size() << std::endl;
                if(this->text_size() < 1000){
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text: \t\t\t\t\t" << stool::ConverterToString::to_visible_string(this->get_text_str()) << std::endl;
                }else{
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text: \t\t\t\t\t" << "[Omitted]" << std::endl;
                }
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet size: \t\t\t\t" << this->get_alphabet_size() << std::endl;
                auto alphabet = this->get_effective_alphabet();
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet: \t\t\t\t\t" << stool::ConverterToString::to_integer_string_with_characters(alphabet) << std::endl;

            

                if(this->text_size() < 1000){
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "BWT: \t\t\t\t\t\t" << stool::ConverterToString::to_visible_string(this->get_bwt_str()) << std::endl;
                }else{
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "BWT: \t\t\t\t\t\t" << "[Omitted]" << std::endl;
                }

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The number of runs in BWT: \t\t\t" << this->run_count() << std::endl;
            }

            const DynamicRLBWT &get_dynamic_rlbwt() const
            {
                return this->dbwt;
            }
            const DynamicPhi &get_dynamic_phi() const
            {
                return this->disa;
            }
            void clear()
            {
                this->dbwt.clear();
                this->disa.clear();
            }
            void swap(DynamicRIndex &item)
            {
                this->dbwt.swap(item.dbwt);
                this->disa.swap(item.disa);
            }

            void verify(int mode = 0) const
            {
                if (this->dbwt.run_count() != this->disa.run_count())
                {
                    throw std::logic_error("Verification Error: DBWT and DISA have the different size!");
                }

                this->dbwt.verify(mode);
                this->disa.verify(mode);
            }

            /**
             * @brief Save the dynamic r-index to a binary file
             * @param item The DynamicRIndex instance to save
             * @param os Output file stream (must be opened in binary mode)
             * @param message_paragraph Message indentation level for progress output
             */
            static void store_to_file(DynamicRIndex &item, std::ofstream &os, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Writing Dynamic r-index..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t key = DynamicRIndex::LOAD_KEY;
                os.write(reinterpret_cast<const char *>(&key), sizeof(uint64_t));

                DynamicRLBWT::store_to_file(item.dbwt, os);

                DynamicPhi::store_to_file(item.disa, os);

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
            static DynamicRIndex build_from_text(const std::vector<uint8_t> &text_with_end_marker, const std::vector<uint8_t> &alphabet_with_end_marker, int message_paragraph = stool::Message::NO_MESSAGE)
            {
                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text_with_end_marker, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text_with_end_marker, sa, stool::Message::NO_MESSAGE);
                DynamicRIndex r = stool::dynamic_r_index::DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, message_paragraph);

                return r;
            }

            /**
             * @brief Load a dynamic r-index from a binary file
             * @param ifs Input file stream (must be opened in binary mode)
             * @param message_paragraph Message indentation level for progress output
             * @return A new DynamicRIndex instance loaded from the file
             * @throws std::runtime_error if the file is not a valid dynamic r-index
             */
            static DynamicRIndex load_from_file(std::ifstream &ifs, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic R-index from Data..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t _key = 0;
                ifs.read(reinterpret_cast<char *>(&_key), sizeof(uint64_t));
                if (_key != DynamicRIndex::LOAD_KEY)
                {
                    throw std::runtime_error("This data is not Dynamic r-index!");
                }

                DynamicRLBWT tmp_dbwt = DynamicRLBWT::load_from_file(ifs);
                DynamicPhi tmp_disa = DynamicPhi::load_from_file(ifs);

                DynamicRIndex r;
                r.dbwt.swap(tmp_dbwt);
                r.disa.swap(tmp_disa);

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
            uint64_t _j = UINT64_MAX;
            uint64_t _j_prime = UINT64_MAX;
            */

            /**
             * @brief Get the text length
             * @return The length of the indexed text (including end marker)
             */
            uint64_t size() const
            {
                return this->dbwt.text_size();
            }
            
            /**
             * @brief Get the BWT as a byte vector
             * @return The Burrows-Wheeler Transform of the text
             */
            std::vector<uint8_t> get_bwt() const
            {
                return this->dbwt.get_bwt();
            }
            
            /**
             * @brief Get the original text as a byte vector
             * @return The original text (including end marker)
             */
            std::vector<uint8_t> get_text() const
            {
                return this->dbwt.get_text();
            }
            
            /**
             * @brief Get the BWT as a string
             * @return The BWT represented as a string
             */
            std::string get_bwt_str() const
            {
                return this->dbwt.get_bwt_str();
            }
            
            /**
             * @brief Get the original text as a string
             * @return The original text represented as a string
             */
            std::string get_text_str() const
            {
                return this->dbwt.get_text_str();
            }

            std::vector<uint64_t> get_sa() const
            {
                return this->disa.get_sa();
            }
            std::vector<uint64_t> get_isa() const
            {
                return this->disa.get_isa();
            }
            std::vector<uint64_t> get_sampling_sa() const
            {
                return this->disa.get_sampling_sa(this->dbwt);
            }
            uint64_t text_size() const
            {
                return this->dbwt.text_size();
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Initializers and Builders
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            static DynamicRIndex build_r_index_of_end_marker(const std::vector<uint8_t> &alphabet)
            {
                std::vector<uint8_t> bwt;
                bwt.push_back(alphabet[0]);
                return DynamicRIndex::build_from_BWT(bwt, alphabet, stool::Message::NO_MESSAGE);
            }

            /**
             * @brief Build a dynamic r-index from a BWT
             * @param bwt The Burrows-Wheeler Transform
             * @param alphabet The alphabet (distinct characters including end marker)
             * @param message_paragraph Message indentation level for progress output
             * @return A new DynamicRIndex instance
             * @note Time complexity: O(n log σ log n) where n is text length
             */
            static DynamicRIndex build_from_BWT(const std::vector<uint8_t> &bwt, const std::vector<uint8_t> &alphabet, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = bwt.size();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic r-index from BWT..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicRIndex r;
                {
                    std::vector<uint64_t> sampled_last_sa_indexes;
                    std::vector<uint64_t> sampled_first_sa_indexes;

                    {
                        stool::rlbwt2::RLE<uint8_t> static_rlbwt = stool::rlbwt2::RLE<uint8_t>::build_from_BWT(bwt, stool::Message::increment_paragraph_level(message_paragraph));
                        SamplingSATBuilder::build(static_rlbwt, sampled_first_sa_indexes, sampled_last_sa_indexes, stool::Message::increment_paragraph_level(message_paragraph));
                    }

                    DynamicPhi tmp_disa = DynamicPhi::build_from_sampled_sa_indexes(sampled_last_sa_indexes, sampled_first_sa_indexes, text_size, stool::Message::increment_paragraph_level(message_paragraph));
                    r.disa.swap(tmp_disa);
                }

                {
                    DynamicRLBWT tmp_dbwt = DynamicRLBWT::build_from_BWT(bwt, alphabet, stool::Message::increment_paragraph_level(message_paragraph));
                    r.dbwt.swap(tmp_dbwt);
                }

                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;

                // this->disa.build(sampled_last_sa_indexes, sampled_first_sa_indexes, this->dbwt.text_size(), stool::Message::increment_paragraph_level(message_paragraph));
            }

            DynamicRLBWT *_get_dbwt_pointer()
            {
                return &this->dbwt;
            }
            DynamicPhi *_get_dsa_pointer()
            {
                return &this->disa;
            }

            /**
             * @brief Build a dynamic r-index from a BWT file
             * @param file_path Path to the BWT file
             * @param message_paragraph Message indentation level for progress output
             * @return A new DynamicRIndex instance
             */
            static DynamicRIndex build_from_BWT_file(std::string file_path, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic r-index from BWT file..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicRIndex r;
                uint64_t text_size = 0;
                {
                    std::vector<uint64_t> sampled_last_sa_indexes;
                    std::vector<uint64_t> sampled_first_sa_indexes;
                    {
                        stool::rlbwt2::RLE<uint8_t> static_rlbwt = stool::rlbwt2::RLE<uint8_t>::build_from_file(file_path, message_paragraph);
                        text_size = static_rlbwt.str_size();
                        SamplingSATBuilder::build(static_rlbwt, sampled_first_sa_indexes, sampled_last_sa_indexes, stool::Message::increment_paragraph_level(message_paragraph));
                    }

                    DynamicPhi tmp_disa = DynamicPhi::build_from_sampled_sa_indexes(sampled_last_sa_indexes, sampled_first_sa_indexes, text_size, stool::Message::increment_paragraph_level(message_paragraph));
                    r.disa.swap(tmp_disa);
                }

                {
                    std::vector<uint8_t> rlbwt_char_vector;
                    std::vector<uint64_t> rlbwt_run_length_vector;
                    stool::RLEIO::build_RLBWT_from_BWT_file(file_path, rlbwt_char_vector, rlbwt_run_length_vector, stool::Message::increment_paragraph_level(message_paragraph));
                    std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(rlbwt_char_vector);
                    DynamicRLBWT tmp_dbwt = DynamicRLBWT::build_from_RLBWT(rlbwt_char_vector, rlbwt_run_length_vector, alphabet, stool::Message::increment_paragraph_level(message_paragraph));
                    r.dbwt.swap(tmp_dbwt);
                }

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
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Private const Methods
            ///   The private const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
        private:
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public const Methods
            ///   The public const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{

        public:
            uint8_t access_character_of_text(uint64_t i) const
            {
                if (i + 1 < this->text_size())
                {
                    uint64_t j = this->disa.isa(i + 1, this->dbwt);
                    return this->dbwt.access(j);
                }
                else
                {
                    return this->dbwt.get_end_marker();
                }
            }
            std::vector<uint8_t> access_substring_of_text(uint64_t i, uint64_t len) const
            {
                uint64_t j;
                if (i + len < this->text_size())
                {
                    j = this->disa.isa(i + len, this->dbwt);
                }
                else
                {
                    j = this->dbwt.get_end_marker_pos();
                }

                std::vector<uint8_t> output;
                output.resize(len, UINT8_MAX);
                for (int64_t x = len - 1; x >= 0; x--)
                {
                    output[x] = this->dbwt.access(j);
                    j = this->dbwt.LF(j);
                }
                return output;
            }

            void print_bwt_table() const
            {
                std::vector<uint64_t> sa = this->get_sa();
                std::vector<uint8_t> bwt = this->get_bwt();
                stool::DebugPrinter::print_bwt_table(bwt, sa);
            }

            void print_content(int message_paragraph = 1) const
            {
                this->dbwt.print_content();
                this->disa.print_content(stool::Message::increment_paragraph_level(message_paragraph));
            }

            uint64_t size_in_bytes() const
            {
                return sizeof(*this) + this->disa.size_in_bytes() + this->dbwt.size_in_bytes();
            }

            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log1 = this->dbwt.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->disa.get_memory_usage_info(message_paragraph + 1);

                uint64_t total_bytes = this->size_in_bytes();
                uint64_t run_count = this->dbwt.run_count();
                uint64_t byte_per_run = total_bytes / run_count;

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=Dynamic r-index: " + std::to_string(total_bytes) + " bytes (" + std::to_string(byte_per_run) + " bytes per run in BWT) =");
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + " The number of runs: " + std::to_string(run_count));
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

            RunPosition backward_search_sub1(RunPosition rp, uint8_t c) const
            {
                uint8_t c_rp = this->dbwt.access_character_by_run_index(rp.run_index);

                if (c == c_rp)
                {
                    return rp;
                }
                else
                {
                    uint64_t rank = this->dbwt.rank_on_first_characters_of_RLBWT(c, rp.run_index);
                    uint64_t count = this->dbwt.c_run_count(c);
                    if (rank < count)
                    {
                        uint64_t y = this->dbwt.select_on_first_characters_of_RLBWT(rank, c);
                        return RunPosition(y, 0);
                    }
                    else
                    {
                        return RunPosition::create_null_value();
                    }
                }
            }
            RunPosition backward_search_sub2(RunPosition rp, uint8_t c) const
            {
                uint8_t c_rp = this->dbwt.access_character_by_run_index(rp.run_index);

                if (c == c_rp)
                {
                    return rp;
                }
                else
                {
                    uint64_t rank = this->dbwt.rank_on_first_characters_of_RLBWT(c, rp.run_index);
                    if (rank == 0)
                    {
                        return RunPosition::create_null_value();
                    }
                    else
                    {
                        uint64_t y = this->dbwt.select_on_first_characters_of_RLBWT(rank - 1, c);
                        uint64_t d = this->dbwt.get_run_length(y);
                        return RunPosition(y, d - 1);
                    }
                }
            }

            BackwardSearchResult backward_search(const BackwardSearchResult &bsr, uint8_t c) const
            {
                RunPosition bx = this->dbwt.to_run_position(bsr.b);
                RunPosition ex = this->dbwt.to_run_position(bsr.e);
                uint8_t c_bx = this->dbwt.access_character_by_run_index(bx.run_index);
                uint8_t c_ex = this->dbwt.access_character_by_run_index(ex.run_index);

                if (c == c_bx)
                {

                    int64_t next_b = this->dbwt.LF(bx);
                    int64_t next_sa_b_ = bsr.sa_b_ - 1;
                    int64_t next_e;
                    if (c_bx == c_ex)
                    {
                        next_e = this->dbwt.LF(ex);
                    }
                    else
                    {
                        RunPosition ey = this->backward_search_sub2(ex, c);
                        next_e = this->dbwt.LF(ey);
                    }
                    return BackwardSearchResult(next_b, next_e, next_sa_b_);
                }
                else
                {

                    RunPosition by = this->backward_search_sub1(bx, c);
                    if (c == c_ex)
                    {

                        int64_t next_b = this->dbwt.LF(by);
                        int64_t next_e = this->dbwt.LF(ex);                        
                        int64_t next_sa_b_ = this->disa.get_sampled_first_sa_value(by.run_index) - 1;
                        if(next_sa_b_ == -1){
                            next_sa_b_ = this->text_size() - 1;
                        }
                        return BackwardSearchResult(next_b, next_e, next_sa_b_);
                    }
                    else
                    {
                        if (by.run_index <= ex.run_index)
                        {

                            RunPosition ey = this->backward_search_sub2(ex, c);
                            int64_t next_b = this->dbwt.LF(by);
                            int64_t next_e = this->dbwt.LF(ey);

                            int64_t next_sa_b_ = this->disa.get_sampled_first_sa_value(by.run_index) - 1;
                            if(next_sa_b_ == -1){
                                next_sa_b_ = this->text_size() - 1;
                            }
    
                            return BackwardSearchResult(next_b, next_e, next_sa_b_);
                        }
                        else
                        {
                            return BackwardSearchResult::create_empty_result();
                        }
                    }
                }
            }
            /**
             * @brief Perform backward search to find the SA-interval of a pattern
             * @param pattern The pattern to search for
             * @return BackwardSearchResult containing the SA-interval [b, e]
             * @note Time complexity: O(m log σ log n) where m is pattern length
             * @note Returns empty interval if pattern does not exist
             */
            BackwardSearchResult backward_search(const std::vector<uint8_t> &pattern) const
            {
                BackwardSearchResult tmp;
                tmp.b = 0;
                tmp.e = this->text_size() - 1;
                tmp.sa_b_ = this->disa.get_sampled_first_sa_value(0);

                for (int64_t i = pattern.size() - 1; i >= 0; i--)
                {
                    tmp = this->backward_search(tmp, pattern[i]);
                    if (tmp.is_empty())
                    {
                        break;
                    }
                }
                return tmp;
            }
            /**
             * @brief Compute SA values from a backward search result
             * @param bsr The backward search result containing SA-interval
             * @return Vector of SA values for all positions in the interval
             * @note Used internally by locate_query()
             */
            std::vector<uint64_t> compute_sa_values(const BackwardSearchResult &bsr) const
            {
                std::vector<uint64_t> r;
                if (!bsr.is_empty())
                {
                    uint64_t x = bsr.sa_b_;
                    r.push_back(x);
                    for (int64_t i = bsr.b + 1; i <= bsr.e; i++)
                    {
                        x = this->disa.inverse_phi(x);
                        r.push_back(x);
                    }
                }
                return r;
            }

            /**
             * @brief Count the number of occurrences of a pattern
             * @param pattern The pattern to search for
             * @return The number of occurrences (0 if pattern does not exist)
             * @note Time complexity: O(m log σ log n) where m is pattern length
             */
            uint64_t count_query(const std::vector<uint8_t> &pattern) const
            {

                BackwardSearchResult intv = this->backward_search(pattern);
                if (intv.is_empty())
                {
                    return 0;
                }
                else
                {
                    return intv.e - intv.b + 1;
                }
            }
            /**
             * @brief Find all occurrence positions of a pattern
             * @param pattern The pattern to search for
             * @return Vector of occurrence positions (SA values)
             * @note Time complexity: O((m + occ) log σ log n) where occ is number of occurrences
             * @note Returns empty vector if pattern does not exist
             */
            std::vector<uint64_t> locate_query(const std::vector<uint8_t> &pattern) const
            {
                BackwardSearchResult bsr = this->backward_search(pattern);
                return this->compute_sa_values(bsr);
            }
            uint8_t access(int64_t i) const
            {
                return this->dbwt.access(i);
            }

            //@}

            //@}
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Non-const Methods for Updates
            ///   The public non-const methods of this class for updates.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
        public:
        /**
         * @brief Insert a single character at a given position
         * @param u The text position to insert at (0-indexed)
         * @param c The character to insert
         * @return The number of BWT reorder operations performed
         * @note Time complexity: Average O((1 + L_avg) log σ log n)
         */
        uint64_t insert_string(TextIndex u, uint8_t c)
        {
            FMIndexEditHistory editHistory;

            return this->insert_char(u, c, editHistory);
        }

        /**
         * @brief Insert a string at a given position
         * @param u The text position to insert at (0-indexed)
         * @param inserted_string The string to insert
         * @return The number of BWT reorder operations performed
         * @note Time complexity: Average O((m + L_avg) log σ log n) where m is string length
         */
        uint64_t insert_string(TextIndex u, const std::vector<uint8_t> &inserted_string)
            {
                FMIndexEditHistory editHistory;
                return this->insert_string(u, inserted_string, editHistory);
            }
            
            /**
             * @brief Insert a string at a given position (with edit history)
             * @param u The text position to insert at (0-indexed)
             * @param inserted_string The string to insert
             * @param output_history Output parameter for edit history
             * @return The number of BWT reorder operations performed
             */
            uint64_t insert_string(TextIndex u, const std::vector<uint8_t> &inserted_string, FMIndexEditHistory &output_history)
            {

                output_history.clear();
                AdditionalInformationUpdatingRIndex inf = RIndexHelperForUpdate::preprocess_of_string_insertion_operation(u, inserted_string, output_history, dbwt, disa);

                
                PositionInformation y_PI;
                y_PI.p = inf.y;
                y_PI.value_at_p = inf.value_at_y;
                //y_PI.p_on_rlbwt = dbwt.to_run_position(y_PI.p);
                y_PI.value_at_p_minus = inf.value_at_y_minus;
                y_PI.value_at_p_plus = inf.value_at_y_plus;
                
                PositionInformation z_PI;
                z_PI.p = inf.z;
                z_PI.value_at_p_minus = inf.value_at_z_minus;
                z_PI.value_at_p_plus = inf.value_at_z_plus;
                


                bool b = false;
                while (!b)
                {
                    
                    b = RIndexHelperForUpdate::phase_D(output_history, this->dbwt, this->disa, y_PI, z_PI);
                }

                return output_history.move_history.size();
            }

            uint64_t delete_substring(TextIndex u)
            {
                return this->delete_string(u, 1);
            }

            /**
             * @brief Delete a substring from the text
             * @param u The starting position of the substring to delete (0-indexed)
             * @param len The length of the substring to delete
             * @return The number of BWT reorder operations performed
             * @throws std::logic_error if len < 1 or u + len >= text_size()
             * @note Time complexity: Average O((len + L_avg) log σ log n)
             */
            uint64_t delete_string(TextIndex u, uint64_t len)
            {
                if(len < 1){
                    throw std::logic_error("The length of the deleted substring is at least 1.");
                }
                if(u + len >= this->text_size()){
                    throw std::logic_error("The ending position of the deleted substring must be less than the ending position of the text.");
                }

                FMIndexEditHistory editHistory;
                return this->delete_string(u, len, editHistory);
            }
            
            /**
             * @brief Delete a substring from the text (with edit history)
             * @param u The starting position of the substring to delete (0-indexed)
             * @param len The length of the substring to delete
             * @param output_history Output parameter for edit history
             * @return The number of BWT reorder operations performed
             */
            uint64_t delete_string(TextIndex u, uint64_t len, FMIndexEditHistory &output_history)
            {
                if(len < 1){
                    throw std::logic_error("The length of the deleted substring is at least 1.");
                }
                if(u + len >= this->text_size()){
                    throw std::logic_error("The ending position of the deleted substring must be less than the ending position of the text.");
                }
                
                output_history.clear();
                AdditionalInformationUpdatingRIndex inf = RIndexHelperForUpdate::preprocess_of_string_deletion_operation(u, len, output_history, dbwt, disa, nullptr);

                PositionInformation y_PI;
                y_PI.p = inf.y;
                y_PI.value_at_p = inf.value_at_y;
                y_PI.value_at_p_minus = inf.value_at_y_minus;
                y_PI.value_at_p_plus = inf.value_at_y_plus;
                
                PositionInformation z_PI;
                z_PI.p = inf.z;
                z_PI.value_at_p_minus = inf.value_at_z_minus;
                z_PI.value_at_p_plus = inf.value_at_z_plus;


                bool b = false;
                while (!b)
                {
                    b = RIndexHelperForUpdate::phase_D(output_history, this->dbwt, this->disa, y_PI, z_PI);
                    //b = RIndexOldUpdateOperations::reorder_RLBWT2(output_history, this->dbwt, this->disa, sub, inf);
                    //b = RIndexHelperForUpdate::phase_D_prime(output_history, this->dbwt, this->disa, inf);
                }
                // RIndexHelperForUpdate::merge_non_maximal_runs_in_dbwt(output_history, true, dbwt, disa);

                return output_history.move_history.size();
            }


            uint64_t insert_char(TextIndex u, uint8_t c)
            {
                FMIndexEditHistory editHistory;
                return this->insert_char(u, c, editHistory);
            }
            uint64_t insert_char(TextIndex u, uint8_t c, FMIndexEditHistory &output_history)
            {
                const std::vector<uint8_t> inserted_string = {c};
                return this->insert_string(u, inserted_string, output_history);

            }

            uint64_t delete_char(TextIndex u)
            {
                FMIndexEditHistory edit_history;
                return this->delete_char(u, edit_history);
            }
            uint64_t delete_char(TextIndex u, FMIndexEditHistory &output_history)
            {
                output_history.clear();

                AdditionalInformationUpdatingRIndex inf = RIndexHelperForUpdate::preprocess_of_string_deletion_operation(u, 1,output_history, dbwt, disa, nullptr);


                PositionInformation y_PI;
                y_PI.p = inf.y;
                y_PI.value_at_p = inf.value_at_y;
                y_PI.value_at_p_minus = inf.value_at_y_minus;
                y_PI.value_at_p_plus = inf.value_at_y_plus;
                
                PositionInformation z_PI;
                z_PI.p = inf.z;
                z_PI.value_at_p_minus = inf.value_at_z_minus;
                z_PI.value_at_p_plus = inf.value_at_z_plus;

                bool b = false;
                while (!b)
                {
                    b = RIndexHelperForUpdate::phase_D(output_history, this->dbwt, this->disa, y_PI, z_PI);
                    //b = RIndexOldUpdateOperations::reorder_RLBWT2(output_history, this->dbwt, this->disa, sub, inf);
                    //b = RIndexHelperForUpdate::phase_D_prime(output_history, this->dbwt, this->disa, inf);
                }

                return output_history.move_history.size();
            }

            uint64_t compute_RLBWT_hash(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                return this->dbwt.compute_RLBWT_hash(message_paragraph);
            }

            //@}
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Methods for Debug
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            
            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief This method is used for debug
            ////////////////////////////////////////////////////////////////////////////////

            AdditionalInformationUpdatingRIndex __preprocess_of_string_deletion_operation(TextIndex u, int64_t len, FMIndexEditHistory &editHistory, std::vector<std::vector<uint64_t>> *sa_arrays_for_debug)
            {
                return RIndexHelperForUpdate::preprocess_of_string_deletion_operation(u, len, editHistory, dbwt, disa, sa_arrays_for_debug);
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief This method is used for debug
            ////////////////////////////////////////////////////////////////////////////////
            AdditionalInformationUpdatingRIndex __preprocess_of_string_insertion_operation(TextIndex u, std::vector<uint8_t> &inserted_string, FMIndexEditHistory &editHistory)
            {
                return RIndexHelperForUpdate::preprocess_of_string_insertion_operation(u, inserted_string, editHistory, dbwt, disa);
            }
            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief This method is used for debug
            ////////////////////////////////////////////////////////////////////////////////
            AdditionalInformationUpdatingRIndex __preprocess_of_char_insertion_operation(TextIndex u, uint8_t c, FMIndexEditHistory &editHistory)
            {
                std::vector<uint8_t> inserted_string = {c};
                return RIndexHelperForUpdate::preprocess_of_string_insertion_operation(u, inserted_string, editHistory, dbwt, disa);
            }
            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief This method is used for debug
            ////////////////////////////////////////////////////////////////////////////////
            AdditionalInformationUpdatingRIndex __preprocess_of_char_deletion_operation(TextIndex u, FMIndexEditHistory &editHistory)
            {
                //return RIndexOldUpdateOperations::preprocess_of_char_deletion_operation(u, editHistory, dbwt, disa);
                return RIndexHelperForUpdate::preprocess_of_string_deletion_operation(u, 1, editHistory, dbwt, disa, nullptr);

            }

            AdditionalInformationUpdatingRIndex __preprocess_of_char_deletion_operation2(TextIndex u, FMIndexEditHistory &editHistory)
            {
                return RIndexHelperForUpdate::preprocess_of_string_deletion_operation(u, 1, editHistory, dbwt, disa, nullptr);
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief This method is used for debug
            ////////////////////////////////////////////////////////////////////////////////
            
            bool __reorder_RLBWT_for_insertion(FMIndexEditHistory &editHistory, AdditionalInformationUpdatingRIndex &inf)
            {
                
                PositionInformation y_PI;
                y_PI.p = inf.y;
                y_PI.value_at_p = inf.value_at_y;
                //y_PI.p_on_rlbwt = dbwt.to_run_position(y_PI.p);
                y_PI.value_at_p_minus = inf.value_at_y_minus;
                y_PI.value_at_p_plus = inf.value_at_y_plus;
                
                PositionInformation z_PI;
                z_PI.p = inf.z;
                z_PI.value_at_p_minus = inf.value_at_z_minus;
                z_PI.value_at_p_plus = inf.value_at_z_plus;
                

                bool b = RIndexHelperForUpdate::phase_D(editHistory, this->dbwt, this->disa, y_PI, z_PI);

                inf.y = y_PI.p;
                inf.z = z_PI.p;
                inf.value_at_y = y_PI.value_at_p;
                inf.value_at_y_minus = y_PI.value_at_p_minus;
                inf.value_at_y_plus = y_PI.value_at_p_plus;
                inf.value_at_z_minus = z_PI.value_at_p_minus;
                inf.value_at_z_plus = z_PI.value_at_p_plus;
                return b;
            }
            
            



            //@}
        };

    }
}
