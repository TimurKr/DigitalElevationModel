#include "ViewerWidget.h"

ViewerWidget::ViewerWidget(QSize imgSize, QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);
    if (imgSize != QSize(0, 0))
    {
        img = new QImage(imgSize, QImage::Format_ARGB32);
        img->fill(Qt::white);
        resizeWidget(img->size());
        setPainter();
        setDataPtr();
        z_index = new double[width() * height()];
        clear();
    }
}
ViewerWidget::~ViewerWidget()
{
    delete painter;
    delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
    this->resize(size);
    this->setMinimumSize(size);
    this->setMaximumSize(size);
}

// Image functions
bool ViewerWidget::setImage(const QImage &inputImg)
{
    if (img != nullptr)
    {
        delete painter;
        delete img;
    }
    img = new QImage(inputImg);
    if (!img)
    {
        return false;
    }
    resizeWidget(img->size());
    setPainter();
    setDataPtr();
    update();

    return true;
}
bool ViewerWidget::isEmpty()
{
    if (img == nullptr)
    {
        return true;
    }

    if (img->size() == QSize(0, 0))
    {
        return true;
    }
    return false;
}

bool ViewerWidget::isPolygonInside(std::list<QVector3D> polygon)
{
    for (std::list<QVector3D>::iterator i = polygon.begin(); i != polygon.end(); ++i)
    {
        if (isInside(*i))
        {
            return true;
        }
    }
    return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
    QSize newSize(width, height);

    if (newSize != QSize(0, 0))
    {
        if (img != nullptr)
        {
            delete painter;
            delete img;
        }

        img = new QImage(newSize, QImage::Format_ARGB32);
        if (!img)
        {
            return false;
        }
        img->fill(Qt::white);
        resizeWidget(img->size());
        setPainter();
        setDataPtr();
        update();
    }

    return true;
}

// void ViewerWidget::setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
// {
//     r = r > 255 ? 255 : (r < 0 ? 0 : r);
//     g = g > 255 ? 255 : (g < 0 ? 0 : g);
//     b = b > 255 ? 255 : (b < 0 ? 0 : b);
//     a = a > 255 ? 255 : (a < 0 ? 0 : a);
//     size_t startbyte = y * img->bytesPerLine() + x * 4;
//     data[startbyte] = b;
//     data[startbyte + 1] = g;
//     data[startbyte + 2] = r;
//     data[startbyte + 3] = a;
// }
// void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
// {
//     valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
//     valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
//     valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
//     valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);
//     size_t startbyte = y * img->bytesPerLine() + x * 4;
//     data[startbyte] = static_cast<uchar>(255 * valB);
//     data[startbyte + 1] = static_cast<uchar>(255 * valG);
//     data[startbyte + 2] = static_cast<uchar>(255 * valR);
//     data[startbyte + 3] = static_cast<uchar>(255 * valA);
// }
void ViewerWidget::setPixel(int x, int y, float z, const QColor &color)
{
    if (z_index[y * width() + x] < z)
    {
        return;
    }
    if (color.isValid())
    {
        size_t startbyte = y * img->bytesPerLine() + x * 4;

        data[startbyte] = color.blue();
        data[startbyte + 1] = color.green();
        data[startbyte + 2] = color.red();
        data[startbyte + 3] = color.alpha();
    }
}

//// OBJECT ////

