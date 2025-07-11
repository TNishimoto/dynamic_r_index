#include "./temporary_r_index.hpp"

std::vector<TemporaryRIndex> construct_temp_array(const std::vector<uint8_t> &text, const std::vector<uint8_t> &chars, uint64_t insertion_pos, const std::vector<uint8_t> &inserted_string, stool::dynamic_r_index::FMIndexEditHistory &editHistory)
{
    stool::DebugPrinter::print_characters(text, "text");
    stool::DebugPrinter::print_characters(inserted_string, "inserted_string");
    std::cout << "Pos: " << insertion_pos << std::endl;

    std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
    std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
    std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

    stool::dynamic_r_index::DynamicRIndex drfmi = stool::dynamic_r_index::DynamicRIndex::build_from_BWT(bwt, chars, stool::Message::NO_MESSAGE);

    drfmi.insert_string(insertion_pos, inserted_string, editHistory);

    std::vector<TemporaryRIndex> arr;
    arr.push_back(TemporaryRIndex(bwt, sa));
    arr.push_back(arr[0].expand(insertion_pos, inserted_string.size(), editHistory.replaced_sa_index, inserted_string[inserted_string.size() - 1]));

    for (uint64_t i = 0; i < inserted_string.size(); i++)
    {
        uint8_t c = i + 1 < inserted_string.size() ? inserted_string[inserted_string.size() - i - 2] : editHistory.replaced_char;
        uint64_t v = editHistory.inserted_sa_indexes[i];
        arr.push_back(arr[i + 1].r_insert(v, c, insertion_pos + inserted_string.size() - i - 1));
    }

    for (uint64_t i = 0; i < editHistory.move_history.size(); i++)
    {
        uint64_t delete_index = editHistory.move_history[i].first;
        uint64_t move_index = editHistory.move_history[i].second;
        arr.push_back(arr[arr.size() - 1].r_move(delete_index, move_index));
    }
    return arr;
}

void func3(const std::vector<uint8_t> &text, const std::vector<uint8_t> &chars, uint64_t insertion_pos, const std::vector<uint8_t> &inserted_string)
{
    stool::dynamic_r_index::FMIndexEditHistory editHistory;
    std::vector<TemporaryRIndex> arr = construct_temp_array(text, chars, insertion_pos, inserted_string, editHistory);

    for (uint64_t i = 0; i < arr.size(); i++)
    {
        if (i == 0)
        {
            arr[i].check_LF_for_default();
        }
        else if (i == 1)
        {
            std::cout << "REP: " << (char)editHistory.replaced_char << "/" << editHistory.replaced_sa_index << std::endl;
        }
        else if (2 <= i && i < 2 + inserted_string.size())
        {
            std::cout << "ISNERT: " << editHistory.inserted_sa_indexes[i - 2] << std::endl;
        }
        else
        {
            std::cout << "MOVE" << std::endl;
        }
        arr[i].print();
    }
}

