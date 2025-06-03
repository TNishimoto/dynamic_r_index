#pragma once
#include "../dynamic_fm_index/dynamic_bwt.hpp"
#include "../dynamic_fm_index/dynamic_isa.hpp"
#include "./dynamic_rlbwt_helper.hpp"
#include "./dynamic_phi.hpp"
#include "./sampling_sa_builder.hpp"
#include "./rlbwt_builder.hpp"

#include "./additional_information_updating_r_index.hpp"


namespace stool
{
    namespace dynamic_r_index
    {
#ifdef TIME_DEBUG
        uint64_t __single_reorder_BWT_count = 0;
        uint64_t __single_reorder_BWT_time = 0;

        void __reset_time()
        {
            __single_reorder_BWT_count = 0;
            __single_reorder_BWT_time = 0;
            __reset_time_DynamicRLBWT();
        }

        void __print_time()
        {
            std::cout << "single_reorder_BWT, count: " << __single_reorder_BWT_count << ", time: " << __single_reorder_BWT_time << " ns, " << (__single_reorder_BWT_time / __single_reorder_BWT_count) << " ns/per" << std::endl;

            __print_time_DynamicRLBWT();
        }

#endif

        struct PositionInformation
        {
            uint64_t p = UINT64_MAX;
            SAValue value_at_p = UINT64_MAX;
            SAValue value_at_p_minus = UINT64_MAX;
            SAValue value_at_p_plus = UINT64_MAX;

            void update_for_deletion(uint64_t removed_position_on_text, SAIndex removed_position_on_sa, uint64_t upper_sa_value, uint64_t lower_sa_value)
            {
                if (this->value_at_p > removed_position_on_text)
                {
                    this->value_at_p--;
                }
                if (removed_position_on_sa < this->p)
                {
                    this->p--;
                }
                if (this->value_at_p_minus == removed_position_on_text)
                {
                    this->value_at_p_minus = upper_sa_value;
                }
                if (this->value_at_p_minus > removed_position_on_text)
                {
                    this->value_at_p_minus--;
                }

                if (this->value_at_p_plus == removed_position_on_text)
                {
                    this->value_at_p_plus = lower_sa_value;
                }
                if (this->value_at_p_plus > removed_position_on_text)
                {
                    this->value_at_p_plus--;
                }
            }
        };

        struct PhaseABResult
        {
        public:
            PositionInformation ISA_i_PI;
            SAValue value_at_y_minus = UINT64_MAX;
            SAValue value_at_y_plus = UINT64_MAX;
            uint8_t old_char = UINT8_MAX;
            SAIndex i_minus = UINT64_MAX;
            bool isSpecialLF = false;
        };

        struct PhaseABResultForDeletion
        {
        public:
            PositionInformation ISA_v_PI;
            PositionInformation ISA_u_minus_PI;

            // SAIndex i = UINT64_MAX;
            //  uint8_t i_char = UINT8_MAX;
            // SAIndex j = UINT64_MAX;
            uint8_t old_char = UINT8_MAX;
            uint8_t new_char = UINT8_MAX;

            uint64_t LF_v = UINT64_MAX;
            // uint64_t v_on_sa = UINT64_MAX;
            // RunPosition v_on_rlbwt;

            // SAIndex i_minus = UINT64_MAX;
            // bool isSpecialLF = false;
        };

        class RIndexPrimitiveUpdateOperations
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

        class RIndexHelperForUpdate
        {

