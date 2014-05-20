#include "GLNetworkWindow.h"
#include "NetworkLayout.h"

#include <QCoreApplication>
#include <QMatrix4x4>
// #include <QScreen>

#include <qmath.h>

#include <QDebug>


GLNetworkWindow::GLNetworkWindow(QWindow *parent)
 : QWindow(parent)
 ,_updatePending(false)
 , _vertRadUnit(NetworkItem::VERTRAD)
 ,_networkData({.model=0,.layout=0})
 ,_backgroundColour(246, 255, 213)
 ,_vertColour(127, 127, 127) // 50% black
 ,_colourTheme(ColourTheme::Vibrant)
 ,_vbo(QOpenGLBuffer::VertexBuffer)
 //,_colbo(QOpenGLBuffer::VertexBuffer)
 ,_idxbo(QOpenGLBuffer::IndexBuffer)
 //,_matbo(QOpenGLBuffer::VertexBuffer)
 //,_normmatbo(QOpenGLBuffer::VertexBuffer)
 ,_program(0)
 ,_context(0)
 ,_device(0)
 , _func(0)
 ,_aspect(4.0/3)
 ,LIGHTINTENSITY(QVector4D(0.8, 0.8, 0.8, 0.8))
 ,AMBIENTLIGHT(QVector4D(0.2, 0.2, 0.2, 0.2))
 ,LIGHTDIRECTION(QVector3D(0.866, 0.5, 0.0))
 //,_initialised(false)
{
  setSurfaceType(QWindow::OpenGLSurface);
}

GLNetworkWindow::~GLNetworkWindow()
{
  if (_device)
    delete _device;
}

void GLNetworkWindow::setNetworkData(Network::layoutData netData)
{
  _networkData = netData;
  
  clearModel();
  generateModel();
  
  if (_vbo.isCreated())
    _vbo.destroy();
  
  //if (_colbo.isCreated())
  //  _colbo.destroy();
  
  if (_idxbo.isCreated())
    _idxbo.destroy();
  
  //if (_matbo.isCreated())
  //  _matbo.destroy();

  //if (_normmatbo.isCreated())
  //  _normmatbo.destroy();

  
  if (_vao.isCreated())
    _vao.destroy();
  
  _vao.create();
  _vao.bind();
  
  _vbo.create();
  _vbo.bind();
  _vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
  
  size_t positionSize = _vertices.size() * sizeof(QVector3D);
  size_t colourSize = _colours.size() * sizeof(QVector3D); // TODO: add transparency: make this 4D
  /*_vbo.allocate(positionSize * 2); // positions, colours
  _vbo.write(0, _vertices.constData(), positionSize);*/
  _vbo.allocate(_vertices.constData(), positionSize + colourSize);
  _vbo.write(positionSize, _colours.constData(), colourSize);
  //_vbo.write(positionSize * 2, _normals.constData(), positionSize);
  
  
  // TODO write a separate shader for network edges. They don't need to reflect light the same way as the nodes
  glVertexAttribPointer(_positionAttr, 3, GL_FLOAT, GL_FALSE, 0, 0);
  //glVertexAttribPointer(_colourAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)positionSize);
  glEnableVertexAttribArray(_positionAttr);
  //glEnableVertexAttribArray(_colourAttr);
  
  
    
  
  _idxbo.create();
  _idxbo.bind();
  _idxbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
  _idxbo.allocate(_indices.constData(), _indices.size() * sizeof(GLuint));
  
   _vao.release();  
  
  // Check this! Need to make sure new network is drawn when new data is set
  if (isExposed())
  {
    const qreal retinaScale = devicePixelRatio();
    resizeGL(width() * retinaScale, height() * retinaScale);
    drawGL();
  }
}