void ViewerWidget::debugObject(ThreeDObject &object)
{
    for (std::list<Face>::iterator it = object.faces.begin(); it != object.faces.end(); ++it)
    {
        Face *face_ptr = &(*it);
        Edge *edge = face_ptr->edge;
        qDebug() << "Face" << face_ptr << ":";
        while (true)
        {
            // qDebug() << "\tEdge" << edge << ":";
            qDebug() << "\tVrchol start: (" << edge->origin->x << ", " << edge->origin->y << ", " << edge->origin->z << ")";
            // qDebug() << "\tVrchol start: (" << edge->next->origin->x << ", " << edge->next->origin->y << ", " << edge->next->origin->z << ")";
            // qDebug() << "\tFace ptr: " << edge->face;
            // qDebug() << "\tEdge next: " << edge->next;
            // qDebug() << "\tEdge prev: " << edge->prev;
            // qDebug() << "\tEdge pair: " << edge->pair;
            // qDebug() << "";
            edge = edge->next;
            if (edge == face_ptr->edge)
                break;
        }
        qDebug() << "---------------------";
    }
}
void ViewerWidget::loadObject(QVector<QVector3D> vertices, QVector<QVector<unsigned int>> polygons)
{
    std::vector<QColor> colors = {Qt::red, Qt::red, Qt::green, Qt::green, Qt::blue, Qt::blue, Qt::black, Qt::black, Qt::lightGray, Qt::lightGray, Qt::black, Qt::black};

    object.clear();

    // Load vertices
    for (int i = 0; i < vertices.size(); i++)
    {
        Vertex vertex;
        vertex.x = vertices[i].x();
        vertex.y = vertices[i].y();
        vertex.z = vertices[i].z();
        vertex.color = colors[i % colors.size()];
        object.vertices.push_back(vertex);
    }

    // Load edges and faces
    for (int i = 0; i < polygons.size(); i++)
    {
        object.faces.push_back(Face());
        Face *face_ptr = &object.faces.back();
        face_ptr->color = colors[i % colors.size()];

        Edge *prev_edge = nullptr;
        for (int j = 0; j < polygons[i].size(); j++)
        {
            object.edges.push_back(Edge());
            Edge *edge = &object.edges.back();

            // Origin
            edge->origin = &(*std::next(object.vertices.begin(), polygons[i][j]));
            edge->origin->edges.push_back(edge);

            // Face
            edge->face = face_ptr;
            if (j == 0)
                face_ptr->edge = edge;

            // Linking
            if (prev_edge != nullptr)
            {
                prev_edge->next = edge;
                edge->prev = prev_edge;
            }

            // Pairing
            Vertex *end = &(*std::next(object.vertices.begin(), polygons[i][(j + 1) % polygons[i].size()]));
            for (int k = 0; k < end->edges.size(); k++)
            {
                if (end->edges[k]->next->origin == edge->origin)
                {
                    edge->pair = end->edges[k];
                    end->edges[k]->pair = edge;
                }
            }

            prev_edge = edge;
        }
        // Linking last edge
        prev_edge->next = face_ptr->edge;
        face_ptr->edge->prev = prev_edge;
    }

    // Translate object to the middle of the coordinate system
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();
    float maxZ = std::numeric_limits<float>::min();

    for (Vertex vertex : object.vertices)
    {
        if (vertex.x < minX)
            minX = vertex.x;
        if (vertex.x > maxX)
            maxX = vertex.x;
        if (vertex.y < minY)
            minY = vertex.y;
        if (vertex.y > maxY)
            maxY = vertex.y;
        if (vertex.z < minZ)
            minZ = vertex.z;
        if (vertex.z > maxZ)
            maxZ = vertex.z;
    }

    float x = (maxX + minX) / 2;
    float y = (maxY + minY) / 2;
    float z = (maxZ + minZ) / 2;

    translateObject(QVector3D(-x, -y, -z));
    drawObject();
}
void ViewerWidget::translateObject(QVector3D offset)
{
    object.translate(offset);
}

//// CAMERA ////

void ViewerWidget::rotateCamera(QPoint mouse_pos)
{
    double dx = 0.03 * (mouse_pos.x() - last_mouse_pos.x());
    double dy = 0.03 * (mouse_pos.y() - last_mouse_pos.y());

    camera.azimuth = std::fmod((camera.azimuth + dx), 2 * M_PI);

    camera.zenit += dy;

    if (camera.zenit > M_PI / 2)
        camera.zenit = M_PI / 2;
    if (camera.zenit < -M_PI / 2)
        camera.zenit = -M_PI / 2;

    last_mouse_pos = mouse_pos;
    qDebug() << "Azimuth: " << camera.azimuth;
    qDebug() << "Zenit: " << camera.zenit;
    qDebug() << "";
    clear();
    drawObject();
}

//// DRAWING ////

