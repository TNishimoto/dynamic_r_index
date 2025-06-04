/**
 * @file fm_index_edit_history.hpp
 * @brief Header file for the FMIndexEditHistory class and related enums.
 */

#pragma once
#include "stool/include/light_stool.hpp"
#include "./tools.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        

        ////////////////////////////////////////////////////////////////////////////////
        /// @class      FMIndexEditHistory
        /// @brief      Values for updating dynamic FM-index.
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class FMIndexEditHistory
        {
        public:
            EditType type;  ///< The type of edit performed.
            int64_t replaced_sa_index;  ///< The index of the suffix array that was replaced.
            int64_t inserted_sa_index;  ///< The index of the suffix array where the insertion occurred.
            // int64_t checker_sa_index;
            uint8_t replaced_char;  ///< The character that was replaced.

            int64_t deleted_sa_index;  ///< The index of the suffix array that was deleted.
            std::vector<uint8_t> inserted_string;  ///< The string that was inserted.
            std::vector<uint64_t> inserted_sa_indexes;  ///< Indices of the suffix array where insertions occurred.
            std::vector<uint64_t> deleted_sa_indexes;  ///< Indices of the suffix array where deletions occurred.
            std::vector<uint64_t> current_replaced_sa_indexes;  ///< Current indices of the suffix array that were replaced.

            std::vector<SAMove> move_history;  ///< History of moves made during the edit.

            uint64_t first_j;  ///< The first index in the suffix array.
            uint64_t first_j_prime;  ///< The first index in the suffix array after the edit.

            /**
             * @brief Clears all the data stored in the edit history.
             */
            void clear(){
                this->inserted_string.clear();
                this->inserted_sa_indexes.clear();
                this->deleted_sa_indexes.clear();
                this->move_history.clear();
                
            }

            /**
             * @brief Prints the edit history to the console.
             */
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