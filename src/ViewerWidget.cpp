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

bool ViewerWidget::isPolygonInside(std::list<Vertex> polygon)
{
    for (std::list<Vertex>::iterator i = polygon.begin(); i != polygon.end(); ++i)
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
    double current_z = z_index[y * width() + x];
    if (z_index[y * width() + x] > z)
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
        z_index[y * width() + x] = z;
    }
}

void ViewerWidget::setGlobalColor(QColor color)
{
    globalColor = color;
    object.recolor(color);
    clear();
    drawObject();
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
    object.clear();

    // Load vertices
    for (int i = 0; i < vertices.size(); i++)
    {
        Vertex vertex;
        vertex.x = vertices[i].x();
        vertex.y = vertices[i].y();
        vertex.z = vertices[i].z();
        vertex.color = globalColor;
        object.vertices.push_back(vertex);
    }

    // Load edges and faces
    for (int i = 0; i < polygons.size(); i++)
    {
        object.faces.push_back(Face());
        Face *face_ptr = &object.faces.back();
        face_ptr->color = globalColor;

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

    //// Tranform the object for nicer viewing ////
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

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

    // Scale the object to fit the screen nicely
    double scaleX = 0.5 * std::fabs(width() / (maxX - minX + std::numeric_limits<float>::min()));
    double scaleY = 0.5 * std::fabs(height() / (maxY - minY + std::numeric_limits<float>::min()));

    // Translate object to the middle of the coordinate system
    float x = (maxX + minX) / 2;
    float y = (maxY + minY) / 2;
    float z = (maxZ + minZ) / 2;

    translateObject(QVector3D(-x, -y, -z));
    object.scale(scaleX < scaleY ? scaleX : scaleY);
    drawObject();
}
void ViewerWidget::translateObject(QVector3D offset)
{
    object.translate(offset);
}

//// CAMERA ////

void ViewerWidget::rotateCamera(QPointF mouse_pos)
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
    clear();
    drawObject();
}

//// LIGHTING ////
void ViewerWidget::setLightIntensity(int intensity)
{
    if (intensity < 0)
        intensity = 0;
    if (intensity > 255)
        intensity = 255;
    lightSource.intensity = intensity;
    redraw();
}

//// DRAWING ////

// Draw Line functions
void ViewerWidget::drawLine(Vertex start, Vertex end)
{
    if (start.toVector3D() == end.toVector3D())
        return;
    if (!isInside(start) && !isInside(end))
        return;
    if (!isInside(start) || !isInside(end))
    {
        Vertex tmp_start = start;
        Vertex tmp_end = end;
        clipLine(tmp_start, tmp_end, start, end);
    }
    if (rasterizationAlgorithm == RasterizationAlgorithm::DDA)
    {
        Dda(start, end);
    }
    else if (rasterizationAlgorithm == RasterizationAlgorithm::BRESENHAMM)
    {
        Bresenhamm(start, end);
    }

    update();
}

void ViewerWidget::Dda(Vertex start, Vertex end)
{
    float d_x = end.x - start.x;
    float d_y = end.y - start.y;
    double m = DBL_MAX;

    if (d_x != 0)
        m = d_y / d_x;

    if (-1 < m && m < 1)
    {
        if (start.x < end.x)
        {
            Dda_x(start, end, m);
        }
        else
        {
            Dda_x(end, start, m);
        }
    }
    else
    {
        if (start.y < end.y)
        {
            Dda_y(start, end, 1 / m);
        }
        else
        {
            Dda_y(end, start, 1 / m);
        }
    }
}
void ViewerWidget::Dda_x(Vertex start, Vertex end, double m)
{

    setPixel(start);

    int x;
    double y = start.y;

    for (x = start.x; x < end.x; x++)
    {
        y += m;
        Vertex p = start.interpolate(end, (x - start.x) / (end.x - start.x));
        p.x = x;
        p.y = (int)(y + 0.5);
        setPixel(p);
    }
}
void ViewerWidget::Dda_y(Vertex start, Vertex end, double w)
{

    setPixel(start);

    int y;
    double x = start.x;

    for (y = start.y; y < end.y; y++)
    {
        x += w;
        double t = (y - start.y) / (end.y - start.y);
        Vertex p = start.interpolate(end, (y - start.y) / (end.y - start.y));
        p.x = (int)(x + 0.5);
        p.y = y;
        setPixel(p);
    }
}

