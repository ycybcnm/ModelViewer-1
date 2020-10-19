#pragma once
#include "OpenGLWindow.h"
#include "ModelLoader.h"

#include <QOpenGLShaderProgram>

class ViewerGraphicsWindow : public OpenGLWindow
{
public:
    using OpenGLWindow::OpenGLWindow;

    ViewerGraphicsWindow(QWindow* parent = nullptr);

    void initialize() override;
    void render() override;
    void resetView();

    bool loadModel(QString filepath = QString());

private:
    bool initialized = false;

    GLint m_posAttr = 0;
    GLint m_normAttr = -1;
    GLint m_uvAttr = -1;
    GLint m_colAttr = 0;
    GLint m_matrixUniform = 0;

    QOpenGLShaderProgram* m_program = nullptr;
    int m_frame = 0;

    Model m_currentModel;

    // Mouse functions
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);

    // Mouse variables
    QMatrix4x4* sceneMatrix;
    int lastX;
    int lastY;
    float viewportX = 0;
    float viewportY = 0;

    // Mouse state
    bool leftMousePressed;
    bool rightMousePressed;

    // Mouse settings | % adjustment
    float viewportXSensitivity = 1;
    float viewportYSensitivity = 1;
    float panXSensitivity = .01;
    float panYSensitivity = .01;
    float xRotateSensitivity = 1;
    float yRotateSensitivity = 1;
    float zoomSensitivity = 1;
};
