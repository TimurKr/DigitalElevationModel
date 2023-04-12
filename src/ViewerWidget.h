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
public:
    enum ColoringType
    {
        WIREFRAME,
        SIDE,
        VERTEX
    };
    enum RasterizationAlgorithm
    {
        DDA,
        BRESENHAMM
    };

private:
    QSize areaSize = QSize(0, 0);
    QImage *img = nullptr;
    QPainter *painter = nullptr;
    uchar *data = nullptr;
    double *z_index = nullptr;

    QColor globalColor = Qt::blue;
    RasterizationAlgorithm rasterizationAlgorithm = DDA;
    ColoringType coloringType = WIREFRAME;

    // Object
    ThreeDObject object;

    // Camera
    Camera camera;
    bool isCameraRotating = false;
    QPoint last_mouse_pos;

public:
    ViewerWidget(QSize imgSize, QWidget *parent = Q_NULLPTR);
    ~ViewerWidget();
    void resizeWidget(QSize size);

    void setGlobalColor(QColor color) { globalColor = color; }
    QColor getGlobalColor() { return globalColor; }
    void setColoringType(ColoringType type)
    {
        clear();
        coloringType = type;
        drawObject();
    }
    ColoringType getColoringType() { return coloringType; }
    void setRasterizationAlgorithm(RasterizationAlgorithm algorithm) { rasterizationAlgorithm = algorithm; }
    RasterizationAlgorithm getRasterizationAlgorithm() { return rasterizationAlgorithm; }

    // Image functions
    bool setImage(const QImage &inputImg);
    QImage *getImage() { return img; };
    bool isEmpty();
    bool changeSize(int width, int height);

    // void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a = 255);
    // void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
    void setPixel(int x, int y, float z, const QColor &color);
    void setPixel(QVector3D point, const QColor &color) { setPixel(point.x() + 0.5, point.y() + 0.5, point.z(), color); }
    void setPixel(Vertex vertex) { setPixel(vertex.x, vertex.y, vertex.z, vertex.color); }
    bool isInside(int x, int y) { return (x >= 10 && y >= 10 && x < img->width() - 10 && y < img->height() - 10) ? true : false; }
    bool isInside(QPoint point) { return isInside(point.x(), point.y()); }
    bool isInside(Vertex vertex) { return isInside(vertex.x, vertex.y); }
    bool isPolygonInside(std::list<Vertex> polygon);

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
    void setIsCameraRotating(bool isRotating) { isCameraRotating = isRotating; }
    bool getIsCameraRotating() { return isCameraRotating; }
    void setLastMousePos(QPoint pos) { last_mouse_pos = pos; }
    void rotateCamera(QPoint mouse_pos);

    //// Drawing ////

    // Line
    void drawLine(Vertex start, Vertex end);

    void Dda(Vertex start, Vertex end);
    void Dda_x(Vertex start, Vertex end, double m);
    void Dda_y(Vertex start, Vertex end, double w);

    void Bresenhamm(Vertex start, Vertex end);
    void Bresenhamm_x(Vertex start, Vertex end, double m);
    void Bresenhamm_y(Vertex start, Vertex end, double m);

    // Polygon
    // void drawPolygon(std::list<Vertex> polygon) { drawPolygon(polygon, globalColor); }
    void drawPolygon(std::list<Vertex> polygon, QColor color);
    void drawPolygon(std::list<Vertex> polygon);
    void fillPolygon(std::list<Vertex> polygon);
    void fillTriangle(std::vector<Vertex> polygon);

    // 3D Object
    void drawObject() { drawObject(object, camera, coloringType); }
    void drawObject(ThreeDObject obj, Camera camera, ColoringType coloring);
    void transformToViewingCoordinates(ThreeDObject &object, Camera camera);
    void transformToOrtograpicCoordinates(ThreeDObject &obj);
    void transformToPerspectiveCoordinates(ThreeDObject &object, double center_of_projection);
    void drawObject(ThreeDObject *object, ColoringType coloring);

    //// Clipping ////

    // Cyrus-Beck
    void clipLine(Vertex start, Vertex end, Vertex &clip_start, Vertex &clip_end);

    // Sutherland-Hodgman
    void clipPolygonLeftSide(std::list<Vertex> &polygon, int x_min);
    void clipPolygon(std::list<Vertex> &polygon);

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
