#pragma once
#include "./vectorized_pom_tree.hpp"

namespace stool
{
    namespace old_implementations
    {
    //template<typename T = int64_t>
    class VPomPermutation
    {
        uint8_t page_size_limit;

    public:
        VectorizedPomTree<> pi_tree;
        VectorizedPomTree<> inverse_pi_tree;

        void initialize(uint8_t _page_size_limit)
        {
            this->page_size_limit = _page_size_limit;
            this->pi_tree.initialize(this->page_size_limit, &this->inverse_pi_tree, false);
            this->inverse_pi_tree.initialize(this->page_size_limit, &this->pi_tree, false);
            this->inverse_pi_tree.set_pi_permutation_flag(false);
            //this->inverse_pi_tree.is_pi_permutation = false;
        }
        
        void build(std::vector<uint64_t> &pi)
        {
            uint64_t counter = 0;
            this->clear();

            for (uint64_t i = 0; i < pi.size(); i++)
            {
                counter++;
                if (counter % 100000 == 0)
                {
                    std::cout << "+" << std::flush;
                }

                this->pi_tree.insert(0);
                this->inverse_pi_tree.insert(0);
            }
            if(counter >= 100000){
                std::cout << std::endl;
            }

            counter = 0;


            for (uint64_t i = 0; i < pi.size(); i++)
            {
                counter++;
                if (counter % 100000 == 0)
                {
                    std::cout << "+" << std::flush;
                }
                POMLink pointer1 = this->pi_tree.search_element_by_index(i);
                POMLink pointer2 = this->inverse_pi_tree.search_element_by_index(pi[i]);
                this->pi_tree.set_link(pointer1, pointer2);
                this->inverse_pi_tree.set_link(pointer2, pointer1);
            }
            if(counter >= 100000){
                std::cout << std::endl;
            }

        }
        /*
        void build(const std::vector<uint64_t> &pi, const std::vector<T> &values)
        {
            uint64_t counter = 0;
            this->clear();

            for (uint64_t i = 0; i < pi.size(); i++)
            {
                counter++;
                if (counter % 100000 == 0)
                {
                    std::cout << "+" << std::flush;
                }

                this->pi_tree.insert(0);
                this->inverse_pi_tree.insert(0);
            }

            counter = 0;
            if(counter > 100000){
                std::cout << std::endl;
            }

            for (uint64_t i = 0; i < pi.size(); i++)
            {
                counter++;
                if (counter % 100000 == 0)
                {
                    std::cout << "+" << std::flush;
                }
                POMLink pointer1 = this->pi_tree.search_element_by_index(i);
                POMLink pointer2 = this->inverse_pi_tree.search_element_by_index(pi[i]);
                this->pi_tree.set_link(pointer1, pointer2);                
                this->inverse_pi_tree.set_link(pointer2, pointer1);
                this->pi_tree.set_value(pointer1, values[i]);
            }
            if(counter > 100000){
                std::cout << std::endl;
            }
        }
        */
    

        void insert(int64_t pi_index, int64_t inverse_pi_index)
        {
            //assert(this->pi_tree.is_linked());
            //assert(this->inverse_pi_tree.is_linked());

            POMLink result1 = this->pi_tree.insert(pi_index);
            POMLink result2 = this->inverse_pi_tree.insert(inverse_pi_index);


            this->pi_tree.set_link(result1, result2);
            this->inverse_pi_tree.set_link(result2, result1);
        }
        /*   
             void set_value(int64_t pi_index, T value){
            POMLink idx = this->pi_tree.search_element_by_index(pi_index);
            this->pi_tree.set_value(idx, value);
        }
        T get_value(int64_t pi_index) const {
            POMLink idx = this->pi_tree.search_element_by_index(pi_index);
            return this->pi_tree.get_value(idx);
        }
        */

