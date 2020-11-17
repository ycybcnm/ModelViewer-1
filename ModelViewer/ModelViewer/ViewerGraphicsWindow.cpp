#include "ViewerGraphicsWindow.h"

#include "ModelLoader.h"

#include <QGuiApplication>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QtMath>
#include <QFileDialog>
#include <QMouseEvent>
#include <QdesktopServices>
#include <QUrl>
#include <QtMath>


ViewerGraphicsWindow::ViewerGraphicsWindow(QWindow* parent)
    : OpenGLWindow(parent)
{
    QSurfaceFormat format;
    format.setSamples(16);
    setFormat(format);

    resetView();

    setAnimating(true);
}

bool ViewerGraphicsWindow::loadModel(QString filepath) {
    if (!initialized) {
        return false;
    }

    // If no filepath was provided, open a file dialog for the user to choose a model
    if (filepath.isEmpty()) {
        filepath = QFileDialog::getOpenFileName(nullptr, "Load Model", "../Data/Models/", "");
        if (filepath.isEmpty()) {
            return false;
        }
    }

    // Let other widgets know that we are beginning a load operation (may take some time)
    emit BeginModelLoading(filepath);

    // Load the model
    ModelLoader m;
    m_currentModel = m.LoadModel(filepath);
    
    // Let other widgets know that a model has been loaded
    emit EndModelLoading(m_currentModel.m_isValid, filepath);

    // Reset the view to size properly for the new model
    resetView();

    return m_currentModel.m_isValid;
}

bool ViewerGraphicsWindow::unloadModel()
{
    m_currentModel = Model();
    emit ModelUnloaded();

    return true;
}

bool ViewerGraphicsWindow::loadVertexShader(QString vertfilepath)
{
    if (!initialized)
    {
        return false;
    }
    if (vertfilepath.isEmpty()) {
        vertfilepath = QFileDialog::getOpenFileName(nullptr, "Load Vertex Shader", "../Data/Shaders/", "");
        if (vertfilepath.isEmpty()) {
            return false;
        }
    }

    m_program->removeAllShaders();

    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertfilepath)) 
    {
        m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, currentVertFile);
        vertfilepath = currentVertFile;
    }
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, currentFragFile);
    if (!m_program->link())
    {
        emit Error("Failed to link shader program.");

        currentVertFile = vertfilepath;
        return false;
    }

    currentVertFile = vertfilepath;
    setUniformLocations();
    
    emit ClearError();
    return true;
}

bool ViewerGraphicsWindow::loadFragmentShader(QString fragfilepath)
{
    if (!initialized)
    {
        return false;
    }
    
    if (fragfilepath.isEmpty()) {
        fragfilepath = QFileDialog::getOpenFileName(nullptr, "Load Fragment Shader", "../Data/Shaders/", "");
        if (fragfilepath.isEmpty()) {
            return false;
        }
    }

    m_program->removeAllShaders();

    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, currentVertFile);
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragfilepath)) 
    {
        m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, currentFragFile);
        fragfilepath = currentFragFile;
    }
    if (!m_program->link())
    {
        QString error = m_program->log();
        qDebug() << error << endl;

        currentFragFile = fragfilepath;

        emit Error("Failed to link shader program.");

        return false;
    }
    
    currentFragFile = fragfilepath;
    setUniformLocations();

    emit ClearError();
    return true;
}

bool ViewerGraphicsWindow::editCurrentShaders()
{
    QDesktopServices desk;
    QDir dir;
    QString currentPath = dir.currentPath();
    //qDebug() << currentPath << endl;

    QString fullVertPath = "//" + currentPath + "/" + currentVertFile;
    QString fullFragPath = "//" + currentPath + "/" + currentFragFile;
    //qDebug() << fullVertPath << endl;
    
    if (!desk.openUrl(QUrl::fromLocalFile(fullVertPath)))
    {
        return false;
    }
    if(!desk.openUrl(QUrl::fromLocalFile(fullFragPath)))
    {
        return false;
    }
    return true;
}