        public:
            static PhaseABResult phase_AB_for_insertion(TextIndex i, const std::vector<uint8_t> &inserted_string, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {
                uint8_t prev_c = inserted_string[inserted_string.size() - 1];

                PhaseABResult result;
                uint64_t original_text_size = dbwt.text_size();
                result.ISA_i_PI.p = disa.isa(i, dbwt);
                RunPosition i_on_rlbwt = dbwt.to_run_position(result.ISA_i_PI.p);
                result.value_at_y_minus = disa.phi(i > 0 ? i - 1 : original_text_size - 1);
                result.value_at_y_plus = disa.inverse_phi(i > 0 ? i - 1 : original_text_size - 1);
                result.old_char = dbwt.get_char(i_on_rlbwt.run_index);
                result.isSpecialLF = dbwt.check_special_LF(result.ISA_i_PI.p, result.ISA_i_PI.p, prev_c, result.old_char);
                result.i_minus = dbwt.LF(i_on_rlbwt.run_index, i_on_rlbwt.position_in_run);
                result.ISA_i_PI.value_at_p_minus = disa.phi(i);
                result.ISA_i_PI.value_at_p_plus = disa.inverse_phi(i);

                RIndexPrimitiveUpdateOperations::r_replace(i, i_on_rlbwt, prev_c, result.ISA_i_PI.value_at_p_minus, result.ISA_i_PI.value_at_p_plus, dbwt, disa);
                // result.ISA_i_PI.p_on_rlbwt = dbwt.to_run_position(result.ISA_i_PI.p_on_sa);

                editHistory.replaced_sa_index = result.ISA_i_PI.p;
                editHistory.replaced_char = result.old_char;

                return result;
            }
            static AdditionalInformationUpdatingRIndex phase_C_for_insertion(TextIndex i, const std::vector<uint8_t> &inserted_string, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, const PhaseABResult &phaseABReuslt)
            {
                PositionInformation current_ISA_i_PI = phaseABReuslt.ISA_i_PI;
                SAValue current_value_at_y_plus = phaseABReuslt.value_at_y_plus;
                SAValue current_value_at_y_minus = phaseABReuslt.value_at_y_minus;
                SAIndex current_i_minus = phaseABReuslt.i_minus;
                bool current_isSpecialLF = phaseABReuslt.isSpecialLF;

                PositionInformation final_x_PI;
                SAValue final_value_at_y_plus = UINT64_MAX;
                SAValue final_value_at_y_minus = UINT64_MAX;

                for (uint64_t k = 0; k < inserted_string.size(); k++)
                {
                    disa.extend_text(i);
                    if (current_ISA_i_PI.value_at_p_plus >= i)
                    {
                        current_ISA_i_PI.value_at_p_plus++;
                    }
                    if (current_ISA_i_PI.value_at_p_minus >= i)
                    {
                        current_ISA_i_PI.value_at_p_minus++;
                    }

                    if (current_value_at_y_plus >= i)
                    {
                        current_value_at_y_plus++;
                    }
                    if (current_value_at_y_minus >= i)
                    {
                        current_value_at_y_minus++;
                    }
                }

                PositionInformation next_x_PI;
                // Initialize x_PI.
                {
                    next_x_PI.p = dbwt.LF(current_ISA_i_PI.p) + (current_isSpecialLF ? 1 : 0);
                    // next_x_PI.p_on_rlbwt = dbwt.to_run_position(next_x_PI.p, true);
                    RunPosition current_i_on_rlbwt = dbwt.to_run_position(current_ISA_i_PI.p);
                    next_x_PI.value_at_p_plus = disa.LF_inverse_phi_for_insertion(current_i_on_rlbwt, current_ISA_i_PI.value_at_p_plus, phaseABReuslt.old_char, current_ISA_i_PI.p, i, dbwt);
                    next_x_PI.value_at_p_minus = disa.LF_phi_for_insertion(current_i_on_rlbwt, current_ISA_i_PI.value_at_p_minus, phaseABReuslt.old_char, current_ISA_i_PI.p, i, dbwt);
                }

                for (uint64_t w = 0; w < inserted_string.size(); w++)
                {
                    const uint64_t ins_character = w + 1 < inserted_string.size() ? inserted_string[inserted_string.size() - w - 2] : phaseABReuslt.old_char;
                    uint64_t insert_sa_value = i + inserted_string.size() - w - 1;

                    RIndexPrimitiveUpdateOperations::r_insert(insert_sa_value, next_x_PI.p, ins_character, next_x_PI.value_at_p_minus, next_x_PI.value_at_p_plus, dbwt, disa);

                    // Update positions and sa-values.
                    {
                        if (next_x_PI.p == current_i_minus)
                        {
                            current_value_at_y_minus = insert_sa_value;
                        }
                        if (next_x_PI.p == current_i_minus + 1)
                        {
                            current_value_at_y_plus = insert_sa_value;
                        }
                        if (current_i_minus >= next_x_PI.p)
                        {
                            current_i_minus++;
                        }
                        if (current_ISA_i_PI.p >= next_x_PI.p)
                        {
                            current_ISA_i_PI.p++;
                        }
                    }

                    editHistory.inserted_sa_indexes.push_back(next_x_PI.p);
                    editHistory.current_replaced_sa_indexes.push_back(current_ISA_i_PI.p);

                    if (w + 1 < inserted_string.size())
                    {
                        RunPosition prev_x_on_rlbwt = dbwt.to_run_position(next_x_PI.p);
                        current_isSpecialLF = dbwt.check_special_LF(current_ISA_i_PI.p, next_x_PI.p, ins_character, phaseABReuslt.old_char);
                        SAIndex _lf_x = dbwt.LF(next_x_PI.p);
                        uint64_t new_x_position = _lf_x + (current_isSpecialLF ? 1 : 0);
                        uint64_t new_i_minus = current_i_minus >= new_x_position ? current_i_minus + 1 : current_i_minus;

                        // next_x_PI.value_at_p_plus = disa.LF_inverse_phi_for_insertion(prev_x_on_rlbwt, next_x_PI.value_at_p_plus, phaseABReuslt.old_char, current_ISA_i_PI.p, i, dbwt);
                        // next_x_PI.value_at_p_minus = disa.LF_phi_for_insertion(prev_x_on_rlbwt, next_x_PI.value_at_p_minus, phaseABReuslt.old_char, current_ISA_i_PI.p, i, dbwt);
                        uint64_t debug1 = disa.LF_inverse_phi_for_insertion(prev_x_on_rlbwt, next_x_PI.value_at_p_plus, phaseABReuslt.old_char, current_ISA_i_PI.p, i, dbwt);
                        next_x_PI.value_at_p_plus = disa.LF_inverse_phi_for_insertionX(prev_x_on_rlbwt, next_x_PI.value_at_p_plus, new_i_minus, new_x_position, i, dbwt);

                        if (next_x_PI.value_at_p_plus != debug1)
                        {
                            std::cout << "Error!" << std::endl;
                            throw -1;
                        }

                        uint64_t debug2 = disa.LF_phi_for_insertion(prev_x_on_rlbwt, next_x_PI.value_at_p_minus, phaseABReuslt.old_char, current_ISA_i_PI.p, i, dbwt);
                        next_x_PI.value_at_p_minus = disa.LF_phi_for_insertionX(prev_x_on_rlbwt, next_x_PI.value_at_p_minus, new_i_minus, new_x_position, i, dbwt);
                        if (next_x_PI.value_at_p_minus != debug2)
                        {
                            std::cout << "Error2!" << std::endl;
                            throw -1;
                        }

                        next_x_PI.p = new_x_position;
                        // next_x_PI.p_on_rlbwt = dbwt.to_run_position(next_x_PI.p, true);
                    }
                    else
                    {
                        final_x_PI.p = next_x_PI.p;
                        // final_x_PI.p_on_rlbwt = dbwt.to_run_position(next_x_PI.p);
                        final_x_PI.value_at_p_minus = next_x_PI.value_at_p_minus;
                        final_x_PI.value_at_p_plus = next_x_PI.value_at_p_plus;

                        final_value_at_y_minus = current_value_at_y_minus;
                        final_value_at_y_plus = current_value_at_y_plus;
                    }
                }

                AdditionalInformationUpdatingRIndex inf;
                inf.y = current_i_minus;
                inf.z = dbwt.LF(final_x_PI.p);
                inf.value_at_y = i > 0 ? i - 1 : dbwt.text_size() - 1;
                inf.value_at_y_minus = final_value_at_y_minus;
                inf.value_at_y_plus = final_value_at_y_plus;
                RunPosition final_x_on_rlbwt = dbwt.to_run_position(final_x_PI.p);
                inf.value_at_z_minus = disa.LF_phi_for_move(final_x_on_rlbwt, final_x_PI.value_at_p_minus, dbwt);
                inf.value_at_z_plus = disa.LF_inverse_phi_for_move(final_x_on_rlbwt, final_x_PI.value_at_p_plus, dbwt);

                editHistory.first_j = inf.y;
                editHistory.first_j_prime = inf.z;

                return inf;
            }

            static AdditionalInformationUpdatingRIndex preprocess_of_string_insertion_operation(TextIndex i, const std::vector<uint8_t> &inserted_string, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {
                PhaseABResult phaseABReuslt = phase_AB_for_insertion(i, inserted_string, editHistory, dbwt, disa);
                return phase_C_for_insertion(i, inserted_string, editHistory, dbwt, disa, phaseABReuslt);
            }

            static bool phase_D(FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, PositionInformation &y_PI, PositionInformation &z_PI)
            {

                if (y_PI.p != z_PI.p)
                {

                    editHistory.move_history.push_back(SAMove(y_PI.p, z_PI.p));
                    RIndexHelperForUpdate::phase_D_iteration(dbwt, disa, y_PI, z_PI);

                    return false;
                }
                else
                {
                    return true;
                }
            }
            static AdditionalInformationUpdatingRIndex preprocess_of_string_deletion_operation(TextIndex u, int64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, std::vector<std::vector<uint64_t>> *sa_arrays_for_debug)
            {
                PhaseABResultForDeletion phaseABResult = phase_AB_for_deletion(u, len, editHistory, dbwt, disa);
                return phase_C_for_deletion(u, len, editHistory, dbwt, disa, phaseABResult, sa_arrays_for_debug);
            }

        private:
            static PhaseABResultForDeletion phase_AB_for_deletion(TextIndex u, int64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {
                PhaseABResultForDeletion inf;

                inf.ISA_v_PI.value_at_p = u + len;
                inf.ISA_v_PI.p = disa.isa(inf.ISA_v_PI.value_at_p, dbwt);
                inf.ISA_v_PI.value_at_p_minus = disa.phi(inf.ISA_v_PI.value_at_p);
                inf.ISA_v_PI.value_at_p_plus = disa.inverse_phi(inf.ISA_v_PI.value_at_p);

                RunPosition v_on_rlbwt = dbwt.to_run_position(inf.ISA_v_PI.p);
                inf.old_char = dbwt.get_char(v_on_rlbwt.run_index);

                inf.LF_v = dbwt.LF(v_on_rlbwt.run_index, v_on_rlbwt.position_in_run);
                SAIndex u_on_sa = disa.isa(u, dbwt);
                RunPosition u_on_rlbwt = dbwt.to_run_position(u_on_sa);
                inf.new_char = dbwt.get_char(u_on_rlbwt.run_index);

                inf.ISA_u_minus_PI.value_at_p = u > 0 ? u - 1 : dbwt.text_size() - 1;
                inf.ISA_u_minus_PI.p = dbwt.LF(u_on_rlbwt.run_index, u_on_rlbwt.position_in_run);
                inf.ISA_u_minus_PI.value_at_p_minus = disa.phi(inf.ISA_u_minus_PI.value_at_p);
                inf.ISA_u_minus_PI.value_at_p_plus = disa.inverse_phi(inf.ISA_u_minus_PI.value_at_p);

                editHistory.replaced_sa_index = inf.ISA_v_PI.p;
                editHistory.type = EditType::DeletionOfString;

                return inf;
            }
            static AdditionalInformationUpdatingRIndex phase_C_for_deletion(TextIndex u, int64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, const PhaseABResultForDeletion &phaseABResult, std::vector<std::vector<uint64_t>> *sa_arrays_for_debug)
            {

                PositionInformation ISA_u_minus_PI = phaseABResult.ISA_u_minus_PI;
                PositionInformation ISA_v_PI = phaseABResult.ISA_v_PI;

                uint64_t current_x = phaseABResult.LF_v;
                RunPosition x_on_rlbwt = dbwt.to_run_position(current_x);
                uint8_t x_character = dbwt.get_char(x_on_rlbwt.run_index);
                SAValue phi_x = disa.phi(u + len - 1);
                SAValue inv_phi_x = disa.inverse_phi(u + len - 1);

                //std::vector<std::vector<uint64_t>> sa_arrays;

                uint64_t debug_phi_x;
                uint64_t debug_inv_phi_x;
                if (sa_arrays_for_debug != nullptr)
                {
                    //nds->construct_SA_arrays_for_deletion(u, len, sa_arrays);
                    debug_phi_x = current_x > 0 ? (*sa_arrays_for_debug)[0][current_x - 1] : (*sa_arrays_for_debug)[0][(*sa_arrays_for_debug)[0].size() - 1];
                    debug_inv_phi_x = current_x + 1 < dbwt.size() ? (*sa_arrays_for_debug)[0][current_x + 1] : (*sa_arrays_for_debug)[0][0];

                    if (phi_x != debug_phi_x)
                    {
                        throw std::logic_error("phi_x != sa_arrays[0][current_x-1]");
                    }
                    if (inv_phi_x != debug_inv_phi_x)
                    {
                        throw std::logic_error("inv_phi_x != sa_arrays[0][current_x+1]");
                    }
                }

                uint64_t counter = 0;
                for (int64_t w = len - 1; w >= 0; w--)
                {
                    // uint64_t lf_u = (u + w) > 0 ? u + w - 1 : dbwt.size() - 1;
                    uint64_t next_x = dbwt.LF(current_x);

                    if (ISA_v_PI.p <= current_x && x_character == phaseABResult.old_char)
                    {
                        next_x--;
                    }
                    else if (phaseABResult.old_char < x_character)
                    {

                        next_x--;
                    }

                    SAValue next_phi_x = disa.LF_phi_for_deletion(x_on_rlbwt, phi_x, dbwt, current_x, next_x, u + w + 1, ISA_v_PI.value_at_p_minus);
                    SAValue next_inv_phi_x = disa.LF_inverse_phi_for_deletion(x_on_rlbwt, inv_phi_x, dbwt, current_x, next_x, u + w + 1, ISA_v_PI.value_at_p_plus);

                    RIndexPrimitiveUpdateOperations::r_delete_for_deletion(x_on_rlbwt, x_character, phi_x, inv_phi_x, u + w, dbwt, disa);
                    if (next_phi_x > u + w)
                    {
                        next_phi_x--;
                    }
                    if (next_inv_phi_x > u + w)
                    {
                        next_inv_phi_x--;
                    }

                    ISA_u_minus_PI.update_for_deletion(u + w, current_x, phi_x, inv_phi_x);
                    ISA_v_PI.update_for_deletion(u + w, current_x, phi_x, inv_phi_x);

                    counter++;

                    if (sa_arrays_for_debug != nullptr)
                    {
                        uint64_t debug_phi_x = next_x > 0 ? (*sa_arrays_for_debug)[counter][next_x - 1] : (*sa_arrays_for_debug)[counter][(*sa_arrays_for_debug)[counter].size() - 1];
                        uint64_t debug_inv_phi_x = next_x + 1 < dbwt.size() ? (*sa_arrays_for_debug)[counter][next_x + 1] : (*sa_arrays_for_debug)[counter][0];
                        if (next_phi_x != debug_phi_x)
                        {
                            throw std::logic_error("phi_x != sa_arrays[current][current_x-1]");
                        }
                        if (next_inv_phi_x != debug_inv_phi_x)
                        {
                            throw std::logic_error("inv_phi_x != sa_arrays[current][current_x+1]");
                        }
                    }

                    if (w > 0)
                    {
                        x_on_rlbwt = dbwt.to_run_position(next_x);
                        x_character = dbwt.get_char(x_on_rlbwt.run_index);
                        current_x = next_x;
                        phi_x = next_phi_x;
                        inv_phi_x = next_inv_phi_x;
                    }
                }

                RunPosition v_on_rlbwt = dbwt.to_run_position(ISA_v_PI.p);
                RIndexPrimitiveUpdateOperations::r_replace(u, v_on_rlbwt, phaseABResult.new_char, ISA_v_PI.value_at_p_minus, ISA_v_PI.value_at_p_plus, dbwt, disa);

                AdditionalInformationUpdatingRIndex inf;
                inf.y = ISA_u_minus_PI.p;
                // inf.z = current_x;
                inf.z = dbwt.LF(ISA_v_PI.p);

                // inf.z = dbwt.LF(v_on_sa);
                inf.value_at_y = ISA_u_minus_PI.value_at_p;
                inf.value_at_y_minus = ISA_u_minus_PI.value_at_p_minus;
                inf.value_at_y_plus = ISA_u_minus_PI.value_at_p_plus;
                RunPosition final_x_on_rlbwt = dbwt.to_run_position(ISA_v_PI.p);
                inf.value_at_z_minus = disa.LF_phi_for_move(final_x_on_rlbwt, ISA_v_PI.value_at_p_minus, dbwt);
                inf.value_at_z_plus = disa.LF_inverse_phi_for_move(final_x_on_rlbwt, ISA_v_PI.value_at_p_plus, dbwt);

                editHistory.first_j = inf.y;
                editHistory.first_j_prime = inf.z;

                return inf;
            }

            static void phase_D_iteration(DynamicRLBWT &dbwt, DynamicPhi &disa, PositionInformation &y_PI, PositionInformation &z_PI)
            {

#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point __st1, __st2;
                __st1 = std::chrono::system_clock::now();

#endif

                RunPosition y_on_rlbwt = dbwt.to_run_position(y_PI.p);
                uint8_t y_char = dbwt.get_char(y_on_rlbwt.run_index);

                PositionInformation next_y_PI;
                next_y_PI.p = dbwt.LF(y_on_rlbwt.run_index, y_on_rlbwt.position_in_run);
                next_y_PI.value_at_p_minus = disa.LF_phi_for_move(y_on_rlbwt, y_PI.value_at_p_minus, dbwt);
                next_y_PI.value_at_p_plus = disa.LF_inverse_phi_for_move(y_on_rlbwt, y_PI.value_at_p_plus, dbwt);
                next_y_PI.value_at_p = y_PI.value_at_p > 0 ? y_PI.value_at_p - 1 : dbwt.text_size() - 1;

                RIndexPrimitiveUpdateOperations::r_delete(y_on_rlbwt, y_char, y_PI.value_at_p_minus, y_PI.value_at_p_plus, dbwt, disa);
                RIndexPrimitiveUpdateOperations::r_insert(y_PI.value_at_p, z_PI.p, y_char, z_PI.value_at_p_minus, z_PI.value_at_p_plus, dbwt, disa);

                RunPosition z_on_rlbwt = dbwt.to_run_position(z_PI.p);

                PositionInformation next_z_PI;
                next_z_PI.p = dbwt.LF(z_on_rlbwt.run_index, z_on_rlbwt.position_in_run);
                next_z_PI.value_at_p_minus = disa.LF_phi_for_move(z_on_rlbwt, z_PI.value_at_p_minus, dbwt);
                next_z_PI.value_at_p_plus = disa.LF_inverse_phi_for_move(z_on_rlbwt, z_PI.value_at_p_plus, dbwt);

                y_PI = next_y_PI;
                z_PI = next_z_PI;

                assert(dbwt.run_count() == disa.run_count());

#ifdef TIME_DEBUG
                __st2 = std::chrono::system_clock::now();
                uint64_t __ns_time = std::chrono::duration_cast<std::chrono::nanoseconds>(__st2 - __st1).count();
                __single_reorder_BWT_count++;
                __single_reorder_BWT_time += __ns_time;
#endif
            }
        };
    }
}
