#pragma once
#include <vector>
#include "stool/include/all.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        /**
         * @brief A dynamic data structure for stroing two arrays \p C[0..σ-1] and \p D[0..σ'-1] related to an alphabet \p Σ = {c_{1}, c_{2}, ..., c_{σ-1}} of a string \p T[0..n-1]. 
         * @details The details of this data structure are as follows:
         * @li Each C[i] stores the number of occurrences of c_{1}, c_{2}, ..., c_{i} in \p T[0..n-1].
         * @li Each D[i] stores the i-th character c'_{i} in Σ'. Here, Σ = {c'_{1}, c'_{2}, ..., c'_{σ'-1}} is a set of characters that appear in the string \p T[0..n-1].
         * \ingroup DynamicFMIndexes
         * \ingroup MainDataStructures
         */
         class CArray
        {
            static inline constexpr int C_ARRAY_MAX_SIZE = 257;

            std::vector<uint64_t> C;
            std::vector<uint8_t> effective_alphabet; // D

        public:

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Returns the total memory usage in bytes
             */
            uint64_t size_in_bytes() const
            {
                return (2 * sizeof(std::vector<uint64_t>)) + (this->C.capacity() * sizeof(uint64_t)) + (this->effective_alphabet.capacity() * sizeof(uint64_t));
            }
            /**
             * @brief Return \p σ'
             */
            uint64_t effective_alphabet_size() const
            {
                return this->effective_alphabet.size();
            }

            //@}
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Return \p D[0..σ'-1] as a vector.
             * @note O(σ) time
             */
            std::vector<uint8_t> get_effective_alphabet() const
            {
                std::vector<uint8_t> r = this->effective_alphabet;
                return r;
            }

            /**
             * @brief Return the number of occurrences of a character \p c in \p T[0..n-1].
             * @note O(1) time
             */
            uint64_t get_c_count(uint8_t c) const
            {
                uint64_t x = c;
                return this->C[x + 1] - this->C[x];
            }

            /**
             * @brief Return \p C[i]
             * @note O(1) time
             */
            uint64_t at(uint64_t i) const
            {
                return this->C[i];
            }

            /**
             * @brief Return the rank of a given character \p c in \p Σ' if it exists, otherwise return -1.
             */
            int64_t get_c_id(uint8_t c) const
            {
                
                int64_t size = this->effective_alphabet.size();
                for (int64_t i = 0; i < size; i++)
                {
                    if (this->effective_alphabet[i] == c)
                    {
                        return i;
                    }
                }
                return -1;
                
            }

            /**
             * @brief Return the index of the successor of a character \p c in \p Σ' if it exists, otherwise return -1.
             * @note O(σ') time
             */
            int64_t successor_on_effective_alphabet(uint8_t value) const
            {
                return stool::BasicSearch::successor_by_linear_search(this->effective_alphabet, value);
            }

            /**
             * @brief Return the index of the predecessor of a character \p c in \p Σ' if it exists, otherwise return -1.
             * @note O(σ') time
             */
            int64_t predecessor_on_effective_alphabet(uint8_t value) const
            {
                return stool::BasicSearch::predecessor_by_linear_search(this->effective_alphabet, value);
            }

            /**
             * @brief Checks if a character \p c occurs in \p Σ'
             * @note O(1) time
             */
            bool occurs_in_effective_alphabet(uint8_t c) const
            {
                return this->get_c_count(c) > 0;
            }

            /**
             * @brief Return the i-th character in \p Σ'
             * @note O(1) time
             */
            uint8_t get_character_in_effective_alphabet(uint64_t i) const
            {
                return this->effective_alphabet[i];
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Update operations
            ////////////////////////////////////////////////////////////////////////////////
            //@{
        /**
             * @brief Initializes the two arrays \p C and \p D
             */
            void initialize()
            {
                this->C.clear();
                this->C.resize(CArray::C_ARRAY_MAX_SIZE, 0);
                this->effective_alphabet.clear();
                assert(this->verify());
            }

            /**
             * @brief Swap operation
             */
            void swap(CArray &item)
            {
                this->C.swap(item.C);
                this->effective_alphabet.swap(item.effective_alphabet);
            }


            /**
             * @brief Alias of initialize()
             */
            void clear()
            {
                this->initialize();
            }

            /**
             * @brief Increases the number of occurrences of a character \p c in \p T[0..n-1] by \p delta
             * @note O(σ) time
             */
            void increase(uint8_t c, int64_t delta)
            {
                if (delta > 0)
                {
                    bool b = this->occurs_in_effective_alphabet(c);
                    if (!b)
                    {
                        int64_t ins_pos = 0;
                        int64_t size = this->effective_alphabet.size();
                        for (int64_t i = 0; i < size; i++)
                        {
                            uint8_t p2 = i + 1 < size ? this->effective_alphabet[i + 1] : UINT8_MAX;
                            if (this->effective_alphabet[i] < c && c <= p2)
                            {
                                ins_pos = i + 1;
                            }
                        }
                        this->effective_alphabet.insert(this->effective_alphabet.begin() + ins_pos, c);
                    }
                }

                for (uint64_t i = c + 1; i < this->C.size(); i++)
                {
                    this->C[i] += delta;
                }

                if (this->get_c_count(c) == 0)
                {
                    int64_t id = this->get_c_id(c);
                    this->effective_alphabet.erase(this->effective_alphabet.begin() + id);
                }
                assert(this->verify());
            }

            /**
             * @brief Increases the number of occurrences of a character \p c in \p T[0..n-1] by 1
             * @note O(σ) time
             */
            void increase(uint8_t c)
            {
                this->increase(c, 1);
                assert(this->verify());
            }

            /**
             * @brief Decreases the number of occurrences of a character \p c in \p T[0..n-1] by \p delta
             * @note O(σ) time
             */
            void decrease(uint8_t c, int64_t delta)
            {
                this->increase(c, -delta);
                assert(this->verify());
            }

            /**
             * @brief Decreases the number of occurrences of a character \p c in \p T[0..n-1] by 1
             * @note O(σ) time
             */
            void decrease(uint8_t removedChar)
            {
                this->increase(removedChar, -1);
                assert(this->verify());
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Print and verification functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Return the memory usage information of this data structure as a vector of strings
             * @param message_paragraph The paragraph depth of message logs
             */
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log;
                log.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=CArray: " + std::to_string(this->size_in_bytes()) + " bytes =");
                log.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");
                return log;
            }

            /**
             * @brief Prints detailed information about this instance
             */
            void print_info() const
            {
                std::cout << "====== CArray ======" << std::endl;
                stool::DebugPrinter::print_integers(this->C);
                stool::DebugPrinter::print_integers_with_characters(this->effective_alphabet);
                std::cout << "====================" << std::endl;
            }

            /**
             * @brief Print the memory usage information of this data structure
             * @param message_paragraph The paragraph depth of message logs (-1 for no output)
             */
            void print_memory_usage(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log = this->get_memory_usage_info(message_paragraph);
                for (std::string &s : log)
                {
                    std::cout << s << std::endl;
                }
            }

            /**
             * @brief Print the statistics of this data structure
             * @param message_paragraph The paragraph depth of message logs
             */
            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(CArray):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Effective Alphabet: " << stool::ConverterToString::to_character_string(this->effective_alphabet) << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            /**
             * @brief Verify the internal consistency of this data structure.
             */
            bool verify() const {
                for (uint64_t i = 1; i < this->C.size(); i++) {
                    if(this->C[i] < this->C[i-1]){
                        this->print_info();
                        std::cout << "i = " << i << ", C[i] = " << this->C[i] << ", C[i-1] = " << this->C[i-1] << std::endl;
                        throw std::logic_error("Error: CArray::verify(), C[i] < C[i-1]");
                    }
                }
                return true;
            }
            //@}


            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Load, save, and builder functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Build CArray from a vector \p Q, where \p Q[c] is the number of occurrences of the character \p c in \p T[0..n-1].
             */
            static CArray build(const std::vector<uint64_t> &vector_Q)
            {
                CArray r;
                r.initialize();
                for (uint64_t i = 0; i < vector_Q.size(); i++)
                {
                    if (vector_Q[i] > 0)
                    {
                        r.increase(i, vector_Q[i]);
                    }
                }
                return r;
            }
            /**
             * @brief Save the given instance \p item to a file stream \p os
             */
            static void store_to_file(const CArray &item, std::ofstream &os)
            {
                uint64_t sz1 = item.C.size();
                uint64_t sz2 = item.effective_alphabet.size();
                os.write(reinterpret_cast<const char *>(&sz1), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(&sz2), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(item.C.data()), item.C.size() * sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(item.effective_alphabet.data()), item.effective_alphabet.size() * sizeof(uint8_t));
            }

            /**
             * @brief Return the CArray loaded from a file stream \p ifs
             */
            static CArray load_from_file(std::ifstream &ifs)
            {
                uint64_t size1 = 0;
                uint64_t size2 = 0;

                ifs.read(reinterpret_cast<char *>(&size1), sizeof(uint64_t));
                ifs.read(reinterpret_cast<char *>(&size2), sizeof(uint64_t));
                CArray r;
                r.C.resize(size1);
                r.effective_alphabet.resize(size2);

                ifs.read(reinterpret_cast<char *>(r.C.data()), size1 * sizeof(uint64_t));
                ifs.read(reinterpret_cast<char *>(r.effective_alphabet.data()), size2 * sizeof(uint8_t));

                return r;
            }

            //@}




        };
    }
}