// TODO write setupShaders()
void GLNetworkWindow::initializeGL()
{

  const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);  
  qDebug() << "GLSL version:" << (const char*)glslVersion;
  QString vertShaderSrcStr("#version 330\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec4 colour;\n"
    //"layout(location = 1) in mat4 modelToCamera;\n" // actually 4 locations (columns)
    //"layout(location = 5) in mat3 normalModelToCamera;\n"
    "smooth out vec4 col;\n"
    "uniform mat4 modelToCamera;\n"
    "uniform mat4 cameraToClip;\n"
    "uniform mat3 normalModelToCamera;\n"
    "uniform vec3 dirToLight;\n"
    "uniform vec4 lightIntensity;\n"
    "uniform vec4 ambientLight;\n"
    "void main() {\n"
    "   gl_Position = cameraToClip * (modelToCamera * position);\n"
    "   vec3 normal = vec3(position.x, position.y, position.z);\n"
    //"   vec4 colour = vec4(1,0,0,1);\n"
    "   vec3 normCamSpace = normalize(normalModelToCamera * normal);\n"
    "   float cosAngIncidence = dot(normCamSpace, dirToLight);\n"
    "   cosAngIncidence = clamp(cosAngIncidence, 0, 1);\n"
    "   col = lightIntensity * colour * cosAngIncidence + colour * ambientLight;\n"
    "}\n");
  
  QString fragShaderSrcStr("#version 330\n"
    "smooth in vec4 col;\n"
    "void main() {\n"
    "   gl_FragColor = col;\n"
    "}\n");
  
  
  _program = new QOpenGLShaderProgram(this);
  _program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertShaderSrcStr);
  _program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragShaderSrcStr);
  _program->link();
  
  _program->bind();
  
  _positionAttr = _program->attributeLocation("position");
  _colourAttr = _program->attributeLocation("colour");
  //_modelToCameraAttr = _program->attributeLocation("modelToCamera");
  //_normalModelToCameraAttr = _program->attributeLocation("normalModelToCamera");
  
  //_normalAttr = _program->attributeLocation("normal");
 
  _modelToCameraUniform = _program->uniformLocation("modelToCamera");
  _dirToLightUniform = _program->uniformLocation("dirToLight");
  _lightIntensityUniform = _program->uniformLocation("lightIntensity");
  _ambientLightUniform = _program->uniformLocation("ambientLight");
  _normalModelToCameraUniform = _program->uniformLocation("normalModelToCamera");
  _cameraToClipUniform = _program->uniformLocation("cameraToClip");
  
  _program->release();
  
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  
  const qreal retinaScale = devicePixelRatio();
  resizeGL(width() * retinaScale, height() * retinaScale);

}

void GLNetworkWindow::renderLater()
{
  if (!_updatePending)
  {
    _updatePending = true;
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
  }
}

void GLNetworkWindow::drawGL()
{
  if (! isExposed())  return;
  
  bool needsInit = false;
  if (! _context)
  {
    _context = new QOpenGLContext(this);
    QSurfaceFormat format(requestedFormat());
    format.setVersion(3,3);
    format.setDepthBufferSize(24);
  
    //setFormat(format);

    _context->setFormat(format);
    _context->create();
    needsInit = true;
  }
  
  _context->makeCurrent(this);
  
  if (needsInit)
  {
    _func = _context->versionFunctions<QOpenGLFunctions_3_3_Core>();
    if (_func)
      _func->initializeOpenGLFunctions();
    else
    {
       qWarning() << "Could not obtain required OpenGL context version";
      exit(1);
    }
    initializeOpenGLFunctions();
    initializeGL();
  }
  
   //const qreal retinaScale = devicePixelRatio();
   //glViewport(0, 0, width() * retinaScale, height() * retinaScale);
   //resizeGL(width() * retinaScale, height() * retinaScale);
   glClearColor(_backgroundColour.redF(), _backgroundColour.greenF(), _backgroundColour.blueF(), 1.0f);
   glClearDepth(1.0f);
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   
   if (_networkData.model)
     render();
  
  _context->swapBuffers(this);
}

