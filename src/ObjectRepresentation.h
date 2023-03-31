#pragma once

class Edge;

class Vertex
{
public:
    double x, y, z;
    std::vector<Edge *> edges;
};

class Face
{
public:
    Edge *edge;
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