#include "wubi_candidate_provider.h"
#include "../quanpin/quanpin_query.h"
#include <spdlog/spdlog.h>
#include <utility>

namespace
{
constexpr int kNoMutation = 0;
}

WubiCandidateProvider::WubiCandidateProvider(std::string db_path)
    : db_path_(db_path.empty() ? quanpin::get_default_db_path() : std::move(db_path))
{
}

WubiCandidateProvider::~WubiCandidateProvider()
{
    close_database();
}

std::vector<WordItem> WubiCandidateProvider::query(const QueryRequest &request)
{
    if (!request.valid || request.scheme != SchemeType::Wubi || request.normalized_input.empty() ||
        !ensure_query_statement())
    {
        return {};
    }

    sqlite3_reset(query_statement_);
    sqlite3_clear_bindings(query_statement_);
    if (sqlite3_bind_text(query_statement_, 1, request.normalized_input.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        return {};
    }

    std::vector<WordItem> candidates;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(query_statement_)) == SQLITE_ROW)
    {
        const auto *key = reinterpret_cast<const char *>(sqlite3_column_text(query_statement_, 0));
        const auto *value = reinterpret_cast<const char *>(sqlite3_column_text(query_statement_, 1));
        if (key == nullptr || value == nullptr)
        {
            continue;
        }
        candidates.emplace_back(key, value, sqlite3_column_int64(query_statement_, 2));
    }

    if (result != SQLITE_DONE)
    {
        (void)0;
        return {};
    }
    return candidates;
}

void WubiCandidateProvider::reset_cache()
{
    close_database();
}

int WubiCandidateProvider::create_word(SchemeType, std::string, std::string)
{
    return kNoMutation;
}

int WubiCandidateProvider::update_weight_by_pinyin_and_word(SchemeType, std::string, std::string)
{
    return kNoMutation;
}

int WubiCandidateProvider::delete_by_pinyin_and_word(SchemeType, std::string, std::string)
{
    return kNoMutation;
}

int WubiCandidateProvider::cache_dynamic_candidate(SchemeType, const std::string &, const std::string &,
                                                   CandidateSource)
{
    return kNoMutation;
}

int WubiCandidateProvider::cache_dynamic_candidate_for_request(const QueryRequest &, const std::string &,
                                                               CandidateSource)
{
    return kNoMutation;
}

bool WubiCandidateProvider::ensure_query_statement()
{
    if (query_statement_ != nullptr)
    {
        return true;
    }

    if (db_ == nullptr && sqlite3_open_v2(db_path_.c_str(), &db_, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
    {
        (void)0;
        close_database();
        return false;
    }

    constexpr const char *query_sql = "SELECT \"key\", \"value\", \"weight\" FROM wubi86 "
                                      "WHERE \"key\" = ?1 ORDER BY \"weight\" DESC, rowid ASC";
    if (sqlite3_prepare_v2(db_, query_sql, -1, &query_statement_, nullptr) != SQLITE_OK)
    {
        (void)0;
        close_database();
        return false;
    }
    return true;
}

void WubiCandidateProvider::close_database()
{
    if (query_statement_ != nullptr)
    {
        sqlite3_finalize(query_statement_);
        query_statement_ = nullptr;
    }
    if (db_ != nullptr)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}