void ViewerWidget::Bresenhamm(Vertex start, Vertex end)
{
    float d_x = end.x - start.x;
    float d_y = end.y - start.y;
    double m = DBL_MAX;

    if (d_x != 0)
        m = d_y / d_x;

    if (-1 < m && m < 1)
    {
        if (start.x < end.x)
        {
            Bresenhamm_x(start, end, m);
        }
        else
        {
            Bresenhamm_x(end, start, m);
        }
    }
    else
    {
        if (start.y < end.y)
        {
            Bresenhamm_y(start, end, m);
        }
        else
        {
            Bresenhamm_y(end, start, m);
        }
    }
}
void ViewerWidget::Bresenhamm_x(Vertex start, Vertex end, double m)
{
    if (m > 0)
    {
        int k1 = 2 * (end.y - start.y);
        int k2 = k1 - 2 * (end.x - start.x);
        int p = k1 - (end.x - start.x);

        int x = start.x + 0.5;
        int y = start.y + 0.5;

        setPixel(start);

        for (; x < end.x; x++)
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
            Vertex p = start.interpolate(end, (x - start.x) / (end.x - start.x));
            p.x = x;
            p.y = y;
            setPixel(p);
        }
    }
    else
    {
        int k1 = 2 * (end.y - start.y);
        int k2 = k1 + 2 * (end.x - start.x);
        int p = k1 + (end.x - start.x);

        int x = start.x + 0.5;
        ;
        int y = start.y + 0.5;
        ;

        setPixel(start);

        for (; x < end.x; x++)
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
            Vertex p = start.interpolate(end, (x - start.x) / (end.x - start.x));
            p.x = x;
            p.y = y;
            setPixel(p);
        }
    }
}
void ViewerWidget::Bresenhamm_y(Vertex start, Vertex end, double m)
{
    if (m > 0)
    {
        int k1 = 2 * (end.x - start.x);
        int k2 = k1 - 2 * (end.y - start.y);
        int p = k1 - (end.y - start.y);

        int x = start.x + 0.5;
        ;
        int y = start.y + 0.5;
        ;

        setPixel(start);

        for (; y < end.y; y++)
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
            Vertex p = start.interpolate(end, (y - start.y) / (end.y - start.y));
            p.x = x;
            p.y = y;
            setPixel(p);
        }
    }
    else
    {
        int k1 = 2 * (end.x - start.x);
        int k2 = k1 + 2 * (end.y - start.y);
        int p = k1 + (end.y - start.y);

        int x = start.x + 0.5;
        ;
        int y = start.y + 0.5;
        ;

        setPixel(start);

        for (; y < end.y; y++)
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
            Vertex p = start.interpolate(end, (y - start.y) / (end.y - start.y));
            p.x = x;
            p.y = y;
            setPixel(p);
        }
    }
}