// Draw Line functions
void ViewerWidget::drawLine(QVector3D start, QVector3D end, QColor color)
{
    if (start == end)
        return;
    if (!isInside(start) && !isInside(end))
        return;
    if (!isInside(start) || !isInside(end))
    {
        QVector3D tmp_start = start;
        QVector3D tmp_end = end;
        clipLine(tmp_start, tmp_end, start, end);
    }
    if (rasterizationAlgorithm == RasterizationAlgorithm::DDA)
    {
        Dda(start, end, color);
    }
    else if (rasterizationAlgorithm == RasterizationAlgorithm::BRESENHAMM)
    {
        Bresenhamm(start, end, color);
    }

    update();
}

void ViewerWidget::Dda(QVector3D start, QVector3D end, QColor color)
{
    float d_x = end.x() - start.x();
    float d_y = end.y() - start.y();
    double m = DBL_MAX;

    if (d_x != 0)
        m = d_y / d_x;

    if (-1 < m && m < 1)
    {
        if (start.x() < end.x())
        {
            Dda_x(start, end, color, m);
        }
        else
        {
            Dda_x(end, start, color, m);
        }
    }
    else
    {
        if (start.y() < end.y())
        {
            Dda_y(start, end, color, 1 / m);
        }
        else
        {
            Dda_y(end, start, color, 1 / m);
        }
    }
}
void ViewerWidget::Dda_x(QVector3D start, QVector3D end, QColor color, double m)
{

    setPixel(start, color);

    int x;
    double y = start.y();

    for (x = start.x(); x < end.x(); x++)
    {
        y += m;
        setPixel(x, (int)(y + 0.5), start.z() + (end.z() - start.z()) * (x - start.x()) / (end.x() - start.x()), color);
    }
}
void ViewerWidget::Dda_y(QVector3D start, QVector3D end, QColor color, double w)
{

    setPixel(start, color);

    int y;
    double x = start.x();

    for (y = start.y(); y < end.y(); y++)
    {
        x += w;
        setPixel((int)(x + 0.5), y, start.z() + (end.z() - start.z()) * (y - start.y()) / (end.y() - start.y()), color);
    }
}

void ViewerWidget::Bresenhamm(QVector3D start, QVector3D end, QColor color)
{
    float d_x = end.x() - start.x();
    float d_y = end.y() - start.y();
    double m = DBL_MAX;

    if (d_x != 0)
        m = d_y / d_x;

    if (-1 < m && m < 1)
    {
        if (start.x() < end.x())
        {
            Bresenhamm_x(start, end, color, m);
        }
        else
        {
            Bresenhamm_x(end, start, color, m);
        }
    }
    else
    {
        if (start.y() < end.y())
        {
            Bresenhamm_y(start, end, color, m);
        }
        else
        {
            Bresenhamm_y(end, start, color, m);
        }
    }
}
void ViewerWidget::Bresenhamm_x(QVector3D start, QVector3D end, QColor color, double m)
{
    if (m > 0)
    {
        int k1 = 2 * (end.y() - start.y());
        int k2 = k1 - 2 * (end.x() - start.x());
        int p = k1 - (end.x() - start.x());

        int x = start.x() + 0.5;
        int y = start.y() + 0.5;

        setPixel(start, color);

        for (; x < end.x(); x++)
        {
            if (p > 0)
            {
                y++;
                p += k2;
            }
            else
            {
                p += k1;
            }
            setPixel(x, y, start.z() + (end.z() - start.z()) * (x - start.x()) / (end.x() - start.x()), color);
        }
    }
    else
    {
        int k1 = 2 * (end.y() - start.y());
        int k2 = k1 + 2 * (end.x() - start.x());
        int p = k1 + (end.x() - start.x());

        int x = start.x() + 0.5;
        ;
        int y = start.y() + 0.5;
        ;

        setPixel(start, color);

        for (; x < end.x(); x++)
        {
            if (p < 0)
            {
                y--;
                p += k2;
            }
            else
            {
                p += k1;
            }
            setPixel(x, y, start.z() + (end.z() - start.z()) * (x - start.x()) / (end.x() - start.x()), color);
        }
    }
}
void ViewerWidget::Bresenhamm_y(QVector3D start, QVector3D end, QColor color, double m)
{
    if (m > 0)
    {
        int k1 = 2 * (end.x() - start.x());
        int k2 = k1 - 2 * (end.y() - start.y());
        int p = k1 - (end.y() - start.y());

        int x = start.x() + 0.5;
        ;
        int y = start.y() + 0.5;
        ;

        setPixel(start, color);

        for (; y < end.y(); y++)
        {
            if (p > 0)
            {
                x++;
                p += k2;
            }
            else
            {
                p += k1;
            }
            setPixel(x, y, start.z() + (end.z() - start.z()) * (y - start.y()) / (end.y() - start.y()), color);
        }
    }
    else
    {
        int k1 = 2 * (end.x() - start.x());
        int k2 = k1 + 2 * (end.y() - start.y());
        int p = k1 + (end.y() - start.y());

        int x = start.x() + 0.5;
        ;
        int y = start.y() + 0.5;
        ;

        setPixel(start, color);

        for (; y < end.y(); y++)
        {
            if (p < 0)
            {
                x--;
                p += k2;
            }
            else
            {
                p += k1;
            }
            setPixel(x, y, start.z() + (end.z() - start.z()) * (y - start.y()) / (end.y() - start.y()), color);
        }
    }
}

