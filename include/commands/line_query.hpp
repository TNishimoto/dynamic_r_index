#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include "./tsv_parser.hpp"
namespace stool
{
    /**
     * @brief Enumeration of supported query types
     */
    enum class QueryType
    {
        NONE = 0,         ///< No operation
        INSERT = 1,       ///< Insert a string
        DELETE = 2,       ///< Delete a substring
        COUNT = 3,        ///< Count pattern occurrences
        LOCATE = 4,       ///< Locate all pattern occurrences
        LOCATE_DETAIL = 5,///< Locate with detailed timing information
        LOCATE_SUM = 6,   ///< Compute sum of occurrence positions
        PRINT = 7         ///< Print current text and BWT
    };

    /**
     * @brief Structure representing a single query from a command file
     * 
     * This structure stores the query type and parameters parsed from
     * a TSV command file line.
     */
    struct LineQuery
    {
    public:
        QueryType type = QueryType::NONE;        ///< The type of query
        uint64_t position = UINT64_MAX;          ///< Position parameter (for INSERT/DELETE)
        uint64_t length = UINT64_MAX;            ///< Length parameter (for DELETE)
        std::vector<uint8_t> pattern;            ///< Pattern string (for COUNT/LOCATE/INSERT)

        /**
         * @brief Sanitize a string by converting special characters
         * @param text The input string
         * @return Vector of bytes with special characters converted
         * @note Converts null bytes to newlines and 0x01 to tabs
         */
        static std::vector<uint8_t> sanityze(const std::string &text)
        {
            std::vector<uint8_t> r;
            for (char c : text)
            {
                if (c == 0)
                {
                    r.push_back('\n');
                }
                else if (c == 1)
                {
                    r.push_back('\t');
                }
                else
                {
                    r.push_back(c);
                }
            }
            return r;
        }

        /**
         * @brief Create a NONE query
         * @return A LineQuery with type NONE
         */
        static LineQuery create_NONE_query()
        {
            LineQuery r;
            r.type = QueryType::NONE;
            return r;
        }
        
        /**
         * @brief Create a PRINT query
         * @return A LineQuery with type PRINT
         */
        static LineQuery create_PRINT_query()
        {
            LineQuery r;
            r.type = QueryType::PRINT;
            return r;
        }

        /**
         * @brief Create an INSERT query
         * @param insertion_position The position to insert at
         * @param text The string to insert
         * @return A LineQuery with type INSERT
         */
        static LineQuery create_INSERT_query(uint64_t insertion_position, const std::string &text)
        {
            LineQuery r;
            r.type = QueryType::INSERT;
            r.position = insertion_position;
            r.pattern.clear();

            std::vector<uint8_t> sanitized_text = sanityze(text);
            r.pattern.swap(sanitized_text);

            return r;
        }
        
        /**
         * @brief Create a DELETE query
         * @param deletion_position The starting position to delete from
         * @param length The number of characters to delete
         * @return A LineQuery with type DELETE
         */
        static LineQuery create_DELETE_query(uint64_t deletion_position, uint64_t length)
        {
            LineQuery r;
            r.type = QueryType::DELETE;
            r.position = deletion_position;
            r.length = length;
            r.pattern.clear();
            return r;
        }
        
        /**
         * @brief Create a COUNT query
         * @param text The pattern to count
         * @return A LineQuery with type COUNT
         */
        static LineQuery create_COUNT_query(const std::string &text)
        {
            LineQuery r;
            r.type = QueryType::COUNT;
            r.pattern.clear();

            std::vector<uint8_t> sanitized_text = sanityze(text);
            r.pattern.swap(sanitized_text);

            return r;
        }
        
        /**
         * @brief Create a LOCATE query
         * @param text The pattern to locate
         * @return A LineQuery with type LOCATE
         */
        static LineQuery create_LOCATE_query(const std::string &text)
        {
            LineQuery r;
            r.type = QueryType::LOCATE;
            r.pattern.clear();

            std::vector<uint8_t> sanitized_text = sanityze(text);
            r.pattern.swap(sanitized_text);

            return r;
        }
        /**
         * @brief Create a LOCATE_SUM query
         * @param text The pattern to locate
         * @return A LineQuery with type LOCATE_SUM
         */
        static LineQuery create_LOCATE_SUM_query(const std::string &text)
        {
            LineQuery r;
            r.type = QueryType::LOCATE_SUM;
            r.pattern.clear();

            std::vector<uint8_t> sanitized_text = sanityze(text);
            r.pattern.swap(sanitized_text);

            return r;
        }
        
        /*
        static std::vector<std::string> split_by_tab(const std::string &input)
        {
            std::vector<std::string> result;
            std::stringstream ss(input);
            std::string item;

            while (std::getline(ss, item, '\t'))
            {
                result.push_back(item);
            }

            return result;
        }
        */
        
        /**
         * @brief Convert a string to uint64_t
         * @param str The string to convert
         * @return The converted uint64_t value
         * @throws std::invalid_argument if conversion fails
         */
        static uint64_t string_to_uint64(const std::string &str)
        {
            try
            {
                size_t idx;
                uint64_t value = std::stoull(str, &idx);

                if (idx != str.length())
                {
                    throw std::invalid_argument("Invalid character in input string");
                }

                return value;
            }
            catch (const std::exception &e)
            {
                throw std::invalid_argument("Conversion to uint64_t failed: " + std::string(e.what()));
            }
        }
        /**
         * @brief Parse a line from a command file into a LineQuery
         * @param line The line to parse (TSV format)
         * @param alternative_tab_key Alternative tab character for parsing
         * @param alternative_line_break_key Alternative line break character for parsing
         * @return A LineQuery parsed from the line, or NONE query if parsing fails
         * 
         * Supported formats:
         *   - INSERT\t<position>\t<string>
         *   - DELETE\t<position>\t<length>
         *   - COUNT\t<pattern>
         *   - LOCATE\t<pattern>
         *   - LOCATE_SUM\t<pattern>
         *   - PRINT
         */
        static LineQuery load_line(const std::string &line, std::string alternative_tab_key, std::string alternative_line_break_key)
        {
            std::vector<std::string> result = stool::dynamic_r_index::TSVParser::line_parse(line, alternative_tab_key, alternative_line_break_key);
            if (result.size() > 0)
            {
                if (result[0] == "INSERT" && result.size() == 3)
                {
                    uint64_t insertion_pos = string_to_uint64(result[1]);
                    return LineQuery::create_INSERT_query(insertion_pos, result[2]);
                }
                else if (result[0] == "DELETE" && result.size() == 3)
                {
                    uint64_t deletion_pos = string_to_uint64(result[1]);
                    uint64_t length = string_to_uint64(result[2]);
                    return LineQuery::create_DELETE_query(deletion_pos, length);
                }
                else if (result[0] == "COUNT" && result.size() == 2)
                {
                    return LineQuery::create_COUNT_query(result[1]);
                }
                else if (result[0] == "LOCATE" && result.size() == 2)
                {
                    return LineQuery::create_LOCATE_query(result[1]);
                }
                else if (result[0] == "LOCATE_SUM" && result.size() == 2)
                {
                    return LineQuery::create_LOCATE_SUM_query(result[1]);
                }
                else if (result[0] == "PRINT" && result.size() == 1)
                {
                    return LineQuery::create_PRINT_query();
                }
                else
                {
                    std::cout << "Skipped: " << line << std::endl;
                    return LineQuery::create_NONE_query();
                }
            }
            else
            {
                return LineQuery::create_NONE_query();
            }
        }
    };

}