// Draw polygon functions
void ViewerWidget::drawPolygon(std::list<Vertex> polygon, QColor color)
{
    for (Vertex &vertex : polygon)
    {
        vertex.color = color;
    }
    drawPolygon(polygon);
}
void ViewerWidget::drawPolygon(std::list<Vertex> polygon)
{
    if (!isPolygonInside(polygon))
        return;

    if (polygon.size() < 2)
        return;

    if (polygon.size() == 2)
    {
        Vertex start, end;
        clipLine(polygon.front(), polygon.back(), start, end);
        drawLine(start, end);
        return;
    }
    clipPolygon(polygon);

    if (polygon.size() < 1)
        return;

    for (std::list<Vertex>::iterator it = polygon.begin(); it != --polygon.end();)
    {
        drawLine(*it, *(++it));
    }
    drawLine(polygon.back(), polygon.front());
}
void ViewerWidget::fillPolygon(std::list<Vertex> polygon)
{
    if (polygon.size() < 3)
        return;
    if (polygon.size() == 3)
    {
        std::vector<Vertex> triangle;
        for (std::list<Vertex>::iterator it = polygon.begin(); it != polygon.end(); it++)
        {
            triangle.push_back(*it);
        }
        fillTriangle(triangle);
        return;
    }

    struct Edge
    {
        Vertex start;
        Vertex end;
        int dy;
        double x;
        double dx;
    };

    polygon.push_back(polygon.front());

    // Define sides
    QVector<Edge> edges;
    for (std::list<Vertex>::iterator it = polygon.begin(); it != --polygon.end(); it++)
    {
        Vertex start = *it;
        Vertex end = *(++it);

        // Remove horizontal lines
        if ((int)start.y + 0.5 == (int)end.y + 0.5)
            continue;

        // Orientate the edge
        if (start.y > end.y)
        {
            Vertex temp = start;
            start = end;
            end = temp;
        }

        Vertex new_end = end;
        new_end.y -= 1;

        Edge edge;
        edge.start = start;
        edge.end = new_end;
        edge.dy = (int)edge.end.y - start.y + 0.5;
        edge.x = start.x;
        edge.dx = (end.x - start.x) / (end.y - start.y);

        edges.push_back(edge);
    }

    // Sort by y
    std::sort(edges.begin(), edges.end(), [](Edge e1, Edge e2)
              { return e1.start.y < e2.start.y; });

    int y_min = (int)edges[0].start.y + 0.5;
    int y_max = y_min;
    for (int i = 0; i < edges.size(); i++)
    {
        if ((int)edges[i].end.y + 0.5 > y_max)
        {
            y_max = (int)edges[i].end.y + 0.5;
        }
    }

    // Add edges to the edges_table
    QVector<QList<Edge>> edges_table;
    edges_table.resize(y_max - y_min + 1);
    for (int i = 0; i < edges.size(); i++)
    {
        edges_table[edges[i].start.y - y_min].push_back(edges[i]);
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
                Vertex start = active_edges[j].start.interpolate(active_edges[j].end, (y - active_edges[j].start.y) / (active_edges[j].end.y - active_edges[j].start.y + 1));
                start.x = active_edges[j].x;
                start.y = y;

                Vertex end = active_edges[j + 1].start.interpolate(active_edges[j + 1].end, (y - active_edges[j + 1].start.y) / (active_edges[j + 1].end.y - active_edges[j + 1].start.y + 1));
                end.x = active_edges[j + 1].x;
                end.y = y;

                drawLine(start, end);
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
                active_edges[j].dy--;
            }
        }

        y++;
    }
}
void ViewerWidget::fillTriangle(std::vector<Vertex> polygon)
{
    if (polygon.size() != 3)
        throw std::runtime_error("fillTriangle called with polygon.size() != 3");

    std::sort(polygon.begin(), polygon.end(),
              [](Vertex a, Vertex b)
              {
                  if ((int)a.y + 0.5 == (int)b.y + 0.5)
                      return a.x < b.x;
                  return a.y < b.y;
              });

    struct Edge
    {
        Vertex start, end;
        double dx;
    };

    Edge e1;
    Edge e2;

    if ((int)(polygon[0].y + 0.5) == (int)(polygon[1].y + 0.5))
    {
        // Filling the bottom flat triangle
        e1.start = polygon[0];
        e1.end = polygon[2];

        e2.start = polygon[1];
        e2.end = polygon[2];
    }
    else if ((int)(polygon[1].y + 0.5) == (int)(polygon[2].y + 0.5))
    {
        // Filling the top flat triangle
        e1.start = polygon[0];
        e1.end = polygon[1];

        e2.start = polygon[0];
        e2.end = polygon[2];
    }
    else if ((int)(polygon[0].y + 0.5) == (int)(polygon[2].y + 0.5))
    {
        // The triangle is a horizontal line, lets just skip it
        return;
    }
    else
    {
        // Splitting the triangle into two
        Vertex split_point(polygon[0].interpolate(polygon[2], (polygon[1].y - polygon[0].y) / (polygon[2].y - polygon[0].y)));

        if (polygon[1].x < split_point.x)
        {
            fillTriangle({polygon[0], polygon[1], split_point});
            fillTriangle({polygon[1], split_point, polygon[2]});
        }
        else
        {
            fillTriangle({polygon[0], split_point, polygon[1]});
            fillTriangle({split_point, polygon[1], polygon[2]});
        }
        return;
    }

    if (e1.start.y == e1.end.y)
        return;
    if (e2.start.y == e2.end.y)
        return;

    e1.dx = (double)(e1.end.x - e1.start.x) / (double)(e1.end.y - e1.start.y);
    e2.dx = (double)(e2.end.x - e2.start.x) / (double)(e2.end.y - e2.start.y);

    double x1 = e1.start.x;
    double x2 = e2.start.x;
    // double z1 = e1.start.z;
    // double z2 = e2.start.z;
    for (int y = e1.start.y + 0.5; y < e1.end.y; y++)
    {
        if (x1 != x2)
        {
            drawLine(
                e1.start.interpolate(e1.end, (y - e1.start.y) / (e1.end.y - e1.start.y)),
                e2.start.interpolate(e2.end, (y - e2.start.y) / (e2.end.y - e2.start.y)));
            // drawLine(
            //     QVector3D(x1, y, z1),
            //     QVector3D(x2, y, z2),
            //     color);
        }
        x1 += e1.dx;
        x2 += e2.dx;
        // z1 += e1.dz;
        // z2 += e2.dz;
    }
}

