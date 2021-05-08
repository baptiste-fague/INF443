#include "tree.hpp"

using namespace vcl;


mesh create_tree_trunk_cylinder(float radius, float height)
{
    mesh m;

    const int N = 10;
    
    m.position.resize(N * 2);

    for (int i = 0; i < N; i++) {
        const float u = radius * cos(2 * i * pi / N);
        const float v = radius * sin(2 * i * pi / N);

        vec3 const p1 = { u, v, 0.0f };
        vec3 const p2 = { u, v, height };

        m.position[2 * i] = p1;
        m.position[2 * i + 1] = p2;
    }

    for (int i = 0; i < N; i++) {

        const uint3 triangle1 = { 2*i, (2*i + 2)%(2*N), 2*i + 1 };
        const uint3 triangle2 = { (2*i + 2)%(2*N), (2*i + 3)%(2*N), 2*i + 1 };

        m.connectivity.push_back(triangle1);
        m.connectivity.push_back(triangle2);
    }

    m.fill_empty_field();
    return m;
}

mesh create_cone(float radius, float height, float z_offset)
{
    mesh m;

    const int N = 10;

    m.position.resize(N+2);

    for (int i = 0; i < N; i++) {
        const float u = radius * cos(2 * i * pi / N);
        const float v = radius * sin(2 * i * pi / N);

        vec3 const p = { u, v, z_offset };

        m.position[i] = p;
    }

    m.position[N] = { 0.0f, 0.0f, z_offset };
    m.position[N + 1] = { 0.0f, 0.0f, z_offset + height };

    for (int i = 0; i < N; i++) {

        const uint3 triangle1 = { i, (i + 1) % N, N };
        const uint3 triangle2 = { i, (i + 1) % N, N + 1 };

        m.connectivity.push_back(triangle1);
        m.connectivity.push_back(triangle2);
    }

    m.fill_empty_field();
    return m;
}

mesh create_tree()
{
    float const h = 0.7f; // trunk height
    float const r = 0.1f; // trunk radius

    // Create a brown trunk
    mesh trunk = create_tree_trunk_cylinder(r, h);
    trunk.color.fill({ 0.4f, 0.3f, 0.3f });


    // Create a green foliage from 3 cones
    mesh foliage = create_cone(4 * r, 6 * r, 0.0f);      // base-cone
    foliage.push_back(create_cone(4 * r, 6 * r, 2 * r));   // middle-cone
    foliage.push_back(create_cone(4 * r, 6 * r, 4 * r));   // top-cone
    foliage.position += vec3(0, 0, h);                 // place foliage at the top of the trunk
    foliage.color.fill({ 0.4f, 0.6f, 0.3f });

    // The tree is composted of the trunk and the foliage
    mesh tree = trunk;
    tree.push_back(foliage);

    return tree;
}


