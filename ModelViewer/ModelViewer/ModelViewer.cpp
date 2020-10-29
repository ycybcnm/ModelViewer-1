#include "ModelViewer.h"
#include "ViewerGraphicsWindow.h"

#include <QWidget>
#include <QLayout>
#include <QMenuBar>
#include <QMenu>
#include <QMatrix4x4>

ModelViewer::ModelViewer(QWidget *parent)
    : QMainWindow(parent)
{
    // Create a new graphics window, and set it as the central widget
    m_pGraphicsWindow = new ViewerGraphicsWindow();
    QWidget* pContainer = QWidget::createWindowContainer(m_pGraphicsWindow);
    setCentralWidget(pContainer);

    // Change the size to something usable
    resize(640, 480);

    // Menu bar
    // -> File menu
    QMenu* pFileMenu = menuBar()->addMenu("File");
    pFileMenu->setObjectName("FileMenu");

    QMenu* pLoadMenu = pFileMenu->addMenu("Load");
    pLoadMenu->setObjectName("LoadMenu");
    pLoadMenu->addAction("Model", [=] {m_pGraphicsWindow->loadModel(); });

    QMenu* pShaderMenu = pLoadMenu->addMenu("Shader");
    pShaderMenu->addAction("Vertex", [=]{m_pGraphicsWindow->loadVertexShader(); });
    pShaderMenu->addAction("Fragment", [=]{m_pGraphicsWindow->loadFragmentShader(); });
    pShaderMenu->addAction("Reload Current Shaders", [=]{m_pGraphicsWindow->reloadCurrentShaders(); });


    // Primitive
    QMenu* pPrimitiveMenu = pLoadMenu->addMenu("Primitive");
    pPrimitiveMenu->setObjectName("PrimitiveMenu");
    pPrimitiveMenu->addAction("Sphere", [=]{m_pGraphicsWindow->addPrimitive("Sphere.obj"); });
    pPrimitiveMenu->addAction("Cube", [=] {m_pGraphicsWindow->addPrimitive("Cube.obj"); });
    pPrimitiveMenu->addAction("Torus", [=]{m_pGraphicsWindow->addPrimitive("Torus.obj"); });
    pPrimitiveMenu->addAction("Cylinder", [=] {m_pGraphicsWindow->addPrimitive("Cylinder.obj"); });
    pPrimitiveMenu->addAction("Diamond", [=] {m_pGraphicsWindow->addPrimitive("diamond.obj"); });
    pPrimitiveMenu->addAction("Tetrahedron", [=] {m_pGraphicsWindow->addPrimitive("Tetrahedron.obj"); });
    pPrimitiveMenu->addAction("Octahedron", [=] {m_pGraphicsWindow->addPrimitive("Octahedron.stl"); });
    pPrimitiveMenu->addAction("Icosahedron", [=] {m_pGraphicsWindow->addPrimitive("Icosahedron.stl"); });
    pPrimitiveMenu->addAction("Dodecahedron", [=] {m_pGraphicsWindow->addPrimitive("Dodecahedron.stl"); });

    QMenu* pSaveMenu = pFileMenu->addMenu("Save");
    pSaveMenu->setObjectName("SaveMenu");
    pSaveMenu->addAction("Model", [=] { /* TODO: m_pGraphicsWindow->saveModel(); */ });
    pSaveMenu->addAction("Shader", [=] { /* TODO: m_pGraphicsWindow->saveShader(); */ });

    pFileMenu->addAction("Screenshot", [=] { /* TODO: m_pGraphicsWindow->screenshot(); */ });
    pFileMenu->addAction("Quit", [=] { /* TODO: m_pGraphicsWindow->exitGracefully(); */ });

    // -> Edit menu
    QMenu* pEditMenu = menuBar()->addMenu("Edit");
    pEditMenu->setObjectName("EditMenu");

    pEditMenu->addAction("Shader File", [=] { m_pGraphicsWindow->openShaderFile(); });
    pEditMenu->addAction("Current Shaders", [=] { m_pGraphicsWindow->editCurrentShaders(); });

    // -> View menu
    QMenu* pViewMenu = menuBar()->addMenu("View");
    pViewMenu->setObjectName("ViewMenu");
    pViewMenu->addAction("Reset", [=] { m_pGraphicsWindow->resetView(); });

    // -> Help menu
    menuBar()->addAction("Help", [=] { /* TODO: m_pGraphicsWindow->displayHelpDoc(); */ }); 
};

ViewerGraphicsWindow* ModelViewer::GetGraphicsWindow() {
    return m_pGraphicsWindow;
}