void GLNetworkWindow::resizeGL(int w, int h)
{
  _aspect = ((float)w)/h;
  //_vertRadUnit = NetworkItem::VERTRAD / h;
  
  
  if (_program)
  {
    QMatrix4x4 cameraToClip;
    cameraToClip.perspective(45, _aspect, 0.1, 100.0);
    _program->bind();
    _program->setUniformValue(_cameraToClipUniform, cameraToClip);
    _program->release();
  }

  glViewport(0, 0,  w, h);

}

void GLNetworkWindow::render()
{
  if (! _device)
    _device = new QOpenGLPaintDevice;
  
  _device->setSize(size());
  QPainter painter(_device);
  render(&painter);
}

void GLNetworkWindow::render(QPainter *painter)
{
  
  glEnable(GL_DEPTH_TEST);

  _program->bind();
  _vao.bind();

  QMatrix4x4 modelToCamera;
  modelToCamera.translate(-1, -1, -2);
  double scaleFactor = 1.0/_worldScale;
  modelToCamera.scale(scaleFactor);  
  
  _program->setUniformValue(_lightIntensityUniform, QVector4D(0.8, 0.8, 0.8, 0.8));
  _program->setUniformValue(_ambientLightUniform, QVector4D(0.2, 0.2, 0.2, 0.2));
  _program->setUniformValue(_dirToLightUniform, QVector3D(0.866, 0.5, 0.0));
  
  double minVertRad = _vertRadUnit * 2 / 3;
  
  size_t colourOffset = _vertices.size() * sizeof(QVector3D);
  size_t colourSize = _vertices.size() * sizeof(QVector3D);
  GLfloat *tmp = new GLfloat[_vertices.size() * 3];
  
  for (unsigned i = 0; i < layout()->vertexCount(); i++)
  {
    
    // TODO rotate model according to camera position; Rotate normals as well. Normalize normals?
    // With one copy of sphere positions, won't need normals anymore?
    
    double freq = (double)(model()->index(i, 0).data(NetworkItem::SizeRole).toUInt());
    double radius = qMax(_vertRadUnit * sqrt(freq), minVertRad);
    
    QVector3D position(layout()->vertexCoords3D(i));
    position.setZ(-position.z()); 
    
    QMatrix4x4 vertModelToCamera(modelToCamera);
    vertModelToCamera.translate(position);
    vertModelToCamera.rotate(90.0f, 1, 0, 0);
    vertModelToCamera.scale(radius);    

    QMatrix3x3 normalModelToCamera;
      for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++)
          normalModelToCamera.data()[j * 3 + k] = vertModelToCamera.constData()[j * 4 + k];
 
    _program->setUniformValue(_modelToCameraUniform, vertModelToCamera);
    _program->setUniformValue(_normalModelToCameraUniform, normalModelToCamera);
    //_func->glDrawElementsBaseVertex(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0, i * _verticesPerSphere);
    glVertexAttribPointer(_colourAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)(colourOffset + i * colourSize));
    
    _vbo.read(colourOffset + i * colourSize, tmp, colourSize);
    qDebug() << "colour data:";
    for (unsigned j = 0; j < _vertices.size() * 3; j+=3)
      qDebug() << tmp[j] << tmp[j + 1] << tmp[j + 2];
    
    glEnableVertexAttribArray(_colourAttr);
    _func->glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(_colourAttr);

  }
  
  delete [] tmp;


  //_func->glDrawElementsInstanced(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0, layout()->vertexCount());
  
  
  
  _program->release();
  _vao.release();
   
}

void GLNetworkWindow::clearModel()
{
  _vertices.clear();
  //_normals.clear();
  //_colours.clear();
  _indices.clear();
}