// 3D Object
void ViewerWidget::drawObject(ThreeDObject obj, Camera camera, LightSource light, ColoringType coloring)
{
    if (obj.vertices.size() == 0)
        return;
    transformToViewingCoordinates(obj, camera);

    calculateColors(obj, light, camera, coloring);

    if (camera.center_of_projection != 0)
        transformToPerspectiveCoordinates(obj, camera.center_of_projection);

    obj.translate(QVector3D(width() / 2, height() / 2, 0));

    drawObject(&obj, coloring);
}
void ViewerWidget::transformToViewingCoordinates(ThreeDObject &object, Camera camera)
{
    object.translate(-camera.position);

    // Rotate around Z axis based on azimuth

    for (std::list<Vertex>::iterator it = object.vertices.begin(); it != object.vertices.end(); it++)
    {
        double x = it->x;
        double y = it->y;
        it->x = x * cos(-camera.azimuth) - y * sin(-camera.azimuth);
        it->y = x * sin(-camera.azimuth) + y * cos(-camera.azimuth);
    }

    // Rotate around Y axis based on zenit

    for (std::list<Vertex>::iterator it = object.vertices.begin(); it != object.vertices.end(); it++)
    {
        double x = it->x;
        double z = it->z;
        // M_PI / 2 to convert the angle from the angle from horizon, into angle from vertical
        it->x = x * cos(-(M_PI / 2 - camera.zenit)) - z * sin(-(M_PI / 2 - camera.zenit));
        it->z = x * sin(-(M_PI / 2 - camera.zenit)) + z * cos(-(M_PI / 2 - camera.zenit));
    }

    // Switch axis to look from the front

    for (std::list<Vertex>::iterator it = object.vertices.begin(); it != object.vertices.end(); it++)
    {
        double x = it->x;
        double y = it->y;
        double z = it->z;
        it->x = y;
        it->y = -x;
    }
}
void ViewerWidget::calculateColors(ThreeDObject &object, LightSource light, Camera camera, ColoringType coloring)
{
    QVector3D norm_v, ligh_v, refl_v, view_v;

    if (coloring == ColoringType::SIDE)
    {
        for (Face &face : object.faces)
        {
            QVector3D center = face.center();
            norm_v = face.normal().normalized();
            ligh_v = (light.position - center).normalized();
            refl_v = 2 * QVector3D::dotProduct(norm_v, ligh_v) * norm_v - ligh_v;
            view_v = (QVector3D(0, 0, 400) - center).normalized();

            QVector3D Ia, Id, Im;

            // Ambient
            Ia = QVector3D(lightModel.ambient_color.red() * lightModel.ambient.x() / 255.,
                           lightModel.ambient_color.green() * lightModel.ambient.y() / 255.,
                           lightModel.ambient_color.blue() * lightModel.ambient.z() / 255.);

            // Diffuse
            float diffuse = QVector3D::dotProduct(norm_v, ligh_v) * light.intensity / 100.;
            if (diffuse > 0)
                Id = diffuse * QVector3D(light.color.red() * lightModel.diffuse.x() / 255.,
                                         light.color.green() * lightModel.diffuse.y() / 255.,
                                         light.color.blue() * lightModel.diffuse.z() / 255.);

            // Mirror
            float mirror = QVector3D::dotProduct(refl_v, view_v) * light.intensity / 255.;
            if (mirror > 0)
            {
                Im = pow(mirror, lightModel.specular_sharpness) *
                     QVector3D(light.color.red() * lightModel.specular.x() / 255.,
                               light.color.green() * lightModel.specular.y() / 255.,
                               light.color.blue() * lightModel.specular.z() / 255.);
            }

            QVector3D final_light = Ia + Id + Im;
            if (final_light.x() > 1)
                final_light.setX(1);
            if (final_light.y() > 1)
                final_light.setY(1);
            if (final_light.z() > 1)
                final_light.setZ(1);
            face.color = QColor(final_light.x() * 255, final_light.y() * 255, final_light.z() * 255);
        }
    }
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
    for (Face face : object->faces)
    {
        std::list<Vertex> polygon;
        Edge *e = face.edge;

        polygon.push_back(e->origin->copy());
        for (e = e->next; e != face.edge; e = e->next)
        {
            polygon.push_back(e->origin->copy());
        }

        for (Vertex &v : polygon)
        {
            if (coloring == WIREFRAME)
                v.color = globalColor;
            else if (coloring == SIDE)
                v.color = face.color;
        }

        if (coloring == WIREFRAME)
        {
            drawPolygon(polygon);
        }
        else if (coloring == SIDE || coloring == VERTEX)
        {
            fillPolygon(polygon);
        }
    }
}

