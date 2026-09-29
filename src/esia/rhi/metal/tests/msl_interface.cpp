// Metal backend tests: parser of the generated MSL's resource interface (see msl_interface.hpp).
// UNVERIFIED: needs macOS (runs on Linux and under Wine).
#include "msl_interface.hpp"
#include <cctype>
#include <cstring>

namespace esia::rhi::metal::test
{
    namespace
    {
        std::string Trim(const std::string& s)
        {
            const std::size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
            return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
        }

        // "type name [[attr]]" -> type, name, attr
        bool SplitDeclaration(const std::string& decl, std::string& type, std::string& name, std::string& attr)
        {
            const std::size_t open = decl.find("[["), close = decl.rfind("]]");
            if (open == std::string::npos || close == std::string::npos || close < open)
                return false;
            attr = Trim(decl.substr(open + 2, close - open - 2));
            const std::string head = Trim(decl.substr(0, open));
            const std::size_t space = head.find_last_of(" \t&*");
            if (space == std::string::npos)
                return false;
            name = Trim(head.substr(space + 1));
            type = Trim(head.substr(0, space + 1));
            return !name.empty() && !type.empty();
        }

        bool ParseStruct(const std::string& src, const std::string& structName, std::vector<MslVarying>& out)
        {
            const std::string key = "struct " + structName + "\n{";
            const std::size_t at = src.find(key);
            if (at == std::string::npos)
                return false;
            const std::size_t end = src.find("};", at);
            const std::string body = src.substr(at + key.size(), end - at - key.size());
            std::size_t pos = 0;
            while (pos < body.size())
            {
                const std::size_t semi = body.find(';', pos);
                if (semi == std::string::npos)
                    break;
                MslVarying v;
                if (SplitDeclaration(body.substr(pos, semi - pos), v.type, v.name, v.attribute))
                    out.push_back(v);
                pos = semi + 1;
            }
            return true;
        }

        // "buffer(N)" / "texture(N)" / "sampler(N)"
        bool ParseBinding(const std::string& attr, MslResource& r)
        {
            const std::size_t open = attr.find('(');
            if (open == std::string::npos || attr.back() != ')' || open + 2 > attr.size() - 1)
                return false;
            const std::string kind = attr.substr(0, open), digits = attr.substr(open + 1, attr.size() - open - 2);
            for (char c : digits)
                if (!std::isdigit((unsigned char)c))
                    return false;
            if (kind == "buffer")
                r.kind = MslResource::Buffer;
            else if (kind == "texture")
                r.kind = MslResource::Texture;
            else if (kind == "sampler")
                r.kind = MslResource::Sampler;
            else
                return false;
            r.index = std::stoi(digits);
            return true;
        }
    }

    const MslResource* MslInterface::Find(MslResource::Kind kind, int index) const
    {
        for (const MslResource& r : resources)
            if (r.kind == kind && r.index == index)
                return &r;
        return nullptr;
    }

    const MslResource* MslInterface::FindByName(const std::string& name) const
    {
        for (const MslResource& r : resources)
            if (r.name == name)
                return &r;
        return nullptr;
    }

    MslInterface ParseMsl(const std::string& src)
    {
        MslInterface m;
        // `vertex|fragment <type> esia_main(`
        const std::size_t at = src.find(" esia_main(");
        if (at == std::string::npos)
        {
            m.error = "no esia_main entry point";
            return m;
        }
        const std::size_t line = src.rfind('\n', at) + 1;
        const std::string qualifier = src.substr(line, src.find(' ', line) - line);
        if (qualifier != "vertex" && qualifier != "fragment")
        {
            m.error = "esia_main is not a vertex or fragment function";
            return m;
        }
        m.vertex = qualifier == "vertex";
        // the parameter list: up to the parenthesis that closes esia_main(
        std::size_t pos = at + std::strlen(" esia_main(");
        int depth = 1;
        std::size_t end = pos;
        for (; end < src.size() && depth > 0; ++end)
        {
            if (src[end] == '(')
                ++depth;
            else if (src[end] == ')')
                --depth;
        }
        const std::string params = src.substr(pos, end - 1 - pos);
        // split at top-level commas (the attributes hold parentheses, the types angle brackets)
        std::vector<std::string> list;
        int nest = 0;
        std::size_t start = 0;
        for (std::size_t i = 0; i < params.size(); ++i)
        {
            const char c = params[i];
            if (c == '(' || c == '<' || c == '[')
                ++nest;
            else if (c == ')' || c == '>' || c == ']')
                --nest;
            else if (c == ',' && nest == 0)
            {
                list.push_back(params.substr(start, i - start));
                start = i + 1;
            }
        }
        if (!Trim(params).empty())
            list.push_back(params.substr(start));

        for (const std::string& p : list)
        {
            std::string type, name, attr;
            if (!SplitDeclaration(p, type, name, attr))
            {
                m.error = "cannot parse parameter '" + Trim(p) + "'";
                return m;
            }
            MslResource r;
            if (ParseBinding(attr, r))
            {
                r.type = type;
                r.name = name;
                m.resources.push_back(r);
            }
            else if (attr == "vertex_id")
                m.usesVertexId = true;
            else if (attr == "instance_id")
                m.usesInstanceId = true;
            else if (attr != "stage_in" && attr != "position")
            {
                m.error = "unexpected parameter attribute [[" + attr + "]]";
                return m;
            }
        }
        ParseStruct(src, "esia_main_in", m.inputs);
        if (!ParseStruct(src, "esia_main_out", m.outputs))
        {
            m.error = "no esia_main_out";
            return m;
        }
        m.ok = true;
        return m;
    }
}