void ViewerGraphicsWindow::setUniformLocations()
{
    m_posAttr = m_program->attributeLocation("posAttr");
    Q_ASSERT(m_posAttr != -1);
    m_colAttr = m_program->attributeLocation("colAttr");
    Q_ASSERT(m_colAttr != -1);
    m_matrixUniform = m_program->uniformLocation("matrix");
    Q_ASSERT(m_matrixUniform != -1);

    m_normAttr = m_program->attributeLocation("normAttr");
    m_uvAttr = m_program->attributeLocation("uvAttr");

    m_modelviewUniform = m_program->uniformLocation("modelview");
    m_normalUniform = m_program->uniformLocation("normalMat");

    m_lightPosUniform = m_program->uniformLocation("uLightPos");
    m_uKa = m_program->uniformLocation("uKa");
    m_uKd = m_program->uniformLocation("uKd");
    m_uKs = m_program->uniformLocation("uKs");
    m_uSpecularColor = m_program->uniformLocation("uSpecularColor");
    m_uShininess = m_program->uniformLocation("uShininess");

    m_uMat4_1 = m_program->uniformLocation("uMat4_1");
    m_uVec3_1 = m_program->uniformLocation("uVec3_1");
    m_uVec4_1 = m_program->uniformLocation("uVec4_1");
    m_uFloat_1 = m_program->uniformLocation("uFloat_1");
    m_uInt_1 = m_program->uniformLocation("uInt_1");
}

bool ViewerGraphicsWindow::reloadCurrentShaders()
{
    if (loadVertexShader(currentVertFile) &&
        loadFragmentShader(currentFragFile)) {
        return true;
    }
}

bool ViewerGraphicsWindow::openShaderFile(QString filepath)
{
    if (filepath.isEmpty()) {
        filepath = QFileDialog::getOpenFileName(nullptr, "Open Shader File", "../Data/Shaders/", "");
        if (filepath.isEmpty()) {
            return false;
        }
    }

    QDesktopServices desk;
    QDir dir;
    QString currentPath = dir.currentPath();
    QString fullPath = "//" + currentPath + "/" + filepath;
    if (!desk.openUrl(QUrl::fromLocalFile(filepath)))
    {
        return false;
    }
    return true;
}

void ViewerGraphicsWindow::mousePressEvent(QMouseEvent* event)
{
    // Set class vars
    if (event->button() == Qt::LeftButton) {
        m_leftMousePressed = true;
    }

    if (event->button() == Qt::RightButton) {
        m_rightMousePressed = true;
    }

    // Make sure that these are set before the mouseMoveEvent triggers
    lastX = event->x();
    lastY = event->y();

    // Call the parent class 
    QWindow::mouseReleaseEvent(event);
}

void ViewerGraphicsWindow::mouseReleaseEvent(QMouseEvent* event)
{
    // Set class vars
    if (event->button() == Qt::LeftButton) {
        m_leftMousePressed = false;
    }

    if (event->button() == Qt::RightButton) {
        m_rightMousePressed = false;
    }

    // Call the parent class
    QWindow::mouseReleaseEvent(event);
}

void ViewerGraphicsWindow::mouseMoveEvent(QMouseEvent* event)
{
    float deltaX = lastX - event->x();
    float deltaY = lastY - event->y();

    // RMB: Rotate off of x y movement
    if (event->buttons() & Qt::LeftButton && m_leftMousePressed) {
        QVector3D xAxis(1, 0, 0);
        QVector3D yAxis(0, 1, 0);
        
        QMatrix4x4 newRot;
        newRot.rotate(-deltaX * xRotateSensitivity, yAxis);
        newRot.rotate(-deltaY * yRotateSensitivity, xAxis);

        // Perform the new rotation AFTER the previous rotations
        m_rotMatrix = newRot * m_rotMatrix;
    }

    // MMB: Pan off of x y movement
    if (event->buttons() & Qt::RightButton && m_rightMousePressed) {
        // Adjust pan sensitivity based on the size of the window and FOV
        const float panAdj = (480.f / (float)height()) * (fieldOfView / 60.f);

        m_transMatrix.translate(-deltaX * panXSensitivity * panAdj, 0, 0);
        m_transMatrix.translate(0, deltaY * panYSensitivity * panAdj, 0);
    }

    // After moving update the lastX/Y
    lastX = event->x();
    lastY = event->y();

    // Call the parent class 
    QWindow::mouseMoveEvent(event);
}

