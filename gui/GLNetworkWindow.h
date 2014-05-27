#ifndef GLNETWORKWINDOW_H_
#define GLNETWORKWINDOW_H_

#include <QColor>
#include <QEvent>
#include <QExposeEvent>
#include <QMouseEvent>

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <QPainter>
#include <QQuaternion>
#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QWindow>

#include "ColourTheme.h"
#include "NetworkItem.h"
#include "NetworkLayout.h"
#include "NetworkModel.h"


namespace Network
{
  typedef struct { const NetworkModel *model; const NetworkLayout *layout;} layoutData; 
};


class GLNetworkWindow: public QWindow, protected QOpenGLFunctions
{
  Q_OBJECT
public: 
  GLNetworkWindow(QWindow *parent = 0);
  ~GLNetworkWindow();
  
  void initializeGL();
  void moveCamera(QPoint, QPoint);
  void moveWorld(QPoint, QPoint);
  
  virtual void render(QPainter *);
  virtual void render();
  
  void setNetworkData(Network::layoutData);
  
  
public slots:
  void drawGL();
  void resizeGL(int, int);
  void renderLater();
  
protected:
  
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseMoveEvent(QMouseEvent *);  
  virtual bool event(QEvent *); 
  virtual void exposeEvent(QExposeEvent *); 
  const NetworkLayout * layout() { return _networkData.layout; };
  const NetworkModel * model() { return _networkData.model; };
  
private:
  
  void clearModel();
  void generateModel();
  void generateSphere(unsigned, unsigned, QVector<QVector3D>&, QVector<unsigned> &, QVector<unsigned> &);
  QVector<QVector4D> vertexColours(unsigned, QVector<unsigned>, unsigned);
  
  QVector3D resolveCameraPosition() const;
  QQuaternion lookAtQuat(const QVector3D &, const QVector3D &, const QVector3D &) const;
  
  bool _updatePending;
  double _vertRadUnit;
  
  Network::layoutData _networkData;
  
  QColor _backgroundColour;
  QColor _vertColour;
  QColor _edgeColour;
  ColourTheme _colourTheme;
  QVector<QVector3D> _vertices;
  QVector<QVector3D> _edgeData;
  QVector<QVector4D> _colours;
  QVector<unsigned> _indices;
  unsigned _verticesPerSphere;
  
  QOpenGLVertexArrayObject _vao;
  QOpenGLBuffer _vbo;
  QOpenGLBuffer _edgeBuffer;
  QList<QOpenGLBuffer*> _colourBuffers;
  QOpenGLBuffer _idxbo;
  QOpenGLShaderProgram *_program;
  size_t _matOffset;
  
  GLuint _positionAttr;
  GLuint _colourAttr;
  
  GLuint _modelToCameraUniform;
  GLuint _normalModelToCameraUniform;
  GLuint _cameraToClipUniform;
  GLuint _dirToLightUniform;
  GLuint _lightIntensityUniform;
  GLuint _ambientLightUniform;
    
  QOpenGLContext *_context;
  QOpenGLPaintDevice *_device;
  
  double _aspect;
  double _worldScale;
  QQuaternion _orientation;
  QVector3D _camSpherical;
  QVector3D _target;
  QPoint _lastPos;

  
  const QVector4D LIGHTINTENSITY;
  const QVector4D AMBIENTLIGHT;
  const QVector3D LIGHTDIRECTION;  
  
  static const double PI = 3.141592653589793238462643383279502884197169399375105821;
  static const int PERSPECTIVEANGLE = 45;

};


#endif
