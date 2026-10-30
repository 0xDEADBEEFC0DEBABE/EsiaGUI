// Esia - the XML of SVG files (svg_xml.hpp): one pass over the text, elements and attributes as views into it; only
// values with entities and the text of <style> elements are copied.
#include "svg_xml.hpp"
#include <algorithm>

namespace esia::svg::xml
{
    namespace
    {
        bool Space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
        bool NameEnd(char c) { return Space(c) || c == '=' || c == '/' || c == '>'; }

        void AppendUtf8(std::string& out, std::uint32_t cp)
        {
            if (cp < 0x80)
                out += (char)cp;
            else if (cp < 0x800)
            {
                out += (char)(0xC0 | (cp >> 6));
                out += (char)(0x80 | (cp & 0x3F));
            }
            else if (cp < 0x10000)
            {
                out += (char)(0xE0 | (cp >> 12));
                out += (char)(0x80 | ((cp >> 6) & 0x3F));
                out += (char)(0x80 | (cp & 0x3F));
            }
            else
            {
                out += (char)(0xF0 | (cp >> 18));
                out += (char)(0x80 | ((cp >> 12) & 0x3F));
                out += (char)(0x80 | ((cp >> 6) & 0x3F));
                out += (char)(0x80 | (cp & 0x3F));
            }
        }

        // `s` with the predefined and numeric character references replaced (others stay as written)
        void Decode(std::string_view s, std::string& out)
        {
            for (std::size_t i = 0; i < s.size();)
            {
                if (s[i] != '&')
                {
                    out += s[i++];
                    continue;
                }
                const std::size_t semi = s.find(';', i + 1);
                if (semi == std::string_view::npos || semi - i > 12)
                {
                    out += s[i++];
                    continue;
                }
                const std::string_view ref = s.substr(i + 1, semi - i - 1);
                std::uint32_t cp = 0;
                bool ok = true;
                if (ref == "lt")
                    cp = '<';
                else if (ref == "gt")
                    cp = '>';
                else if (ref == "amp")
                    cp = '&';
                else if (ref == "quot")
                    cp = '"';
                else if (ref == "apos")
                    cp = '\'';
                else if (ref.size() >= 2 && ref[0] == '#')
                {
                    const bool hex = ref[1] == 'x' || ref[1] == 'X';
                    std::size_t k = hex ? 2 : 1;
                    if (k >= ref.size())
                        ok = false;
                    for (; ok && k < ref.size(); ++k)
                    {
                        const char c = ref[k];
                        int v = -1;
                        if (c >= '0' && c <= '9')
                            v = c - '0';
                        else if (hex && c >= 'a' && c <= 'f')
                            v = c - 'a' + 10;
                        else if (hex && c >= 'A' && c <= 'F')
                            v = c - 'A' + 10;
                        if (v < 0 || cp > 0x10FFFF)
                            ok = false;
                        else
                            cp = cp * (hex ? 16u : 10u) + (std::uint32_t)v;
                    }
                    ok = ok && cp > 0 && cp <= 0x10FFFF;
                }
                else
                    ok = false;
                if (!ok)
                {
                    out += s[i++];
                    continue;
                }
                AppendUtf8(out, cp);
                i = semi + 1;
            }
        }

        std::string_view StripSvgPrefix(std::string_view name)
        {
            return name.size() > 4 && name.substr(0, 4) == "svg:" ? name.substr(4) : name;
        }
    }

    std::string_view Document::Attr(const Element& e, std::string_view name, bool* found) const
    {
        for (std::uint32_t i = e.attrBegin; i < e.attrEnd; ++i)
            if (attributes[i].name == name)
            {
                if (found)
                    *found = true;
                return attributes[i].value;
            }
        if (found)
            *found = false;
        return {};
    }