void func2(const std::vector<uint8_t> &text, const std::vector<uint8_t> &chars, uint64_t insertion_pos, const std::vector<uint8_t> &inserted_string)
{
    stool::dynamic_r_index::FMIndexEditHistory editHistory;
    std::vector<TemporaryRIndex> arr = construct_temp_array(text, chars, insertion_pos, inserted_string, editHistory);

    for (uint64_t i = 0; i < arr.size(); i++)
    {
        // std::cout << "IDX: " << i << std::endl;
        arr[i].print();

        if (i == 0)
        {
            arr[i].check_LF_for_default();
        }
        else if (i == 1)
        {
            std::cout << "Phase A: " << (char)editHistory.replaced_char << "/" << editHistory.replaced_sa_index << std::endl;
        }
        else if (2 <= i && i < 2 + inserted_string.size())
        {
            std::cout << "Phase B: " << (i - 1) << std::endl;

            uint64_t j = i - 2;
            [[maybe_unused]]uint64_t inserted_sa_value = insertion_pos + inserted_string.size() - j - 1;

            [[maybe_unused]]bool is_special_LF = false;
            uint64_t _inx = j > 0 ? editHistory.inserted_sa_indexes[j - 1] : editHistory.replaced_sa_index;
            uint64_t _inx2 = j > 0 ? editHistory.current_replaced_sa_indexes[j - 1] : editHistory.replaced_sa_index;

            std::cout << "rep: " << _inx2 << "/ins: " << editHistory.inserted_sa_indexes[j] << ", prev_ins:" << _inx << "/old Char: " << (char)editHistory.replaced_char << std::endl;

            [[maybe_unused]]uint8_t c = j + 1 < inserted_string.size() ? inserted_string[inserted_string.size() - j - 2] : editHistory.replaced_char;
            uint8_t prev_c = inserted_string[inserted_string.size() - j - 1];

            if (editHistory.replaced_char < prev_c)
            {
                is_special_LF = true;
            }
            else if (editHistory.replaced_char == prev_c && _inx2 <= _inx)
            {
                is_special_LF = true;
            }

            {
                auto test_phi_pairs = arr[i - 1].compute_LF_phi_for_insertion_all(editHistory.replaced_char, _inx2, insertion_pos, text.size() + inserted_string.size() - 1);
                /*
                stool::DebugPrinter::print_characters(arr[i - 1].bwt, "prev BWT");
                stool::DebugPrinter::print_characters(arr[i].bwt, "BWT     ");

                stool::DebugPrinter::print_integers(arr[i - 1].sa, "prev SA");
                stool::DebugPrinter::print_integers(arr[i].sa, "SA     ");
                */

                for (uint64_t k = 0; k < test_phi_pairs.size(); k++)
                {
                    [[maybe_unused]]uint64_t test_sa_value = test_phi_pairs[k].second;
                    [[maybe_unused]]uint64_t correct_sa_value = arr[i].phi_function(test_phi_pairs[k].first);
                    //std::cout << test_phi_pairs[k].first << " -> " << test_sa_value << std::endl;
                    // assert(test_sa_value == correct_sa_value);
                }

                auto test_inverse_phi_pairs = arr[i - 1].compute_inverse_LF_phi_for_insertion_all(editHistory.replaced_char, _inx2, insertion_pos, text.size() + inserted_string.size() - 1);

                /*
                for(uint64_t k = 0; k < test_inverse_phi_pairs.size(); k++){
                    uint64_t test_sa_value = test_inverse_phi_pairs[k].second;
                    uint64_t correct_sa_value = arr[i].inverse_phi_function(test_inverse_phi_pairs[k].first);
                    //std::cout << test_inverse_phi_pairs[k].first << " -> " << test_sa_value << "/" << correct_sa_value << std::endl;
                    assert(test_sa_value == correct_sa_value);
                }
                */
                // std::cout << "OK! "  << std::endl;

                // uint64_t correct_sa_value = editHistory.inserted_sa_indexes[j] > 0 ? arr[i].sa[editHistory.inserted_sa_indexes[j] - 1] : arr[i].sa[arr[i].sa.size() - 1];
                // std::cout << "correct_sa_value: " << correct_sa_value << std::endl;
                // std::cout << "test_sa_value: " << test_sa_value << std::endl;
                /*
                std::vector<uint64_t> test_sa = arr[i - 1].accessSA_by_LF_for_insertion(editHistory.replaced_char, _inx2, insertion_pos, text.size() + inserted_string.size() - 1);
                stool::DebugPrinter::print_integers(arr[i].sa, "sa");

                stool::DebugPrinter::print_integers(test_sa, "test_sa");
                stool::equal_check("TestSACheck", test_sa, arr[i].sa);
                */

                /*
                std::cout << "repChar: " << (char)editHistory.replaced_char << "/rep_pos: " << editHistory.current_replaced_sa_indexes[j] << "/ ins_idx: " << editHistory.inserted_sa_indexes[j] << std::endl;
                std::vector<uint64_t> test_sa = arr[i].construct_suffix_array_for_insertion(editHistory.replaced_char, editHistory.current_replaced_sa_indexes[j], inserted_string.size(), text.size(), editHistory.inserted_sa_indexes[j], insertion_pos);

                std::vector<uint64_t> dummy_sa;
                for(uint64_t w=0;w<arr[i].sa.size();w++){
                    if(w == editHistory.inserted_sa_indexes[j]-1){
                        //dummy_sa.push_back(UINT64_MAX);
                    }
                    dummy_sa.push_back(arr[i].sa[w]);
                }
                stool::DebugPrinter::print_integers(test_sa, "test_sa");
                stool::DebugPrinter::print_integers(dummy_sa, "dummy_sa");


                stool::equal_check("TestSACheck", test_sa, dummy_sa);
                */
            }

            {

                uint64_t test_sa_value = arr[i - 1].compute_LF_phi_for_insertion(_inx, editHistory.replaced_char, _inx2, insertion_pos, text.size() + inserted_string.size() - 1);

                uint64_t correct_sa_value = editHistory.inserted_sa_indexes[j] > 0 ? arr[i].sa[editHistory.inserted_sa_indexes[j] - 1] : arr[i].sa[arr[i].sa.size() - 1];

                uint64_t test_sa_value2 = arr[i - 1].compute_LF_inverse_phi_for_insertion(_inx, editHistory.replaced_char, _inx2, insertion_pos, text.size() + inserted_string.size() - 1);

                uint64_t correct_sa_value2 = editHistory.inserted_sa_indexes[j] + 1 < arr[i].sa.size() ? arr[i].sa[editHistory.inserted_sa_indexes[j] + 1] : arr[i].sa[0];

                // std::cout << "INS: " << "/ins_idex: " << editHistory.inserted_sa_indexes[j] << "_inx: " << _inx << "/" << correct_sa_value << "=?" << test_sa_value << "/is_special_LF: " << is_special_LF << std::endl;
                // std::cout << "INS2: " << "/ins_idex: " << editHistory.inserted_sa_indexes[j] << "_inx: " << _inx << "/" << correct_sa_value2 << "=?" << test_sa_value2 << "/is_special_LF: " << is_special_LF << ", inspos: " << insertion_pos << ", inserted_string.size: " << inserted_string.size() << std::endl;

                assert(correct_sa_value == test_sa_value);
                assert(correct_sa_value2 == test_sa_value2);
            }
        }
        else
        {
            std::cout << "Phase C: " << (i - 1 - inserted_string.size()) << std::endl;
            uint64_t j = i - 2 - inserted_string.size();
            uint64_t i_pos = j > 0 ? editHistory.move_history[j - 1].second : editHistory.inserted_sa_indexes[editHistory.inserted_sa_indexes.size() - 1];
            [[maybe_unused]]uint64_t j_prime_pos = editHistory.move_history[j].second;

            auto next_sa = arr[i - 1].construct_next_SA_for_move(editHistory.replaced_char, i_pos, insertion_pos, text.size() + inserted_string.size() - 1);
                stool::DebugPrinter::print_integers(next_sa, "next_sa");
                stool::equal_check("TestSACheck", next_sa, arr[i].sa);
                std::cout << "OK!" << std::endl;


            /*

            uint64_t test_sa_value1 = arr[i - 1].compute_LF_phi_for_move(i_pos);
            uint64_t test_sa_value2 = arr[i - 1].compute_LF_inverse_phi_for_move(i_pos);

            uint64_t correct_sa_value1 = j_prime_pos > 0 ? arr[i].sa[j_prime_pos - 1] : arr[i].sa[arr[i].sa.size() - 1];
            uint64_t correct_sa_value2 = j_prime_pos + 1 < arr[i].sa.size() ? arr[i].sa[j_prime_pos + 1] : arr[i].sa[0];

            std::cout << "MOVE/" << editHistory.move_history[j].first << "->" << editHistory.move_history[j].second << std::endl;
            assert(correct_sa_value1 == test_sa_value1);
            assert(correct_sa_value2 == test_sa_value2);

            if (j + 1 < editHistory.move_history.size() - 1)
            {
                uint64_t e_pos = editHistory.move_history[j].first;
                uint64_t j_pos = editHistory.move_history[j + 1].first;

                uint64_t test_sa_value3 = arr[i - 1].compute_LF_phi_for_move(e_pos);
                uint64_t test_sa_value4 = arr[i - 1].compute_LF_inverse_phi_for_move(e_pos);

                uint64_t correct_sa_value3 = j_pos > 0 ? arr[i].sa[j_pos - 1] : arr[i].sa[arr[i].sa.size() - 1];
                uint64_t correct_sa_value4 = j_pos + 1 < arr[i].sa.size() ? arr[i].sa[j_pos + 1] : arr[i].sa[0];

                // std::cout << "MOVE2/" << editHistory.move_history[j + 1].first << "->" << editHistory.move_history[j + 1].second << std::endl;
                // std::cout << "MOVE3/" << e_pos << ", " << j_pos << "/" << correct_sa_value3 << "=?" << test_sa_value3 << "/" << correct_sa_value4 << "=?" << test_sa_value4 << std::endl;

                assert(correct_sa_value3 == test_sa_value3);
                assert(correct_sa_value4 == test_sa_value4);
            }
            */
        }

    }

}

