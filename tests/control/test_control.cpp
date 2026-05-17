#include <memory>
#include <tuple>
#include <string>
#include <vector>
#include <mutex>

#include <gtest/gtest.h>
#include "pid.hpp"
#include "cntr_system.hpp"
#include "csv_parser.hpp"

std::string path = TEST_DATA_PATH; // From build

static std::vector<std::tuple<double, double>> loadTestCases(const std::string &name)
{
    std::vector<std::tuple<double, double>> test_values;

    csv::CSVFormat format;
    format.delimiter(';').header_row(0);

    try
    {
        csv::CSVReader reader(name, format); // Dateiname + Format benutzen

        for (csv::CSVRow &row : reader)
        {
            test_values.emplace_back(
                parseGermanDouble(row["w"].get<std::string>()),
                parseGermanDouble(row["y"].get<std::string>()));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler beim Lesen: " << e.what() << "\n";
    }

    return test_values;
}

// =======================
// CSV-Logger für Testergebnisse
// =======================
static std::mutex csv_mutex;

static void appendMeasuredValueToCsv(double w_target, double y_expected, double y_measured)
{
    std::lock_guard<std::mutex> lock(csv_mutex);

    const std::string out_file = "pid_test_results.csv";

    // Prüfen ob Datei existiert/leer ist -> Header schreiben
    bool write_header = false;
    {
        std::ifstream check(out_file);
        write_header = (!check.good() || check.peek() == std::ifstream::traits_type::eof());
    }

    std::ofstream file(out_file, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Konnte Ergebnis-CSV nicht öffnen: " << out_file << "\n";
        return;
    }

    if (write_header)
    {
        file << "w_target;y_expected;y_measured\n";
    }

    // deutsches Format willst du vermutlich wieder mit ';'
    file << w_target << ";" << y_expected << ";" << y_measured << "\n";
}

TEST(ClosedLoopTest, PIDcontrolSystemTrajectory)
{
    double R = 0.398 / 2;
    double L = 876e-6 / 2;
    double Ts = 1e-5;

    cntr_system<double> plant(1.0, 2.0, Ts);
    PID<double> pid(1.0, 10.0, 0.0, Ts);

    double y = 0.0;

    auto testcases = loadTestCases(path);

    for (size_t k = 1; k < testcases.size(); ++k)
    {
        auto [w, y_expected] = testcases[k];

        double e = w - y;
        double u = pid.update(e);
        y = plant.update(u);
        // 👇 Hier wird dein gemessenes y in die CSV geschrieben
        // appendMeasuredValueToCsv(w, y_expected, y);
        SCOPED_TRACE("k = " + std::to_string(k));
        EXPECT_NEAR(y, y_expected, 0.01);
    }
}

TEST(ClosedLoopTest, PIDFcontrolSystemTrajectory)
{
    double Ts = 1e-5;
    double Tf = 1e-3;

    cntr_system<double> plant(1.0, 2.0, Ts);
    PIDF<double> pid(1.0, 10.0, 1.0, Ts, Tf, 10000.0, -10000.0);

    double y = 0.0;

    auto testcases = loadTestCases(TEST_DATA_PIDF_PATH);

    for (size_t k = 1; k < testcases.size(); ++k)
    {
        auto [w, y_expected] = testcases[k];

        double e = w - y;
        double u = pid.update(e);
        y = plant.update(u);
        // Hier wird dein gemessenes y in die CSV geschrieben
        // appendMeasuredValueToCsv(w, y_expected, y);
        SCOPED_TRACE("k = " + std::to_string(k));
        EXPECT_NEAR(y, y_expected, 0.01);
    }
}

// //old:
// class LeapYearParameterizedTestFixture : public ::testing::TestWithParam<std::tuple<double, double>>
// {
// protected:
//     std::unique_ptr<cntr_system<double>> str_ptr;
//     std::unique_ptr<PID<double>> pid_ptr;

//     double e;
//     double y;
//     void SetUp() override
//     {
//         double R = 0.398 / 2;
//         double L = 876e-6 / 2;
//         double Ts = 1e-2;
//         str_ptr = std::make_unique<cntr_system<double>>(1.0, L / R, Ts);
//         pid_ptr = std::make_unique<PID<double>>(1.0, 10.0, 4.0, Ts);

//         e = 0.0;
//         y = 0.0;
//     }
// };

// TEST_P(LeapYearParameterizedTestFixture, PIDcontrolSystem)
// {
//     auto [target_value, output_value] = GetParam();
//     e = target_value - y;
//     auto u = pid_ptr->update(e);
//     y = str_ptr->update(u);
//     appendMeasuredValueToCsv(target_value, output_value, y);
//     EXPECT_NEAR(y, output_value, 0.001);
// }

// INSTANTIATE_TEST_SUITE_P(
//     LeapYearTests,
//     LeapYearParameterizedTestFixture,
//     ::testing::ValuesIn(loadTestCases(path)) // ValuesIn
// );
