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
 ,_edgeColour(Qt::black)
 ,_colourTheme(ColourTheme::Vibrant)
 ,_vbo(QOpenGLBuffer::VertexBuffer)
 ,_edgeBuffer(QOpenGLBuffer::VertexBuffer)
 ,_idxbo(QOpenGLBuffer::IndexBuffer)
 ,_program(0)
 ,_context(0)
 ,_device(0)
 ,_aspect(4.0/3)
 ,_camSpherical(2, 0, 0) // r, theta, phi
 ,LIGHTINTENSITY(QVector4D(0.7, 0.7, 0.7, 0.0))
 ,AMBIENTLIGHT(QVector4D(0.3, 0.3, 0.3, 1.0))
 ,LIGHTDIRECTION(QVector3D(0.866, 0.5, 0.5))
{
  setSurfaceType(QWindow::OpenGLSurface);
}

GLNetworkWindow::~GLNetworkWindow()
{
  clearModel();
  if (_device)
    delete _device;

  
}

void GLNetworkWindow::setNetworkData(Network::layoutData netData)
{
  _networkData = netData;
  
  clearModel();
  generateModel();
  
  if (_vao.isCreated())
    _vao.destroy();
  
  _vao.create();
  _vao.bind();
  
 
  size_t colourSize = _verticesPerSphere * sizeof(QVector4D); 
  
  
  for (unsigned i = 0; i < layout()->vertexCount(); i++)
  {
    QOpenGLBuffer *colbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    colbo->create();
    colbo->bind();
    colbo->setUsagePattern(QOpenGLBuffer::StreamDraw);
    colbo->allocate(_colours.constData() + i * _verticesPerSphere, colourSize);
    colbo->release();
    _colourBuffers.push_back(colbo);
  }
  
  _vbo.create();
  _vbo.bind();
  _vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
  size_t positionSize = _verticesPerSphere * sizeof(QVector3D);
  _vbo.allocate(_vertices.constData(), positionSize);
  _vbo.release();
  
  _edgeBuffer.create();
  _edgeBuffer.bind();
  _edgeBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
  _edgeBuffer.allocate(_edgeData.constData(), _edgeData.size() * sizeof(QVector3D));
  _edgeBuffer.release();
   
  
  _idxbo.create();
  _idxbo.bind();
  _idxbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
  _idxbo.allocate(_indices.constData(), _indices.size() * sizeof(GLuint));
  
   _vao.release();  
  
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
    "smooth out vec4 col;\n"
    "uniform mat4 modelToCamera;\n"
    "uniform mat4 cameraToClip;\n"
    "uniform mat3 normalModelToCamera;\n"
    "uniform vec3 dirToLight;\n"
    "uniform vec4 lightIntensity;\n"
    "uniform vec4 ambientLight;\n"
    "void main() {\n"
    "   gl_Position = cameraToClip * (modelToCamera * position);\n"
    "   vec3 normal = position.xyz;\n"//vec3(position.x, position.y, position.z);\n"
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
  
    _context->setFormat(format);
    _context->create();
    needsInit = true;
  }
  
  _context->makeCurrent(this);
  
  if (needsInit)
  {
    initializeOpenGLFunctions();
    initializeGL();
  }
  
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
  
  if (_program)
  {
    QMatrix4x4 cameraToClip;
    cameraToClip.perspective(PERSPECTIVEANGLE, _aspect, 0.1, 100.0);
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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  _program->bind();
  
  _program->setUniformValue(_lightIntensityUniform, LIGHTINTENSITY);
  _program->setUniformValue(_ambientLightUniform, AMBIENTLIGHT);
  _program->setUniformValue(_dirToLightUniform, LIGHTDIRECTION);
  
  _vao.bind();
  
  _vbo.bind();
  _program->setAttributeBuffer(_positionAttr, GL_FLOAT, 0, 3);
  _program->enableAttributeArray(_positionAttr);
  _vbo.release();

  const QVector3D & cameraPos = resolveCameraPosition();
  
  QMatrix4x4 worldToCamera;

  double scaleFactor = 1.0/_worldScale;
  worldToCamera.scale(scaleFactor);    

  worldToCamera.lookAt(cameraPos, _target, QVector3D(0, 1, 0));
  
  worldToCamera.translate(_target);
  worldToCamera.rotate(_orientation);
  worldToCamera.translate(-_target);
  
  double minVertRad = _vertRadUnit * 2 / 3;
  
  // TODO write a function to resolve this based on rotation and zoom of current view
  
  for (unsigned i = 0; i < layout()->vertexCount(); i++)
  {
    
    // TODO rotate model according to camera position; Rotate normals as well. Normalize normals?
    // With one copy of sphere positions, won't need normals anymore?
    
    double freq = (double)(model()->index(i, 0).data(NetworkItem::SizeRole).toUInt());
    double radius = qMax(_vertRadUnit * sqrt(freq), minVertRad);
    
    QVector3D position(layout()->vertexCoords3D(i));
    
    // "flip coordinates" rel. to layout
    position.setY(-position.y());
    position.setZ(-position.z());
    
    QVector3D dirToCamera = cameraPos - position;
    
    QMatrix4x4 vertModelToCamera(worldToCamera);
    vertModelToCamera.translate(position);

    // Rotation in (y,z) plane
    // Starts facing down pos y
    double thetaRad = qAtan2(dirToCamera.z(), dirToCamera.y());
    thetaRad += PI; // Turn SOUTH pole to camera
    
    // Rotation in (z,x) plane
    // Starts facing down pos z
    double phiRad =  qAtan2(dirToCamera.x(), dirToCamera.z());
    
    
    vertModelToCamera.rotate(phiRad * 180 / PI, 0, 1, 0);
    vertModelToCamera.rotate(thetaRad * 180. / PI, 1, 0, 0);
    vertModelToCamera.scale(radius); 

    QMatrix3x3 normalModelToCamera;
      for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++)
          normalModelToCamera.data()[j * 3 + k] = vertModelToCamera.constData()[j * 4 + k];
        
    _program->setUniformValue(_modelToCameraUniform, vertModelToCamera);
    _program->setUniformValue(_normalModelToCameraUniform, normalModelToCamera);

    _colourBuffers.at(i)->bind();
    _program->setAttributeBuffer(_colourAttr, GL_FLOAT, 0, 4);
    _program->enableAttributeArray(_colourAttr);
    _colourBuffers.at(i)->release();
    
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
    _program->disableAttributeArray(_colourAttr);
  }
  
  _edgeBuffer.bind();
  size_t edgePositionSize = _edgeData.size() / 2;
  _program->setAttributeBuffer(_positionAttr, GL_FLOAT, 0, 3);
  _program->setAttributeBuffer(_colourAttr, GL_FLOAT, edgePositionSize * sizeof(QVector3D), 3);
  _program->enableAttributeArray(_positionAttr);
  _program->enableAttributeArray(_colourAttr);
  _edgeBuffer.release();


  QMatrix3x3 normalModelToCamera;
    for (int j = 0; j < 3; j++)
      for (int k = 0; k < 3; k++)
        normalModelToCamera.data()[j * 3 + k] = worldToCamera.constData()[j * 4 + k];
  
  _program->setUniformValue(_modelToCameraUniform, worldToCamera);
  _program->setUniformValue(_normalModelToCameraUniform, normalModelToCamera);
  
  glLineWidth(5.0f);
  glDrawArrays(GL_LINES, 0, edgePositionSize);
  
  
  _program->disableAttributeArray(_positionAttr);
  _program->disableAttributeArray(_colourAttr);
  _program->release();
  _vao.release();
   
}