    bool Parse(std::string_view s, Document& doc, std::string* error)
    {
        doc = Document();
        std::size_t i = 0;
        const std::size_t n = s.size();
        if (n >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF)
            i = 3;   // UTF-8 byte order mark
        std::vector<std::int32_t> open;
        const auto fail = [&](const char* what) {
            if (error)
            {
                const std::size_t at = std::min(i, n);
                const long line = 1 + (long)std::count(s.begin(), s.begin() + (std::ptrdiff_t)at, '\n');
                *error = "line " + std::to_string(line) + ": " + what;
            }
            doc = Document();
            return false;
        };
        const auto starts = [&](std::size_t at, std::string_view what) { return s.substr(at, what.size()) == what; };
        // character data goes to the open <style> element, if any
        const auto text = [&](std::string_view chunk, bool decode) {
            if (open.empty())
                return;
            Element& e = doc.elements[(std::size_t)open.back()];
            if (e.name != "style")
                return;
            if (e.textIndex < 0)
            {
                e.textIndex = (std::int32_t)doc.strings.size();
                doc.strings.emplace_back();
            }
            std::string& dst = doc.strings[(std::size_t)e.textIndex];
            if (decode)
                Decode(chunk, dst);
            else
                dst.append(chunk);
        };

        while (i < n)
        {
            if (s[i] != '<')
            {
                std::size_t j = s.find('<', i);
                if (j == std::string_view::npos)
                    j = n;
                const std::string_view chunk = s.substr(i, j - i);
                if (open.empty() && std::any_of(chunk.begin(), chunk.end(), [](char c) { return !Space(c); }))
                    return fail("text outside the root element");
                text(chunk, true);
                i = j;
                continue;
            }
            if (starts(i, "<!--"))
            {
                const std::size_t j = s.find("-->", i + 4);
                if (j == std::string_view::npos)
                    return fail("unterminated comment");
                i = j + 3;
                continue;
            }
            if (starts(i, "<![CDATA["))
            {
                const std::size_t j = s.find("]]>", i + 9);
                if (j == std::string_view::npos)
                    return fail("unterminated CDATA section");
                text(s.substr(i + 9, j - i - 9), false);
                i = j + 3;
                continue;
            }
            if (starts(i, "<?"))
            {
                const std::size_t j = s.find("?>", i + 2);
                if (j == std::string_view::npos)
                    return fail("unterminated processing instruction");
                i = j + 2;
                continue;
            }
            if (starts(i, "<!"))
            {
                // <!DOCTYPE ...> with an internal subset in [...] (quoted strings may hold '>' and brackets)
                int depth = 0;
                char quote = 0;
                std::size_t j = i + 2;
                for (; j < n; ++j)
                {
                    const char c = s[j];
                    if (quote)
                    {
                        if (c == quote)
                            quote = 0;
                    }
                    else if (c == '"' || c == '\'')
                        quote = c;
                    else if (c == '[')
                        ++depth;
                    else if (c == ']')
                        --depth;
                    else if (c == '>' && depth <= 0)
                        break;
                }
                if (j >= n)
                    return fail("unterminated declaration");
                i = j + 1;
                continue;
            }
            if (i + 1 < n && s[i + 1] == '/')
            {
                std::size_t j = i + 2;
                while (j < n && !NameEnd(s[j]))
                    ++j;
                const std::string_view name = s.substr(i + 2, j - i - 2);
                while (j < n && Space(s[j]))
                    ++j;
                if (j >= n || s[j] != '>')
                    return fail("malformed end tag");
                if (open.empty() || doc.elements[(std::size_t)open.back()].rawName != name)
                    return fail("end tag does not match its start tag");
                open.pop_back();
                i = j + 1;
                continue;
            }

            // a start tag
            std::size_t j = i + 1;
            while (j < n && !NameEnd(s[j]))
                ++j;
            const std::string_view raw = s.substr(i + 1, j - i - 1);
            if (raw.empty())
                return fail("malformed start tag");
            if (open.empty() && !doc.elements.empty())
                return fail("more than one root element");
            Element e;
            e.rawName = raw;
            e.name = StripSvgPrefix(raw);
            e.parent = open.empty() ? -1 : open.back();
            e.attrBegin = e.attrEnd = (std::uint32_t)doc.attributes.size();
            bool closed = false, selfClosing = false;
            while (j < n)
            {
                while (j < n && Space(s[j]))
                    ++j;
                if (j >= n)
                    break;
                if (s[j] == '>')
                {
                    ++j;
                    closed = true;
                    break;
                }
                if (s[j] == '/' && j + 1 < n && s[j + 1] == '>')
                {
                    j += 2;
                    closed = selfClosing = true;
                    break;
                }
                const std::size_t a0 = j;
                while (j < n && !NameEnd(s[j]))
                    ++j;
                const std::string_view aname = s.substr(a0, j - a0);
                while (j < n && Space(s[j]))
                    ++j;
                if (aname.empty() || j >= n || s[j] != '=')
                    return fail("malformed attribute");
                ++j;
                while (j < n && Space(s[j]))
                    ++j;
                if (j >= n || (s[j] != '"' && s[j] != '\''))
                    return fail("attribute value without quotes");
                const char quote = s[j];
                const std::size_t v0 = j + 1;
                const std::size_t v1 = s.find(quote, v0);
                if (v1 == std::string_view::npos)
                    return fail("unterminated attribute value");
                std::string_view value = s.substr(v0, v1 - v0);
                if (value.find('&') != std::string_view::npos)
                {
                    doc.strings.emplace_back();
                    Decode(value, doc.strings.back());
                    value = doc.strings.back();
                }
                doc.attributes.push_back({aname, value});
                j = v1 + 1;
            }
            if (!closed)
                return fail("unterminated start tag");
            e.attrEnd = (std::uint32_t)doc.attributes.size();
            const std::int32_t index = (std::int32_t)doc.elements.size();
            if (e.parent >= 0)
            {
                Element& p = doc.elements[(std::size_t)e.parent];
                if (p.lastChild >= 0)
                    doc.elements[(std::size_t)p.lastChild].next = index;
                else
                    p.firstChild = index;
                p.lastChild = index;
            }
            doc.elements.push_back(e);
            if (!selfClosing)
                open.push_back(index);
            i = j;
        }
        if (!open.empty())
            return fail("unclosed element");
        if (doc.elements.empty())
            return fail("no element");
        for (Element& e : doc.elements)
            if (e.textIndex >= 0)
                e.text = doc.strings[(std::size_t)e.textIndex];
        return true;
    }
}
