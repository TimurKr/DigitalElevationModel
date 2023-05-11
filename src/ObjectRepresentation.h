#pragma once

#include <QtWidgets>

class Edge;
class Face;

class Vertex
{
public:
    double x, y, z;
    std::vector<Edge *> edges;
    QColor color;

    QVector3D toVector3D() const { return QVector3D(x, y, z); }
    QPoint toPoint() const { return QPoint((int)x + 0.5, (int)y + 0.5); }

    Vertex copy() const
    {
        Vertex result;
        result.x = x;
        result.y = y;
        result.z = z;
        result.color = color;
        return result;
    }

    Vertex interpolate(Vertex &other, double t)
    {
        Vertex result;
        result.x = x + (other.x - x) * t;
        result.y = y + (other.y - y) * t;
        result.z = z + (other.z - z) * t;
        result.color =
            QColor(
                color.red() + (other.color.red() - color.red()) * t,
                color.green() + (other.color.green() - color.green()) * t,
                color.blue() + (other.color.blue() - color.blue()) * t);
        return result;
    }
};

class Edge
{
public:
    Vertex *origin;
    Face *face;
    Edge *next, *prev;
    Edge *pair;
};

class Face
{
public:
    Edge *edge;
    QColor color;

    QVector3D center() const
    {
        QVector3D v1 = edge->origin->toVector3D();
        QVector3D v2 = edge->next->origin->toVector3D();
        QVector3D v3 = edge->next->next->origin->toVector3D();
        return (v1 + v2 + v3) / 3;
    }

    QVector3D normal() const
    {
        QVector3D v1 = edge->origin->toVector3D();
        QVector3D v2 = edge->next->origin->toVector3D();
        QVector3D v3 = edge->next->next->origin->toVector3D();
        return QVector3D::crossProduct(v2 - v1, v3 - v2);
    }
};

class ThreeDObject
{
public:
    std::list<Vertex> vertices;
    std::list<Face> faces;
    std::list<Edge> edges;

    ThreeDObject() {}

    // Copy constructor
    ThreeDObject(const ThreeDObject &other)
    {
        if (other.vertices.empty())
            return;
        // Copy vertices
        std::unordered_map<const Vertex *, Vertex *> vertexMap;
        for (const Vertex &v : other.vertices)
        {
            Vertex newVertex;
            newVertex.x = v.x;
            newVertex.y = v.y;
            newVertex.z = v.z;
            newVertex.color = v.color;
            vertices.push_back(newVertex);
            vertexMap[&v] = &vertices.back();
        }

        // Create faces
        std::unordered_map<const Face *, Face *> faceMap;
        for (const Face &f : other.faces)
        {
            Face newFace;
            newFace.color = f.color;
            faces.push_back(newFace);
            faceMap[&f] = &faces.back();
        }

        // Copy edges
        std::unordered_map<const Edge *, Edge *> edgeMap;
        for (const Edge &e : other.edges)
        {
            Edge newEdge;
            newEdge.origin = vertexMap[e.origin];
            newEdge.face = faceMap[e.face];
            edges.push_back(newEdge);
            edgeMap[&e] = &edges.back();
        }

        // Fill pointers to edges
        // - for each vertex
        for (const Vertex &v : other.vertices)
        {
            for (Edge *e : v.edges)
            {
                vertexMap[&v]->edges.push_back(edgeMap[e]);
            }
        }
        // - for each face
        for (const Face &f : other.faces)
        {
            faceMap[&f]->edge = edgeMap[f.edge];
        }
        // - for each edge
        for (const Edge &e : other.edges)
        {
            edgeMap[&e]->next = edgeMap[e.next];
            edgeMap[&e]->prev = edgeMap[e.prev];
            edgeMap[&e]->pair = edgeMap[e.pair];
        }
    }

    void clear()
    {
        vertices.clear();
        faces.clear();
        edges.clear();
    }
    void translate(QVector3D offset)
    {
        for (auto &v : vertices)
        {
            v.x += offset.x();
            v.y += offset.y();
            v.z += offset.z();
        }
    }
    void scale(double factor)
    {
        for (auto &v : vertices)
        {
            v.x *= factor;
            v.y *= factor;
            v.z *= factor;
        }
    }
    void scaleZ(double factor)
    {
        for (auto &vertex : vertices)
        {
            vertex.z *= factor;
        }
    }
    void recolor(QColor color)
    {
        for (auto &v : vertices)
        {
            v.color = color;
        }
        for (auto &f : faces)
        {
            f.color = color;
        }
    }
};