// Draw polygon functions
void ViewerWidget::drawPolygon(std::list<QVector3D> polygon, QColor color)
{
    if (!isPolygonInside(polygon))
        return;

    if (polygon.size() < 2)
        return;

    if (polygon.size() == 2)
    {
        QVector3D start, end;
        clipLine(polygon.front(), polygon.back(), start, end);
        drawLine(start, end, color);
        return;
    }
    clipPolygon(polygon);

    if (polygon.size() < 1)
        return;

    for (std::list<QVector3D>::iterator it = polygon.begin(); it != --polygon.end();)
    {
        drawLine(*it, *(++it), color);
    }
    drawLine(polygon.back(), polygon.front(), color);
}
void ViewerWidget::fillPolygon(std::list<QVector3D> polygon, QColor color)
{
    if (polygon.size() < 3)
        return;
    if (polygon.size() == 3)
    {
        std::vector<QVector3D> triangle;
        for (std::list<QVector3D>::iterator it = polygon.begin(); it != polygon.end(); it++)
        {
            triangle.push_back(*it);
        }
        fillTriangle(triangle, color);
        return;
    }

    struct Edge
    {
        QVector3D start;
        QVector3D end;
        int dy;
        double x;
        double dx;
        double z;
        double dz;
    };

    polygon.push_back(polygon.front());

    // Define sides
    QVector<Edge> edges;
    for (std::list<QVector3D>::iterator it = polygon.begin(); it != --polygon.end(); it++)
    {
        QVector3D start = *it;
        QVector3D end = *(++it);

        // Remove horizontal lines
        if (start.y() == end.y())
            continue;

        // Orientate the edge
        if (start.y() > end.y())
        {
            QVector3D temp = start;
            start = end;
            end = temp;
        }

        Edge edge;
        edge.start = start;
        edge.end = end + QVector3D(0, -1, 0);
        edge.dy = (int)edge.end.y() - start.y() + 0.5;
        edge.x = start.x();
        edge.dx = (end.x() - start.x()) / (end.y() - start.y());
        edge.z = start.z();
        edge.dz = (end.z() - start.z()) / (end.y() - start.y());

        edges.push_back(edge);
    }

    // Sort by y
    std::sort(edges.begin(), edges.end(), [](Edge e1, Edge e2)
              { return e1.start.y() < e2.start.y(); });

    int y_min = (int)edges[0].start.y() + 0.5;
    int y_max = y_min;
    for (int i = 0; i < edges.size(); i++)
    {
        if ((int)edges[i].end.y() + 0.5 > y_max)
        {
            y_max = (int)edges[i].end.y() + 0.5;
        }
    }

    // Add edges to the edges_table
    QVector<QList<Edge>> edges_table;
    edges_table.resize(y_max - y_min + 1);
    for (int i = 0; i < edges.size(); i++)
    {
        edges_table[edges[i].start.y() - y_min].push_back(edges[i]);
    }

    // Scanline, one line at a time from the top to the bottom
    QVector<Edge> active_edges;
    double y = y_min;
    for (int i = 0; i < edges_table.size(); i++)
    {
        // Add new edges to the active edges
        if (edges_table[i].size() != 0)
        {
            for (int j = 0; j < edges_table[i].size(); j++)
            {
                active_edges.push_back(edges_table[i][j]);
            }
        }
        // Sort active edges by x
        std::sort(active_edges.begin(), active_edges.end(), [](Edge e1, Edge e2)
                  { return e1.x < e2.x; });

        // Draw lines between active adges
        for (int j = 0; j < active_edges.size(); j += 2)
        {
            if (active_edges[j].x != active_edges[j + 1].x)
            {
                drawLine(
                    QVector3D(active_edges[j].x, y, active_edges[j].z),
                    QVector3D(active_edges[j + 1].x, y, active_edges[j + 1].z),
                    color);
            }
        }

        // Update active edges
        for (int j = 0; j < active_edges.size(); j++)
        {
            // Remove edges that are done
            if (active_edges[j].dy == 0)
            {
                active_edges.remove(j);
                j--;
            }
            // Update edges that are not done
            else
            {
                active_edges[j].x += active_edges[j].dx;
                active_edges[j].z += active_edges[j].dz;
                active_edges[j].dy--;
            }
        }

        y++;
    }
}
void ViewerWidget::fillTriangle(std::vector<QVector3D> polygon, QColor color)
{
    if (polygon.size() != 3)
        throw std::runtime_error("fillTriangle called with polygon.size() != 3");

    std::sort(polygon.begin(), polygon.end(),
              [](QVector3D a, QVector3D b)
              {
                  if ((int)a.y() + 0.5 == (int)b.y() + 0.5)
                      return a.x() < b.x();
                  return a.y() < b.y();
              });

    struct Edge
    {
        QVector3D start, end;
        double dx, dz;
    };

    Edge e1;
    Edge e2;

    if ((int)polygon[0].y() + 0.5 == (int)polygon[1].y() + 0.5)
    {
        // Filling the bottom flat triangle
        e1.start = polygon[0];
        e1.end = polygon[2];
        e1.dx = (double)(polygon[2].x() - polygon[0].x()) / (double)(polygon[2].y() - polygon[0].y());
        e1.dz = (double)(polygon[2].z() - polygon[0].z()) / (double)(polygon[2].y() - polygon[0].y());

        e2.start = polygon[1];
        e2.end = polygon[2];
        e2.dx = (double)(polygon[2].x() - polygon[1].x()) / (double)(polygon[2].y() - polygon[1].y());
        e2.dz = (double)(polygon[2].z() - polygon[1].z()) / (double)(polygon[2].y() - polygon[1].y());
    }
    else if ((int)polygon[1].y() + 0.5 == (int)polygon[2].y() + 0.5)
    {
        // Filling the top flat triangle
        e1.start = polygon[0];
        e1.end = polygon[1];
        e1.dx = (double)(polygon[1].x() - polygon[0].x()) / (double)(polygon[1].y() - polygon[0].y());
        e1.dz = (double)(polygon[1].z() - polygon[0].z()) / (double)(polygon[1].y() - polygon[0].y());

        e2.start = polygon[0];
        e2.end = polygon[2];
        e2.dx = (double)(polygon[2].x() - polygon[0].x()) / (double)(polygon[2].y() - polygon[0].y());
        e2.dz = (double)(polygon[2].z() - polygon[0].z()) / (double)(polygon[2].y() - polygon[0].y());
    }
    else
    {
        // Splitting the triangle into two
        double dx = (double)(polygon[2].y() - polygon[0].y()) / (double)(polygon[2].x() - polygon[0].x());
        double dz = (double)(polygon[2].y() - polygon[0].y()) / (double)(polygon[2].z() - polygon[0].z());
        QVector3D split_point(
            (polygon[1].y() - polygon[0].y()) / dx + polygon[0].x(),
            polygon[1].y(),
            (polygon[1].y() - polygon[0].y()) / dz + polygon[0].z());

        if (polygon[1].x() < split_point.x())
        {
            fillTriangle({polygon[0], polygon[1], split_point}, color);
            fillTriangle({polygon[1], split_point, polygon[2]}, color);
        }
        else
        {
            fillTriangle({polygon[0], split_point, polygon[1]}, color);
            fillTriangle({split_point, polygon[1], polygon[2]}, color);
        }
        return;
    }

    double x1 = e1.start.x();
    double x2 = e2.start.x();
    double z1 = e1.start.z();
    double z2 = e2.start.z();
    for (int y = e1.start.y(); y < e1.end.y(); y++)
    {
        if (x1 != x2)
        {
            drawLine(
                QVector3D(x1, y, z1),
                QVector3D(x2, y, z2),
                color);
        }
        x1 += e1.dx;
        x2 += e2.dx;
        z1 += e1.dz;
        z2 += e2.dz;
    }
}

