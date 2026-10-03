#pragma once

#include "direct_lattice.h"

#include "../common/helpcode_utils.h"
#include "../core/query_request.h"

#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// 双拼直接辅助码的入口：把用户敲的原串（uiauiq）整句解码，选出切分，再把请求改写成句中辅助码
// 的形状（ui'ui + 每个音节上的约束），之后的查词、整句、选词推进、预编辑全部复用句中辅助码那条
// 已有的路径。规则见 spelling_graph.h，开关关着时这里一行都不会跑。
namespace direct_helpcode
{

struct ResolveContext
{
    SpanLookup lookup;
    // 当前辅助码方案的码表；为空时没有辅码边，整串按普通双拼解。
    const HelpcodeUtils::Keymap *keymap = nullptr;
    DecodeOptions options;
};

class Resolver
{
  public:
    explicit Resolver(const ShuangpinProfile &profile);

    // 改写了请求返回 true。选中的切分里没有辅码、和普通双拼切得一样时不改写，那时请求本来就对。
    bool resolve(QueryRequest &request, const ResolveContext &context);
    // 词库、码表或整句打分选项变了，索引和解码结果都要作废。
    void reset_cache();

  private:
    struct SyllableAux
    {
        std::array<bool, 26> first{};
        std::unordered_set<std::string> pairs;
    };

    bool has_aux(const std::string &quanpin, char first, char second, const ResolveContext &context);
    const std::vector<quanpin::LatticeLexeme> &span_rows(const quanpin::Segments &span, bool constrained,
                                                         const ResolveContext &context);

    const ShuangpinProfile profile_;
    const HelpcodeUtils::Keymap *indexed_keymap_ = nullptr;
    std::unordered_map<std::string, SyllableAux> aux_index_;
    // 跨按键复用的跨度查询：每敲一个键整串重新解码，前面那些跨度的行没有变。
    std::unordered_map<std::string, std::vector<quanpin::LatticeLexeme>> span_cache_;
    std::unordered_map<std::string, std::vector<SyllableSpelling>> resolution_cache_;
};

// 按选中的切分改写请求，纯函数，单测直接用。typed 是原串（保留大小写）。
void apply_spellings(QueryRequest &request, const std::string &typed, const std::vector<SyllableSpelling> &spellings,
                     const ShuangpinProfile &profile);

} // namespace direct_helpcode
