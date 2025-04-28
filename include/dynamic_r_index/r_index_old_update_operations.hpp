#pragma once
#include "./r_index_helper_for_update.hpp"
namespace stool
{
    namespace r_index
    {
        class RIndexOldUpdateOperations
        {
            static SAValue update_sa_value_for_insertion_operation(SAValue value, TextIndex positionToInsert)
            {
                return value < positionToInsert ? value : value + 1;
            }
            static SAIndex update_sa_index_for_insertion_operation(SAIndex index, SAIndex positionToInsert)
            {
                return index < positionToInsert ? index : index + 1;
            }

        public:
            static SAValue compute_sa_value_of_z_minus_or_z(RunPosition i_on_rlbwt, SAIndex i, SAIndex j, SAIndex z, SAValue value_at_i_minus, const DynamicRLBWT &dbwt, const DynamicPhi &disa)
            {
                i_on_rlbwt = dbwt.to_run_position(i);
                if (j > z)
                {
                    SAValue value_at_z_minus = disa.LF_phi(i_on_rlbwt, value_at_i_minus, dbwt);
                    return value_at_z_minus;
                }
                else
                {
                    SAValue value_at_jp = disa.LF_phi(i_on_rlbwt, value_at_i_minus, dbwt);
                    return value_at_jp;
                }
            }

            static void r_insert(TextIndex u, RunPosition i_on_rlbwt, uint8_t c, SAValue value_at_i_minus, SAValue value_at_i_plus, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub)
            {
                // disa.extend_text(u);
                // sub.extend_text(u);

                //*value_at_i_minus = update_sa_value_for_insertion_operation(*value_at_i_minus, u);
                //*value_at_i_plus = update_sa_value_for_insertion_operation(*value_at_i_plus, u);

                auto pair = DynamicRLBWTHelper::compute_insertion_type_and_position(dbwt, i_on_rlbwt, c);

                DynamicRLBWTHelper::insert_char_for_insertion(dbwt, pair.second, c, pair.first);
                disa.insert_element_for_insertion(pair.second.run_index, value_at_i_minus, u, value_at_i_plus, pair.first);
                sub.insert_sa_value(value_at_i_minus, u, value_at_i_plus);
                assert(dbwt.run_count() == disa.run_count());
            }

            static void replace_char_phase(TextIndex x, RunPosition x_on_rlbwt, uint8_t new_char, SAValue phi_x, SAValue inv_phi_x, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub)
            {
                assert(dbwt.run_count() == disa.run_count());

                DetailedReplacement type = DynamicRLBWTHelper::replace_char(dbwt, x_on_rlbwt, new_char);

                disa.replace_element_for_insertion(x_on_rlbwt.run_index, phi_x, x, inv_phi_x, type);
                sub.insert_sa_value(phi_x, x, inv_phi_x);

                assert(dbwt.run_count() == disa.run_count());
            }
            static void insert_char_phase(TextIndex u, RunPosition i_on_rlbwt, uint8_t old_char, SAValue *value_at_i_minus, SAValue *value_at_i_plus, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub)
            {
                disa.extend_text(u);
                sub.extend_text(u);

                *value_at_i_minus = RIndexOldUpdateOperations::update_sa_value_for_insertion_operation(*value_at_i_minus, u);
                *value_at_i_plus = RIndexOldUpdateOperations::update_sa_value_for_insertion_operation(*value_at_i_plus, u);

                auto pair = DynamicRLBWTHelper::compute_insertion_type_and_position(dbwt, i_on_rlbwt, old_char);

                DynamicRLBWTHelper::insert_char_for_insertion(dbwt, pair.second, old_char, pair.first);
                disa.insert_element_for_insertion(pair.second.run_index, *value_at_i_minus, u, *value_at_i_plus, pair.first);
                sub.insert_sa_value(*value_at_i_minus, u, *value_at_i_plus);
                assert(dbwt.run_count() == disa.run_count());
            }

            static void r_extend(uint64_t u, DynamicPhi &disa, SubPhiDataStructure &sub)
            {
                disa.extend_text(u);
                sub.extend_text(u);
            }