// TODO maybe do this within drawing code. It only has to be done once, when the model is loaded, but maybe faster to just combine sphere-to-model transformations with model-to-camera?
void GLNetworkWindow::generateModel()
{
  unsigned nlong = 50;
  unsigned nlat = 2 * nlong;
  
  QVector<QVector3D> spherePositions;
  QVector<unsigned> sphereIndices;
  QVector<unsigned> latIndices; // Used to determine which colour to use
  generateSphere(nlong, nlat, spherePositions, sphereIndices, latIndices);
  _verticesPerSphere = spherePositions.size();

  // normals should be the same for all nodes, only affected by rotation
  //QMatrix4x4 normalMat;
  // turn North Pole forward
  //normalMat.rotate(90.0f, 1, 0, 0);
  //QVector<QVector3D> sphereNormals;
  //sphereNormals << spherePositions;
  //for (unsigned i = 0; i < spherePositions.size(); i++)
  //  sphereNormals.push_back(normalMat * spherePositions.at(i));
  
  const QVector3D & layoutSize = layout()->southEast3D();  
  _worldScale = qMax(qMax(layoutSize.x(), layoutSize.y()), layoutSize.z()) / 2.0;
  //double minVertRad = _vertRadUnit * 2 / 3;
  
  for (unsigned i = 0; i < layout()->vertexCount(); i++)
  {        
    _colours << vertexColours(i, latIndices, nlat);
    //_normals << sphereNormals;
    /*unsigned offset = (unsigned)_vertices.size();
    
    double freq = (double)(model()->index(i, 0).data(NetworkItem::SizeRole).toUInt());
    double radius = _vertRadUnit * sqrt(freq);//model()->index(i, 0).data(NetworkItem::SizeRole).toUInt());
    radius = qMax(radius, minVertRad);
    
    QVector3D position(layout()->vertexCoords3D(i));
    position.setZ(-position.z());
    
    QMatrix4x4 matrix;
    
    matrix.translate(position);
    matrix.rotate(90.0f, 1, 0, 0);
    matrix.scale(radius);
    

    for (unsigned j = 0; j < spherePositions.size(); j++)
      _vertices.push_back(matrix * spherePositions.at(j));
    
    _vertices << spherePositions;
    
    //for (unsigned j = 0; j < sphereIndices.size(); j++)
    //  _indices.push_back(sphereIndices.at(j) + offset);
    */
  }
  
  _vertices << spherePositions;
  _indices << sphereIndices;
  
  // add edges
  /*for (unsigned i = 0; i < layout()->edgeCount(); i++)
  {
    
  }*/
}

QVector<QVector3D> GLNetworkWindow::vertexColours(unsigned nodeID, QVector<unsigned> latByVertex, unsigned nlat)
{

  QList<QVariant> traits = model()->index(nodeID,0).data(NetworkItem::TraitRole).toList();
  
  // no traits info, use default colours
  if (traits.empty()) 
  {
    QVector<QVector3D> defaultColours(latByVertex.size(), QVector3D(_vertColour.redF(), _vertColour.greenF(), _vertColour.blueF()));
    
    return defaultColours;
  }
  
  QList<QColor> colourLookup;//(nlat, _colourTheme.colour(traits.back().toUInt()));
  double freq = (double)(model()->index(nodeID, 0).data(NetworkItem::SizeRole).toUInt());

  for (unsigned i = 0; i < traits.size(); i++)
  {
    unsigned count = traits.at(i).toUInt();
    unsigned colLat = (unsigned)(count/freq * nlat + 0.5); // number of latitude lines this colour gets
    colLat = qMin(colLat, nlat - 1);
    
    for (int j = 0; j < colLat; j++)
      colourLookup << _colourTheme.colour(i);
  }
  
  QColor lastColour = colourLookup.back();
  
  while (colourLookup.size() < nlat)
    colourLookup << lastColour;

  QVector<QVector3D> colourData;
  
  for (unsigned i = 0; i < latByVertex.size(); i++)
  {
    const QColor & latCol = colourLookup.at(latByVertex.at(i));
    colourData.push_back(QVector3D(latCol.redF(), latCol.greenF(), latCol.blueF()));
  }
  
  return colourData;
}


