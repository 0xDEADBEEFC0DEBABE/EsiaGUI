// Esia - the fields of many shapes at once: worker threads and a cache on disk (include/esia/text/distance_field.hpp).
#include "esia/text/distance_field.hpp"
#include "font_file.hpp"
#include <algorithm>
#include <atomic>
#include <bit>
#include <cfloat>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>

namespace esia::text
{
    namespace
    {
        // What else decides a field's bits: the processor and the compiler. The generator's arithmetic is IEEE's, with
        // exact square roots and divisions, so builds of one kind make the same bits - but a compiler may fuse
        // multiply-adds where the processor has them (ARM; __FP_FAST_FMAF) or keep wider floats (x87; FLT_EVAL_METHOD).
        constexpr const char* Processor()
        {
#if defined(__x86_64__) || defined(_M_X64)
            return "x86-64";
#elif defined(__aarch64__) || defined(_M_ARM64)
            return "arm64";
#elif defined(__i386__) || defined(_M_IX86)
            return "x86";
#elif defined(__arm__) || defined(_M_ARM)
            return "arm";
#else
            return "other";
#endif
        }

        constexpr const char* Compiler()
        {
#if defined(__clang__)
            return "clang";
#elif defined(_MSC_VER)
            return "msvc";
#elif defined(__GNUC__)
            return "gcc";
#else
            return "other";
#endif
        }

#if defined(__FP_FAST_FMAF)
        constexpr std::uint64_t kFusedMultiplyAdd = 1;
#else
        constexpr std::uint64_t kFusedMultiplyAdd = 0;
#endif

        // splitmix64's finalizer: every bit of the input moves about half of the output's
        constexpr std::uint64_t Mix(std::uint64_t x)
        {
            x ^= x >> 30;
            x *= 0xBF58476D1CE4E5B9ull;
            x ^= x >> 27;
            x *= 0x94D049BB133111EBull;
            return x ^ (x >> 31);
        }

        // Two 64-bit hashes of the same words from two seeds: a file's name, and the check inside it (a collision of the
        // name alone cannot load another shape's field)
        struct Hashes
        {
            std::uint64_t name = 0x243F6A8885A308D3ull, check = 0x13198A2E03707344ull;

            void Add(std::uint64_t v)
            {
                name = Mix(name ^ v);
                check = Mix(check ^ v);
            }
            void Add(float a, float b) { Add((std::uint64_t)std::bit_cast<std::uint32_t>(a) << 32 | std::bit_cast<std::uint32_t>(b)); }
            void Add(const char* s)
            {
                for (; *s; ++s)
                    Add((std::uint64_t)(unsigned char)*s);
                Add(0xFFu);
            }
        };

        // the field's bytes: a check of what was written (a file damaged on disk is not taken)
        std::uint64_t Sum(const std::vector<std::uint8_t>& bytes)
        {
            std::uint64_t h = 0x452821E638D01377ull ^ bytes.size();
            std::size_t i = 0;
            for (; i + 8 <= bytes.size(); i += 8)
            {
                std::uint64_t v;
                std::memcpy(&v, bytes.data() + i, 8);
                h = (h ^ v) * 0xBF58476D1CE4E5B9ull;
                h ^= h >> 29;
            }
            for (; i < bytes.size(); ++i)
                h = (h ^ bytes[i]) * 0x100000001B3ull;
            return Mix(h);
        }

        struct Job
        {
            GlyphShape shape;
            DistanceFieldDesc desc;
            GlyphBitmap field;
            std::uint64_t name = 0, check = 0;   // in the cache (with a directory)
            bool ok = false;
            bool fromDisk = false;
        };

        // The cache's file: "ESIAMTSD", the check of what it is the field of, the check of its bytes, left, top,
        // width, height, then the RGBA8 texels.
        constexpr char kMagic[8] = {'E', 'S', 'I', 'A', 'M', 'T', 'S', 'D'};
        constexpr std::int32_t kMaxSide = 4096;   // GenerateMtsdf's largest field

        std::filesystem::path FileOf(const std::filesystem::path& dir, std::uint64_t name)
        {
            char s[32];
            std::snprintf(s, sizeof(s), "%016llx.mtsdf", (unsigned long long)name);
            return dir / s;
        }

        std::FILE* Open(const std::filesystem::path& path, bool write)
        {
#if defined(_WIN32)
            return _wfopen(path.c_str(), write ? L"wb" : L"rb");
#else
            return std::fopen(path.c_str(), write ? "wb" : "rb");
#endif
        }