            static void r_shrink(uint64_t u, DynamicPhi &disa, SubPhiDataStructure &sub)
            {
                disa.shrink_text(u);
                sub.shrink_text(u);
            }
            static std::pair<SAValue, SAValue> r_delete(RunPosition u_on_rlbwt, uint8_t u_c, SAValue u, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub)
            {
                RunRemovalType type = DynamicRLBWTHelper::remove_char(dbwt, u_on_rlbwt, u_c);
                SAValue phi_u = sub.phi(u, disa);
                SAValue inv_phi_u = sub.inverse_phi(u, disa);

                disa.remove_element_for_insertion(u_on_rlbwt.run_index, phi_u, inv_phi_u, type);
                sub.remove_sa_value(phi_u, u, inv_phi_u);

                return std::make_pair(phi_u, inv_phi_u);
            }

            static void delete_char_phase(RunPosition u_on_rlbwt, uint8_t u_c, SAValue u, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub)
            {
                RunRemovalType type = DynamicRLBWTHelper::remove_char(dbwt, u_on_rlbwt, u_c);
                SAValue phi_u = sub.phi(u, disa);
                SAValue inv_phi_u = sub.inverse_phi(u, disa);

                disa.remove_element_for_insertion(u_on_rlbwt.run_index, phi_u, inv_phi_u, type);
                sub.remove_sa_value(phi_u, u, inv_phi_u);

                disa.shrink_text(u);
                sub.shrink_text(u);
            }
            static void print(const DynamicRLBWT &dbwt, const DynamicPhi &disa, const SubPhiDataStructure &sub, std::string name, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "====== Print(" << name << ") ======" << std::endl;
                dbwt.print(message_paragraph + 1);
                disa.print_content(message_paragraph + 1);
                sub.print(message_paragraph + 1);
                auto sa = sub.get_sa(disa);
                stool::DebugPrinter::print_integers(sa, "SA");
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==============================" << std::endl;
            }

            static void verify_inf(const DynamicRLBWT &dbwt, const DynamicPhi &disa, const SubPhiDataStructure &sub, const AdditionalInformationUpdatingRIndex &inf, std::string name)
            {
                auto sa = sub.get_sa(disa);

                if (inf.value_at_y != sa[inf.y])
                {
                    RIndexOldUpdateOperations::print(dbwt, disa, sub, name);
                    inf.print();
                }
                assert(inf.value_at_y == sa[inf.y]);

                if (inf.y != inf.get_z())
                {
                    uint64_t p = inf.y_star != -1 ? inf.y_star : dbwt.text_size() - 1;
                    if (inf.value_at_y_star != sa[p])
                    {
                        RIndexOldUpdateOperations::print(dbwt, disa, sub, name);
                        inf.print();
                    }
                    assert(inf.value_at_y_star == sa[p]);
                }
            }