        void erase(int64_t pi_index)
        {
            //assert(this->pi_tree.is_linked());
            //assert(this->inverse_pi_tree.is_linked());

            uint64_t inverse_pi_index = this->access(pi_index);
            this->pi_tree.erase(pi_index);
            this->inverse_pi_tree.erase(inverse_pi_index);


        }
        void clear()
        {
            this->pi_tree.clear();
            this->inverse_pi_tree.clear();
            this->pi_tree.initialize(this->page_size_limit, &this->inverse_pi_tree, true);
            this->inverse_pi_tree.initialize(this->page_size_limit, &this->pi_tree, false);

        }
        
        void move_pi_index(int64_t from, int64_t to)
        {
            int64_t inverse_pi_index = this->access(from);
            if (from < to)
            {
                this->insert(to + 1, inverse_pi_index);
                this->erase(from);
            }
            else if (from > to)
            {
                this->erase(from);
                this->insert(to, inverse_pi_index);
            }


        }
        void verify() const {
            this->pi_tree.verify();
            this->inverse_pi_tree.verify();
        }


        uint64_t size() const
        {
            return this->pi_tree.size();
        }
        int64_t access(int64_t pi_index) const
        {
            assert(pi_index >= 0);
            assert(pi_index < this->pi_tree.size());

#ifdef TIME_DEBUG
            std::chrono::system_clock::time_point start, end;
            start = std::chrono::system_clock::now();
#endif
            POMLink pointer = this->pi_tree.search_element_by_index(pi_index);
#ifdef TIME_DEBUG
            end = std::chrono::system_clock::now();
            uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            stool::pom_access_time += elapsed;
            stool::pom_access_count += 1;
#endif
            POMLink link = this->pi_tree.get_link(pointer);
            assert(link.first >= 0);
            assert(link.second < this->inverse_pi_tree.get_page_size(link.first));
            int64_t index = this->inverse_pi_tree.get_global_element_index(link);
            return index;

        }
        int64_t inverse(int64_t inverse_pi_index) const
        {
            assert(inverse_pi_index >= 0);
            assert(inverse_pi_index < this->inverse_pi_tree.size());

#ifdef TIME_DEBUG
            std::chrono::system_clock::time_point start, end;
            start = std::chrono::system_clock::now();
#endif
            POMLink pointer = this->inverse_pi_tree.search_element_by_index(inverse_pi_index);
#ifdef TIME_DEBUG
            end = std::chrono::system_clock::now();
            uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            stool::pom_access_time += elapsed;
            stool::pom_access_count += 1;
#endif

            POMLink link = this->inverse_pi_tree.get_link(pointer);
            assert(link.second < this->pi_tree.get_page_size(link.first));
            int64_t index = this->pi_tree.get_global_element_index(link);
            return index;

        }

        std::vector<uint64_t> get_pi_vector() const
        {
            std::vector<uint64_t> r;
            uint64_t size = this->size();
            for (uint64_t i = 0; i < size; i++)
            {
                r.push_back(this->access(i));
            }

            return r;
        }

        std::vector<uint64_t> get_inverse_pi_vector() const
        {
            std::vector<uint64_t> r;
            uint64_t size = this->size();
            for (uint64_t i = 0; i < size; i++)
            {
                r.push_back(this->inverse(i));
            }
            return r;
        }
        /*
        std::vector<T> get_value_vector() const
        {
            std::vector<T> r;
            uint64_t size = this->size();
            for (uint64_t i = 0; i < size; i++)
            {
                POMLink p = this->pi_tree.search_element_by_index(i);
                T value = this->pi_tree.get_value(p);
                r.push_back(value);
            }

            return r;
        }
        */

        void print() const
        {
            std::cout << "=== PermutationOrderMaintenance ===" << std::endl;
            this->pi_tree.print();
            this->inverse_pi_tree.print();
        }
        void print_memory() const {
            this->pi_tree.print_memory();
            this->inverse_pi_tree.print_memory();
        }
    };
    }
}
