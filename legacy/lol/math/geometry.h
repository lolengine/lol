//
//  Lol Engine
//
//  Copyright © 2010—2020 Sam Hocevar <sam@hocevar.net>
//            © 2010—2015 Benjamin “Touky” Huet <huet.benjamin@gmail.com>
//
//  Lol Engine is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

//
// Various geometry functions
// --------------------------
//

#include <lol/base/enum.h>
#include <lol/math>      // distance, clamp…
#include <lol/vector>    // vec_t
#include <lol/transform> // mat_t

#include <algorithm>
#include <map>
#include <cmath>    // std::fabs
#include <stdint.h>

namespace lol
{

//AxisBase --------------------------------------------------------------------
struct AxisBase : public StructSafeEnum
{
    enum Type
    {
        X = 0, Y, Z, MAX, XY = 2, XYZ = 3,
    };
protected:
    virtual bool BuildEnumMap(std::map<int64_t, std::string>& enum_map)
    {
        enum_map[X]   = "X";
        enum_map[Y]   = "Y";
        enum_map[Z]   = "Z";
        enum_map[MAX] = "MAX";
        enum_map[XY]  = "XY";
        enum_map[XYZ] = "XYZ";
        return true;
    }
};
typedef SafeEnum<AxisBase> Axis;

//DirectionBase ---------------------------------------------------------------
struct DirectionBase : public StructSafeEnum
{
    enum Type
    {
        Up = 0, Down, Left, Right, MAX,
    };
protected:
    virtual bool BuildEnumMap(std::map<int64_t, std::string>& enum_map)
    {
        enum_map[Up]    = "Up";
        enum_map[Down]  = "Down";
        enum_map[Left]  = "Left";
        enum_map[Right] = "Right";
        enum_map[MAX]   = "MAX";
        return true;
    }
};
typedef SafeEnum<DirectionBase> Direction;

template<typename T, int N>
struct [[nodiscard]] box_t
{
    inline box_t()
      : aa(vec_t<T, N>(T(0))),
        bb(vec_t<T, N>(T(0)))
    {}

    inline box_t(vec_t<T,N> const &a, vec_t<T,N> const &b)
      : aa(a),
        bb(b)
    {}

    inline box_t(T const &ax, T const &ay, T const &bx, T const &by)
      : aa(ax, ay),
        bb(bx, by)
    {}

    inline box_t(T const &ax, T const &ay, T const &az,
                 T const &bx, T const &by, T const &bz)
      : aa(ax, ay, az),
        bb(bx, by, bz)
    {}

    box_t<T,N> operator +(vec_t<T,N> const &v) const
    {
        return box_t<T,N>(aa + v, bb + v);
    }

    box_t<T,N> &operator +=(vec_t<T,N> const &v)
    {
        return *this = *this + v;
    }

    box_t<T,N> operator -(vec_t<T,N> const &v) const
    {
        return box_t<T,N>(aa - v, bb - v);
    }

    box_t<T,N> &operator -=(vec_t<T,N> const &v)
    {
        return *this = *this - v;
    }

    box_t<T,N> operator *(vec_t<T,N> const &v) const
    {
        return box_t<T,N>(aa * v, bb * v);
    }

    box_t<T,N> &operator *=(vec_t<T,N> const &v)
    {
        return *this = *this * v;
    }

    box_t<T,N> operator *(T const &s) const
    {
        return box_t<T,N>(aa * s, bb * s);
    }

    box_t<T,N> &operator *=(T const &s)
    {
        return *this = *this * s;
    }

    [[nodiscard]] bool operator ==(box_t<T,N> const &box) const
    {
        return aa == box.aa && bb == box.bb;
    }

    [[nodiscard]] bool operator !=(box_t<T,N> const &box) const
    {
        return aa != box.aa || bb != box.bb;
    }

    inline vec_t<T,N> center() const { return (bb + aa) / 2; }

    inline vec_t<T,N> extent() const { return bb - aa; }

