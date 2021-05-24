
#include "terrain.hpp"
#include "Triangle.hpp"
#include "Gridcell.hpp"
#include "Tables.hpp"
#include <stdlib.h>
#include <vector>
#include "vcl/vcl.hpp"
#include <thread>


using namespace vcl;

static int terrain_size;
static float isolevel;
static float scale;
std::vector<mesh> terrain_slices;


void update_tree_position(std::vector<vcl::vec3>& tree_positions, vcl::mesh& terrain, int count, bool minDist) {
    int N = count;
    int size = terrain.position.size();
    tree_positions.clear();
    int i = 0;
    while (i < N) {
        int n = rand() % size;
        vec3 pos(terrain.position[n]);
        if (!minDist || pointFarEnoughFrom(pos.xy(), tree_positions, 0.8f)) {
            pos.z -= 0.1f;
            tree_positions.push_back(pos);
            i++;
        }
    }
}

bool pointFarEnoughFrom(vcl::vec2 point, std::vector<vcl::vec3>& positions, float distance) {
    for (int i = 0; i < positions.size(); i++) {
        if (norm(positions[i].xy() - point) < distance) {
            return false;
        }
    }
    return true;
}


std::vector<Triangle> polygonize(Gridcell grid, float isolevel) {
    std::vector<Triangle> triangles;
    int i;
    int cubeindex = 0;
    int hexa;
    vcl::vec3 vertlist[12];
    if (grid.values[0] < isolevel) cubeindex |= 1;
    if (grid.values[1] < isolevel) cubeindex |= 2;
    if (grid.values[2] < isolevel) cubeindex |= 4;
    if (grid.values[3] < isolevel) cubeindex |= 8;
    if (grid.values[4] < isolevel) cubeindex |= 16;
    if (grid.values[5] < isolevel) cubeindex |= 32;
    if (grid.values[6] < isolevel) cubeindex |= 64;
    if (grid.values[7] < isolevel) cubeindex |= 128;
    hexa = edgeTable[cubeindex];
    if (hexa == 0)
        return std::vector<Triangle>();
    if (hexa & 1)
        vertlist[0] =
        VertexInterp(grid.p[0], grid.p[1], grid.values[0], grid.values[1], isolevel);
    if (hexa & 2)
        vertlist[1] =                             
        VertexInterp(grid.p[1], grid.p[2], grid.values[1], grid.values[2], isolevel);
    if (hexa & 4)
        vertlist[2] =                                      
        VertexInterp(grid.p[2], grid.p[3], grid.values[2], grid.values[3], isolevel);
    if (hexa & 8)
        vertlist[3] =                                    
        VertexInterp(grid.p[3], grid.p[0], grid.values[3], grid.values[0], isolevel);
    if (hexa & 16)
        vertlist[4] =                                              
        VertexInterp(grid.p[4], grid.p[5], grid.values[4], grid.values[5], isolevel);
    if (hexa & 32)
        vertlist[5] =                                              
        VertexInterp(grid.p[5], grid.p[6], grid.values[5], grid.values[6], isolevel);
    if (hexa & 64)
        vertlist[6] =                                           
        VertexInterp(grid.p[6], grid.p[7], grid.values[6], grid.values[7], isolevel);
    if (hexa & 128)
        vertlist[7] =                                                  
        VertexInterp(grid.p[7], grid.p[4], grid.values[7], grid.values[4], isolevel);
    if (hexa & 256)
        vertlist[8] =                                                   
        VertexInterp(grid.p[0], grid.p[4], grid.values[0], grid.values[4], isolevel);
    if (hexa & 512)
        vertlist[9] =                                             
        VertexInterp(grid.p[1], grid.p[5], grid.values[1], grid.values[5], isolevel);
    if (hexa & 1024)
        vertlist[10] =                                             
        VertexInterp(grid.p[2], grid.p[6], grid.values[2], grid.values[6], isolevel);
    if (hexa & 2048)
        vertlist[11] =                                                 
        VertexInterp(grid.p[3], grid.p[7], grid.values[3], grid.values[7], isolevel);
    for (i = 0; triTable[cubeindex][i] != -1; i += 3) {
       triangles.push_back(Triangle(vertlist[triTable[cubeindex][i]], vertlist[triTable[cubeindex][i + 1]], vertlist[triTable[cubeindex][i + 2]]));
    }

    return triangles;
}