void GLNetworkWindow::clearModel()
{
  
  for (int i = 0; i < _colourBuffers.size(); i++)
  {
    if (_colourBuffers.at(i)->isCreated())
      _colourBuffers.at(i)->destroy();
    
    delete _colourBuffers.at(i);
  }
  
  _colourBuffers.clear();

  if (_vbo.isCreated())
    _vbo.destroy();
  
  if (_edgeBuffer.isCreated())
    _edgeBuffer.destroy();
  
  if (_idxbo.isCreated())
    _idxbo.destroy();
  
  
  _vertices.clear();
  _edgeData.clear();
  _colours.clear();
  _indices.clear();
  
  
  
}

void GLNetworkWindow::generateModel()
{
  unsigned nlat = 50;
  unsigned nlon = 2 * nlat;
  
  QVector<QVector3D> spherePositions;
  QVector<unsigned> sphereIndices;
  QVector<unsigned> lonIndices; // Used to determine which colour to use
  generateSphere(nlat, nlon, spherePositions, sphereIndices, lonIndices);
  _verticesPerSphere = spherePositions.size();

  const QVector3D & layoutSize = layout()->southEast3D();  
  _worldScale = qMax(qMax(layoutSize.x(), layoutSize.y()), layoutSize.z()) / 2.0;
  _target = QVector3D(_worldScale, -_worldScale, -layoutSize.z()/2);
  
  for (unsigned i = 0; i < layout()->vertexCount(); i++)
  {
    _colours << vertexColours(i, lonIndices, nlon);
    //qDebug() << "vertex" << i << layout()->vertexCoords3D(i);
  }
  
  _vertices << spherePositions;
  _indices << sphereIndices;
  
  QVector<QVector3D> edgeColours(layout()->edgeCount() * 2, QVector3D(_edgeColour.redF(), _edgeColour.greenF(), _edgeColour.blueF()));

  for (unsigned i = 0; i < layout()->edgeCount(); i++)
  {
    // "flip coordinates" relative to layout
    QVector3D start(layout()->edgeStart3D(i));
    start.setY(-start.y());
    start.setZ(-start.z());
    
    QVector3D end(layout()->edgeEnd3D(i));
    end.setY(-end.y());
    end.setZ(-end.z());
    
    
    _edgeData << start << end;
  }
  
  _edgeData << edgeColours;
    
}

