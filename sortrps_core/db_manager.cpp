#include "pch.h"
#include "db_manager.h"
#include "sqlite3.h"

#include <sstream>
#include <ctime>
#include <cstdio>

// ---------- helpers ----------
static std::string now_iso_utc()
{
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    tm = *std::gmtime(&t);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02dZ",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    return std::string(buf);
}

static std::string serialize_vec(const std::vector<int>& v)
{
    std::ostringstream oss;
    for (size_t i = 0; i < v.size(); ++i)
    {
        oss << v[i];
        if (i + 1 < v.size()) oss << ' ';
    }
    return oss.str();
}

static std::vector<int> parse_vec(const std::string& s)
{
    std::vector<int> v;
    std::istringstream iss(s);
    int x;
    while (iss >> x) v.push_back(x);
    return v;
}

static bool select_rows(sqlite3* db, const char* sql, int count,
    std::vector<ArrayRow>& out_rows, std::string& err)
{
    out_rows.clear();

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        err = sqlite3_errmsg(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, count);

    while (true)
    {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            ArrayRow row;
            row.id = sqlite3_column_int(stmt, 0);

            const unsigned char* o = sqlite3_column_text(stmt, 1);
            const unsigned char* s = sqlite3_column_text(stmt, 2);
            row.size = sqlite3_column_int(stmt, 3);

            const unsigned char* ct = sqlite3_column_text(stmt, 4);
            row.created_at = ct ? reinterpret_cast<const char*>(ct) : "";

            row.original = parse_vec(o ? reinterpret_cast<const char*>(o) : "");
            row.sorted = parse_vec(s ? reinterpret_cast<const char*>(s) : "");

            out_rows.push_back(std::move(row));
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            err = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

// ---------- DbManager ----------
DbManager::DbManager(const std::string& db_path) : db_path_(db_path) {}

DbManager::~DbManager()
{
    if (db_) sqlite3_close(reinterpret_cast<sqlite3*>(db_));
    db_ = nullptr;
}

std::string DbManager::last_error() const { return last_error_; }

bool DbManager::init()
{
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK)
    {
        last_error_ = "sqlite3_open failed";
        return false;
    }
    db_ = db;

    const char* sql =
        "CREATE TABLE IF NOT EXISTS arrays ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "original_text TEXT NOT NULL,"
        "sorted_text TEXT NOT NULL,"
        "size INTEGER NOT NULL,"
        "created_at TEXT NOT NULL"
        ");";

    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK)
    {
        last_error_ = err ? err : "sqlite3_exec failed";
        sqlite3_free(err);
        return false;
    }

    return true;
}

bool DbManager::insert_array(const std::vector<int>& original, const std::vector<int>& sorted)
{
    auto* db = reinterpret_cast<sqlite3*>(db_);
    if (!db) { last_error_ = "db not initialized"; return false; }

    const char* sql =
        "INSERT INTO arrays (original_text, sorted_text, size, created_at) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        last_error_ = sqlite3_errmsg(db);
        return false;
    }

    std::string o = serialize_vec(original);
    std::string s = serialize_vec(sorted);
    std::string t = now_iso_utc();

    sqlite3_bind_text(stmt, 1, o.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, s.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, (int)original.size());
    sqlite3_bind_text(stmt, 4, t.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        last_error_ = sqlite3_errmsg(db);
        return false;
    }
    return true;
}

bool DbManager::get_latest(int count, std::vector<ArrayRow>& out_rows)
{
    auto* db = reinterpret_cast<sqlite3*>(db_);
    if (!db) { last_error_ = "db not initialized"; return false; }

    const char* sql =
        "SELECT id, original_text, sorted_text, size, created_at "
        "FROM arrays ORDER BY id DESC LIMIT ?;";

    return select_rows(db, sql, count, out_rows, last_error_);
}

bool DbManager::get_random(int count, std::vector<ArrayRow>& out_rows)
{
    auto* db = reinterpret_cast<sqlite3*>(db_);
    if (!db) { last_error_ = "db not initialized"; return false; }

    const char* sql =
        "SELECT id, original_text, sorted_text, size, created_at "
        "FROM arrays ORDER BY RANDOM() LIMIT ?;";

    return select_rows(db, sql, count, out_rows, last_error_);
}

bool DbManager::clear()
{
    auto* db = reinterpret_cast<sqlite3*>(db_);
    if (!db) { last_error_ = "db not initialized"; return false; }

    const char* sql = "DELETE FROM arrays;";
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK)
    {
        last_error_ = err ? err : "delete failed";
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool DbManager::count_rows(int& out_count) const
{
    out_count = 0;
    auto* db = reinterpret_cast<sqlite3*>(db_);
    if (!db) { last_error_ = "db not initialized"; return false; }

    const char* sql = "SELECT COUNT(*) FROM arrays;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        last_error_ = sqlite3_errmsg(db);
        return false;
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        out_count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    last_error_ = sqlite3_errmsg(db);
    return false;
}