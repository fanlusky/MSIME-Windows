#include "tests/includes/test_framework.h"

#include "msimeui/Layout.h"
#include "msimeui/Window.h"

#include <memory>
#include <windowsx.h>

using namespace msimeui;

namespace
{
class FocusProbe : public Visual
{
  public:
    SizeF Measure(const SizeF &) override
    {
        return {80.0f, 30.0f};
    }

    void Arrange(const RectF &rect) override
    {
        bounds_ = rect;
    }

    bool IsFocusable() const override
    {
        return true;
    }

    void Render(DeviceResources &) override
    {
    }

    void OnFocusChanged(bool focused) override
    {
        if (!focused)
            ++blurCount;
    }

    bool OnChar(wchar_t, LPARAM) override
    {
        ++charCount;
        return true;
    }

    int blurCount = 0;
    int charCount = 0;
};

class FocusTestWindow : public Window
{
  public:
    FocusTestWindow() : Window(L"window-focus-test", L"", 200, 100)
    {
        REQUIRE(AdoptExistingHwnd(CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 0, 0, 200, 100, nullptr, nullptr,
                                                  GetModuleHandleW(nullptr), nullptr)));
    }

    ~FocusTestWindow()
    {
        DestroyWindow(GetHandle());
    }
};

void CheckClickFocus(float dpi, bool clickInside)
{
    FocusTestWindow window;
    window.SetDpiOverride(dpi);
    window.SetStealFocusOnClick(false);
    auto field = std::make_shared<FocusProbe>();
    auto root = std::make_shared<OverlayPanel>();
    root->AddChildAt(field, 0.0f, 0.0f);
    auto scene = std::make_unique<Scene>();
    scene->SetRoot(root);
    window.SetScene(std::move(scene));
    // Keep a non-focusable background around the field, as on a settings page.
    root->ArrangeInLayout({0.0f, 0.0f, 200.0f, 100.0f});
    field->ArrangeInLayout({0.0f, 0.0f, 80.0f, 30.0f});
    window.FocusVisual(field.get());

    const int x = static_cast<int>((clickInside ? 20.0f : 150.0f) * dpi / 96.0f);
    const int y = static_cast<int>(15.0f * dpi / 96.0f);
    window.DispatchImportedMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
    window.DispatchImportedMessage(WM_LBUTTONUP, 0, MAKELPARAM(x, y));
    window.DispatchImportedMessage(WM_CHAR, L'a', 0);
    REQUIRE(field->blurCount == (clickInside ? 0 : 1));
    REQUIRE(field->charCount == (clickInside ? 1 : 0));

    // Repeated clicks on the background must not submit the field again.
    window.DispatchImportedMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
    window.DispatchImportedMessage(WM_LBUTTONUP, 0, MAKELPARAM(x, y));
    REQUIRE(field->blurCount == (clickInside ? 0 : 1));
}
} // namespace

TEST_CASE(window_background_click_blurs_field_and_stops_text_input)
{
    for (float dpi : {96.0f, 120.0f, 144.0f, 192.0f})
        CheckClickFocus(dpi, false);
}

TEST_CASE(window_click_inside_focused_field_keeps_text_input)
{
    for (float dpi : {96.0f, 120.0f, 144.0f, 192.0f})
        CheckClickFocus(dpi, true);
}
