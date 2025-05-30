#pragma once
#include "./time_debug.hpp"
#include "./fm_index_edit_history.hpp"
#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"
#include "./c_array.hpp"

// #include "./packed_spsi_wrapper.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        bool __view_flag = false;

        ////////////////////////////////////////////////////////////////////////////////
        /// @class      DynamicBWT
        /// @brief      A dynamic data structure storing BWT. This implementation requires $O(n log σ)$ bits of space for the input string of length $n$ and alphabet size $σ$.
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class DynamicBWT
        {
            stool::dynamic_r_index::CArray cArray;
            stool::bptree::DynamicWaveletTree bwt;

        public:
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Initializers and Builders
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            DynamicBWT()
            {
            }
            DynamicBWT &operator=(const DynamicBWT &) = delete;
            DynamicBWT(DynamicBWT &&) noexcept = default;
            DynamicBWT &operator=(DynamicBWT &&) noexcept = default;

            uint64_t size_in_bytes() const
            {
                return this->cArray.size_in_bytes() + this->bwt.size_in_bytes();
            }

            int64_t get_c_id(uint8_t c) const
            {
                return this->cArray.get_c_id(c);
            }
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log1 = this->cArray.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->bwt.get_memory_usage_info(message_paragraph + 1);

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicBWT: " + std::to_string(this->size_in_bytes()) + " bytes =");
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
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicBWT):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: " << this->size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet size: " << this->get_alphabet_size() << std::endl;

                this->cArray.print_statistics(message_paragraph + 1);
                this->bwt.print_statistics(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            void print_content(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Content(DynamicBWT): text length = " << this->size() << std::endl;
                this->bwt.print_content(message_paragraph + 1);

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            uint64_t get_alphabet_size() const
            {
                return this->bwt.get_alphabet_size();
            }
            std::vector<uint8_t> get_alphabet() const
            {
                return this->cArray.get_alphabet();
            }

            /**
             * @brief Initializes the DynamicBWT with the given characters and end marker.
             *
             * This function sets up the BWT (Burrows-Wheeler Transform) structure with the provided characters.
             * It ensures the end marker is included, initializes the dynamic rank-select structure, and updates
             * the character distribution and C array.
             *
             * @param _chars The characters contained in the input dynamic text.
             */
            void initialize(const std::vector<uint8_t> &_alphabet)
            {
                if (_alphabet.size() == 0)
                {
                    std::cerr << "The alphabet of FM-index must contain a character" << std::endl;
                    assert(_alphabet.size() > 0);
                    throw std::invalid_argument("The alphabet of FM-index must contain a character");
                }
                // this->end_marker = *std::min_element(std::begin(_alphabet), std::end(_alphabet));;

                this->bwt.set_alphabet(_alphabet);

                this->clear();
            }
            void swap(DynamicBWT &item)
            {
                this->cArray.swap(item.cArray);
                this->bwt.swap(item.bwt);
            }
            uint8_t get_end_marker() const
            {
                return this->bwt.get_smallest_character_in_alphabet();
            }
            const stool::dynamic_r_index::CArray &get_c_array() const
            {
                return this->cArray;
            }

            void clear(int message_paragraph = stool::Message::NO_MESSAGE)
            {
                // DynRankSWrapper::clear(*this->bwt);

                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Clearing Dynamic BWT..." << std::endl;
                }

                uint8_t end_marker = this->get_end_marker();
                this->bwt.clear();
                this->cArray.clear();
                this->bwt.push_back(end_marker);
                this->cArray.increase(end_marker, 1);

                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
                }
            }

            static DynamicBWT build(const std::vector<uint8_t> &_bwt, const std::vector<uint8_t> &_alphabet, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                if (message_paragraph >= 0 && _bwt.size() > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic BWT from BWT... " << std::flush;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicBWT r;
                r.initialize(_alphabet);
                r.bwt.clear();
                r.cArray.clear();

                // uint64_t counter = 0;

                {
                    stool::bptree::DynamicWaveletTree tmp_dwt = stool::bptree::DynamicWaveletTree::build(_bwt, _alphabet);
                    r.bwt.swap(tmp_dwt);
                }

                {
                    std::vector<uint64_t> count_c_vector;
                    count_c_vector.resize(256, 0);
                    for (uint8_t c : _alphabet)
                    {
                        uint64_t count = r.bwt.count_c(c);
                        count_c_vector[c] = count;
                    }

                    auto tmp_c_array = CArray::build(count_c_vector);
                    r.cArray.swap(tmp_c_array);
                }

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && _bwt.size() > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)_bwt.size()) * 1000000;

                    std::cout << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Properties
            ///   The properties of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            int64_t size() const
            {
                return this->bwt.size();
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Const Methods
            ///   The const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            std::vector<uint8_t> get_text() const
            {
                std::vector<uint8_t> r;
                r.resize(this->size(), 0);
                r[r.size() - 1] = this->get_end_marker();
                int64_t current_pos_on_bwt = this->get_end_marker_pos();
                int64_t x = this->size() - 1;
                while (x > 0)
                {
                    current_pos_on_bwt = this->LF(current_pos_on_bwt);
                    r[--x] = this->access(current_pos_on_bwt);
                }
                return r;
            }
            std::vector<uint8_t> get_bwt() const
            {
                std::vector<uint8_t> r;
                r.resize(this->size(), 0);
                for (int64_t i = 0; i < this->size(); i++)
                {
                    r[i] = this->access(i);
                }
                return r;
            }

            std::string get_bwt_str(int64_t endmarker = -1) const
            {
                std::vector<uint8_t> r = this->get_bwt();
                std::string s;
                for (auto c : r)
                {
                    if (endmarker != -1 && c == this->get_end_marker())
                    {
                        s.push_back(endmarker);
                    }
                    else
                    {
                        s.push_back(c);
                    }
                }
                return s;
            }
            std::string get_text_str() const
            {
                std::vector<uint8_t> r = this->get_text();
                std::string s;
                for (auto c : r)
                {
                    s.push_back(c);
                }
                return s;
            }
            int64_t get_end_marker_pos() const
            {
                return this->bwt.select(0, this->get_end_marker());
            }
            uint8_t access(int64_t i) const
            {
                return this->bwt.at(i);
            }
            uint64_t count_c(uint8_t c) const
            {
                return this->bwt.count_c(c);
            }
            int64_t rank(uint8_t c, int64_t i) const
            {
                if (i >= 0)
                {
                    return this->bwt.rank(i + 1, c);
                }
                else
                {
                    return 0;
                }
            }
            int64_t select(uint8_t c, int64_t ith) const
            {
                return this->bwt.select(ith - 1, c);
            }
            int64_t LF_for_deletion(int64_t i, uint8_t new_char, uint64_t replace_pos, uint64_t current_processing_position) const
            {
                if(i == replace_pos){
                    return LF_for_deletion(current_processing_position, new_char, replace_pos, current_processing_position);
                }else{
                uint8_t c = this->access(i);
                uint64_t p = this->LF(i);

                if (c > new_char || (i > replace_pos && c == new_char))
                {
                    if (p > 0)
                    {
                        p--;
                    }
                    else
                    {
                        p = this->size() - 2;
                    }
                }
                return p;

                }

            }

            int64_t LF(int64_t i) const
            {
                assert(i >= 0);
                assert(i < (int64_t)this->size());
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif

                // uint8_t c = this->bwt[i];
                uint8_t c = this->access(i);

                // uint64_t lf = this->C[c] + this->rank(c, i);
                uint64_t lf = (this->cArray.at(c) + this->rank(c, i)) - 1;
                // assert((int64_t)this->bwt->rank(c, i + 1) == this->rank(c, i));

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::LF_time += elapsed;
                stool::LF_count += 1;
#endif
                // assert(lf == naive_LF(i));
                return lf;
            }

            //@}
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Non-const Methods
            ///   The public non-const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            void replace_BWT_character(int64_t pos, uint8_t c)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif

                this->cArray.decrease(this->access(pos));
                this->cArray.increase(c);

                this->bwt.remove(pos);
                this->bwt.insert(pos, c);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::wavelet_tree_update_time += elapsed;
                stool::wavelet_tree_update_count += 2;
#endif
            }
            void insert_BWT_character(int64_t pos, uint8_t c)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif
                this->cArray.increase(c, 1);
                this->bwt.insert(pos, c);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::wavelet_tree_update_time += elapsed;
                stool::wavelet_tree_update_count += 1;
#endif
            }

            void remove_BWT_character(int64_t pos)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif
                assert(pos < this->size());
                this->cArray.decrease(this->access(pos));
                this->bwt.remove(pos);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::wavelet_tree_update_time += elapsed;
                stool::wavelet_tree_update_count += 1;
#endif
            }

            static void save(DynamicBWT &item, std::ofstream &os)
            {
                stool::dynamic_r_index::CArray::save(item.cArray, os);
                stool::bptree::DynamicWaveletTree::save(item.bwt, os);
            }
            static DynamicBWT build_from_data(std::ifstream &ifs)
            {
                stool::dynamic_r_index::CArray cArray = stool::dynamic_r_index::CArray::load(ifs);
                stool::bptree::DynamicWaveletTree bwt = stool::bptree::DynamicWaveletTree::build_from_data(ifs);

                DynamicBWT r;
                r.cArray.swap(cArray);
                r.bwt.swap(bwt);
                return r;
            }
        };
    }
}