        // The words that make a field's bits: the generator's version, the build's kind, the ranges, every point of the
        // shape
        void Name(Job& job)
        {
            Hashes h;
            h.Add("esia-mtsdf");
            h.Add(kDistanceFieldVersion);
            h.Add(Processor());
            h.Add(Compiler());
            h.Add((std::uint64_t)(FLT_EVAL_METHOD + 16) | kFusedMultiplyAdd << 8);
            h.Add(job.desc.range, job.desc.alphaRange);
            h.Add(job.shape.ContourCount());
            for (std::size_t c = 0; c < job.shape.ContourCount(); ++c)
            {
                const std::span<const GlyphShape::Edge> edges = job.shape.Contour(c);
                h.Add(edges.size());
                for (const GlyphShape::Edge& e : edges)
                {
                    h.Add((std::uint64_t)e.degree);
                    for (int k = 0; k <= e.degree; ++k)
                        h.Add(e.p[k].x, e.p[k].y);
                }
            }
            job.name = h.name;
            job.check = h.check;
        }

        bool Load(const std::filesystem::path& file, Job& job)
        {
            std::FILE* f = Open(file, false);
            if (!f)
                return false;
            char magic[8] = {};
            std::uint64_t check = 0, sum = 0;
            std::int32_t box[4] = {};
            bool ok = std::fread(magic, 8, 1, f) == 1 && std::memcmp(magic, kMagic, 8) == 0 && std::fread(&check, 8, 1, f) == 1 && check == job.check &&
                      std::fread(&sum, 8, 1, f) == 1 && std::fread(box, sizeof(box), 1, f) == 1 && box[2] >= 0 && box[3] >= 0 && box[2] <= kMaxSide &&
                      box[3] <= kMaxSide && (box[2] == 0) == (box[3] == 0);
            if (ok)
            {
                GlyphBitmap& b = job.field;
                b = GlyphBitmap{};
                b.channels = 4;
                b.sdf = true;
                b.left = box[0];
                b.top = box[1];
                b.width = box[2];
                b.height = box[3];
                b.pixels.resize((std::size_t)b.width * (std::size_t)b.height * 4);
                // all of it and nothing after it: a file cut short or grown is not this field
                ok = (b.pixels.empty() || std::fread(b.pixels.data(), b.pixels.size(), 1, f) == 1) && std::fgetc(f) == EOF && Sum(b.pixels) == sum;
                if (!ok)
                    b = GlyphBitmap{};
            }
            std::fclose(f);
            return ok;
        }

        // A name no other writer uses: this process's (its clock and where it was loaded) and a count
        std::string TempSuffix()
        {
            static std::atomic<std::uint64_t> next{0};
            static const std::uint64_t process =
                Mix((std::uint64_t)std::chrono::steady_clock::now().time_since_epoch().count() ^ (std::uint64_t)reinterpret_cast<std::uintptr_t>(&next));
            char s[48];
            std::snprintf(s, sizeof(s), ".%016llx-%llu.tmp", (unsigned long long)process, (unsigned long long)next.fetch_add(1));
            return s;
        }

        // Written beside and renamed over: a reader never sees half a file. A rename that fails (another process put
        // the same field there) leaves the one there.
        void Store(const std::filesystem::path& dir, const Job& job)
        {
            const std::filesystem::path file = FileOf(dir, job.name);
            std::filesystem::path tmp = file;
            tmp += TempSuffix();
            std::FILE* f = Open(tmp, true);
            std::error_code ec;
            if (!f)
            {
                std::filesystem::create_directories(dir, ec);
                f = Open(tmp, true);
                if (!f)
                    return;
            }
            const GlyphBitmap& b = job.field;
            const std::uint64_t sum = Sum(b.pixels);
            const std::int32_t box[4] = {b.left, b.top, b.width, b.height};
            bool ok = std::fwrite(kMagic, 8, 1, f) == 1 && std::fwrite(&job.check, 8, 1, f) == 1 && std::fwrite(&sum, 8, 1, f) == 1 &&
                      std::fwrite(box, sizeof(box), 1, f) == 1 && (b.pixels.empty() || std::fwrite(b.pixels.data(), b.pixels.size(), 1, f) == 1);
            ok = std::fclose(f) == 0 && ok;
            if (ok)
                std::filesystem::rename(tmp, file, ec);
            if (!ok || ec)
                std::filesystem::remove(tmp, ec);
        }
    }

    struct FieldMaker::Impl
    {
        int workers = 0;
        std::filesystem::path dir;   // empty: no cache
        std::vector<std::shared_ptr<Job>> batch;   // the owner's; a store's too once made
        bool made = true;                          // the batch was made: the next Add starts another

        std::mutex mutex;
        std::condition_variable wake;   // the workers: a shape, a field to write, or stop
        std::condition_variable done;   // the owner: the batch is made
        std::deque<Job*> queue;         // the batch's shapes no thread has taken
        std::deque<std::shared_ptr<Job>> stores;   // fields made, for the disk
        std::size_t pending = 0;        // the batch's fields not done
        bool writing = false;           // a worker writes the stores
        bool stop = false;
        std::vector<std::thread> threads;

