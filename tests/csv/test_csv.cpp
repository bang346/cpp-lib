#include <gtest/gtest.h>

#include "csv_parser.hpp"

std::string path = TEST_CSV_PATH; // From build

class LeapYearCalendar
{
public:
    bool isLeap(int year) { return !(year % 4); }
};

static std::vector<double> loadTestCases(const std::string &name)
{
    std::vector<double> test_values;

    csv::CSVFormat format;
    format.delimiter(';').header_row(0);

    try
    {
        csv::CSVReader reader(name, format); // Dateiname + Format benutzen

        for (csv::CSVRow &row : reader)
        {
            test_values.push_back(
                parseGermanDouble(row["Test_values"].get<std::string>()));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler beim Lesen: " << e.what() << "\n";
    }

    return test_values;
}

class LeapYearParameterizedTestFixture : public ::testing::TestWithParam<double>
{
protected:
    LeapYearCalendar leapYearCalendar;
};

TEST_P(LeapYearParameterizedTestFixture, ReadYearsFromCsv)
{
    double year = GetParam();
    // SCOPED_TRACE("year = " + std::to_string(year));
    EXPECT_EQ(year, year);
}

INSTANTIATE_TEST_SUITE_P(
    LeapYearTests,
    LeapYearParameterizedTestFixture,
    ::testing::ValuesIn(loadTestCases(TEST_CSV_PATH)) // ValuesIn
);
