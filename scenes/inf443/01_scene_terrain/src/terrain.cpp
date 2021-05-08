
#include "terrain.hpp"
#include "Triangle.h"
#include "Gridcell.h"
#include "Tables.h"
#include "Position.h"
#include <stdlib.h>
#include <vector>
#include "vcl/vcl.hpp"

using namespace vcl;


// Evaluate 3D position of the terrain for any (u,v) \in [0,1]



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
    vcl::vec3 vertlist[12];
    if (grid.values[0] < isolevel) cubeindex |= 1;
    if (grid.values[1] < isolevel) cubeindex |= 2;
    if (grid.values[2] < isolevel) cubeindex |= 4;
    if (grid.values[3] < isolevel) cubeindex |= 8;
    if (grid.values[4] < isolevel) cubeindex |= 16;
    if (grid.values[5] < isolevel) cubeindex |= 32;
    if (grid.values[6] < isolevel) cubeindex |= 64;
    if (grid.values[7] < isolevel) cubeindex |= 128;
    if (edgeTable[cubeindex] == 0)
        return std::vector<Triangle>();
    if (edgeTable[cubeindex] & 1)
        vertlist[0] =
        VertexInterp(grid.p[0], grid.p[1], grid.values[0], grid.values[1], isolevel);
    if (edgeTable[cubeindex] & 2)                 
        vertlist[1] =                             
        VertexInterp(grid.p[1], grid.p[2], grid.values[1], grid.values[2], isolevel);
    if (edgeTable[cubeindex] & 4)                          
        vertlist[2] =                                      
        VertexInterp(grid.p[2], grid.p[3], grid.values[2], grid.values[3], isolevel);
    if (edgeTable[cubeindex] & 8)                        
        vertlist[3] =                                    
        VertexInterp(grid.p[3], grid.p[0], grid.values[3], grid.values[0], isolevel);
    if (edgeTable[cubeindex] & 16)                                 
        vertlist[4] =                                              
        VertexInterp(grid.p[4], grid.p[5], grid.values[4], grid.values[5], isolevel);
    if (edgeTable[cubeindex] & 32)                                 
        vertlist[5] =                                              
        VertexInterp(grid.p[5], grid.p[6], grid.values[5], grid.values[6], isolevel);
    if (edgeTable[cubeindex] & 64)                              
        vertlist[6] =                                           
        VertexInterp(grid.p[6], grid.p[7], grid.values[6], grid.values[7], isolevel);
    if (edgeTable[cubeindex] & 128)                                    
        vertlist[7] =                                                  
        VertexInterp(grid.p[7], grid.p[4], grid.values[7], grid.values[4], isolevel);
    if (edgeTable[cubeindex] & 256)                                     
        vertlist[8] =                                                   
        VertexInterp(grid.p[0], grid.p[4], grid.values[0], grid.values[4], isolevel);
    if (edgeTable[cubeindex] & 512)                               
        vertlist[9] =                                             
        VertexInterp(grid.p[1], grid.p[5], grid.values[1], grid.values[5], isolevel);
    if (edgeTable[cubeindex] & 1024)                               
        vertlist[10] =                                             
        VertexInterp(grid.p[2], grid.p[6], grid.values[2], grid.values[6], isolevel);
    if (edgeTable[cubeindex] & 2048)                                   
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