        void Run(Job& job) const
        {
            if (!dir.empty())
            {
                Name(job);
                if (Load(FileOf(dir, job.name), job))
                {
                    job.ok = job.fromDisk = true;
                    job.shape = GlyphShape();
                    return;
                }
            }
            job.ok = GenerateMtsdf(job.shape, job.desc, job.field);
            job.shape = GlyphShape();   // its memory back now: the field stays until the batch is done with
        }

        void Work()
        {
            std::unique_lock lock(mutex);
            for (;;)
            {
                wake.wait(lock, [&] { return stop || !queue.empty() || (!stores.empty() && !writing); });
                if (!queue.empty())
                {
                    Job* job = queue.front();
                    queue.pop_front();
                    lock.unlock();
                    Run(*job);
                    lock.lock();
                    if (--pending == 0)
                        done.notify_one();
                }
                else if (!stores.empty() && (!writing || stop))
                {
                    // One writer at a time, behind the shapes: a batch that comes meanwhile finds the other workers free,
                    // and its reads do not queue behind a crowd of writes into the directory. When the maker goes, every
                    // worker writes (no batch comes any more).
                    const bool writer = !writing;
                    writing = true;
                    std::shared_ptr<Job> job = std::move(stores.front());
                    stores.pop_front();
                    lock.unlock();
                    Store(dir, *job);
                    job.reset();
                    lock.lock();
                    if (writer)
                        writing = false;
                }
                else
                    return;   // stopped, every field written
            }
        }
    };

    FieldMaker::FieldMaker(const FieldMakerDesc& desc) : impl_(new Impl)
    {
        const int processors = (int)std::thread::hardware_concurrency();
        impl_->workers = desc.workers >= 0 ? std::min(desc.workers, 63) : std::clamp(processors - 1, 0, 15);
        if (!desc.cacheDirectory.empty())
            impl_->dir = detail::PathFromUtf8(desc.cacheDirectory);
    }

    FieldMaker::~FieldMaker()
    {
        Impl& m = *impl_;
        {
            std::lock_guard lock(m.mutex);
            m.queue.clear();   // a batch never made: nobody waits for it
            m.pending = 0;
            m.stop = true;
        }
        m.wake.notify_all();
        for (std::thread& t : m.threads)
            t.join();
    }

    std::size_t FieldMaker::Add(GlyphShape&& shape, const DistanceFieldDesc& desc)
    {
        Impl& m = *impl_;
        if (m.made)
        {
            m.batch.clear();
            m.made = false;
        }
        // not make_shared: its control block compares type_info, which MinGW's libstdc++ then defines twice
        std::shared_ptr<Job> job(new Job);
        job->shape = std::move(shape);
        job->desc = desc;
        Job* const j = job.get();
        m.batch.push_back(std::move(job));
        if (j->shape.Empty())
        {
            j->ok = GenerateMtsdf(j->shape, j->desc, j->field);   // nothing to make: an empty field
            return m.batch.size() - 1;
        }
        while (m.threads.size() < (std::size_t)m.workers)
            m.threads.emplace_back([&m] { m.Work(); });
        {
            std::lock_guard lock(m.mutex);
            m.queue.push_back(j);
            ++m.pending;
        }
        if (m.workers > 0)
            m.wake.notify_one();
        return m.batch.size() - 1;
    }

    void FieldMaker::Make()
    {
        Impl& m = *impl_;
        if (m.made)
            return;
        {
            // the shapes no worker has taken, here; then what the workers are making
            std::unique_lock lock(m.mutex);
            while (m.pending > 0)
            {
                if (m.queue.empty())
                {
                    m.done.wait(lock, [&] { return m.pending == 0; });
                    break;
                }
                Job* job = m.queue.front();
                m.queue.pop_front();
                lock.unlock();
                m.Run(*job);
                lock.lock();
                --m.pending;
            }
        }
        m.made = true;
        if (m.dir.empty())
            return;
        // the fields made go to the disk, from the workers (without them: here)
        std::size_t queued = 0;
        {
            std::lock_guard lock(m.mutex);
            for (const std::shared_ptr<Job>& job : m.batch)
                if (job->ok && !job->fromDisk && job->name != 0 && m.workers > 0)
                {
                    m.stores.push_back(job);
                    ++queued;
                }
        }
        if (queued > 0)
            m.wake.notify_all();
        if (m.workers == 0)
            for (const std::shared_ptr<Job>& job : m.batch)
                if (job->ok && !job->fromDisk && job->name != 0)
                    Store(m.dir, *job);
    }

    const GlyphBitmap* FieldMaker::Field(std::size_t i) const
    {
        const Job& job = *impl_->batch[i];
        return job.ok ? &job.field : nullptr;
    }

    bool FieldMaker::FromDisk(std::size_t i) const { return impl_->batch[i]->fromDisk; }

    int FieldMaker::Workers() const { return impl_->workers; }
}
