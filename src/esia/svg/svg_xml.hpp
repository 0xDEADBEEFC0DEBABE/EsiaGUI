// Esia - the XML an SVG file is written in: elements, attributes and the character data of <style> elements. Well-formed
// XML, as browsers ask of an SVG file - but attributes without white space between them pass (an old exporter writes
// them so); a DOCTYPE is skipped, its entities are not expanded.
#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace esia::svg::xml
{
    struct Attribute
    {
        std::string_view name, value;   // value: entities decoded
    };

    struct Element
    {
        std::string_view name;      // without an "svg:" prefix
        std::string_view rawName;   // as written
        std::uint32_t attrBegin = 0, attrEnd = 0;
        std::int32_t parent = -1, firstChild = -1, lastChild = -1, next = -1;
        std::string_view text;      // the character data of a <style> element (CDATA included)
        std::int32_t textIndex = -1;
    };

    struct Document
    {
        std::vector<Element> elements;   // [0]: the root
        std::vector<Attribute> attributes;
        std::deque<std::string> strings;   // decoded attribute values and <style> text (stable addresses)

        // The attribute's value, or an empty view when the element has none (`found` tells the two apart).
        std::string_view Attr(const Element& e, std::string_view name, bool* found = nullptr) const;
    };

    // false: not well-formed XML; `error` says where.
    bool Parse(std::string_view text, Document& doc, std::string* error);
}
