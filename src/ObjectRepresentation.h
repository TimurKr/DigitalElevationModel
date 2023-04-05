#pragma once

#include <QtWidgets>

class Edge;

class Vertex
{
public:
    double x, y, z;
    std::vector<Edge *> edges;
    QColor color;

    QVector3D toVector3D() const { return QVector3D(x, y, z); }
};

class Face
{
public:
    Edge *edge;
    QColor color;
};

class Edge
{
public:
    Vertex *origin;
    Face *face;
    Edge *next, *prev;
    Edge *pair;
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
};