void func(const std::vector<uint8_t> &text, const std::vector<uint8_t> &chars, uint64_t insertion_pos, const std::vector<uint8_t> &inserted_string)
{
    stool::dynamic_r_index::FMIndexEditHistory editHistory;
    std::vector<TemporaryRIndex> arr = construct_temp_array(text, chars, insertion_pos, inserted_string, editHistory);

    for (uint64_t i = 0; i < arr.size(); i++)
    {
        std::cout << "IDX: " << i << std::endl;
        arr[i].print();

        if (i == 0)
        {
            arr[i].check_LF_for_default();
        }
        else if (i == 1)
        {
            std::cout << "REP: " << (char)editHistory.replaced_char << "/" << editHistory.replaced_sa_index << std::endl;

            arr[i].check_LF_for_replace_phase(editHistory.replaced_char, editHistory.replaced_sa_index);
        }
        else if (2 <= i && i < 2 + inserted_string.size())
        {
            uint64_t j = i - 2;
            uint64_t inserted_sa_value = insertion_pos + inserted_string.size() - j - 1;
            uint8_t c = j + 1 < inserted_string.size() ? inserted_string[inserted_string.size() - j - 2] : editHistory.replaced_char;
            std::cout << "INS: " << (char)c << "/ins_idex: " << editHistory.inserted_sa_indexes[j] << "/" << inserted_sa_value << "/ u_on_sa; " << editHistory.current_replaced_sa_indexes[j] << "/tsize: " << arr[i].bwt.size() << std::endl;
            if (j < inserted_string.size() - 1)
            {
                arr[i].check_LF_for_insertion_phase(editHistory.replaced_char, editHistory.current_replaced_sa_indexes[j], inserted_sa_value, editHistory.inserted_sa_indexes[j]);
            }
            else
            {
                arr[i].check_LF_for_insertion_last_phase(editHistory.replaced_char, editHistory.current_replaced_sa_indexes[j], inserted_sa_value, editHistory.inserted_sa_indexes[j]);
            }
        }
        else
        {
            uint64_t j = i - 2 - inserted_string.size();
            [[maybe_unused]]uint64_t m = j == 0 ? editHistory.inserted_sa_indexes[editHistory.inserted_sa_indexes.size() - 1] : editHistory.move_history[j - 1].second;

            std::cout << "MOVE/" << j << "/" << editHistory.move_history[j].first << " -> " << editHistory.move_history[j].second << "/m = " << (editHistory.current_replaced_sa_indexes[editHistory.current_replaced_sa_indexes.size() - 1]) << std::endl;
            arr[i].check_LF_for_move_phase(editHistory.move_history[j].first, editHistory.move_history[j].second);
        }
    }
}