// 3D Object
void ViewerWidget::drawObject(ThreeDObject obj, Camera camera, ColoringType coloring)
{
    if (obj.vertices.size() == 0)
        return;
    transformToViewingCoordinates(obj, camera);
    if (camera.center_of_projection == 0)
        transformToOrtograpicCoordinates(obj);
    else
        transformToPerspectiveCoordinates(obj, camera.center_of_projection);

    obj.translate(QVector3D(width() / 2, height() / 2, 0));

    drawObject(&obj, coloring);
}
void ViewerWidget::transformToViewingCoordinates(ThreeDObject &object, Camera camera)
{
    object.translate(-camera.position);

    QVector3D n(sin(camera.zenit - M_PI / 2) * sin(camera.azimuth), sin(camera.zenit - M_PI / 2) * cos(camera.azimuth), cos(camera.zenit - M_PI / 2));
    QVector3D u(-cos(camera.zenit - M_PI / 2) * sin(camera.azimuth), -cos(camera.zenit - M_PI / 2) * cos(camera.azimuth), sin(camera.zenit - M_PI / 2));
    QVector3D v = QVector3D::crossProduct(n, u);

    for (std::list<Vertex>::iterator it = object.vertices.begin(); it != object.vertices.end(); it++)
    {
        QVector3D vertex(it->x, it->y, it->z);
        it->x = QVector3D::dotProduct(vertex, v);
        it->y = QVector3D::dotProduct(vertex, u);
        it->z = QVector3D::dotProduct(vertex, n);
    }
}
void ViewerWidget::transformToOrtograpicCoordinates(ThreeDObject &obj)
{
    // for (std::list<Vertex>::iterator it = obj.vertices.begin(); it != obj.vertices.end(); it++)
    // {
    //     // it->z = 0;
    //     // qDebug() << "Vertex transformed to ortographic coordinates: " << it->x << " " << it->y << " " << it->z << "\n";
    // }
}
void ViewerWidget::transformToPerspectiveCoordinates(ThreeDObject &object, double center_of_projection)
{
    for (std::list<Vertex>::iterator it = object.vertices.begin(); it != object.vertices.end(); it++)
    {
        if (it->z == center_of_projection)
        {
            continue;
        }
        it->x = it->x * center_of_projection / (center_of_projection - it->z);
        it->y = it->y * center_of_projection / (center_of_projection - it->z);
    }
}
void ViewerWidget::drawObject(ThreeDObject *object, ColoringType coloring)
{
    int color_num = 5;
    for (Face face : object->faces)
    {
        std::list<QVector3D> polygon;
        Edge *e = face.edge;
        polygon.push_back(e->origin->toVector3D());
        for (e = e->next; e != face.edge; e = e->next)
        {
            polygon.push_back(e->origin->toVector3D());
        }

        if (coloring == WIREFRAME || color_num == 0)
        {
            drawPolygon(polygon);
        }
        else if (coloring == SIDE)
        {
            fillPolygon(polygon, face.color);
            color_num--;
        }
        else if (coloring == VERTEX)
        {
            qDebug() << "Vertex coloring\n";
        }
    }
}

