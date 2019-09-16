#include "pch.h"

#include "glerr.h"
#include "mesh.h"

#include "glmesh.h"

GLMesh::GLMesh(const Mesh* const mesh)
    : vertices(QOpenGLBuffer::VertexBuffer), indices(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions(); GET_GL_ERRORS;

    bool b;
    b = vertices.create(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! GLMesh::`ctor: vertices.create failed\n" );
    }
    b = indices.create(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! GLMesh::`ctor: indices.create failed\n" );
    }

    vertices.setUsagePattern(QOpenGLBuffer::StaticDraw); GET_GL_ERRORS;
    indices.setUsagePattern(QOpenGLBuffer::StaticDraw); GET_GL_ERRORS;

    b = vertices.bind(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! GLMesh::`ctor: vertices.bind failed\n" );
    }
    vertices.allocate(mesh->vertices.data(), mesh->vertices.size() * sizeof(float)); GET_GL_ERRORS;
    vertices.release(); GET_GL_ERRORS;

    b = indices.bind(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! GLMesh::`ctor: indices.bind failed\n" );
    }
    indices.allocate(mesh->indices.data(), mesh->indices.size() * sizeof(uint32_t)); GET_GL_ERRORS;
    indices.release(); GET_GL_ERRORS;
}

void GLMesh::draw(GLuint vp)
{
    bool b;
    b = vertices.bind(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! GLMesh::draw: vertices.bind failed\n" );
    }
    b = indices.bind(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! GLMesh::draw: indices.bind failed\n" );
    }

    glVertexAttribPointer(vp, 3, GL_FLOAT, false, 3*sizeof(float), NULL); GET_GL_ERRORS;
    glDrawElements(GL_TRIANGLES, indices.size() / sizeof(uint32_t), GL_UNSIGNED_INT, NULL); GET_GL_ERRORS;

    vertices.release(); GET_GL_ERRORS;
    indices.release(); GET_GL_ERRORS;
}
