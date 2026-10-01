// Esia - texture registry (see esia/core/texture.hpp)
#include "esia/core/texture.hpp"
#include <algorithm>
#include <cstring>

namespace esia
{
    namespace
    {
        void CopyRows(std::vector<std::uint8_t>& dst, const void* src, int width, int height, int bpp, int rowPitch)
        {
            const std::size_t row = (std::size_t)width * (std::size_t)bpp;
            dst.resize(row * (std::size_t)height);
            if (!src)
            {
                std::fill(dst.begin(), dst.end(), (std::uint8_t)0);
                return;
            }
            const std::size_t pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
            for (int y = 0; y < height; ++y)
                std::memcpy(dst.data() + row * (std::size_t)y, static_cast<const std::uint8_t*>(src) + pitch * (std::size_t)y, row);
        }
    }

    TextureId TextureRegistry::Create(const TextureInfo& info, const void* pixels, int rowPitch)
    {
        if (info.width <= 0 || info.height <= 0)
            return 0;
        TextureChange c;
        c.kind = TextureChange::Kind::Create;
        c.info = info;
        c.width = info.width;
        c.height = info.height;
        CopyRows(c.pixels, pixels, info.width, info.height, BytesPerPixel(info.format), rowPitch);
        std::lock_guard lock(mutex_);
        c.id = next_++;
        live_[c.id] = info;
        pending_.push_back(std::move(c));
        return pending_.back().id;
    }

    bool TextureRegistry::Update(TextureId id, int x, int y, int width, int height, const void* pixels, int rowPitch)
    {
        TextureInfo info;
        if (!Info(id, info) || width <= 0 || height <= 0 || x < 0 || y < 0 || x + width > info.width || y + height > info.height)
            return false;
        TextureChange c;
        c.kind = TextureChange::Kind::Update;
        c.id = id;
        c.info = info;
        c.x = x;
        c.y = y;
        c.width = width;
        c.height = height;
        CopyRows(c.pixels, pixels, width, height, BytesPerPixel(info.format), rowPitch);
        std::lock_guard lock(mutex_);
        if (!live_.count(id))
            return false;   // destroyed meanwhile
        pending_.push_back(std::move(c));
        return true;
    }

    void TextureRegistry::Destroy(TextureId id)
    {
        std::lock_guard lock(mutex_);
        if (live_.erase(id) == 0)
            return;
        // never created on the GPU: forget its pending changes instead of creating and destroying it
        const bool onGpu = std::none_of(pending_.begin(), pending_.end(), [&](const TextureChange& c) {
            return c.id == id && c.kind == TextureChange::Kind::Create;
        });
        if (!onGpu)
        {
            std::erase_if(pending_, [&](const TextureChange& c) { return c.id == id; });
            return;
        }
        std::erase_if(pending_, [&](const TextureChange& c) { return c.id == id && c.kind == TextureChange::Kind::Update; });
        TextureChange c;
        c.kind = TextureChange::Kind::Destroy;
        c.id = id;
        pending_.push_back(std::move(c));
    }

    bool TextureRegistry::Info(TextureId id, TextureInfo& out) const
    {
        std::lock_guard lock(mutex_);
        auto it = live_.find(id);
        if (it == live_.end())
            return false;
        out = it->second;
        return true;
    }

    std::size_t TextureRegistry::Count() const
    {
        std::lock_guard lock(mutex_);
        return live_.size();
    }

    void TextureRegistry::TakeChanges(std::vector<TextureChange>& out)
    {
        std::lock_guard lock(mutex_);
        out = std::move(pending_);
        pending_.clear();
    }
}
