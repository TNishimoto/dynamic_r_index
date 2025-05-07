#pragma once
#include <vector>
#include "stool/include/light_stool.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        class CArray
        {
            static inline constexpr int C_ARRAY_MAX_SIZE = 257;

            std::vector<uint64_t> C;
            std::vector<uint8_t> effective_alphabet;

        public:
            void initialize()
            {
                this->C.clear();
                this->C.resize(CArray::C_ARRAY_MAX_SIZE, 0);
                this->effective_alphabet.clear();
            }
            uint64_t size_in_bytes() const
            {
                return (2 * sizeof(std::vector<uint64_t>)) + (this->C.capacity() * sizeof(uint64_t)) + (this->effective_alphabet.capacity() * sizeof(uint64_t));
            }
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log;
                log.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=CArray: " + std::to_string(this->size_in_bytes()) + " bytes =");
                log.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");
                return log;
            }
            void print() const
            {
                std::cout << "====== CArray ======" << std::endl;
                stool::DebugPrinter::print_integers(this->C);
                stool::DebugPrinter::print_integers_with_characters(this->effective_alphabet);
                std::cout << "====================" << std::endl;
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
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(CArray):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Effective Alphabet: " << stool::DebugPrinter::to_character_string(this->effective_alphabet) << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            std::vector<uint8_t> get_alphabet() const
            {
                std::vector<uint8_t> r = this->effective_alphabet;
                return r;
            }

            static void save(const CArray &item, std::ofstream &os)
            {
                uint64_t sz1 = item.C.size();
                uint64_t sz2 = item.effective_alphabet.size();
                os.write(reinterpret_cast<const char *>(&sz1), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(&sz2), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(item.C.data()), item.C.size() * sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(item.effective_alphabet.data()), item.effective_alphabet.size() * sizeof(uint8_t));
            }
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
            void swap(CArray &item)
            {
                this->C.swap(item.C);
                this->effective_alphabet.swap(item.effective_alphabet);
            }

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

            void clear()
            {
                this->initialize();
            }

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
            void increase(uint8_t newChar)
            {
                this->increase(newChar, 1);
            }

            void decrease(uint8_t removedChar, int64_t delta)
            {
                this->increase(removedChar, -delta);
            }
            void decrease(uint8_t removedChar)
            {
                this->increase(removedChar, -1);
            }

            uint64_t get_c_count(uint8_t c) const
            {
                uint64_t x = c;
                return this->C[x + 1] - this->C[x];
            }
            uint64_t at(uint64_t i) const
            {
                return this->C[i];
            }

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
            int64_t successor_on_effective_alphabet(uint8_t value) const
            {
                return stool::BasicSearch::successor(this->effective_alphabet, value);
            }
            int64_t predecessor_on_effective_alphabet(uint8_t value) const
            {
                return stool::BasicSearch::predecessor(this->effective_alphabet, value);
            }
            bool occurs_in_effective_alphabet(uint8_t value) const
            {
                return this->get_c_count(value) > 0;
            }
            uint8_t get_character_in_effective_alphabet(uint64_t i) const
            {
                return this->effective_alphabet[i];
            }
            uint64_t effective_alphabet_size() const
            {
                return this->effective_alphabet.size();
            }
        };
    }
}