void ViewerGraphicsWindow::wheelEvent(QWheelEvent* event)
{
    const float zoomAmount = zoomSensitivity * event->angleDelta().y();
    m_scaleMatrix.scale(1.f + zoomAmount);
}

void ViewerGraphicsWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    currentVertFile = "../Data/Shaders/ads.vert";
    currentFragFile = "../Data/Shaders/ads.frag";
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, currentVertFile);
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, currentFragFile);
    m_program->link();

    setUniformLocations();

    // Set up the default view
    resetView();

    lightPos = QVector3D(1., 1., -1.);
    uKa = 0.30;
    uKd = 0.40;
    uKs = 0.35;
    ADColor = QVector4D(0., 1., 0., 1.);
    specularColor = QVector4D(1., 1., 1., 1.);
    shininess = 1.0;

    uMat4_1 = QMatrix4x4();
    uVec3_1 = QVector3D(0.5, 0.5, 0.);
    uVec4_1 = QVector4D(1., 1., 1., 1.);
    uFloat_1 = 0.;
    uInt_1 = 0;

    emit Initialized();

    initialized = true;
}

void ViewerGraphicsWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    QMatrix4x4 viewMatrix;
    viewMatrix.perspective(fieldOfView, float(width() * retinaScale) / float(height() * retinaScale), nearPlane, farPlane);

    QMatrix4x4 modelMatrix = GetModelMatrix();

    QMatrix4x4 modelViewProjectionMatrix;
    modelViewProjectionMatrix = viewMatrix * modelMatrix;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();

    m_program->setUniformValue(m_matrixUniform, modelViewProjectionMatrix);

    m_program->setUniformValue(m_modelviewUniform, modelMatrix);

    QMatrix3x3 normal = modelMatrix.normalMatrix();
    m_program->setUniformValue(m_normalUniform, normal);

    m_program->setUniformValue(m_lightPosUniform, lightPos);

    m_program->setUniformValue(m_uKa, uKa);
    m_program->setUniformValue(m_uKd, uKd);
    m_program->setUniformValue(m_uKs, uKs);

    m_program->setUniformValue(m_uSpecularColor, specularColor);

    m_program->setUniformValue(m_uShininess, shininess);

    m_program->setAttributeValue(m_colAttr, ADColor);

    m_program->setUniformValue(m_uMat4_1, uMat4_1);
    m_program->setUniformValue(m_uVec3_1, uVec3_1);
    m_program->setUniformValue(m_uVec4_1, uVec4_1);
    m_program->setUniformValue(m_uFloat_1, uFloat_1);
    m_program->setUniformValue(m_uInt_1, uInt_1);
    
    glEnable(GL_DEPTH_TEST);

    if (m_currentModel.m_isValid)
    {
        // Loop through all meshes in the current model
        for (Mesh& mesh : m_currentModel.m_meshes) {
            mesh.m_vertexBuffer.bind();
            mesh.m_indexBuffer.bind();

            // Positions
            glVertexAttribPointer(m_posAttr, mesh.m_numPositionComponents, GL_FLOAT, GL_FALSE, 0, (void*)mesh.m_positionOffset);
            glEnableVertexAttribArray(m_posAttr);

            // Normals
            if (mesh.m_hasNormals) {
                glVertexAttribPointer(m_normAttr, mesh.m_numNormalComponents, GL_FLOAT, GL_FALSE, 0, (void*)mesh.m_normalOffset);
                glEnableVertexAttribArray(m_normAttr);
            }

            // UV Coords
            if (mesh.m_hasUVCoordinates) {
                glVertexAttribPointer(m_uvAttr, mesh.m_numUVComponents, GL_FLOAT, GL_FALSE, 0, (void*)mesh.m_uvOffset);
                glEnableVertexAttribArray(m_uvAttr);
            }

            // Colors
            if (mesh.m_hasColors) {
                glVertexAttribPointer(m_colAttr, mesh.m_numColorComponents, GL_FLOAT, GL_FALSE, 0, (void*)mesh.m_colorOffset);
                glEnableVertexAttribArray(m_colAttr);
            }

            glDrawElements(GL_TRIANGLES, mesh.m_indexCount, GL_UNSIGNED_INT, nullptr);

            // Disable all attributes
            if (mesh.m_hasColors) {
                glDisableVertexAttribArray(m_colAttr);
            }
            if (mesh.m_hasNormals) {
                glDisableVertexAttribArray(m_normAttr);
            }
            if (mesh.m_hasUVCoordinates) {
                glDisableVertexAttribArray(m_uvAttr);
            }

            glDisableVertexAttribArray(m_posAttr);
        }
    }

    m_program->release();

    ++m_frame;
}

