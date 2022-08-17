/*
 * @Descripttion: LinEngine
 * @version: 1.0.0
 * @Author: jy1lnz
 * @Date: 2022-08-16 21:41:00
 * @LastEditors: jy1lnz
 * @LastEditTime: 2022-08-17 00:16:45
 */
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"
#include "assert.h"

Model::Model(const char* filename) : verts_(), faces_(), tex_faces_() {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v.raw[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> t;
            int itrash, idx;
            int slashes;
            iss >> trash;
            while (iss >> idx >> trash >> slashes >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                slashes--;
                assert(slashes >= 0);
                f.push_back(idx);
                t.push_back(slashes);
            }
            faces_.push_back(f);
            tex_faces_.push_back(t);
        }
        else if (!line.compare(0, 3, "vt ")){
            iss >> trash;
            iss >> trash;

            Vec3f v;
            for (int i = 0;i < 3; ++i) iss >> v.raw[i];
            texture_.push_back(Vec2f(v.x, 1 - v.y));
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() <<  " vt# " << texture_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::ntex() {
    return (int)texture_.size();
}

int Model::ntexface() {
    return (int)tex_faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::texface(int idx) {
    return tex_faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec2f Model::tex(int i) {
    return texture_[i];
}