//// Clipping ////

// Cyrus-Beck
void ViewerWidget::clipLine(Vertex start, Vertex end, Vertex &clip_start, Vertex &clip_end)
{
    if (!isInside(start) && !isInside(end))
    {
        clip_start = Vertex();
        clip_end = Vertex();
        return;
    }
    if (isInside(start) && isInside(end))
    {
        clip_start = start;
        clip_end = end;
        return;
    }

    double tl = 0, tu = 1;
    QVector3D d = end.toVector3D() - start.toVector3D();

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
        clip_start = start.interpolate(end, tl);
        clip_end = start.interpolate(end, tu);
        // clip_start = start + d * tl;
        // clip_end = start + d * tu;
    }
    // printf("clip_start: %d %d", clip_start.x(), clip_start.y());
    // printf("clip_end: %d %d", clip_end.x(), clip_end.y());
}

// Sutherland-Hodgman
void ViewerWidget::clipPolygonLeftSide(std::list<Vertex> &polygon, int x_min)
{

    if (polygon.size() == 0)
        return;

    std::list<Vertex> result;

    Vertex last_vertex = polygon.back();

    for (Vertex vertex : polygon)
    {
        if (vertex.x >= x_min)
        {
            if (last_vertex.x >= x_min)
            {
                result.push_back(vertex);
            }
            else
            {
                Vertex new_vertex = last_vertex.interpolate(vertex, (x_min - last_vertex.x) / (vertex.x - last_vertex.x));
                new_vertex.x = x_min;
                // QVector3D new_vertex(
                //     x_min,
                //     last_vertex.y + (x_min - last_vertex.x) * (vertex.y - last_vertex.y) / (vertex.x - last_vertex.x),
                //     last_vertex.z + (x_min - last_vertex.x) * (vertex.z - last_vertex.z) / (vertex.x - last_vertex.x));
                result.push_back(new_vertex);
                result.push_back(vertex);
            }
        }
        else
        {
            if (last_vertex.x >= x_min)
            {
                Vertex new_vertex = last_vertex.interpolate(vertex, (x_min - last_vertex.x) / (vertex.x - last_vertex.x));
                new_vertex.x = x_min;

                // QVector3D new_vertex(
                //     x_min,
                //     last_vertex.y + (x_min - last_vertex.x) * (vertex.y - last_vertex.y) / (vertex.x - last_vertex.x),
                //     last_vertex.z + (x_min - last_vertex.x) * (vertex.z - last_vertex.z) / (vertex.x - last_vertex.x));
                result.push_back(new_vertex);
            }
        }
        last_vertex = vertex;
    }
    polygon = result;
    return;
}
void ViewerWidget::clipPolygon(std::list<Vertex> &polygon)
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

        for (std::list<Vertex>::iterator it = polygon.begin(); it != polygon.end(); it++)
        {
            // *it = Vertex(it->y, -it->x, it->z);
            double tmp = it->x;
            it->x = it->y;
            it->y = -tmp;
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
        z_index[i] = -std::numeric_limits<double>::max();
    update();
}
void ViewerWidget::redraw()
{
    clear();
    drawObject();
}

// Slots
void ViewerWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect area = event->rect();
    painter.drawImage(area, *img, area);
}
