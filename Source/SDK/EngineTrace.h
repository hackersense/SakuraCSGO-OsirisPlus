#pragma once

#include <cstddef>

#include "Inconstructible.h"
#include "Vector.h"
#include "VirtualMethod.h"

struct Ray {
    Ray(const Vector& src, const Vector& dest)
        : start(src), delta(dest - src), extents(Vector{ }), startOffset(Vector{ }), worldAxisTransform(NULL), isRay(true) { isSwept = delta.x || delta.y || delta.z; }

    Ray(const Vector& src, const Vector& dest, const Vector& mins, const Vector& maxs)
        : delta(dest - src), extents(maxs - mins), startOffset(maxs + mins), worldAxisTransform(NULL)
    {
        isSwept = delta.x || delta.y || delta.z;
        extents *= 0.5f;
        isRay = (extents.squareLength() < 1e-6);

        startOffset *= 0.5f;
        start = src + startOffset;
        startOffset *= -1.0f;
    }

    Vector start{ };
    float pad{ };
    Vector delta{ };
    float pad1{ };
    Vector startOffset{ };
    float pad2{ };
    Vector extents{ };
    float pad3{ };
    const matrix3x4* worldAxisTransform;
    bool isRay{ };
    bool isSwept{ };
};

class Entity;

struct TraceFilter {
    TraceFilter(const Entity* entity) : skip{ entity } { }
    virtual bool shouldHitEntity(Entity* entity, int) { return entity != skip; }
    virtual int getTraceType() const { return 0; }
    const void* skip;
};

namespace HitGroup {
    enum {
        Invalid = -1,
        Generic,
        Head,
        Chest,
        Stomach,
        LeftArm,
        RightArm,
        LeftLeg,
        RightLeg,
        Gear = 10
    };

    constexpr float getDamageMultiplier(int hitGroup) noexcept
    {
        switch (hitGroup) {
        case Head:
            return 4.0f;
        case Stomach:
            return 1.25f;
        case LeftLeg:
        case RightLeg:
            return 0.75f;
        default:
            return 1.0f;
        }
    }

    constexpr bool isArmored(int hitGroup, bool helmet) noexcept
    {
        switch (hitGroup) {
        case Head:
            return helmet;

        case Chest:
        case Stomach:
        case LeftArm:
        case RightArm:
            return true;
        default:
            return false;
        }
    }
}

struct Trace {
    Vector startpos;
    Vector endpos;
    struct Plane {
        Vector normal;
        float distance;
    } plane;
    std::byte pad[4];
    float fraction;
    int contents;
    unsigned short dispFlags;
    bool allSolid;
    bool startSolid;
    std::byte pad1[4];
    struct Surface {
        const char* name;
        short surfaceProps;
        unsigned short flags;
    } surface;
    int hitgroup;
    std::byte pad2[4];
    Entity* entity;
    int hitbox;

    const bool didHit() { return fraction < 1.0f || allSolid || startSolid; }
};

// #define TRACE_STATS // - enable to see how many rays are cast per frame

#ifdef TRACE_STATS
#include "../Memory.h"
#include "GlobalVars.h"
#endif

class EngineTrace {
public:
    INCONSTRUCTIBLE(EngineTrace)

    VIRTUAL_METHOD(int, getPointContents, 0, (const Vector& absPosition, int contentsMask), (this, std::cref(absPosition), contentsMask, nullptr))
    VIRTUAL_METHOD(void, _traceRay, 5, (const Ray& ray, unsigned int mask, const TraceFilter& filter, Trace& trace), (this, std::cref(ray), mask, std::cref(filter), std::ref(trace)))

    void traceRay(const Ray& ray, unsigned int mask, const TraceFilter& filter, Trace& trace) noexcept
    {
#ifdef TRACE_STATS
        static int tracesThisFrame, lastFrame;

        if (lastFrame != memory->globalVars->framecount) {
            memory->debugMsg("traces: frame - %d | count - %d\n", lastFrame, tracesThisFrame);
            tracesThisFrame = 0;
            lastFrame = memory->globalVars->framecount;
        }

        ++tracesThisFrame;
#endif
        _traceRay(ray, mask, filter, trace);
    }
};
