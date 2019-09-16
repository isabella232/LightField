#ifndef CANVAS_H
#define CANVAS_H

#include <qopenglfunctions_2_0.h>
#include <qopenglfunctions_es2.h>

class GLMesh;
class Mesh;
class Backdrop;

class Canvas:
    public QOpenGLWidget,
#if defined QT_OPENGL_ES_2
    protected QOpenGLFunctions_ES2
#else // !defined QT_OPENGL_ES_2
    protected QOpenGLFunctions_2_0
#endif // defined QT_OPENGL_ES_2
{
    Q_OBJECT

public:
    Canvas(QWidget* parent=0);
    virtual ~Canvas() override;

    void view_orthographic();
    void view_perspective();
    void draw_shaded();
    void draw_wireframe();

public slots:
    void set_status(const QString& s);
    void clear_status();
    void load_mesh(Mesh* m);
    void clear();

protected:
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void set_perspective(float p);
    void set_drawMode(int mode);
    void view_anim(float v);

private:
    void draw_mesh();

    QMatrix4x4 transform_matrix() const;
    QMatrix4x4 view_matrix() const;

    QOpenGLShaderProgram mesh_shader;
    QOpenGLShaderProgram mesh_wireframe_shader;
    QOpenGLShaderProgram quad_shader;

    GLMesh* mesh;
    Backdrop* backdrop;

    QVector3D center;
    float scale;
    float zoom;
    float tilt;
    float yaw;

    float perspective;
    int drawMode;
    Q_PROPERTY(float perspective MEMBER perspective WRITE set_perspective);
    QPropertyAnimation anim;

    QPoint mouse_pos;
    QString status;
};

#endif // CANVAS_H