void ViewerGraphicsWindow::resetView()
{
    // Reset matrices to default values
    m_scaleMatrix = QMatrix4x4();
    m_rotMatrix = QMatrix4x4();
    m_transMatrix = QMatrix4x4();
    m_transMatrix.translate(0, 0, -4);

    // Scale the scene so the entire model can be viewed
    if (m_currentModel.m_isValid)
    {
        // Adjust the effective field of view if the window is taller than it is wide
        const float effectiveFOV = std::min(fieldOfView, fieldOfView * float(width()) / float(height()));

        // Compute optimal viewing distance as modelSize / atan(fov)
        const float modelSize = std::max(m_currentModel.m_AABBMax.length(), m_currentModel.m_AABBMin.length());
        const float optimalViewingDistance = modelSize / qAtan(qDegreesToRadians(effectiveFOV)) * 1.6f;
        
        // Scale the world so 4 looks like optimalViewingDistance
        const float scale = 4.f / optimalViewingDistance;
        m_scaleMatrix.scale(scale);
    }
}

bool ViewerGraphicsWindow::addPrimitive(QString primitiveName) {
    // Load  model
    QString filepath = QString("../Data/Primitives/%1").arg(primitiveName);
    return loadModel(filepath);
}


// ***************************************************
// Getters & Setters
// ***************************************************

bool ViewerGraphicsWindow::GetLeftMousePressed()
{
    return m_leftMousePressed;
}
bool ViewerGraphicsWindow::GetRightMousePressed()
{
    return m_rightMousePressed;
}
QMatrix4x4 ViewerGraphicsWindow::GetScaleMatrix()
{
    return m_scaleMatrix;
}
void ViewerGraphicsWindow::SetScale(float scale)
{
    QMatrix4x4 newMat;
    newMat.scale(scale);
    m_scaleMatrix = newMat;
}
QMatrix4x4 ViewerGraphicsWindow::GetRotationMatrix()
{
    return m_rotMatrix;
}
QMatrix4x4 ViewerGraphicsWindow::GetTranslationMatrix()
{
    return m_transMatrix;
}
QMatrix4x4 ViewerGraphicsWindow::GetModelMatrix()
{
    return m_transMatrix * m_rotMatrix * m_scaleMatrix;
}
bool ViewerGraphicsWindow::IsModelValid() 
{
    return m_currentModel.m_isValid;
}

//uniform vars getters and setters
QVector3D ViewerGraphicsWindow::getLightLocation()
{
    return lightPos;
}

void ViewerGraphicsWindow::setLightLocation(float x, float y, float z)
{
    lightPos = QVector3D(x, y, z);
    //m_program->setUniformValue(m_lightPosUniform, lightPos);
}

QVector3D ViewerGraphicsWindow::getADS()
{
    QVector3D ADS = QVector3D(uKa, uKd, uKs);
    return(ADS);
}

void ViewerGraphicsWindow::setADS(float a, float d, float s)
{
    uKa = a;
    uKd = d;
    uKs = s;
}

QVector4D ViewerGraphicsWindow::getSpecularColor()
{
    return specularColor;
}

void ViewerGraphicsWindow::setSpecularColor(float r, float g, float b)
{
    specularColor = QVector4D(r, g, b, 1.);
}

float ViewerGraphicsWindow::getShininess()
{
    return shininess;
}
void ViewerGraphicsWindow::setShininess(float new_shininess)
{
    shininess = new_shininess;
}
QVector4D ViewerGraphicsWindow::getADColor()
{
    return ADColor;
}
void ViewerGraphicsWindow::setADColor(float r, float g, float b)
{
    ADColor = QVector4D(r, g, b, 1.);
}