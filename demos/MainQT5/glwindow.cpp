/*
  Copyright � 2012 The KoRE Project

  This file is part of KoRE.

  KoRE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

  KoRE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with KoRE.  If not, see <http://www.gnu.org/licenses/>.
*/

/************************************************************************/
/* \author Andreas Weinmann                                             */
/************************************************************************/

#include "glwindow.h"
#include <gl/glew.h>

#include <QGuiApplication>
#include <QKeyEvent>
#include <QOpenGLContext>
#include <QTimer>
#include <string>

#include "KoRE/Shader.h"
#include "KoRE/Components/Mesh.h"
#include "KoRE/Operations/RenderMesh.h"
#include "KoRE/ResourceManager.h"
#include "KoRE/RenderManager.h"

GLWindow::GLWindow(QScreen* screen) 
    : QWindow(screen)
{
    setSurfaceType(OpenGLSurface);
   
    QSurfaceFormat sformat;
    sformat.setDepthBufferSize(24);
    sformat.setMajorVersion(4);
    sformat.setMinorVersion(3);
    //sformat.setSamples(4);        MSAA
    sformat.setProfile(QSurfaceFormat::CoreProfile);

    resize(800,600);
    setFormat(sformat);
    create();

    _context = new QOpenGLContext;
    _context->setFormat(sformat);
    _context->create();   

    initializeGL();
    connect(this,SIGNAL(widthChanged(int)), this, SLOT(resizeGL()));
    connect(this,SIGNAL(heightChanged(int)), this, SLOT(resizeGL()));
    resizeGL();
   
    
    // load resources
    kore::SceneNodePtr pTestScene =
    kore::ResourceManager::getInstance()->
        loadScene("../../assets/meshes/Test_LightCamera.dae");

    // load shader
    kore::ShaderPtr pSimpleShader(new kore::Shader);
    pSimpleShader->loadShader( "../../assets/shader/simple.vp",
        GL_VERTEX_SHADER);
    pSimpleShader->loadShader( "../../assets/shader/simple.fp", 
        GL_FRAGMENT_SHADER);
    pSimpleShader->initShader();

    QTimer* timer = new QTimer(this);
    connect( timer, SIGNAL( timeout() ), this, SLOT( updateScene() ) );
    timer->start();
}

GLWindow::~GLWindow()
{

}

void GLWindow::initializeGL()
{
    _context->makeCurrent(this);  
   
    glewInit();
    
    glClearColor(1,1,1,1);
    kore::Log::getInstance()->write(                                            
        "Render Device: %s\n",
        reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    kore::Log::getInstance()->write(
        "Vendor: %s\n",
        reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    kore::Log::getInstance()->write(
        "OpenGL version: %s\n",
        reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    kore::Log::getInstance()->write(
        "GLSL version: %s\n",
        reinterpret_cast<const char*>
        (glGetString(GL_SHADING_LANGUAGE_VERSION)));
}

void GLWindow::resizeGL()
{
    _context->makeCurrent(this);
    kore::RenderManager::getInstance()->setRenderResolution(
        glm::ivec2(width(),height()));
   
    //kore::Log::getInstance()->write(
    //   "resolution: w: %i h: %i \n",width(),height());        
    //resize scene(width(),height());
}

void GLWindow::paintGL()
{
    _context->makeCurrent(this);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    kore::RenderManager::getInstance()->renderFrame();

    _context->swapBuffers(this);
}

void GLWindow::updateScene()
{
    //update scene
    
    //glClearColor(rand()/(float)RAND_MAX,
    //             rand()/(float)RAND_MAX,
    //             rand()/(float)RAND_MAX,1);
    paintGL();
}

void GLWindow::keyPressEvent(QKeyEvent * evnt)
{
    if (evnt->key() == Qt::Key_Escape) QGuiApplication::quit();
}