            static void verify(DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub, std::string name)
            {
                try
                {
                    dbwt.verify(1);
                    disa.verify(1);
                    sub.verify(disa);
                }
                catch (std::logic_error e)
                {
                    print(dbwt, disa, sub, name);
                    std::cout << "An error occurs in " << name << std::endl;
                    throw e;
                }
            }
            static AdditionalInformationUpdatingRIndex preprocess_of_string_deletion_operation(TextIndex u, int64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub)
            {

                TextIndex v = u + len;
                assert(v < (uint64_t)dbwt.text_size());
                SAIndex v_on_sa = disa.isa(v, dbwt);
                RunPosition v_on_rlbwt = dbwt.to_run_position(v_on_sa);
                uint8_t old_char = dbwt.get_char(v_on_rlbwt.run_index);

                SAIndex i = dbwt.LF(v_on_rlbwt.run_index, v_on_rlbwt.position_in_run);
                SAIndex u_on_sa = disa.isa(u, dbwt);
                RunPosition u_on_rlbwt = dbwt.to_run_position(u_on_sa);
                uint64_t j = dbwt.LF(u_on_rlbwt.run_index, u_on_rlbwt.position_in_run);

                editHistory.replaced_sa_index = v_on_sa;
                editHistory.type = EditType::DeletionOfString;

                uint8_t i_character = UINT8_MAX;

                {
                    uint64_t v_phi = sub.phi(v, disa);
                    uint64_t inv_v_phi = sub.inverse_phi(v, disa);
                    sub.insert_sa_value(v_phi, v, inv_v_phi);
                }

                for (int64_t w = len - 1; w >= 0; w--)
                {

                    RunPosition i_on_rlbwt = dbwt.to_run_position(i);
                    i_character = dbwt.get_char(i_on_rlbwt.run_index);
                    uint64_t next_i = dbwt.LF(i_on_rlbwt.run_index, i_on_rlbwt.position_in_run);
                    bool b = dbwt.check_special_LF(v_on_sa, i, i_character, old_char);
                    if (b)
                    {
                        next_i--;
                    }
                    // int gap = i <= next_i ? 1 : 0;

                    RIndexOldUpdateOperations::delete_char_phase(i_on_rlbwt, i_character, u + w, dbwt, disa, sub);

                    editHistory.deleted_sa_indexes.push_back(i);

                    if (i < j)
                    {
                        j--;
                    }
                    if (i < v_on_sa)
                    {
                        v_on_sa--;
                    }

                    i = next_i;
                }
                v_on_rlbwt = dbwt.to_run_position(v_on_sa);

                SAValue phi_v = sub.phi(u, disa);
                SAValue inv_phi_v = sub.inverse_phi(u, disa);

                RIndexOldUpdateOperations::replace_char_phase(u, v_on_rlbwt, i_character, phi_v, inv_phi_v, dbwt, disa, sub);

                AdditionalInformationUpdatingRIndex inf;
                inf.compute_and_set_y_and_y_star(j, dbwt.LF(v_on_sa));
                inf.value_at_y = u - 1;
                inf.value_at_y_star = UINT64_MAX;

                if (inf.y != inf.get_z())
                {
                    uint64_t i = v_on_sa;
                    RunPosition i_on_rlbwt = dbwt.to_run_position(i);
                    SAValue phi_i = sub.phi(u, disa);
                    inf.value_at_y_star = RIndexOldUpdateOperations::compute_sa_value_of_z_minus_or_z(i_on_rlbwt, i, inf.y, inf.get_z(), phi_i, dbwt, disa);
                }
                editHistory.first_j = inf.y;
                editHistory.first_j_prime = inf.get_z();

                return inf;
            }

