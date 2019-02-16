#include "pch.h"

#include "mesh.h"

////////////////////////////////////////////////////////////////////////////////

Mesh::Mesh(std::vector<GLfloat>&& v, std::vector<GLuint>&& i)
    : vertices(std::move(v)), indices(std::move(i))
{
    // Nothing to do here
}

float Mesh::min(size_t start) const
{
    if (start >= vertices.size())
    {
        return -1;
    }
    float v = vertices[start];
    for (size_t i=start; i < vertices.size(); i += 3)
    {
        v = fmin(v, vertices[i]);
    }
    return v;
}

float Mesh::max(size_t start) const
{
    if (start >= vertices.size())
    {
        return 1;
    }
    float v = vertices[start];
    for (size_t i=start; i < vertices.size(); i += 3)
    {
        v = fmax(v, vertices[i]);
    }
    return v;
}

void Mesh::bounds( size_t& count, float& minX, float& minY, float& minZ, float& maxX, float& maxY, float& maxZ ) {
    count = vertices.size( );
    minX = minY = minZ = std::numeric_limits<float>::max( );
    maxX = maxY = maxZ = std::numeric_limits<float>::min( );

    size_t limit = vertices.size( );
    for ( size_t i = 0; ( i + 2 ) < limit; i += 3 ) {
        float x = vertices[i];
        float y = vertices[i + 1];
        float z = vertices[i + 2];
        minX = std::min( x, minX );
        minY = std::min( y, minY );
        minZ = std::min( z, minZ );
        maxX = std::max( x, maxX );
        maxY = std::max( y, maxY );
        maxZ = std::max( z, maxZ );
    }
}

bool Mesh::empty() const
{
    return vertices.size() == 0;
}
