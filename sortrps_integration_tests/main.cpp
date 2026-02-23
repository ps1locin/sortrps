#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <string>

#include "db_manager.h"
#include "merge_sort.h"

static std::vector<int> random_array(int n, int lo = -1000, int hi = 1000)
{
    static std::mt19937 gen{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(lo, hi);

    std::vector<int> a;
    a.reserve(n);
    for (int i = 0; i < n; ++i) a.push_back(dist(gen));
    return a;
}

static void print_ok_time(const char* title, bool ok, double ms_total, double ms_avg = -1.0)
{
    std::cout << title << ": " << (ok ? "OK" : "FAIL")
        << ", time=" << ms_total << " ms";
    if (ms_avg >= 0.0) std::cout << ", avg=" << ms_avg << " ms";
    std::cout << "\n";
}

static bool fill_db(DbManager& db, int rows_count, double& out_ms)
{
    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();

    if (!db.clear())
        return false;

    // генерируем массивы случайного размера (например 10..100)
    static std::mt19937 gen{ std::random_device{}() };
    std::uniform_int_distribution<int> size_dist(10, 100);

    for (int i = 0; i < rows_count; ++i)
    {
        auto original = random_array(size_dist(gen));
        auto sorted = original;
        sorting::merge_sort(sorted);

        if (!db.insert_array(original, sorted))
            return false;
    }

    auto t1 = clock::now();
    out_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return true;
}

static bool test_fetch_sort_100(DbManager& db, double& out_total_ms, double& out_avg_ms)
{
    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();

    std::vector<ArrayRow> rows;
    if (!db.get_random(100, rows))
        return false;

    // сортируем заново 100 массивов (имитация обработки)
    for (auto& r : rows)
    {
        auto a = r.original;
        sorting::merge_sort(a);
    }

    auto t1 = clock::now();
    out_total_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    out_avg_ms = rows.empty() ? 0.0 : (out_total_ms / rows.size());
    return true;
}

static bool test_clear(DbManager& db, double& out_ms)
{
    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();

    bool ok = db.clear();

    auto t1 = clock::now();
    out_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return ok;
}

int main()
{
    std::cout << "Integration tests (Lab3)\n";

    DbManager db("integration_test.db"); // ОТДЕЛЬНАЯ база для тестов

    if (!db.init())
    {
        std::cout << "DB init FAIL: " << db.last_error() << "\n";
        return 1;
    }

    const int sizes[] = { 100, 1000, 10000 };

    // 1) Тест добавления 100/1000/10000
    for (int n : sizes)
    {
        double ms = 0.0;
        bool ok = fill_db(db, n, ms);
        print_ok_time((std::string("Insert ") + std::to_string(n)).c_str(), ok, ms);
        if (!ok) std::cout << "Error: " << db.last_error() << "\n";
    }

    // 2) Тест выгрузки и сортировки 100 случайных массивов (3 раза для 100/1000/10000)
    for (int n : sizes)
    {
        double ms_fill = 0.0;
        bool ok_fill = fill_db(db, n, ms_fill);
        if (!ok_fill)
        {
            std::cout << "Fill before fetch/sort FAIL: " << db.last_error() << "\n";
            continue;
        }

        for (int run = 1; run <= 3; ++run)
        {
            double total = 0.0, avg = 0.0;
            bool ok = test_fetch_sort_100(db, total, avg);
            std::string title = "Fetch+Sort100 from DB=" + std::to_string(n) + " (run " + std::to_string(run) + ")";
            print_ok_time(title.c_str(), ok, total, avg);
            if (!ok) std::cout << "Error: " << db.last_error() << "\n";
        }
    }

    // 3) Тест очистки (3 раза для 100/1000/10000)
    for (int n : sizes)
    {
        double ms_fill = 0.0;
        bool ok_fill = fill_db(db, n, ms_fill);
        if (!ok_fill)
        {
            std::cout << "Fill before clear FAIL: " << db.last_error() << "\n";
            continue;
        }

        for (int run = 1; run <= 3; ++run)
        {
            double ms = 0.0;
            bool ok = test_clear(db, ms);
            std::string title = "Clear DB=" + std::to_string(n) + " (run " + std::to_string(run) + ")";
            print_ok_time(title.c_str(), ok, ms);
            if (!ok) std::cout << "Error: " << db.last_error() << "\n";
        }
    }

    std::cout << "Done.\n";
    return 0;
}