//// Clipping ////

// Cyrus-Beck
void ViewerWidget::clipLine(QVector3D start, QVector3D end, QVector3D &clip_start, QVector3D &clip_end)
{
    if (!isInside(start) && !isInside(end))
    {
        clip_start = QVector3D();
        clip_end = QVector3D();
        return;
    }
    if (isInside(start) && isInside(end))
    {
        clip_start = start;
        clip_end = end;
        return;
    }

    double tl = 0, tu = 1;
    QVector3D d = end - start;

    QVector<QPoint> E = {QPoint(10, 10), QPoint(10, height() - 10), QPoint(width() - 10, height() - 10), QPoint(width() - 10, 10)};

    for (int i = 0; i < 4; i++)
    {
        QPoint n = E[(i + 1) % 4] - E[i];
        n = QPoint(n.y(), -n.x());
        QPoint w = start.toPoint() - E[i];
        double dn = QPoint::dotProduct(n, d.toPoint());
        double wn = QPoint::dotProduct(n, w);
        if (dn != 0)
        {
            double t = -wn / dn;
            if (dn > 0 && t <= 1)
            {
                if (tl < t)
                    tl = t;
            }
            else if (dn < 0 && 0 <= t)
            {
                if (tu > t)
                    tu = t;
            }
        }
    }

    if (tl == 0 && tu == 1)
    {
        clip_start = start;
        clip_end = end;
    }
    else if (tl < tu)
    {
        clip_start = start + d * tl;
        clip_end = start + d * tu;
    }
    // printf("clip_start: %d %d", clip_start.x(), clip_start.y());
    // printf("clip_end: %d %d", clip_end.x(), clip_end.y());
}

