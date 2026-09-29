// Esia - layout vocabulary of the UI core: laid-out items, slots and layout providers (docs/UI_CORE.md, section 6).
//
// Items are laid out inside containers (the window's content, groups, child regions, widget containers). A container
// without a provider uses the layout cursor (items below each other, SameLine, Indent). A container with a
// LayoutProvider asks it where every child goes: each item submitted at the container's own depth is one child,
// reported to Next() when it is laid out; nested containers are single children.
#pragma once
#include "esia/base/math.hpp"
#include <vector>

namespace esia
{
    // One laid-out item, as reported to providers, the item observer and Window::LaidOutItems().
    struct LaidOutItem
    {
        Rect rect;
        float baseline = -1.0f;   // distance from rect.min.y to the item's text baseline; < 0 = none
        int depth = 0;            // container depth in its window: 0 = the window's content; -1 = floating
        Id window = 0;
        Id container = 0;         // id of the container it was laid out in (0 = the window's content or a group)
        bool floating = false;    // floats over the content (a floating child region): overlaps it by design
    };

    // Where the next child of a container goes: its position, and the region it may take (the work rect: what
    // ContentRegionAvail and WorkRect report while the child is submitted).
    struct LayoutSlot
    {
        Vec2 pos;
        Rect region;
    };

    class LayoutProvider
    {
    public:
        virtual ~LayoutProvider() = default;
        // The container starts at `origin` (inside its padding) with `region` offered by its parent. Returns the
        // first child's slot.
        virtual LayoutSlot Begin(Vec2 origin, const Rect& region) = 0;
        // A child took its slot and measured `child`: returns the next child's slot.
        virtual LayoutSlot Next(const LaidOutItem& child) = 0;
        // The container ends: the size of its content (laid out as one item of the parent, with its padding) and,
        // when it has one, its baseline (distance from the content top).
        virtual Vec2 End(float* baseline) = 0;
    };

    enum class Align : std::uint8_t { Start, Center, End, Stretch, Baseline };

    // The reference provider: children in a row or a column with a fixed spacing, aligned across the axis
    // (Baseline lines up the text baselines of a row). Cross-axis placement uses the sizes the children measured
    // last frame, so keep the object across frames (Context::State<StackLayout>(id)).
    class ESIA_API StackLayout : public LayoutProvider
    {
    public:
        bool horizontal = false;
        float spacing = 8.0f;
        Align align = Align::Start;

        LayoutSlot Begin(Vec2 origin, const Rect& region) override;
        LayoutSlot Next(const LaidOutItem& child) override;
        Vec2 End(float* baseline) override;

    private:
        struct Child
        {
            Vec2 size;
            float baseline = -1.0f;
        };
        LayoutSlot Slot() const;

        Vec2 origin_;
        Rect region_;
        float main_ = 0.0f;
        std::vector<Child> last_, now_;   // last frame's children, this frame's
        float lastCross_ = 0.0f, lastBaseline_ = -1.0f;   // row height and baseline measured last frame
        float crossExtent_ = 0.0f, baseline_ = -1.0f;     // this frame's
    };
}
