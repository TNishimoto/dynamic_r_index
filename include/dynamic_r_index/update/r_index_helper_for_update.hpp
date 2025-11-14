#pragma once
#include "./additional_information_updating_r_index.hpp"
#include "./position_information.hpp"
#include "./preprocessing_result_for_insertion.hpp"
#include "./preprocessing_result_for_deletion.hpp"
#include "./primitive_update_operations.hpp"

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
        /**
         * @brief Helper functions for R-Index updates [Unchecked AI comment].
         * \ingroup DynamicRIndexes
         */
        class RIndexHelperForUpdate
        {

        public:
            static PreprocessingResultForInsertion phase_AB_for_insertion(TextIndex i, const std::vector<uint8_t> &inserted_string, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {
                uint8_t prev_c = inserted_string[inserted_string.size() - 1];

                PreprocessingResultForInsertion result;
                uint64_t original_text_size = dbwt.text_size();
                result.ISA_i_PI.p = disa.isa(i, dbwt);
                RunPosition i_on_rlbwt = dbwt.to_run_position(result.ISA_i_PI.p);
                result.value_at_y_minus = disa.phi(i > 0 ? i - 1 : original_text_size - 1);
                result.value_at_y_plus = disa.inverse_phi(i > 0 ? i - 1 : original_text_size - 1);
                result.old_char = dbwt.get_char(i_on_rlbwt.run_index);
                //result.isSpecialLF = dbwt.check_special_LF(result.ISA_i_PI.p, result.ISA_i_PI.p, prev_c, result.old_char);
                result.i_minus = dbwt.LF(i_on_rlbwt.run_index, i_on_rlbwt.position_in_run);
                result.ISA_i_PI.value_at_p_minus = disa.phi(i);
                result.ISA_i_PI.value_at_p_plus = disa.inverse_phi(i);

                PrimitiveUpdateOperations::r_replace(i, i_on_rlbwt, prev_c, result.ISA_i_PI.value_at_p_minus, result.ISA_i_PI.value_at_p_plus, dbwt, disa);
                // result.ISA_i_PI.p_on_rlbwt = dbwt.to_run_position(result.ISA_i_PI.p_on_sa);

                editHistory.replaced_sa_index = result.ISA_i_PI.p;
                editHistory.replaced_char = result.old_char;

                return result;
            }
            static AdditionalInformationUpdatingRIndex phase_C_for_insertion(TextIndex i, const std::vector<uint8_t> &inserted_string, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, const PreprocessingResultForInsertion &phaseABResult)
            {
                PositionInformation current_ISA_i_PI = phaseABResult.ISA_i_PI;
                SAValue current_value_at_y_plus = phaseABResult.value_at_y_plus;
                SAValue current_value_at_y_minus = phaseABResult.value_at_y_minus;
                SAIndex current_i_minus = phaseABResult.i_minus;

                //bool current_isSpecialLF = phaseABResult.isSpecialLF;

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
                    //next_x_PI.p = dbwt.LF(current_ISA_i_PI.p) + (current_isSpecialLF ? 1 : 0);
                    next_x_PI.p = dbwt.LF_for_insertion(current_ISA_i_PI.p, current_ISA_i_PI.p, inserted_string[inserted_string.size() - 1], phaseABResult.old_char);

                    // next_x_PI.p_on_rlbwt = dbwt.to_run_position(next_x_PI.p, true);
                    RunPosition current_i_on_rlbwt = dbwt.to_run_position(current_ISA_i_PI.p);


                    next_x_PI.value_at_p_plus = disa.LF_inverse_phi_for_insertion(current_i_on_rlbwt, current_ISA_i_PI.value_at_p_plus, phaseABResult.old_char, current_ISA_i_PI.p, i, dbwt);

                    //next_x_PI.value_at_p_plus = disa.LF_inverse_phi_for_insertionX(current_i_on_rlbwt, current_ISA_i_PI.value_at_p_plus, current_i_minus, current_ISA_i_PI.p, i, dbwt);


                    //assert(next_x_PI.value_at_p_plus == value_at_p_plus_debug);


                    next_x_PI.value_at_p_minus = disa.LF_phi_for_insertion(current_i_on_rlbwt, current_ISA_i_PI.value_at_p_minus, phaseABResult.old_char, current_ISA_i_PI.p, i, dbwt);
                }

                for (uint64_t w = 0; w < inserted_string.size(); w++)
                {
                    const uint64_t ins_character = w + 1 < inserted_string.size() ? inserted_string[inserted_string.size() - w - 2] : phaseABResult.old_char;
                    uint64_t insert_sa_value = i + inserted_string.size() - w - 1;

                    PrimitiveUpdateOperations::r_insert(insert_sa_value, next_x_PI.p, ins_character, next_x_PI.value_at_p_minus, next_x_PI.value_at_p_plus, dbwt, disa);

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
                        //current_isSpecialLF = dbwt.check_special_LF(current_ISA_i_PI.p, next_x_PI.p, ins_character, phaseABResult.old_char);
                        //SAIndex _lf_x = dbwt.LF(next_x_PI.p);
                        SAIndex new_x_position = dbwt.LF_for_insertion(next_x_PI.p, current_ISA_i_PI.p, ins_character, phaseABResult.old_char);

                        //uint64_t new_x_position = _lf_x + (current_isSpecialLF ? 1 : 0);
                        uint64_t new_i_minus = current_i_minus >= new_x_position ? current_i_minus + 1 : current_i_minus;

                        // next_x_PI.value_at_p_plus = disa.LF_inverse_phi_for_insertion(prev_x_on_rlbwt, next_x_PI.value_at_p_plus, phaseABResult.old_char, current_ISA_i_PI.p, i, dbwt);
                        // next_x_PI.value_at_p_minus = disa.LF_phi_for_insertion(prev_x_on_rlbwt, next_x_PI.value_at_p_minus, phaseABResult.old_char, current_ISA_i_PI.p, i, dbwt);
                        /*
                        #ifdef DEBUG
                        uint64_t debug1 = disa.LF_inverse_phi_for_insertion(prev_x_on_rlbwt, next_x_PI.value_at_p_plus, phaseABResult.old_char, current_ISA_i_PI.p, i, dbwt);
                        #endif
                        */


                        next_x_PI.value_at_p_plus = disa.LF_inverse_phi_for_insertionX(prev_x_on_rlbwt, next_x_PI.value_at_p_plus, new_i_minus, new_x_position, i, dbwt);

                        /*
                        #ifdef DEBUG
                        if (next_x_PI.value_at_p_plus != debug1)
                        {
                            std::cout << "Error!, w = " << w <<  ", collect_value = " << next_x_PI.value_at_p_plus << ", false_value = " << debug1 << std::endl;
                            throw -1;
                        }
                        #endif
                        */

                        #ifdef DEBUG
                        uint64_t debug2 = disa.LF_phi_for_insertion(prev_x_on_rlbwt, next_x_PI.value_at_p_minus, phaseABResult.old_char, current_ISA_i_PI.p, i, dbwt);
                        #endif 


                        next_x_PI.value_at_p_minus = disa.LF_phi_for_insertionX(prev_x_on_rlbwt, next_x_PI.value_at_p_minus, new_i_minus, new_x_position, i, dbwt);
                        
                        #ifdef DEBUG
                        if (next_x_PI.value_at_p_minus != debug2)
                        {
                            std::cout << "Error2! X = " << debug2  << std::endl;
                            throw -1;
                        }
                        #endif
                        


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
                PreprocessingResultForInsertion phaseABResult = phase_AB_for_insertion(i, inserted_string, editHistory, dbwt, disa);
                return phase_C_for_insertion(i, inserted_string, editHistory, dbwt, disa, phaseABResult);
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
                if (len < 1)
                {
                    throw std::logic_error("The length of the deleted substring is at least 1.");
                }
                PreprocessingResultForDeletion phaseABResult = phase_AB_for_deletion(u, len, editHistory, dbwt, disa);
                return phase_C_for_deletion(u, len, editHistory, dbwt, disa, phaseABResult, sa_arrays_for_debug);
            }

        private:
            static PreprocessingResultForDeletion phase_AB_for_deletion(TextIndex u, uint64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {
                PreprocessingResultForDeletion inf;

                inf.ISA_v_PI.value_at_p = u + len;
                inf.ISA_v_PI.p = disa.isa(inf.ISA_v_PI.value_at_p, dbwt);
                inf.ISA_v_PI.value_at_p_minus = disa.phi(inf.ISA_v_PI.value_at_p);
                inf.ISA_v_PI.value_at_p_plus = disa.inverse_phi(inf.ISA_v_PI.value_at_p);

                RunPosition v_on_rlbwt = dbwt.to_run_position(inf.ISA_v_PI.p);
                inf.old_char = dbwt.get_char(v_on_rlbwt.run_index);

                inf.LF_v = dbwt.LF(v_on_rlbwt.run_index, v_on_rlbwt.position_in_run);

                SAIndex u_on_sa = inf.LF_v;
                for (uint64_t i = 0; i + 1 < len; i++)
                {
                    u_on_sa = dbwt.LF(u_on_sa);
                }
                if (u_on_sa != disa.isa(u, dbwt))
                {
                    std::cout << "u: " << u << ", " << "len = " << len << ", " << dbwt.size() << std::endl;
                    throw -1;
                }

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
            static AdditionalInformationUpdatingRIndex phase_C_for_deletion(TextIndex u, int64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, const PreprocessingResultForDeletion &phaseABResult, std::vector<std::vector<uint64_t>> *sa_arrays_for_debug)
            {

                PositionInformation ISA_u_minus_PI = phaseABResult.ISA_u_minus_PI;
                PositionInformation ISA_v_PI = phaseABResult.ISA_v_PI;

                SAValue phi_x = disa.phi(u + len - 1);
                SAValue inv_phi_x = disa.inverse_phi(u + len - 1);

                RunPosition v_on_rlbwt = dbwt.to_run_position(ISA_v_PI.p);
                PrimitiveUpdateOperations::r_replace(u+len, v_on_rlbwt, phaseABResult.new_char, ISA_v_PI.value_at_p_minus, ISA_v_PI.value_at_p_plus, dbwt, disa);

                uint64_t current_x = phaseABResult.LF_v;
                RunPosition x_on_rlbwt = dbwt.to_run_position(current_x);
                uint8_t x_character = dbwt.get_char(x_on_rlbwt.run_index);

                


                // std::vector<std::vector<uint64_t>> sa_arrays;

                uint64_t debug_phi_x;
                uint64_t debug_inv_phi_x;
                if (sa_arrays_for_debug != nullptr)
                {
                    // nds->construct_SA_arrays_for_deletion(u, len, sa_arrays);
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


                    if ((x_character > phaseABResult.new_char) || (x_character == phaseABResult.new_char && current_x > ISA_v_PI.p ))
                    {
                        next_x--;
                    }

                    SAValue next_phi_x = disa.LF_phi_for_deletion(x_on_rlbwt, phi_x, dbwt, current_x, next_x, u + w + 1, ISA_v_PI.value_at_p_minus);

                    SAValue next_inv_phi_x = disa.LF_inverse_phi_for_deletion(x_on_rlbwt, inv_phi_x, dbwt, current_x, next_x, u + w + 1, ISA_v_PI.value_at_p_plus);

                    PrimitiveUpdateOperations::r_delete_for_deletion(x_on_rlbwt, x_character, phi_x, inv_phi_x, u + w, dbwt, disa);

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
                            stool::DebugPrinter::print_integers((*sa_arrays_for_debug)[counter]);
                            std::cout << "next phi_x: " << phi_x << " -> " << next_phi_x << " / " << debug_phi_x << std::endl;
                            std::cout << "next inv_phi_x: " << inv_phi_x << " -> " << next_inv_phi_x << " / " << debug_inv_phi_x << std::endl;
                            throw std::logic_error("next_phi_x != sa_arrays[current][current_x-1]");
                        }
                        if (next_inv_phi_x != debug_inv_phi_x)
                        {
                            throw std::logic_error("next_inv_phi_x != sa_arrays[current][current_x+1]");
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

                //RunPosition v_on_rlbwt = dbwt.to_run_position(ISA_v_PI.p);
                //PrimitiveUpdateOperations::r_replace(u, v_on_rlbwt, phaseABResult.new_char, ISA_v_PI.value_at_p_minus, ISA_v_PI.value_at_p_plus, dbwt, disa);

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

                PrimitiveUpdateOperations::r_delete(y_on_rlbwt, y_char, y_PI.value_at_p_minus, y_PI.value_at_p_plus, dbwt, disa);
                PrimitiveUpdateOperations::r_insert(y_PI.value_at_p, z_PI.p, y_char, z_PI.value_at_p_minus, z_PI.value_at_p_plus, dbwt, disa);

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
