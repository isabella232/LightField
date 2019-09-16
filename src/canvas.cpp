#include "pch.h"

#include "glerr.h"
#include "backdrop.h"
#include "glmesh.h"
#include "mesh.h"

#include "canvas.h"

namespace {

    char const* GlErrorNames[] {
        "GL_INVALID_ENUM",
        "GL_INVALID_VALUE",
        "GL_INVALID_OPERATION",
        "GL_STACK_OVERFLOW",
        "GL_STACK_UNDERFLOW",
        "GL_OUT_OF_MEMORY",
    };

}

void GetGlErrors( char const* func, int line ) {
    GLenum error;
    char const* errorText;

    while ( GL_NO_ERROR != ( error = glGetError( ) ) ) {
        if ( ( error >= GL_INVALID_ENUM ) && ( error <= GL_OUT_OF_MEMORY ) ) {
            errorText = GlErrorNames[error - GL_INVALID_ENUM];
        } else {
            errorText = "<unknown>";
        }
        debug( "!!! %s:%d: GL error %s [0x%04X]\n", func, line, errorText, error );
    }
}

void ClearGlErrors( ) {
    while ( GL_NO_ERROR != glGetError( ) ) {
        /*empty*/
    }
}

Canvas::Canvas(QWidget *parent)
    : QOpenGLWidget(parent), mesh(nullptr),
      scale(1), zoom(1), tilt(90), yaw(0),
      perspective(0.25), anim(this, "perspective"), status(" ")
{
    setFormat(QSurfaceFormat::defaultFormat());
    anim.setDuration(100);
}

Canvas::~Canvas()
{
    makeCurrent();
    if (mesh)
    {
        delete mesh;
    }
    doneCurrent();
}

void Canvas::view_anim(float v)
{
    anim.setStartValue(perspective);
    anim.setEndValue(v);
    anim.start();
}

void Canvas::view_orthographic()
{
    view_anim(0);
}

void Canvas::view_perspective()
{
    view_anim(0.25);
}

void Canvas::draw_shaded()
{
    set_drawMode(0);
}

void Canvas::draw_wireframe()
{
    set_drawMode(1);
}

void Canvas::load_mesh(Mesh* m)
{
    QVector3D lower(m->xmin(), m->ymin(), m->zmin());
    QVector3D upper(m->xmax(), m->ymax(), m->zmax());

    if (mesh) {
        delete mesh;
    }
    mesh = new GLMesh(m);
    delete m;

    center = (lower + upper) / 2;
    scale = 2 / (upper - lower).length();

    // Reset other camera parameters
    zoom = 1;
    yaw = 0;
    tilt = 90;

    update();
}

void Canvas::clear()
{
    if (mesh) {
        delete mesh;
        mesh = nullptr;
    }
    status.clear();

    update();
}

void Canvas::set_status(const QString &s)
{
    status = s;
    update();
}

void Canvas::set_perspective(float p)
{
    perspective = p;
    update();
}

void Canvas::set_drawMode(int mode)
{
    drawMode = mode;
    update();
}

void Canvas::clear_status()
{
    status.clear();
    update();
}

void Canvas::initializeGL( ) {
    initializeOpenGLFunctions( ); GET_GL_ERRORS;

    bool b;
    b = mesh_shader.addShaderFromSourceFile( QOpenGLShader::Vertex, ":/gl/mesh.vert" ); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Canvas::initializeGL: shader.addShaderFromSourceFile for mesh.vert failed\n" );
    }
    b = mesh_shader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/gl/mesh.frag" ); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Canvas::initializeGL: shader.addShaderFromSourceFile for mesh.frag failed\n" );
    }
    b = mesh_shader.link( ); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Canvas::initializeGL: mesh_shader.link failed\n" );
    }

    b = mesh_wireframe_shader.addShaderFromSourceFile( QOpenGLShader::Vertex, ":/gl/mesh.vert" ); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Canvas::initializeGL: shader.addShaderFromSourceFile for mesh.vert failed\n" );
    }
    b = mesh_wireframe_shader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/gl/mesh_wireframe.frag" ); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Canvas::initializeGL: shader.addShaderFromSourceFile for mesh_wireframe.frag failed\n" );
    }
    b = mesh_wireframe_shader.link( ); GET_GL_ERRORS;
    if ( !b ) {
        debug( "!!! Canvas::initializeGL: mesh_wireframe_shader.link failed\n" );
    }

    backdrop = new Backdrop;
}

