#pragma once
#include "stool/include/light_stool.hpp"
#include "../tools.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        enum class EditType
        {
            InsertionOfChar,
            InsertionOfString,
            DeletionOfChar,
            DeletionOfString
        };

        ////////////////////////////////////////////////////////////////////////////////
        /// @class      FMIndexEditHistory
        /// @brief      Values for updating dynamic FM-index.
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class FMIndexEditHistory
        {
        public:
            EditType type;
            int64_t replaced_sa_index;
            int64_t inserted_sa_index;
            // int64_t checker_sa_index;
            uint8_t replaced_char;

            int64_t deleted_sa_index;
            std::vector<uint8_t> inserted_string;
            std::vector<uint64_t> inserted_sa_indexes;
            std::vector<uint64_t> deleted_sa_indexes;
            std::vector<uint64_t> current_replaced_sa_indexes;

            std::vector<SAMove> move_history;



            uint64_t first_j;
            uint64_t first_j_prime;

            void clear(){
                this->inserted_string.clear();
                this->inserted_sa_indexes.clear();
                this->deleted_sa_indexes.clear();
                this->move_history.clear();
                
            }

            void print() const
            {

                std::cout << "================ FMIndexEditHistory ================" << std::endl;
                if (this->type == EditType::InsertionOfString)
                {
                    std::cout << "PositionToReplace: " << replaced_sa_index << std::endl;
                    stool::Printer::print_chars("Pattern", this->inserted_string);
                    stool::Printer::print("Inserted_sa_indexes", this->inserted_sa_indexes);
                    std::cout << "j: " << this->first_j << ", j': " << this->first_j_prime << std::endl;

                    for (auto pair : move_history)
                    {
                        std::cout << "Move: " << pair.first << " -> " << pair.second << std::endl;
                    }
                }
                else if (this->type == EditType::DeletionOfString)
                {
                    std::cout << "PositionToReplace: " << replaced_sa_index << std::endl;
                    // std::cout << "PositionToReplace: " << replaced_sa_index << std::endl;

                    // stool::Printer::print_chars("Pattern", this->inserted_string);
                    stool::Printer::print("Deleted_sa_indexes", this->deleted_sa_indexes);
                    std::cout << "j: " << this->first_j << ", j': " << this->first_j_prime << std::endl;

                    for (auto pair : move_history)
                    {
                        std::cout << "Move: " << pair.first << " -> " << pair.second << std::endl;
                    }
                }
                else
                {
                    std::cout << "Rep: " << replaced_sa_index << ", Ins: " << inserted_sa_index << std::endl;
                    std::cout << "j: " << this->first_j << ", j': " << this->first_j_prime << std::endl;

                    for (auto pair : move_history)
                    {
                        std::cout << "Move: " << pair.first << " -> " << pair.second << std::endl;
                    }
                }

                std::cout << "================================" << std::endl;
            }
        };
    }
}