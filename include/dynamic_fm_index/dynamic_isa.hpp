#pragma once
#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"
#include "./dynamic_bwt.hpp"

namespace stool
{
    namespace fm_index
    {
        ////////////////////////////////////////////////////////////////////////////////
        /// @class      DynamicISA
        /// @brief      A dynamic data structure for accessing SA and ISA. This implementation requires $O(n log n)$ words for the input string of length $n$.
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class DynamicISA
        {
            stool::bptree::DynamicPermutation dp;
            //stool::old_implementations::VPomPermutation pom;

        public:
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
            void set_degree(int64_t degree)
            {
                this->dp.set_degree(degree);
                this->clear();
            }
            void clear()
            {
                this->dp.clear();
                this->dp.insert(0, 0);
            }

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
            int64_t isa(uint64_t i) const
            {
                assert(i < this->dp.size());
                return this->dp.inverse(i);
            }
            int64_t sa(uint64_t i) const
            {
                assert(i < this->dp.size());
                return this->dp.access(i);
            }

            std::vector<uint64_t> get_sa() const
            {
                return this->dp.get_pi_vector();
            }
            std::vector<uint64_t> get_isa() const
            {
                return this->dp.get_inverse_pi_vector();
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Public Non-const Methods
            ///   The public non-const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{
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