void Canvas::paintGL( ) {
    QPainter painter;
    painter.begin( this );
    painter.beginNativePainting( );

    ClearGlErrors( );
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); GET_GL_ERRORS;
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); GET_GL_ERRORS;
    glEnable( GL_DEPTH_TEST ); GET_GL_ERRORS;

    backdrop->draw( );
    if ( mesh ) {
        draw_mesh( );
    }

    glDisable( GL_DEPTH_TEST ); GET_GL_ERRORS;

    painter.endNativePainting( );

    if ( !status.isEmpty( ) ) {
        painter.setRenderHint( QPainter::Antialiasing );
        painter.drawText( 10, height( ) - 10, status );
    }

    painter.end( );
}

void Canvas::draw_mesh()
{
    QOpenGLShaderProgram* selected_mesh_shader = NULL;
    // Set gl draw mode
    if(drawMode == 1)
    {
        selected_mesh_shader = &mesh_wireframe_shader;
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); GET_GL_ERRORS;
    }
    else
    {
        selected_mesh_shader = &mesh_shader;
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); GET_GL_ERRORS;
    }

    selected_mesh_shader->bind(); GET_GL_ERRORS;

    // Load the transform and view matrices into the shader
    int tm = selected_mesh_shader->uniformLocation("transform_matrix");
    if ( -1 == tm ) {
        debug( "!!! Canvas::draw_mesh: transform_matrix is invalid\n" );
    }
    glUniformMatrix4fv(tm, 1, GL_FALSE, transform_matrix().data()); GET_GL_ERRORS;
    int vm = selected_mesh_shader->uniformLocation("view_matrix");
    if ( -1 == vm ) {
        debug( "!!! Canvas::draw_mesh: view_matrix is invalid\n" );
    }
    glUniformMatrix4fv(vm, 1, GL_FALSE, view_matrix().data()); GET_GL_ERRORS;

    // Compensate for z-flattening when zooming
    GLint z = selected_mesh_shader->uniformLocation("zoom");
    if ( -1 == z ) {
        debug( "!!! Canvas::draw_mesh: zoom is invalid\n" );
    }
    glUniform1f(z, 1/zoom); GET_GL_ERRORS;

    // Find and enable the attribute location for vertex position
    const GLuint vp = selected_mesh_shader->attributeLocation("vertex_position"); GET_GL_ERRORS;
    if ( static_cast<GLuint>( -1 ) == vp ) {
        debug( "!!! Canvas::draw_mesh: vertex_position is invalid\n" );
    }
    glEnableVertexAttribArray(vp); GET_GL_ERRORS;

    // Then draw the mesh with that vertex position
    mesh->draw(vp);

    // Reset draw mode for the background and anything else that needs to be drawn
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); GET_GL_ERRORS;

    // Clean up state machine
    glDisableVertexAttribArray(vp); GET_GL_ERRORS;
    selected_mesh_shader->release(); GET_GL_ERRORS;
}

QMatrix4x4 Canvas::transform_matrix() const
{
    QMatrix4x4 m;
    m.rotate(tilt, QVector3D(1, 0, 0));
    m.rotate(yaw,  QVector3D(0, 0, 1));
    m.scale(-scale, scale, -scale);
    m.translate(-center);
    return m;
}

QMatrix4x4 Canvas::view_matrix() const
{
    QMatrix4x4 m;
    if (width() > height())
    {
        m.scale(-height() / float(width()), 1, 0.5);
    }
    else
    {
        m.scale(-1, width() / float(height()), 0.5);
    }
    m.scale(zoom, zoom, 1);
    m(3, 2) = perspective;
    return m;
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::RightButton)
    {
        mouse_pos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::RightButton)
    {
        unsetCursor();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    auto p = event->pos();
    auto d = p - mouse_pos;


    if (event->buttons() & Qt::LeftButton)
    {
        yaw = fmod(yaw - d.x(), 360);
        tilt = fmod(tilt - d.y(), 360);
        update();
    }
    else if (event->buttons() & Qt::RightButton)
    {
        center = transform_matrix().inverted() *
                 view_matrix().inverted() *
                 QVector3D(-d.x() / (0.5*width()),
                            d.y() / (0.5*height()), 0);
        update();
    }
    mouse_pos = p;
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    // Find GL position before the zoom operation
    // (to zoom about mouse cursor)
    auto p = event->pos();
    QVector3D v(1 - p.x() / (0.5*width()),
                p.y() / (0.5*height()) - 1, 0);
    QVector3D a = transform_matrix().inverted() *
                  view_matrix().inverted() * v;

    if (event->delta() < 0)
    {
        for (int i=0; i > event->delta(); --i)
            zoom *= 1.001;
    }
    else if (event->delta() > 0)
    {
        for (int i=0; i < event->delta(); ++i)
            zoom /= 1.001;
    }

    // Then find the cursor's GL position post-zoom and adjust center.
    QVector3D b = transform_matrix().inverted() *
                  view_matrix().inverted() * v;
    center += b - a;
    update();
}

void Canvas::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height); GET_GL_ERRORS;
}