void GLNetworkWindow::generateSphere(unsigned nlong, unsigned nlat, QVector<QVector3D> &vertices, QVector<unsigned> &indices, QVector<unsigned> &latitudes)
{
  // South Pole
  vertices.push_back(QVector3D(0.0, -1.0, 0.0));
  latitudes.push_back(0);
  //_colours.push_back(QVector3D(1,1,1));
  
  const double pi = 3.141592653589793238462643383279502884197169399375105821;
  const double offset = pi / nlat / 2;
  
  double theta = 0.0;
  double phi = 0.0;
  float x, y, z;
  double rxz;
  
  //QList<QColor> basicColours;
  //basicColours << Qt::red << Qt::green << Qt::blue << Qt::white;
  
  for (unsigned i = 1; i < nlong; i++)
  {
    theta = i * pi / nlong - pi / 2;
    y = (float)(qSin(theta));
    rxz = qCos(theta);
    for (unsigned j = 0; j < nlat; j++)
    {
      phi = j * 2 * pi / nlat + offset * (i % 2);
      x = (float)(qCos(phi) * rxz);
      z = (float)(qSin(phi) * rxz);
      vertices.push_back(QVector3D(x,y,z));
      latitudes.push_back(j);
      //unsigned colourIdx = (unsigned)(j * basicColours.size() / nlat + 0.5);
      //QColor theColour = basicColours.at(colourIdx);
      //_colours.push_back(QVector3D(theColour.redF(), theColour.greenF(), theColour.blueF()));
    }
  }
  
  // North Pole
  vertices.push_back(QVector3D(0.0, 1.0, 0.0));
  latitudes.push_back(0);
  //colours.push_back(QVector3D(1,1,1));
  

  // Generate indices, South Pole fan first
  for (unsigned i = 0; i < nlat; i++)
  {
    indices.push_back(0);
    indices.push_back(i + 1);
    indices.push_back((i + 1) % nlat + 1);
    
  }
      
  QList<unsigned> sequence;
  for (unsigned i = 0; i < nlong - 2; i++)
  {
    sequence.clear();
    for (unsigned j = 0; j < nlat + 1; j++)
    {
      if (i % 2) // odd row to the south (first)
        sequence << (i * nlat + j % nlat + 1) << (i + 1) * nlat + j % nlat + 1;
      else // even row to the south
        sequence << (i + 1) * nlat + j % nlat + 1 << (i * nlat + j % nlat + 1);
    }
    
    
    for (unsigned j = 0; j < sequence.size() - 2; j++)
    {
      indices.push_back(sequence.at(j)); 
      // Maintain CCW order of vertices
      if ((i % 2 && j % 2) || (! (i % 2) && ! (j % 2)))
      {
        indices.push_back(sequence.at(j + 2)); 
        indices.push_back(sequence.at(j + 1));  
      }
      else
      {
        indices.push_back(sequence.at(j + 1));
        indices.push_back(sequence.at(j + 2));
      }
    }
  }
  
  // North Pole fan last
  unsigned bottomRow = 1 + (nlong - 2) * nlat;
  for (unsigned i = 0; i < nlat; i++)
  {
    indices.push_back((nlong - 1) * nlat + 1);
    indices.push_back(bottomRow + (i + 1) % nlat);
    indices.push_back(bottomRow + i);
  }
}

void GLNetworkWindow::exposeEvent(QExposeEvent *event)
{
  Q_UNUSED(event);
  
  //qDebug() << "got an expose event.";

  if (isExposed())
  {   
    const qreal retinaScale = devicePixelRatio();
    resizeGL(width() * retinaScale, height() * retinaScale);
    drawGL();
  }
}


bool GLNetworkWindow::event(QEvent *event)
{
  //qDebug() << "got an event: " << event->type()  << "exposed?" << isExposed();
  switch (event->type()) 
  {
  case QEvent::UpdateRequest:
  {
    _updatePending = false;
    const qreal retinaScale = devicePixelRatio();
    resizeGL(width() * retinaScale, height() * retinaScale);
    drawGL();
    return true;
  }
  default:
    return QWindow::event(event);
  }
}
