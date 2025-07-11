#pragma once
#include <vector>
#include "stool/include/light_stool.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        /**
         * @brief A class representing the C array in a dynamic FM-index.
         *
         * The C array is a fundamental component of the FM-index that stores cumulative
         * character frequencies. This class manages both the C array and the effective alphabet
         * used in the index.
         */
        class CArray
        {
            static inline constexpr int C_ARRAY_MAX_SIZE = 257;

            std::vector<uint64_t> C;
            std::vector<uint8_t> effective_alphabet;

        public:
            /**
             * @brief Initializes the C array and effective alphabet data structures
             *
             * The C array is a fundamental component of the FM-index that stores cumulative
             * character frequencies. This method:
             * - Clears any existing data in the C array
             * - Resizes it to C_ARRAY_MAX_SIZE (257) elements, initialized to 0
             * - Clears the effective_alphabet vector which tracks actual characters used
             */
            void initialize()
            {
                this->C.clear();
                this->C.resize(CArray::C_ARRAY_MAX_SIZE, 0);
                this->effective_alphabet.clear();
            }

            /**
             * @brief Calculates the total memory usage of the CArray in bytes
             * @return The total size in bytes
             */
            uint64_t size_in_bytes() const
            {
                return (2 * sizeof(std::vector<uint64_t>)) + (this->C.capacity() * sizeof(uint64_t)) + (this->effective_alphabet.capacity() * sizeof(uint64_t));
            }

            /**
             * @brief Gets memory usage information as a vector of strings
             * @param message_paragraph The paragraph level for message formatting
             * @return Vector of strings containing memory usage information
             */
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log;
                log.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=CArray: " + std::to_string(this->size_in_bytes()) + " bytes =");
                log.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");
                return log;
            }

            /**
             * @brief Prints the current state of the CArray to stdout
             */
            void print() const
            {
                std::cout << "====== CArray ======" << std::endl;
                stool::DebugPrinter::print_integers(this->C);
                stool::DebugPrinter::print_integers_with_characters(this->effective_alphabet);
                std::cout << "====================" << std::endl;
            }

            /**
             * @brief Prints memory usage information to stdout
             * @param message_paragraph The paragraph level for message formatting
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
             * @brief Prints statistics about the CArray to stdout
             * @param message_paragraph The paragraph level for message formatting
             */
            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(CArray):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Effective Alphabet: " << stool::DebugPrinter::to_character_string(this->effective_alphabet) << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            /**
             * @brief Gets the current effective alphabet
             * @return Vector containing the effective alphabet characters
             */
            std::vector<uint8_t> get_alphabet() const
            {
                std::vector<uint8_t> r = this->effective_alphabet;
                return r;
            }

            /**
             * @brief Saves the CArray to a file stream
             * @param item The CArray to save
             * @param os The output file stream
             */
            static void save(const CArray &item, std::ofstream &os)
            {
                uint64_t sz1 = item.C.size();
                uint64_t sz2 = item.effective_alphabet.size();
                os.write(reinterpret_cast<const char *>(&sz1), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(&sz2), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(item.C.data()), item.C.size() * sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(item.effective_alphabet.data()), item.effective_alphabet.size() * sizeof(uint8_t));
            }

            /**
             * @brief Loads a CArray from a file stream
             * @param ifs The input file stream
             * @return The loaded CArray
             */
            static CArray load(std::ifstream &ifs)
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

            /**
             * @brief Swaps the contents of this CArray with another
             * @param item The CArray to swap with
             */
            void swap(CArray &item)
            {
                this->C.swap(item.C);
                this->effective_alphabet.swap(item.effective_alphabet);
            }

            /**
             * @brief Builds a CArray from a count vector
             * @param count_c_vector The input count vector
             * @return The constructed CArray
             */
            static CArray build(const std::vector<uint64_t> &count_c_vector)
            {
                CArray r;
                r.initialize();
                for (uint64_t i = 0; i < count_c_vector.size(); i++)
                {
                    if (count_c_vector[i] > 0)
                    {
                        r.increase(i, count_c_vector[i]);
                    }
                }
                return r;
            }

            /**
             * @brief Clears the CArray and reinitializes it
             */
            void clear()
            {
                this->initialize();
            }

            /**
             * @brief Increases the count for a character in the CArray
             * @param newChar The character to increase
             * @param delta The amount to increase by
             */
            void increase(uint8_t newChar, int64_t delta)
            {
                if (delta > 0)
                {
                    bool b = this->occurs_in_effective_alphabet(newChar);
                    if (!b)
                    {
                        int64_t ins_pos = 0;
                        int64_t size = this->effective_alphabet.size();
                        for (int64_t i = 0; i < size; i++)
                        {
                            uint8_t p2 = i + 1 < size ? this->effective_alphabet[i + 1] : UINT8_MAX;
                            if (this->effective_alphabet[i] < newChar && newChar <= p2)
                            {
                                ins_pos = i + 1;
                            }
                        }
                        this->effective_alphabet.insert(this->effective_alphabet.begin() + ins_pos, newChar);
                    }
                }

                for (uint64_t i = newChar + 1; i < this->C.size(); i++)
                {
                    this->C[i] += delta;
                }

                if (this->get_c_count(newChar) == 0)
                {
                    int64_t id = this->get_c_id(newChar);
                    this->effective_alphabet.erase(this->effective_alphabet.begin() + id);
                }
            }

            /**
             * @brief Increases the count for a character by 1
             * @param newChar The character to increase
             */
            void increase(uint8_t newChar)
            {
                this->increase(newChar, 1);
            }

            /**
             * @brief Decreases the count for a character in the CArray
             * @param removedChar The character to decrease
             * @param delta The amount to decrease by
             */
            void decrease(uint8_t removedChar, int64_t delta)
            {
                this->increase(removedChar, -delta);
            }

            /**
             * @brief Decreases the count for a character by 1
             * @param removedChar The character to decrease
             */
            void decrease(uint8_t removedChar)
            {
                this->increase(removedChar, -1);
            }

            /**
             * @brief Gets the count for a character in the CArray
             * @param c The character to get the count for
             * @return The count for the character
             */
            uint64_t get_c_count(uint8_t c) const
            {
                uint64_t x = c;
                return this->C[x + 1] - this->C[x];
            }

            /**
             * @brief Gets the value at a specific index in the CArray
             * @param i The index to get the value for
             * @return The value at the index
             */
            uint64_t at(uint64_t i) const
            {
                return this->C[i];
            }

            /**
             * @brief Gets the ID of a character in the effective alphabet
             * @param c The character to get the ID for
             * @return The ID of the character, or -1 if not found
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
             * @brief Finds the successor of a value in the effective alphabet
             * @param value The value to find the successor for
             * @return The index of the successor, or -1 if not found
             */
            int64_t successor_on_effective_alphabet(uint8_t value) const
            {
                return stool::BasicSearch::successor_by_linear_search(this->effective_alphabet, value);
            }

            /**
             * @brief Finds the predecessor of a value in the effective alphabet
             * @param value The value to find the predecessor for
             * @return The index of the predecessor, or -1 if not found
             */
            int64_t predecessor_on_effective_alphabet(uint8_t value) const
            {
                return stool::BasicSearch::predecessor_by_linear_search(this->effective_alphabet, value);
            }

            /**
             * @brief Checks if a character occurs in the effective alphabet
             * @param value The character to check
             * @return True if the character occurs, false otherwise
             */
            bool occurs_in_effective_alphabet(uint8_t value) const
            {
                return this->get_c_count(value) > 0;
            }

            /**
             * @brief Gets a character from the effective alphabet by index
             * @param i The index to get the character for
             * @return The character at the index
             */
            uint8_t get_character_in_effective_alphabet(uint64_t i) const
            {
                return this->effective_alphabet[i];
            }

            /**
             * @brief Gets the size of the effective alphabet
             * @return The number of characters in the effective alphabet
             */
            uint64_t effective_alphabet_size() const
            {
                return this->effective_alphabet.size();
            }
        };
    }
}