// Sutherland-Hodgman
void ViewerWidget::clipPolygonLeftSide(std::list<QVector3D> &polygon, int x_min)
{

    if (polygon.size() == 0)
        return;

    std::list<QVector3D> result;

    QVector3D last_vertex = polygon.back();

    for (QVector3D vertex : polygon)
    {
        if (vertex.x() >= x_min)
        {
            if (last_vertex.x() >= x_min)
            {
                result.push_back(vertex);
            }
            else
            {
                QVector3D new_vertex(
                    x_min,
                    last_vertex.y() + (x_min - last_vertex.x()) * (vertex.y() - last_vertex.y()) / (vertex.x() - last_vertex.x()),
                    last_vertex.z() + (x_min - last_vertex.x()) * (vertex.z() - last_vertex.z()) / (vertex.x() - last_vertex.x()));
                result.push_back(new_vertex);
                result.push_back(vertex);
            }
        }
        else
        {
            if (last_vertex.x() >= x_min)
            {
                QVector3D new_vertex(
                    x_min,
                    last_vertex.y() + (x_min - last_vertex.x()) * (vertex.y() - last_vertex.y()) / (vertex.x() - last_vertex.x()),
                    last_vertex.z() + (x_min - last_vertex.x()) * (vertex.z() - last_vertex.z()) / (vertex.x() - last_vertex.x()));
                result.push_back(new_vertex);
            }
        }
        last_vertex = vertex;
    }
    polygon = result;
    return;
}
void ViewerWidget::clipPolygon(std::list<QVector3D> &polygon)
{
    if (!isPolygonInside(polygon))
    {
        polygon.clear();
        return;
    }

    std::vector<QPoint> E = {QPoint(10, 10), QPoint(width() - 10, 10), QPoint(width() - 10, height() - 10), QPoint(10, height() - 10)};

    for (int i = 0; i < 4; i++)
    {
        if (polygon.size() == 0)
            return;

        clipPolygonLeftSide(polygon, E[i].x());

        for (std::list<QVector3D>::iterator it = polygon.begin(); it != polygon.end(); it++)
        {
            *it = QVector3D(it->y(), -it->x(), it->z());
        }
        for (int i = 0; i < E.size(); i++)
        {
            E[i] = QPoint(E[i].y(), -E[i].x());
        }
    }
}

void ViewerWidget::delete_objects()
{
    update();
}

void ViewerWidget::clear()
{
    img->fill(Qt::white);
    for (int i = 0; i < width() * height(); i++)
        z_index[i] = std::numeric_limits<double>::max();
    update();
}

// Slots
void ViewerWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect area = event->rect();
    painter.drawImage(area, *img, area);
}