            static AdditionalInformationUpdatingRIndex preprocess_of_char_deletion_operation(TextIndex u, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub)
            {

                SAIndex u_on_sa = disa.isa(u, dbwt);
                TextIndex u_plus = u + 1 < (uint64_t)dbwt.text_size() ? u + 1 : 0;
                RunPosition u_on_rlbwt = dbwt.to_run_position(u_on_sa);
                SAIndex u_plus_on_sa = disa.isa(u_plus, dbwt);
                RunPosition u_plus_on_rlbwt = dbwt.to_run_position(u_plus_on_sa);

                editHistory.replaced_sa_index = u_plus_on_sa;
                editHistory.inserted_sa_index = u_on_sa;

                uint8_t u_character = dbwt.get_char(u_on_rlbwt.run_index);
                // uint8_t u_plus_character = dbwt.access(u_plus_on_rlbwt.run_index);
                uint64_t j = dbwt.LF(u_on_rlbwt.run_index, u_on_rlbwt.position_in_run);
                j = j < u_on_sa ? j : j - 1;
                // SAValue value_at_j = u > 0 ? u - 1 : dbwt.text_size() - 2;

                // SAValue phi_u = disa.phi(u);
                // SAValue inv_phi_u = disa.inverse_phi(u);
                SAValue phi_u_plus = disa.phi(u_plus);
                SAValue inv_phi_u_plus = disa.inverse_phi(u_plus);

                RIndexOldUpdateOperations::replace_char_phase(u_plus, u_plus_on_rlbwt, u_character, phi_u_plus, inv_phi_u_plus, dbwt, disa, sub);
                u_on_rlbwt = dbwt.to_run_position(u_on_sa);
                RIndexOldUpdateOperations::delete_char_phase(u_on_rlbwt, u_character, u, dbwt, disa, sub);

                // disa.shrink_text(u);
                // sub.shrink_text(u);

                u_plus_on_sa = u_plus_on_sa < u_on_sa ? u_plus_on_sa : u_plus_on_sa - 1;
                // uint64_t j_prime = this->dbwt.LF(u_plus_on_sa);

                AdditionalInformationUpdatingRIndex inf;
                inf.compute_and_set_y_and_y_star(j, dbwt.LF(u_plus_on_sa));
                inf.value_at_y = u - 1;
                inf.value_at_y_star = UINT64_MAX;

                if (inf.y != inf.get_z())
                {
                    uint64_t i = u_plus_on_sa;
                    RunPosition i_on_rlbwt = dbwt.to_run_position(i);
                    // TextIndex updated_u_plus = u + 1 < (uint64_t)dbwt.text_size() ? u + 1 : 0;
                    TextIndex updated_u_plus = u;

                    SAValue phi_i = sub.phi(updated_u_plus, disa);

                    inf.value_at_y_star = RIndexOldUpdateOperations::compute_sa_value_of_z_minus_or_z(i_on_rlbwt, i, inf.y, inf.get_z(), phi_i, dbwt, disa);
                }
                return inf;

                // FMIndexEditHistory editHistory;
            }
            static void single_reorder_BWT2(DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub, AdditionalInformationUpdatingRIndex &inf)
            {

#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point __st1, __st2;
                __st1 = std::chrono::system_clock::now();

#endif

                RunPosition y_on_rlbwt = dbwt.to_run_position(inf.y);

                uint8_t c = dbwt.get_char(y_on_rlbwt.run_index);
                uint64_t new_y = dbwt.LF(y_on_rlbwt.run_index, y_on_rlbwt.position_in_run);

                auto sa_value_pair = r_delete(y_on_rlbwt, c, inf.value_at_y, dbwt, disa, sub);

                SAValue __value_at_y_minus = sa_value_pair.first;
                SAValue __value_at_y_plus = sa_value_pair.second;
                assert(inf.value_at_y < dbwt.text_size());

                // SAValue __value_at_y_star = inf.value_at_y_star;
                SAValue __value_at_y_star_plus = sub.inverse_phi(inf.value_at_y_star, disa);

                RunPosition z_on_rlbwt = dbwt.to_run_position(inf.get_z(), true);
                SAValue __value_at_z_minus = inf.value_at_y_star != inf.value_at_y ? inf.value_at_y_star : __value_at_y_minus;
                SAValue __value_at_jp = __value_at_y_star_plus != inf.value_at_y ? __value_at_y_star_plus : __value_at_y_plus;

                r_insert(inf.value_at_y, z_on_rlbwt, c, __value_at_z_minus, __value_at_jp, dbwt, disa, sub);

                z_on_rlbwt = dbwt.to_run_position(inf.get_z());
                int64_t new_z = dbwt.LF(z_on_rlbwt.run_index, z_on_rlbwt.position_in_run);
                inf.compute_and_set_y_and_y_star(new_y, new_z);
                inf.value_at_y = inf.value_at_y > 0 ? inf.value_at_y - 1 : dbwt.text_size() - 1;
                inf.value_at_y_star = disa.LF_phi(z_on_rlbwt, __value_at_z_minus, dbwt);

                assert(dbwt.run_count() == disa.run_count());

#ifdef TIME_DEBUG
                __st2 = std::chrono::system_clock::now();
                uint64_t __ns_time = std::chrono::duration_cast<std::chrono::nanoseconds>(__st2 - __st1).count();
                __single_reorder_BWT_count++;
                __single_reorder_BWT_time += __ns_time;
#endif
            }

            static bool reorder_RLBWT2(FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub, AdditionalInformationUpdatingRIndex &inf)
            {

                if (inf.y != inf.get_z())
                {

                    editHistory.move_history.push_back(SAMove(inf.y, inf.get_z()));
                    RIndexOldUpdateOperations::single_reorder_BWT2(dbwt, disa, sub, inf);

                    return false;
                }
                else
                {
                    return true;
                }
            }