QVector<QVector4D> GLNetworkWindow::vertexColours(unsigned nodeID, QVector<unsigned> lonByVertex, unsigned nlon)
{

  QList<QVariant> traits = model()->index(nodeID,0).data(NetworkItem::TraitRole).toList();
  
  // no traits info, use default colours
  if (traits.empty()) 
  {
    QVector<QVector4D> defaultColours(lonByVertex.size(), QVector4D(_vertColour.redF(), _vertColour.greenF(), _vertColour.blueF(), 1.0));
    
    return defaultColours;
  }
  
  QList<QColor> colourLookup;
  double freq = (double)(model()->index(nodeID, 0).data(NetworkItem::SizeRole).toUInt());

  for (unsigned i = 0; i < traits.size(); i++)
  {
    unsigned count = traits.at(i).toUInt();
    unsigned colLat = (unsigned)(count/freq * nlon + 0.5); // number of latitude lines this colour gets
    colLat = qMin(colLat, nlon - 1);
    
    for (int j = 0; j < colLat; j++)
      colourLookup << _colourTheme.colour(i);
  }
  
  QColor lastColour = colourLookup.back();
  
  while (colourLookup.size() < nlon)
    colourLookup << lastColour;

  QVector<QVector4D> colourData;

  for (unsigned i = 0; i < lonByVertex.size(); i++)
  {
    const QColor & lonCol = colourLookup.at(lonByVertex.at(i));
    colourData.push_back(QVector4D(lonCol.redF(), lonCol.greenF(), lonCol.blueF(), 1.0));
  }
  
  return colourData;
}


