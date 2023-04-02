#pragma once
#include <QtWidgets>

#include <float.h>
#include "ObjectRepresentation.h"

struct Camera
{
    QVector3D position;
    double zenit;
    double azimuth;
    double center_of_projection;
};

class ViewerWidget : public QWidget
{
    Q_OBJECT
private:
    QSize areaSize = QSize(0, 0);
    QImage *img = nullptr;
    QPainter *painter = nullptr;
    uchar *data = nullptr;

    QColor globalColor = Qt::blue;
    unsigned char rastAlg = 0;

    // Object
    ThreeDObject object;

    // Camera
    Camera camera;

public:
    ViewerWidget(QSize imgSize, QWidget *parent = Q_NULLPTR);
    ~ViewerWidget();
    void resizeWidget(QSize size);

    void setGlobalColor(QColor color) { globalColor = color; }
    QColor getGlobalColor() { return globalColor; }

    // Image functions
    bool setImage(const QImage &inputImg);
    QImage *getImage() { return img; };
    bool isEmpty();
    bool changeSize(int width, int height);

    void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a = 255);
    void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
    void setPixel(int x, int y, const QColor &color);
    void setPixel(QPoint point, const QColor &color) { setPixel(point.x(), point.y(), color); }
    bool isInside(int x, int y) { return (x >= 10 && y >= 10 && x < img->width() - 10 && y < img->height() - 10) ? true : false; }
    bool isInside(QPoint point) { return isInside(point.x(), point.y()); }
    bool isPolygonInside(std::list<QPoint> polygon);

    //// 3D Object ////
    void debugObject(ThreeDObject &object);
    void loadObject(QVector<QVector3D> vertices, QVector<QVector<unsigned int>> polygons);
    void translateObject(QVector3D offset);

    //// Camera ////
    void setCamera(QVector3D position, double zenit, double azimuth, double center_of_projection)
    {
        camera.position = position;
        camera.zenit = zenit * M_PI / 180;
        camera.azimuth = azimuth * M_PI / 180;
        camera.center_of_projection = center_of_projection;
        clear();
        drawObject();
    }

    //// Drawing ////

    // Line
    void drawLine(QPoint start, QPoint end, QColor color, int algType);

    void DDA(QPoint start, QPoint end, QColor color);
    void DDA_x(QPoint start, QPoint end, QColor color, double m);
    void DDA_y(QPoint start, QPoint end, QColor color, double w);

    void Bresenhamm(QPoint start, QPoint end, QColor color);
    void Bresenhamm_x(QPoint start, QPoint end, QColor color, double m);
    void Bresenhamm_y(QPoint start, QPoint end, QColor color, double m);

    // Polygon
    void drawPolygon(std::list<QPoint> polygon) { drawPolygon(polygon, globalColor, rastAlg); }
    void drawPolygon(std::list<QPoint> polygon, QColor color, int algType);
    void fillPolygon(std::list<QPoint> polygon, QColor color);
    void fillTriangle(std::vector<QPoint> polygon, QColor color);

    // 3D Object
    void drawObject() { drawObject(object, camera); }
    void drawObject(ThreeDObject obj, Camera camera);
    void transformToViewingCoordinates(ThreeDObject &object, Camera camera);
    void transformToOrtograpicCoordinates(ThreeDObject &obj);
    void transformToPerspectiveCoordinates(ThreeDObject &object, double center_of_projection);
    void drawObject(ThreeDObject *object);

    //// Clipping ////

    // Cyrus-Beck
    void clipLine(QPoint start, QPoint end, QPoint &clip_start, QPoint &clip_end);

    // Sutherland-Hodgman
    void clipPolygonLeftSide(std::list<QPoint> &polygon, int x_min);
    void clipPolygon(std::list<QPoint> &polygon);

    // Get/Set functions
    uchar *getData() { return data; }
    void setDataPtr() { data = img->bits(); }
    void setPainter() { painter = new QPainter(img); }

    int getImgWidth() { return img->width(); };
    int getImgHeight() { return img->height(); };

    void delete_objects();
    void clear();

public slots:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
};
