#pragma once
#include <QtWidgets>

#include <float.h>
#include "ObjectRepresentation.h"

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
    bool isPolygonInside(QVector<QPoint> polygon);

    //// 3D Object ////
    void debugObject();
    void loadObject(QVector<QVector3D> vertices, QVector<QVector<unsigned int>> polygons);
    void translateObject(QVector3D offset);

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
    void drawPolygon(QVector<QPoint> points) { drawPolygon(points, globalColor, rastAlg); }
    void drawPolygon(QVector<QPoint> points, QColor color, int algType);
    void fillPolygon(QVector<QPoint> points, QColor color);
    void fillTriangle(QVector<QPoint> points, QColor color);

    // 3D Object
    void drawObject() { drawObject(object, QVector3D(0, 0, 0), 0, 0); }
    void drawObject(ThreeDObject object);
    void drawObject(ThreeDObject object, QVector3D camera_position, double zenit, double azimuth, double center_of_projection = 0);
    void transformToViewingCoordinates(ThreeDObject &object, QVector3D camera_position, double zenit, double azimuth);
    void transformToOrtograpicCoordinates(ThreeDObject &object);
    void transformToPerspectiveCoordinates(ThreeDObject &object, double center_of_projection);

    //// Clipping ////

    // Cyrus-Beck
    void clipLine(QPoint start, QPoint end, QPoint &clip_start, QPoint &clip_end);

    // Sutherland-Hodgman
    QVector<QPoint> clipPolygonLeftSide(QVector<QPoint> polygon, int x_min);
    QVector<QPoint> clipPolygon(QVector<QPoint> polygon);

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