#ifndef CSV_PARER_HELPER_HPP
#define CSV_PARER_HELPER_HPP

#include <string>
#include <algorithm>
#include <csv.hpp>
/// @brief Converts string to double
/// @param s    convertable string
/// @return     corresponding double value
double parseGermanDouble(std::string s);

/// @brief Converts string to float
/// @param s    convertable string
/// @return     corresponding float value
float parseGermanFloat(std::string s);

#endif