            static PhaseABResultForDeletion phase_AB_for_deletion(TextIndex u, int64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa)
            {
                PhaseABResultForDeletion result;

                TextIndex v = u + len;
                assert(v < (uint64_t)dbwt.text_size());

                result.ISA_v_PI.p = disa.isa(v, dbwt);
                RunPosition v_on_rlbwt = dbwt.to_run_position(result.ISA_v_PI.p);
                result.old_char = dbwt.get_char(v_on_rlbwt.run_index);

                result.i = dbwt.LF(v_on_rlbwt.run_index, v_on_rlbwt.position_in_run);
                SAIndex u_on_sa = disa.isa(u, dbwt);
                RunPosition u_on_rlbwt = dbwt.to_run_position(u_on_sa);
                result.j = dbwt.LF(u_on_rlbwt.run_index, u_on_rlbwt.position_in_run);

                editHistory.replaced_sa_index = result.ISA_v_PI.p;
                editHistory.type = EditType::DeletionOfString;

                //uint8_t i_character = UINT8_MAX;

                result.ISA_v_PI.value_at_p_minus = disa.phi(v);
                result.ISA_v_PI.value_at_p_plus = disa.inverse_phi(v);

                return result;
            }
            static AdditionalInformationUpdatingRIndex phase_C_for_deletion(TextIndex u, int64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, const PhaseABResultForDeletion &phaseABReuslt, SubPhiDataStructure &sub)
            {
                
                uint64_t current_i = phaseABReuslt.i;
                uint64_t current_v_on_sa = phaseABReuslt.ISA_v_PI.p;
                uint8_t i_character = UINT8_MAX;
                uint64_t current_j = phaseABReuslt.j;

                for (int64_t w = len - 1; w >= 0; w--)
                {

                    RunPosition i_on_rlbwt = dbwt.to_run_position(current_i);
                    i_character = dbwt.get_char(i_on_rlbwt.run_index);
                    uint64_t next_i = dbwt.LF(i_on_rlbwt.run_index, i_on_rlbwt.position_in_run);
                    bool b = dbwt.check_special_LF(current_v_on_sa, current_i, i_character, phaseABReuslt.old_char);
                    if (b)
                    {
                        next_i--;
                    }
                    // int gap = i <= next_i ? 1 : 0;

                    RIndexOldUpdateOperations::delete_char_phase(i_on_rlbwt, i_character, u + w, dbwt, disa, sub);

                    editHistory.deleted_sa_indexes.push_back(current_i);

                    if (current_i < current_j)
                    {
                        current_j--;
                    }
                    if (current_i < current_v_on_sa)
                    {
                        current_v_on_sa--;
                    }

                    current_i = next_i;
                }
                RunPosition v_on_rlbwt = dbwt.to_run_position(current_v_on_sa);

                SAValue phi_v = sub.phi(u, disa);
                SAValue inv_phi_v = sub.inverse_phi(u, disa);

                RIndexOldUpdateOperations::replace_char_phase(u, v_on_rlbwt, i_character, phi_v, inv_phi_v, dbwt, disa, sub);

                AdditionalInformationUpdatingRIndex inf;
                inf.compute_and_set_y_and_y_star(current_j, dbwt.LF(current_v_on_sa));
                inf.value_at_y = u - 1;
                inf.value_at_y_star = UINT64_MAX;

                if (inf.y != inf.get_z())
                {
                    uint64_t i = current_v_on_sa;
                    RunPosition i_on_rlbwt = dbwt.to_run_position(i);
                    SAValue phi_i = sub.phi(u, disa);
                    inf.value_at_y_star = RIndexOldUpdateOperations::compute_sa_value_of_z_minus_or_z(i_on_rlbwt, i, inf.y, inf.get_z(), phi_i, dbwt, disa);
                }
                editHistory.first_j = inf.y;
                editHistory.first_j_prime = inf.get_z();

                return inf;
            }
            static AdditionalInformationUpdatingRIndex preprocess_of_char_deletion_operation(TextIndex u, uint64_t len, FMIndexEditHistory &editHistory, DynamicRLBWT &dbwt, DynamicPhi &disa, SubPhiDataStructure &sub)
            {
                PhaseABResultForDeletion phaseABReuslt = RIndexOldUpdateOperations::phase_AB_for_deletion(u, len, editHistory, dbwt, disa);
                return RIndexOldUpdateOperations::phase_C_for_deletion(u, len, editHistory, dbwt, disa, phaseABReuslt, sub);
            }


        };
    }
}