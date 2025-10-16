#pragma once
#include "./dynamic_rlbwt.hpp"
#include "stool/include/stool.hpp"


namespace stool
{
    namespace dynamic_r_index
    {
        ////////////////////////////////////////////////////////////////////////////////
        /// @class      RLBWTBuilder
        /// @brief      A builder of RLBWT
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class RLBWTBuilder
        {
        private:
            static uint64_t preprocess(DynamicRLBWT &rlbwt, uint8_t first_char)
            {
                uint64_t pos = 1;


                //rlbwt.insert_new_run(0, first_char, 1);


                RLBWTBuilder::append_char_for_online_construction(rlbwt, first_char, pos);
                return pos;
            }
            static uint64_t finish(DynamicRLBWT &rlbwt, uint8_t last_char, uint64_t current_position, uint8_t end_marker = '$')
            {
                if(rlbwt.get_char(0) != end_marker){
                    throw std::logic_error("Error::finish");
                }
                rlbwt.remove_BWT_run(0);
                current_position--;

                RunPosition pos_on_rlbwt = rlbwt.to_run_position(current_position);
                uint64_t freq = rlbwt.get_c_array().at(last_char);

                uint64_t rank = rlbwt.rank(last_char, pos_on_rlbwt);

                uint64_t new_pos = freq + rank;

                RLBWTBuilder::append_char_for_online_construction(rlbwt, end_marker, new_pos);
                return new_pos;
            }
            static uint64_t compute_next_position_to_insert(DynamicRLBWT &rlbwt, uint8_t c, uint64_t current_position)
            {
                RunPosition pos_on_rlbwt = rlbwt.to_run_position(current_position);
                uint64_t freq = rlbwt.get_c_array().at(c);
                uint64_t rank = rlbwt.rank(c, pos_on_rlbwt);
                uint64_t new_pos = freq + rank;
                return new_pos;
            }
            static void append_char_for_online_construction(DynamicRLBWT &rlbwt, uint8_t c, uint64_t position_on_BWT)
            {
                RunPosition rp = rlbwt.to_run_position(position_on_BWT, true);

                //assert(rp.position_in_run >= 0);

                uint64_t count = rlbwt.run_count();

                if (rp.run_index == count)
                {

                    uint8_t c1 = rlbwt.get_char(rp.run_index - 1);
                    if (c1 == c)
                    {

                        rlbwt.increment_run(rp.run_index - 1, 1);
                    }
                    else
                    {
                        rlbwt.insert_new_run(rp.run_index, c, 1);
                    }
                }
                else if (position_on_BWT == 0)
                {

                    uint8_t c2 = rlbwt.get_char(rp.run_index);
                    if (c2 == c)
                    {
                        rlbwt.increment_run(rp.run_index - 1, 1);
                    }
                    else
                    {
                        rlbwt.insert_new_run(0, c, 1);
                    }
                }
                else
                {

                    uint64_t rl = rlbwt.get_run_length(rp.run_index);
                    uint8_t c1 = rp.position_in_run > 0 ? rlbwt.get_char(rp.run_index) : rlbwt.get_char(rp.run_index - 1);
                    uint8_t c2 = rlbwt.get_char(rp.run_index);

                    if (c1 == c)
                    {

                        if (rp.position_in_run > 0)
                        {
                            rlbwt.increment_run(rp.run_index, 1);
                        }
                        else
                        {

                            rlbwt.increment_run(rp.run_index - 1, 1);
                        }
                    }
                    else if (c2 == c)
                    {
                        rlbwt.increment_run(rp.run_index, 1);
                    }
                    else
                    {

                        if (rp.position_in_run == 0)
                        {
                            rlbwt.insert_new_run(rp.run_index, c, 1);
                        }
                        else
                        {
                            uint64_t d1 = rp.position_in_run;
                            uint64_t d2 = rl - rp.position_in_run;

                            rlbwt.decrement_run(rp.run_index, rl - d1);
                            rlbwt.insert_new_run(rp.run_index + 1, c, 1);
                            rlbwt.insert_new_run(rp.run_index + 2, c2, d2);
                        }
                    }
                }
            }

        public:
            static DynamicRLBWT build(std::vector<uint8_t> &text, uint8_t end_marker = '$')
            {
                std::vector<uint8_t> rev_text = stool::StringFunctions::to_reversed_string(text);
                std::vector<uint8_t> chars = stool::StringFunctions::get_alphabet(rev_text);
                chars.push_back(end_marker);

                uint64_t text_size = rev_text.size();

                if (text_size <= 2)
                {
                    throw std::runtime_error("Error: Short String(online_build_for_reversed_text)");
                }
                else
                {
                    DynamicRLBWT rlbwt = DynamicRLBWT::build_RLBWT_of_end_marker(chars);


                    uint64_t pos = RLBWTBuilder::preprocess(rlbwt, rev_text[0]);
                    uint64_t counter = 0;
                    for (uint64_t i = 0; i + 1 < rev_text.size(); i++)
                    {
                        if(counter == 1000000){
                            std::cout << "Building RLBWT... [" << i << "/" << rev_text.size() << "]" << std::endl;
                            counter = 1;
                        }else{
                            counter++;
                        }
                        uint8_t c1 = rev_text[i];
                        uint8_t c2 = rev_text[i + 1];
                        uint64_t new_pos = RLBWTBuilder::compute_next_position_to_insert(rlbwt, c1, pos);
                        RLBWTBuilder::append_char_for_online_construction(rlbwt, c2, new_pos);
                        pos = new_pos;

                    }

                    RLBWTBuilder::finish(rlbwt, rev_text[rev_text.size() - 1], pos, end_marker);

                    return rlbwt;
                }

            }
            static DynamicRLBWT online_build_for_reversed_text(std::string file_path, uint8_t end_marker = '$', uint64_t buffer_size = 16000)
            {
                std::cout << "Get Alphabets" << std::endl;
                std::vector<uint8_t> chars = stool::OnlineFileReader::get_alphabet(file_path, buffer_size);
                chars.push_back(end_marker);

                std::cout << "Get Text Size" << std::endl;
                uint64_t text_size = stool::OnlineFileReader::get_text_size(file_path);

                stool::DebugPrinter::print_characters(chars,"Alphabet");
                std::cout << "Text length: " << text_size << std::endl;

                std::cout << "Building" << std::endl;

                if (buffer_size < 1000)
                {
                    buffer_size = 1000;
                }

                if (text_size <= 2)
                {
                    throw std::runtime_error("Error: Short String(online_build_for_reversed_text)");
                }
                else
                {
                    DynamicRLBWT rlbwt = DynamicRLBWT::build_RLBWT_of_end_marker(chars);


                    std::vector<uint8_t> buffer;

                    std::ifstream inputStream;
                    inputStream.open(file_path, std::ios::binary);
                    stool::OnlineFileReader::read(inputStream, buffer, buffer_size, text_size);

                    uint64_t pos = RLBWTBuilder::preprocess(rlbwt, buffer[0]);
                    uint64_t pos_on_buffer = 0;
                    uint64_t counter = 0;


                    while (true)
                    {
                        counter++;
                        if (counter % 1000000 == 0)
                        {
                            std::cout << "Building RLBWT... [" << counter << "/" << text_size << "]" << std::endl;
                        }

                        if (pos_on_buffer + 1 < buffer.size())
                        {
                            uint8_t c1 = buffer[pos_on_buffer];
                            uint8_t c2 = buffer[pos_on_buffer + 1];
                            uint64_t new_pos = RLBWTBuilder::compute_next_position_to_insert(rlbwt, c1, pos);
                            RLBWTBuilder::append_char_for_online_construction(rlbwt, c2, new_pos);
                            pos = new_pos;
                            pos_on_buffer++;
                        }
                        else
                        {
                            uint8_t c1 = buffer[pos_on_buffer];
                            bool b = stool::OnlineFileReader::read(inputStream, buffer, buffer_size, text_size);
                            if (b)
                            {
                                uint8_t c2 = buffer[0];
                                uint64_t new_pos = RLBWTBuilder::compute_next_position_to_insert(rlbwt, c1, pos);
                                RLBWTBuilder::append_char_for_online_construction(rlbwt, c2, new_pos);
                                pos = new_pos;
                                pos_on_buffer = 0;
                            }
                            else
                            {
                                RLBWTBuilder::finish(rlbwt, c1, pos, end_marker);
                                break;
                            }
                        }

                    }

                    inputStream.close();
                    return rlbwt;
                }
            }
        };

    }
}
