#pragma once

namespace QuickUI::Components::Internal {

struct Yuv420Vertex
{
    float x = 0.0f;
    float y = 0.0f;
    float u = 0.0f;
    float v = 0.0f;

    friend bool operator==(const Yuv420Vertex& lhs, const Yuv420Vertex& rhs)
    {
        return lhs.x == rhs.x
            && lhs.y == rhs.y
            && lhs.u == rhs.u
            && lhs.v == rhs.v;
    }
};

} // namespace QuickUI::Components::Internal