void main_sub(int mode, bool detailed_check, uint64_t seed)
{
    [[maybe_unused]]uint64_t max_alphabet_type = stool::UInt8VectorGenerator::get_max_alphabet_type();

    if (mode == 1)
    {
        uint64_t text_size = 10;
        uint64_t pattern_size = 5;

        for (uint64_t alphabet_type = 0; alphabet_type <= 3; alphabet_type++)
        {

            for (uint64_t i = 0; i < 300; i++)
            {
                std::cout << alphabet_type << std::flush;

                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);

                uint64_t end_marker = '$';
                std::vector<uint8_t> alphabet_with_end_marker = stool::dynamic_r_index_test::DynamicFMIndexTest::create_alphabet_with_end_marker(chars, end_marker);
                text.push_back(end_marker);

                std::mt19937_64 mt64(seed);

                uint64_t size = text.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                int64_t insertion_pos = get_rand_uni_int(mt64);

                std::vector<uint8_t> inserted_string = stool::UInt8VectorGenerator::create_random_sequence(pattern_size, chars, seed++);
                func2(text, alphabet_with_end_marker, insertion_pos, inserted_string);
            }
        }
    }
    else if (mode == 2)
    {
        for (uint64_t alphabet_type = 0; alphabet_type <= 3; alphabet_type++)
        {

            for (uint64_t i = 0; i < 30; i++)
            {
                std::cout << alphabet_type << std::flush;
                uint64_t text_size = 5;

                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);

                uint64_t end_marker = '$';
                std::vector<uint8_t> alphabet_with_end_marker = stool::dynamic_r_index_test::DynamicFMIndexTest::create_alphabet_with_end_marker(chars, end_marker);
                text.push_back(end_marker);

                std::mt19937_64 mt64(seed);

                uint64_t size = text.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                int64_t insertion_pos = get_rand_uni_int(mt64);

                std::vector<uint8_t> inserted_string = stool::UInt8VectorGenerator::create_random_sequence(5, chars, seed++);
                func(text, alphabet_with_end_marker, insertion_pos, inserted_string);
                // return;
            }
        }
    }
    else if (mode == 3)
    {

        for (uint64_t alphabet_type = 0; alphabet_type <= 3; alphabet_type++)
        {

            for (uint64_t i = 0; i < 3000; i++)
            {
                std::cout << alphabet_type << std::flush;
                uint64_t text_size = 100;

                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);

                uint64_t end_marker = '$';
                std::vector<uint8_t> alphabet_with_end_marker = stool::dynamic_r_index_test::DynamicFMIndexTest::create_alphabet_with_end_marker(chars, end_marker);
                text.push_back(end_marker);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                stool::dynamic_r_index::DynamicRIndex drfmi1 = stool::dynamic_r_index::DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);
                stool::dynamic_r_index::DynamicRIndex drfmi2 = stool::dynamic_r_index::DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                std::mt19937_64 mt64(seed);

                uint64_t size = text.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                int64_t insertion_pos = get_rand_uni_int(mt64);

                std::vector<uint8_t> inserted_string = stool::UInt8VectorGenerator::create_random_sequence(5, chars, seed++);

                std::cout << "Seed: " << seed << std::endl;
                stool::DebugPrinter::print_characters(text, "Text");
                stool::DebugPrinter::print_characters(inserted_string, "Pattern");
                std::cout << "insertion_pos: " << insertion_pos << std::endl;

                stool::dynamic_r_index::FMIndexEditHistory editHistory1;
                stool::dynamic_r_index::FMIndexEditHistory editHistory2;

                [[maybe_unused]]auto inf1 = drfmi1.insert_string(insertion_pos, inserted_string, editHistory1);
                [[maybe_unused]]auto inf2 = drfmi2.insert_string(insertion_pos, inserted_string, editHistory2);

                const stool::dynamic_r_index::DynamicPhi &phi1 = drfmi1.get_dynamic_phi();
                const stool::dynamic_r_index::DynamicPhi &phi2 = drfmi2.get_dynamic_phi();

                auto bwt1 = drfmi1.get_bwt();
                auto bwt2 = drfmi2.get_bwt();

                auto first_sa1 = phi1.get_first_sa_values();
                auto first_sa2 = phi2.get_first_sa_values();

                // stool::DebugPrinter::print_integers(first_sa1, "first_sa1");
                // stool::DebugPrinter::print_integers(first_sa2, "first_sa2");

                auto last_sa1 = phi1.get_last_sa_values();
                auto last_sa2 = phi2.get_last_sa_values();

                stool::equal_check("bwt1", bwt1, bwt2);
                stool::equal_check("first_sa1", first_sa1, first_sa2);
                stool::equal_check("last_sa1", last_sa1, last_sa2);
                /*
                assert(inf1.j == inf2.j);
                assert(inf1.j_star == inf2.j_star);
                assert(inf1.value_at_j == inf2.value_at_j);
                assert(inf1.value_at_j_star == inf2.value_at_j_star);
                */
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 4)
    {
        uint64_t text_size = 20;
        [[maybe_unused]]uint64_t pattern_size = 5;
        uint64_t alphabet_type = 0;
        uint64_t seed = 0;

        std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
        std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);

        uint64_t end_marker = '$';
        std::vector<uint8_t> alphabet_with_end_marker = stool::dynamic_r_index_test::DynamicFMIndexTest::create_alphabet_with_end_marker(chars, end_marker);
        text.push_back(end_marker);

        std::mt19937_64 mt64(seed);

        uint64_t size = text.size();
        std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
        std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

        int64_t insertion_pos = get_rand_uni_int(mt64);

        std::vector<uint8_t> inserted_string = stool::UInt8VectorGenerator::create_random_sequence(5, chars, seed++);

        std::cout << "Seed: " << seed << std::endl;
        stool::DebugPrinter::print_characters(text, "Text");
        stool::DebugPrinter::print_characters(inserted_string, "Pattern");
        std::cout << "insertion_pos: " << insertion_pos << std::endl;

        func3(text, alphabet_with_end_marker, insertion_pos, inserted_string);
    }
    else if (mode == 0)
    {
        for (uint64_t i = 1; i <= 10; i++)
        {
            main_sub(i, detailed_check, seed);
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef DEBUG
    std::cout << "\033[41m";
    std::cout << "DEBUG MODE!" << std::endl;
    std::cout << "\e[m" << std::endl;
    // std::cout << "\033[30m" << std::endl;
#endif
#ifdef SLOWDEBUG
    std::cout << "\033[41m";
    std::cout << "SLOWDEBUG MODE!" << std::endl;
    std::cout << "\e[m" << std::endl;
    // std::cout << "\033[30m" << std::endl;
#endif

    cmdline::parser p;

    // p.add<std::string>("input_file", 'i', "input file name", true);
    p.add<uint>("mode", 'm', "mode", false, 0);
    p.add<uint>("seed", 's', "seed", false, 0);

    p.add<uint>("detailed_check", 'u', "detailed_check", false, 0);

    p.parse_check(argc, argv);
    uint64_t mode = p.get<uint>("mode");
    uint64_t seed = p.get<uint>("seed");

    bool detailed_check = p.get<uint>("detailed_check") == 0 ? false : true;

    main_sub(mode, detailed_check, seed);
}