vcl::vec3 VertexInterp(vcl::vec3 p1, vcl::vec3  p2, float valp1, float valp2, float isolevel){
    double mu;
    vcl::vec3 p;

    if (std::abs(isolevel - valp1) < 0.00001)
        return(p1);
    if (std::abs(isolevel - valp2) < 0.00001)
        return(p2);
    if (std::abs(valp1 - valp2) < 0.00001)
        return(p1);
    mu = (isolevel - valp1) / (valp2 - valp1);
    p[0] = p1[0] + mu * (p2[0] - p1[0]);
    p[1] = p1[1] + mu * (p2[1] - p1[1]);
    p[2] = p1[2] + mu * (p2[2] - p1[2]);

    return(p);
}
std::vector<vcl::mesh_drawable> create_terrain(int terrain_size_, float isolevel_, float scale_) {
    terrain_slices.resize(10);
    terrain_size = terrain_size_;
    isolevel = isolevel_;
    scale = scale_;
    std::thread threads[10];
    for (int i = 0; i < 10; i++) {
        threads[i] = std::thread(create_terrain_thread, i) ;
    }
    for (int i = 0; i < 10; i++) {
        threads[i].join();
    }
    //for (int i = 0; i < 10; i++) {
    //    create_terrain_thread(i);
    //}
    std::vector<mesh_drawable> drawables;
    for (int i = 0; i < 10; i++) {
        mesh_drawable terrain = vcl::mesh_drawable(terrain_slices[i]);
        terrain.shading.color = { 0.8f ,0.35f ,0.2f };
        terrain.shading.phong.specular = 0.0f;
        drawables.push_back(terrain);
    }
    return drawables;
}

void create_terrain_thread( int thread_number) {
    std::vector<mesh> liste;
    vcl::vec3 cube_positions_base[8] = { vcl::vec3(1, 0, 0), vcl::vec3(1, 1, 0), vcl::vec3(0, 1, 0), vcl::vec3(0, 0, 0),  vcl::vec3(1, 0, 1),  vcl::vec3(1, 1, 1),  vcl::vec3(0, 1, 1),  vcl::vec3(0, 0, 1) };
    for (int i = (float)terrain_size*thread_number/10; i < (float)terrain_size*(thread_number +1)/10 ; i++) {
        for (int j = 0; j < terrain_size; j++) {
            for (int k = 0; k < terrain_size; k++) {
                float cube_valeurs[8];
                vcl::vec3 cube_positions[8];
                for (int l = 0; l < 8;l++) {
                    if ((vcl::vec3(i, j, k) + cube_positions_base[l])[0] == terrain_size || (vcl::vec3(i, j, k) + cube_positions_base[l])[1] == terrain_size || (vcl::vec3(i, j, k) + cube_positions_base[l])[2] == terrain_size || (vcl::vec3(i, j, k) + cube_positions_base[l])[0] == 0 || (vcl::vec3(i, j, k) + cube_positions_base[l])[1] == 0 || (vcl::vec3(i, j, k) + cube_positions_base[l])[2] == 0) {
                        cube_valeurs[l] = 0;
                    }
                    else {
                        cube_valeurs[l] = noise_perlin(scale * (vcl::vec3(i, j, k) + cube_positions_base[l]));
                    }
                    cube_positions[l] = vcl::vec3(i, j, k) + cube_positions_base[l];
                }
                Gridcell grid = Gridcell(cube_positions, cube_valeurs);
                vcl::mesh cube;
                std::vector<Triangle> vecteur = polygonize(grid, isolevel);
                cube.position.resize(3 * vecteur.size());
                for (int l = 0; l < vecteur.size(); l++) {
                    cube.position[3 * l] = vecteur[l].p[0];
                    cube.position[3 * l + 1] = vecteur[l].p[1];
                    cube.position[3 * l + 2] = vecteur[l].p[2];
                    cube.connectivity.push_back({ 3 * l, 3 * l + 1, 3 * l + 2 });
                }
                
                if (vecteur.size() > 0) {
                    cube.fill_empty_field();
                    liste.push_back(cube);
                    
                }
            }
        }
    }
    vcl::mesh terrain_mesh;
    for (int i = 0; i < liste.size(); i++) {
        terrain_mesh.push_back(liste[i]);
        mesh& flip_connectivity();
        mesh& compute_normal();
    }
    /*terrain = vcl::mesh_drawable(terrain_mesh);
    terrain.shading.color = { 0.8f ,0.35f ,0.2f };
    terrain.shading.phong.specular = 0.0f;*/
    terrain_slices[thread_number] = terrain_mesh;
}