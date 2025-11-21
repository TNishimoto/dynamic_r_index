#pragma once
#include "b_tree_plus_alpha/include/all.hpp"
#include "../dynamic_fm_index/dynamic_bwt.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        /**
         * @brief A dynamic data structure for accessing SA and ISA. This implementation requires $O(n log n)$ words for the input string of length $n$. [Unchecked AI comment].
         * @note This class is used for debugging purposes.
         */
        class DynamicISA
        {
            stool::bptree::DynamicPermutation dp;
            //stool::old_implementations::VPomPermutation pom;

        public:
            /**
             * @brief Default constructor for DynamicISA.
             */
            DynamicISA(){
                this->clear();
            }
            DynamicISA &operator=(const DynamicISA &) = delete;
            DynamicISA(DynamicISA &&) noexcept = default;
            DynamicISA &operator=(DynamicISA &&) noexcept = default;

            // PermutationOrderMaintenance pom;

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Initializers and Builders
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Sets the degree for the dynamic permutation.
             * @param degree The degree to set.
             */
            /*
            void set_degree(int64_t degree)
            {
                //this->dp.set_degree(degree);
                this->clear();
            }
            */
            /**
             * @brief Clears the dynamic permutation and initializes it with a default value.
             */
            void clear()
            {
                this->dp.clear();
                this->dp.insert(0, 0);
            }

            /**
             * @brief Builds the dynamic permutation using the provided suffix array.
             * @param sa The suffix array to build from.
             */
            void build(std::vector<uint64_t> &sa)
            {
                this->dp.build(sa.begin(), sa.end(), sa.size());
                //this->dp.build(sa);
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Const Methods
            ///   The const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Returns the inverse suffix array value at index i.
             * @param i The index to access.
             * @return The inverse suffix array value.
             */
            int64_t isa(uint64_t i) const
            {
                assert(i < this->dp.size());
                return this->dp.inverse(i);
            }
            /**
             * @brief Returns the suffix array value at index i.
             * @param i The index to access.
             * @return The suffix array value.
             */
            int64_t sa(uint64_t i) const
            {
                assert(i < this->dp.size());
                return this->dp.access(i);
            }

            /**
             * @brief Returns the suffix array as a vector.
             * @return A vector containing the suffix array.
             */
            std::vector<uint64_t> get_sa() const
            {
                return this->dp.to_pi_vector();
            }
            /**
             * @brief Returns the inverse suffix array as a vector.
             * @return A vector containing the inverse suffix array.
             */
            std::vector<uint64_t> get_isa() const
            {
                return this->dp.to_inverse_pi_vector();
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Non-const Methods
            ///   The public non-const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Updates the dynamic permutation by moving an index.
             * @param j The index to move.
             * @param j_prime The new position for the index.
             */
            void move_update(int64_t j, int64_t j_prime)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif

                this->dp.move_pi_index(j, j_prime);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::isa_update_time += elapsed;
// stool::isa_update_count += sa_move_history.size();
#endif
            }

            /**
             * @brief Updates the dynamic permutation for a deletion.
             * @param removed_sa_index The index to remove.
             */
            void update_for_deletion(int64_t removed_sa_index)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif

                this->dp.erase(removed_sa_index);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::isa_update_time += elapsed;
                stool::isa_update_count += 1;
#endif
            }

            /**
             * @brief Updates the dynamic permutation for an insertion.
             * @param inserted_sa_index The index to insert.
             * @param inserted_text_position The position in the text to insert.
             */
            void update_for_insertion(int64_t inserted_sa_index, int64_t inserted_text_position)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif
                this->dp.insert(inserted_sa_index, inserted_text_position);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::isa_update_time += elapsed;
                stool::isa_update_count += 1;
#endif
            }
            //@}
        };

    }
}
