#include "csv_parser.hpp"

double parseGermanDouble(std::string s)
{
    // Komma -> Punkt machen
    std::replace(s.begin(), s.end(), ',', '.');
    return std::stod(s);
}

float parseGermanFloat(std::string s)
{
    // Komma -> Punkt machen
    std::replace(s.begin(), s.end(), ',', '.');
    return std::stof(s);
}