    vec_t<T,N> aa, bb;
};

//
// Generic box type names
//

#define T_(tleft, tright, suffix) \
    typedef tleft float tright suffix; \
    typedef tleft double tright d##suffix; \
    typedef tleft int32_t tright i##suffix; \
    typedef tleft uint32_t tright u##suffix;

// Idiotic hack to put "," inside a macro argument
#define C_ ,

T_(box_t<, C_ 2>, box2)
T_(box_t<, C_ 3>, box3)
T_(box_t<, C_ 4>, box4)

#undef C_
#undef T_

static_assert(sizeof(box2) == 16, "sizeof(box2) == 16");
static_assert(sizeof(box3) == 24, "sizeof(box3) == 24");
static_assert(sizeof(dbox2) == 32, "sizeof(dbox2) == 32");
static_assert(sizeof(dbox3) == 48, "sizeof(dbox3) == 48");

/*
 * Helper geometry functions
 */
class TestEpsilon
{
private:
    float           m_epsilon;
    float           m_value;
public:
    TestEpsilon()   { m_value = 0.f; m_epsilon = .0001f; }
    static float Get();
    static void Set(float epsilon=.0001f);
    static const TestEpsilon& F(float value);
private:
    float Minus() const;
    float Plus()  const;
public:
    [[nodiscard]] bool operator==(float value) const;
    [[nodiscard]] bool operator!=(float value) const;
    [[nodiscard]] bool operator<(float value)  const;
    [[nodiscard]] bool operator<=(float value) const;
    [[nodiscard]] bool operator>(float value)  const;
    [[nodiscard]] bool operator>=(float value) const;
};
[[nodiscard]] bool operator==(float value, const TestEpsilon& epsilon);
[[nodiscard]] bool operator!=(float value, const TestEpsilon& epsilon);
[[nodiscard]] bool operator<(float value, const TestEpsilon& epsilon);
[[nodiscard]] bool operator<=(float value, const TestEpsilon& epsilon);
[[nodiscard]] bool operator>(float value, const TestEpsilon& epsilon);
[[nodiscard]] bool operator>=(float value, const TestEpsilon& epsilon);

//--
static inline bool TestAABBVsAABB(box2 const &b1, box2 const &b2)
{
    using std::fabs;

    vec2 c = b1.center() - b2.center();
    vec2 e1 = 0.5f * b1.extent();
    vec2 e2 = 0.5f * b2.extent();

    return fabs(c.x) <= e1.x + e2.x
        && fabs(c.y) <= e1.y + e2.y;
}
static inline bool TestAABBVsPoint(box2 const &b1, vec2 const &p)
{
    return TestAABBVsAABB(b1, box2(p, p));
}

static inline bool TestAABBVsAABB(box3 const &b1, box3 const &b2)
{
    using std::fabs;

    vec3 c = b1.center() - b2.center();
    vec3 e1 = 0.5f * b1.extent();
    vec3 e2 = 0.5f * b2.extent();

    return fabs(c.x) <= e1.x + e2.x
        && fabs(c.y) <= e1.y + e2.y
        && fabs(c.z) <= e1.z + e2.z;
}
static inline bool TestAABBVsPoint(box3 const &b1, vec3 const &p)
{
    return TestAABBVsAABB(b1, box3(p, p));
}

bool TestTriangleVsTriangle(vec3 const &v00, vec3 const &v01, vec3 const &v02,
                           vec3 const &v10, vec3 const &v11, vec3 const &v12,
                           vec3 &ip00, vec3 &ip10);
bool TestRayVsTriangleSide(vec3 const &v0, vec3 const &v1, vec3 const &v2,
                          vec3 const &ip0, vec3 const &ip1,
                          vec3 &iV0, int &iIdx0, vec3 &iV1, int &iIdx1);
bool TestRayVsTriangle(vec3 const &ray_point, vec3 const &ray_dir,
                      vec3 const &tri_p0, vec3 const &tri_p1, vec3 const &tri_p2,
                      vec3 &vi);

//RayIntersect ----------------------------------------------------------------
struct RayIntersectBase : public StructSafeEnum
{
    enum Type
    {
        Nothing,
        All,
        None,
        P0,
        P1,
    };
    //LOL_DECLARE_ENUM_METHODS(RayIntersectBase)
protected:
    virtual bool BuildEnumMap(std::map<int64_t, std::string>& enum_map)
    {
        enum_map[Nothing] = "Nothing";
        enum_map[All] = "All";
        enum_map[None] = "None";
        enum_map[P0] = "P0";
        enum_map[P1] = "P1";
        return true;
    }
};
typedef SafeEnum<RayIntersectBase> RayIntersect;

#define RAY_ISECT_NOTHING   0
#define RAY_ISECT_ALL       1
#define RAY_ISECT_NONE      2
#define RAY_ISECT_P0        3
#define RAY_ISECT_P1        4
int TestRayVsRay(vec3 const &ray_p00, vec3 const &ray_p01,
                 vec3 const &ray_p10, vec3 const &ray_p11,
                 vec3 &isec_p);
bool TestPointVsFrustum(const vec3& point, const mat4& frustum, vec3* result_point = nullptr);

//Ray/Plane : Normal must be given normalized. returns 1 if succeeded.
template <typename TV>
bool TestRayVsPlane(const TV &ray_p0,  const TV &ray_p1,
                    const TV &plane_p, const TV &plane_n,
                          TV &isec_p,  bool test_line_only=false)
{
    TV ray_dir = ray_p1 - ray_p0;
    float d = dot(ray_dir, plane_n);

    if (d > -TestEpsilon::Get() && d < TestEpsilon::Get())
        return false;

    TV o2p1 = ray_p1 - plane_p;
    TV o2p0 = ray_p0 - plane_p;

    if (!test_line_only)
    {
        d = dot(o2p1, plane_n);
        d *= dot(o2p0, plane_n);

        //point are on the same side, so ray can intersect.
        if (d > .0f)
            return false;
    }

    float t = (dot(ProjectPointOnPlane(ray_p0, plane_p, plane_n) - ray_p0, plane_n)) / dot(ray_dir, plane_n);

    if (!test_line_only && (t < -TestEpsilon::Get() || t > 1.f + TestEpsilon::Get()))
        return false;

    isec_p = ray_p0 + t * ray_dir;
    return true;
}

//PlaneIntersectionBase -------------------------------------------------------
struct PlaneIntersectionBase : public StructSafeEnum
{
    /* A safe enum for Primitive edge face. */
    enum Type
    {
        Back, Front, Plane,
    };
protected:
    virtual bool BuildEnumMap(std::map<int64_t, std::string>& enum_map)
    {
        enum_map[Back]  = "Back";
        enum_map[Front] = "Front";
        enum_map[Plane] = "Plane";
        return true;
    }
};
typedef SafeEnum<PlaneIntersectionBase> PlaneIntersection;

//Point/Plane : Normal must be given normalized.
template <typename TV>
PlaneIntersection TestPointVsPlane(const TV &point, const TV &plane_p, const TV &plane_n)
{
    float d = dot(normalize(point - plane_p), plane_n);
    if (d > TestEpsilon::Get())
        return PlaneIntersection::Front;
    else if (d < -TestEpsilon::Get())
        return PlaneIntersection::Back;
    else
        return PlaneIntersection::Plane;
}

/* Project point on plane */
template <typename TV>
TV ProjectPointOnPlane(TV const &p, TV const &origin, TV const &normal)
{
    return p - dot(p - origin, normal) * normal;
}

/* Project point on line */
template <typename TV>
TV ProjectPointOnRay(TV const &p, TV const &origin, TV const &direction)
{
    return origin + direction * dot(p - origin, direction);
}

/* Distance from point to plane */
template <typename TV>
float PointDistToPlane(TV const &p, TV const &origin, TV const &normal)
{
    return abs(dot(p - origin, normal));
}

/* Distance from point to segment */
template <typename TV>
float PointDistToSegment(TV const &p, TV const &a, TV const &b)
{
    float d2 = sqlength(b - a);
    float u = d2 ? dot(p - a, b - a) / d2 : 0.0f;
    return distance(p, mix(a, b, clamp(u, 0.0f, 1.0f)));
}

} /* namespace lol */

