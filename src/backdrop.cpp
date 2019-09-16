#include "pch.h"

#include "glerr.h"

#include "backdrop.h"

Backdrop::Backdrop()
{
    initializeOpenGLFunctions(); GET_GL_ERRORS;

    bool b;
    b = shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/gl/quad.vert"); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Backdrop::`ctor: shader.addShaderFromSourceFile for quad.vert failed\n" );
    }
    shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/gl/quad.frag"); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Backdrop::`ctor: shader.addShaderFromSourceFile for quad.frag failed\n" );
    }
    b = shader.link(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Backdrop::`ctor: shader.link failed\n" );
    }

    float vbuf[] = {
        -1, -1, 0.00, 0.10, 0.15,
        -1,  1, 0.03, 0.21, 0.26,
         1, -1, 0.00, 0.12, 0.18,
         1,  1, 0.06, 0.26, 0.30};

    b = vertices.create(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Backdrop::`ctor: vertices.create failed\n" );
    }
    b = vertices.bind(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Backdrop::`ctor: vertices.bind failed\n" );
    }
    vertices.allocate(vbuf, sizeof(vbuf)); GET_GL_ERRORS;
    vertices.release(); GET_GL_ERRORS;
}

void Backdrop::draw()
{
    bool b;
    b = shader.bind(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Backdrop::draw: shader.bind failed\n" );
    }
    b = vertices.bind(); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Backdrop::draw: vertices.bind failed\n" );
    }

    const GLuint vp = shader.attributeLocation("vertex_position"); GET_GL_ERRORS;
    if ( static_cast<GLuint>( -1 ) == vp ) {
        debug( "!!! Backdrop::draw: vertex_position is invalid\n" );
    }
    const GLuint vc = shader.attributeLocation("vertex_color"); GET_GL_ERRORS;
    if ( static_cast<GLuint>( -1 ) == vc ) {
        debug( "!!! Backdrop::draw: vertex_color is invalid\n" );
    }

    glEnableVertexAttribArray(vp); GET_GL_ERRORS;
    glEnableVertexAttribArray(vc); GET_GL_ERRORS;

    glVertexAttribPointer(vp, 2, GL_FLOAT, false, 5 * sizeof(GLfloat), 0); GET_GL_ERRORS;
    glVertexAttribPointer(vc, 3, GL_FLOAT, false, 5 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat))); GET_GL_ERRORS;

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 8); GET_GL_ERRORS;

    vertices.release(); GET_GL_ERRORS;
    shader.release(); GET_GL_ERRORS;
}
