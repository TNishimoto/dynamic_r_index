#pragma once
#include "../../dynamic_fm_index/dynamic_bwt.hpp"
#include "../../dynamic_fm_index/dynamic_isa.hpp"
#include "./../dynamic_rlbwt_helper.hpp"
#include "./../dynamic_phi.hpp"
#include "./../sampling_sa_builder.hpp"
#include "./../rlbwt_builder.hpp"

#include "./additional_information_updating_r_index.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        class PrimitiveUpdateOperations
        {
        public:
            static void r_replace(TextIndex x, RunPosition x_on_rlbwt, uint8_t new_char, SAValue phi_x, SAValue inv_phi_x, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {
                assert(dbwt.run_count() == disa.run_count());
                DetailedReplacement type = DynamicRLBWTHelper::replace_char(dbwt, x_on_rlbwt, new_char);
                disa.replace_element_for_insertion(x_on_rlbwt.run_index, phi_x, x, inv_phi_x, type);
                assert(dbwt.run_count() == disa.run_count());
            }

            static void r_insert(TextIndex u, RunPosition i_on_rlbwt, uint8_t c, SAValue value_at_i_minus, SAValue value_at_i_plus, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {

                auto pair = DynamicRLBWTHelper::compute_insertion_type_and_position(dbwt, i_on_rlbwt, c);
                DynamicRLBWTHelper::insert_char_for_insertion(dbwt, pair.second, c, pair.first);
                disa.insert_element_for_insertion(pair.second.run_index, value_at_i_minus, u, value_at_i_plus, pair.first);
                assert(dbwt.run_count() == disa.run_count());
            }
            static void r_insert(TextIndex u, uint64_t i, uint8_t c, SAValue value_at_i_minus, SAValue value_at_i_plus, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {
                RunPosition i_on_rlbwt = dbwt.to_run_position(i, true);
                r_insert(u, i_on_rlbwt, c, value_at_i_minus, value_at_i_plus, dbwt, disa);
            }

            static std::pair<SAValue, SAValue> r_delete(RunPosition u_on_rlbwt, uint8_t u_c, SAValue phi_u, SAValue inv_phi_u, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {

                RunRemovalType type = DynamicRLBWTHelper::remove_char(dbwt, u_on_rlbwt, u_c);
                disa.remove_element_for_insertion(u_on_rlbwt.run_index, phi_u, inv_phi_u, type);
                return std::make_pair(phi_u, inv_phi_u);
            }
            static std::pair<SAValue, SAValue> r_delete_for_deletion(RunPosition u_on_rlbwt, uint8_t u_c, SAValue phi_u, SAValue inv_phi_u, uint64_t remove_value, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {

                RunRemovalType type = DynamicRLBWTHelper::remove_char(dbwt, u_on_rlbwt, u_c);
                disa.remove_element_for_insertion(u_on_rlbwt.run_index, phi_u, inv_phi_u, type);
                disa.shrink_text(remove_value);
                return std::make_pair(phi_u, inv_phi_u);
            }
        };
    }
}