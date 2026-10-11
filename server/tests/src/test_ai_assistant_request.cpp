#include "ai/ai_assistant_request.h"
#include "tests/includes/test_framework.h"

namespace
{
AiAssistant::Request MakeRequest(const std::string &provider, const std::string &thinking)
{
    AiAssistant::Request request;
    request.pinyin_segments = {"shang", "hai"};
    request.context = "我准备去";
    request.config.provider = provider;
    request.config.model = "example-model";
    request.config.prompt = "Return the best candidate.";
    request.config.thinking = thinking;
    return request;
}
} // namespace

TEST_CASE(ai_request_disables_deepseek_thinking_by_default)
{
    const auto body = AiAssistant::detail::BuildRequestBody(MakeRequest("deepseek", "disabled"));
    REQUIRE_EQ(body["thinking"]["type"].get<std::string>(), "disabled");
    REQUIRE(!body.contains("reasoning_effort"));
    REQUIRE_EQ(body["max_tokens"].get<int>(), 512);
    REQUIRE_EQ(AiAssistant::detail::RequestTimeoutMs(MakeRequest("deepseek", "disabled").config), 8000L);
}

TEST_CASE(ai_request_sends_deepseek_thinking_level)
{
    for (const char *level : {"low", "high", "max"})
    {
        const auto request = MakeRequest("deepseek", level);
        const auto body = AiAssistant::detail::BuildRequestBody(request);
        REQUIRE_EQ(body["thinking"]["type"].get<std::string>(), "enabled");
        REQUIRE_EQ(body["reasoning_effort"].get<std::string>(), std::string(level));
        REQUIRE(body["max_tokens"].get<int>() > 512);
        REQUIRE(AiAssistant::detail::RequestTimeoutMs(request.config) > 8000L);
    }
}

TEST_CASE(ai_request_ignores_thinking_level_for_other_providers)
{
    const auto request = MakeRequest("custom", "high");
    const auto body = AiAssistant::detail::BuildRequestBody(request);
    REQUIRE(!body.contains("thinking"));
    REQUIRE(!body.contains("reasoning_effort"));
    REQUIRE_EQ(body["max_tokens"].get<int>(), 512);
    REQUIRE_EQ(AiAssistant::detail::RequestTimeoutMs(request.config), 8000L);
}
