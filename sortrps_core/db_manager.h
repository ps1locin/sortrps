#pragma once
#include <string>
#include <vector>

struct ArrayRow
{
    int id = 0;
    int size = 0;
    std::string created_at;
    std::vector<int> original;
    std::vector<int> sorted;
};

class DbManager
{
public:
    explicit DbManager(const std::string& db_path);
    ~DbManager();

    bool init();
    std::string last_error() const;

    bool insert_array(const std::vector<int>& original, const std::vector<int>& sorted);
    bool get_latest(int count, std::vector<ArrayRow>& out_rows);
    bool get_random(int count, std::vector<ArrayRow>& out_rows);

    bool clear();
    bool count_rows(int& out_count) const;

private:
    std::string db_path_;
    void* db_ = nullptr;              // sqlite3* храним как void* чтобы не тащить sqlite3.h в заголовок
    mutable std::string last_error_;  // mutable чтобы можно было писать в const методах
};