void GLNetworkWindow::generateSphere(unsigned nlat, unsigned nlon, QVector<QVector3D> &vertices, QVector<unsigned> &indices, QVector<unsigned> &longitudes)
{
  // South Pole
  vertices.push_back(QVector3D(0.0, -1.0, 0.0));
  longitudes.push_back(0);
  
  const double offset = PI / nlon / 2;
  
  double theta = 0.0;
  double phi = 0.0;
  float x, y, z;
  double rxz;

  
  for (unsigned i = 1; i < nlat; i++)
  {
    theta = i * PI / nlat - PI / 2;
    y = (float)(qSin(theta));
    rxz = qCos(theta);
    for (unsigned j = 0; j < nlon; j++)
    {
      phi = j * 2 * PI / nlon + offset * (i % 2);
      x = (float)(qCos(phi) * rxz);
      z = (float)(qSin(phi) * rxz);
      vertices.push_back(QVector3D(x,y,z));
      longitudes.push_back(j);

    }
  }
  
  // North Pole
  vertices.push_back(QVector3D(0.0, 1.0, 0.0));
  longitudes.push_back(0);
  

  // Generate indices, South Pole fan first
  for (unsigned i = 0; i < nlon; i++)
  {
    indices.push_back(0);
    indices.push_back(i + 1);
    indices.push_back((i + 1) % nlon + 1);
    
  }
      
  QList<unsigned> sequence;
  for (unsigned i = 0; i < nlat - 2; i++)
  {
    sequence.clear();
    for (unsigned j = 0; j < nlon + 1; j++)
    {
      if (i % 2) // odd row to the south (first)
        sequence << (i * nlon + j % nlon + 1) << (i + 1) * nlon + j % nlon + 1;
      else // even row to the south
        sequence << (i + 1) * nlon + j % nlon + 1 << (i * nlon + j % nlon + 1);
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
  unsigned bottomRow = 1 + (nlat - 2) * nlon;
  for (unsigned i = 0; i < nlon; i++)
  {
    indices.push_back((nlat - 1) * nlon + 1);
    indices.push_back(bottomRow + (i + 1) % nlon);
    indices.push_back(bottomRow + i);
  }
}

QVector3D GLNetworkWindow::resolveCameraPosition() const 
{
  
  qDebug() << "camSpherical:" << _camSpherical;
 
  // Want spherical coordinates of (2, 0, 0) to return:
  //return QVector3D(_worldScale, -_worldScale, 2 * _worldScale);
  double r = _camSpherical.x();
  double thetaRad = _camSpherical.y() * PI / 180.;
  double phiRad = _camSpherical.z() * PI / 180.;
  
  double rxz = r * qCos(thetaRad);
  
  QVector3D dirToCamera(rxz * qCos(phiRad), r * qSin(thetaRad), rxz * qSin(phiRad));
  
  qDebug() << "returning:" << dirToCamera *  _worldScale + _target;
  
  return dirToCamera * _worldScale + _target;
  
}

QQuaternion GLNetworkWindow::lookAtQuat(const QVector3D & eye, const QVector3D & centre, const QVector3D & upVect) const
{
  QVector3D forward = centre - eye;
  forward.normalize();
  
  QVector3D up(upVect);
  up.normalize();
  
  if (qFuzzyCompare(up, forward))
    return QQuaternion();

  if (qFuzzyCompare(up, -forward))
    return QQuaternion(-1, 0, 0, 0);

  QVector3D axis = QVector3D::crossProduct(forward, up);
  axis.normalize();
  
  double angle = qAcos(QVector3D::dotProduct(forward, up));
  
  return QQuaternion::fromAxisAndAngle(axis, (float)angle);
}

void GLNetworkWindow::moveCamera(QPoint start, QPoint end)
{
  
  if (! _networkData.model)
    return;
  
  QPointF displacement(end - start);
  displacement.setY(-displacement.y()/height());
  displacement.setX(displacement.x()/height());
  
  displacement *= PERSPECTIVEANGLE;  
  
  double theta = _camSpherical.y() + displacement.y();
  double phi = _camSpherical.z() + displacement.x();
  
  if (theta < -85.0)  theta = -85.0;
  else if (theta > 85.0)  theta = 85.0;
  
  _camSpherical.setY(theta);
  _camSpherical.setZ(phi);

  if (isExposed())
    drawGL();
  
  else
    renderLater();  
}

void GLNetworkWindow::moveWorld(QPoint start, QPoint end)
{
  if (! _networkData.model)
    return;
  
  QVector3D displacement(end - start);
  
  // scale: positive y up, and y between -1 and +1; scale x by the same value
  // height() corresponds to the perspective angle, use this as a rotation
  displacement.setY(-displacement.y()/height());
  displacement.setX(displacement.x()/height());
    
  double rotationDegrees = PERSPECTIVEANGLE * displacement.length();
  
  // axis is displacement rotated 90 degrees 
  QVector3D axis(-displacement.y(), displacement.x(), 0);  
  axis.normalize();

  QQuaternion offset = QQuaternion::fromAxisAndAngle(axis, (float)rotationDegrees); 
  
  const QVector3D & cameraPos = resolveCameraPosition();
  QQuaternion viewQuat = lookAtQuat(cameraPos, _target, QVector3D(0, 1, 0));
  
  QQuaternion conjViewQuat = viewQuat.conjugate();
  QQuaternion worldQuat = conjViewQuat * offset * viewQuat;
  _orientation = worldQuat * _orientation;
  _orientation.normalize();
  
  //qDebug() << "moving camera, axis:" << axis << "angle:" << rotationDegrees;
  if (isExposed())
    drawGL();
  
  else
    renderLater();
}

void GLNetworkWindow::mousePressEvent(QMouseEvent *event)
{
  _lastPos = event->pos();
  qDebug() << "pressed mouse at" << event->pos();
}

void GLNetworkWindow::mouseMoveEvent(QMouseEvent *event)
{  
  if (event->buttons() & Qt::LeftButton)
  {
    moveWorld(_lastPos, event->pos());
  }
    
  else if (event->buttons() & Qt::RightButton)
  {
    moveCamera(_lastPos, event->pos());
  }
  
  _lastPos = event->pos();
  
}

void GLNetworkWindow::exposeEvent(QExposeEvent *event)
{
  Q_UNUSED(event);
  
  if (isExposed())
  {   
    const qreal retinaScale = devicePixelRatio();
    resizeGL(width() * retinaScale, height() * retinaScale);
    drawGL();
  }
}


bool GLNetworkWindow::event(QEvent *event)
{
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
