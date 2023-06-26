#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;

void readFile(string filename, vector<string>& objFileContents) {
    string line;
    ifstream objFile(filename);
    unsigned int numLines = 0;

    while (getline(objFile, line))
        objFileContents.push_back(line);
}

void addFaceVals(string faceLine, vector<int>& faces) {
    // add 'f' values from obj file into a vector
    string segment;
    stringstream faceLiness(faceLine);
    getline(faceLiness, segment, ' '); //take out the obj command

    while (!faceLiness.eof()) {
        getline(faceLiness, segment, ' ');
        if (segment == "")
            continue;
        string strNum;
        stringstream segmentss(segment);
        while (!segmentss.eof()) {
            getline(segmentss, strNum, '/');
            int num = stoi(strNum);
            faces.push_back(num);
        }
    }
}

void addAttrVals(string line, vector<float>& attrVals) {
    string segment;
    stringstream liness(line);
    getline(liness, segment, ' ');
    int valuesPushed = 0;

    while (!liness.eof()) {
        getline(liness, segment, ' ');
        if (segment == "")
            continue;
        float num = stof(segment);
        attrVals.push_back(num);
        valuesPushed++;
    }
    if (valuesPushed < 3) {
        attrVals.push_back(0.f);
    }
}

int sepAttrFromObjContent(vector<string>& objFileContents, vector<float>& verts,
    vector<float>& textureVerts, vector<float>& normalVerts, vector<int>& faces) {

    int faceCount = 0;
    for (string line : objFileContents) {
        if (line.length() == 0) {
            continue;
        }
        else {
            string attr = line.substr(0, 2);
            if (attr == "# ") {
                continue;
            }
            else if (attr == "v ") {
                addAttrVals(line, verts);
            }
            else if (attr == "vt") {
                addAttrVals(line, textureVerts);
            }
            else if (attr == "vn") {
                addAttrVals(line, normalVerts);
            }
            else if (attr == "f ") {
                addFaceVals(line, faces);
                faceCount++;
            }
        }
    }

    verts.push_back(0.f);
    textureVerts.push_back(0.f);
    normalVerts.push_back(0.f);

    return faceCount;
}

void insertVertex(float* vertices, float* vertex, int idx) {
    copy(vertex, vertex + 9, vertices + idx * 9);
}

void insertVertAttr(float* vertices, int idx, vector<float>& verts, vector<float>& textureVerts, vector<float>& normalVerts, int vIdx, int vtIdx, int vnIdx) {
    float vertex[9];

    copy(&verts[vIdx * 3], &verts[vIdx * 3 + 3], &vertex[0]);
    copy(&textureVerts[vtIdx * 3], &textureVerts[vtIdx * 3 + 3], &vertex[3]);
    copy(&normalVerts[vnIdx * 3], &normalVerts[vnIdx * 3 + 3], &vertex[6]);

    insertVertex(vertices, vertex, idx);
}

void insertFaceVertAttrs(float* vertices, vector<float>& verts, vector<float>& textureVerts, vector<float>& normalVerts, vector<int>& faces, int faceIdx) {
    int vIdxDupe1, vtIdxDupe1, vnIdxDupe1, vIdxDupe2, vtIdxDupe2, vnIdxDupe2;

    for (int vertexIdx = 0; vertexIdx < 4; vertexIdx++) {
        int vIdx = faces[faceIdx * 12 + vertexIdx * 3] - 1;
        int vtIdx = faces[faceIdx * 12 + vertexIdx * 3 + 1] - 1;
        int vnIdx = faces[faceIdx * 12 + vertexIdx * 3 + 2] - 1;
        if (vertexIdx == 0) {
            vIdxDupe1 = vIdx;
            vtIdxDupe1 = vtIdx;
            vnIdxDupe1 = vnIdx;
        }
        else if (vertexIdx == 2) {
            vIdxDupe2 = vIdx;
            vtIdxDupe2 = vtIdx;
            vnIdxDupe2 = vnIdx;
        }
        if (vertexIdx == 3) {
            insertVertAttr(vertices, faceIdx * 6 + vertexIdx, verts, textureVerts, normalVerts, vIdxDupe1, vtIdxDupe1, vnIdxDupe1);
            insertVertAttr(vertices, faceIdx * 6 + vertexIdx + 1, verts, textureVerts, normalVerts, vIdxDupe2, vtIdxDupe2, vnIdxDupe2);
            insertVertAttr(vertices, faceIdx * 6 + vertexIdx + 2, verts, textureVerts, normalVerts, vIdx, vtIdx, vnIdx);
        }
        else {
            insertVertAttr(vertices, faceIdx * 6 + vertexIdx, verts, textureVerts, normalVerts, vIdx, vtIdx, vnIdx);
        }
    }
}

void insertFaceVertAttrsTriangles(float* vertices, vector<float>& verts, vector<float>& textureVerts, vector<float>& normalVerts, vector<int>& faces, int faceIdx) {
    for (int vertexIdx = 0; vertexIdx < 3; vertexIdx++) {
        int vIdx = faces[faceIdx * 9 + vertexIdx * 3] - 1;
        int vtIdx = faces[faceIdx * 9 + vertexIdx * 3 + 1] - 1;
        int vnIdx = faces[faceIdx * 9 + vertexIdx * 3 + 2] - 1;

        insertVertAttr(vertices, faceIdx * 3 + vertexIdx, verts, textureVerts, normalVerts, vIdx, vtIdx, vnIdx);
    }
}

void assembleTriangles(float* vertices, vector<float>& verts, vector<float>& textureVerts, vector<float>& normalVerts, vector<int>& faces, int faceCount, bool isQuad) {
    if (isQuad) {
        for (int faceIdx = 0; faceIdx < faceCount; faceIdx++) {
            insertFaceVertAttrs(vertices, verts, textureVerts, normalVerts, faces, faceIdx);
        }
    }
    else {
        for (int faceIdx = 0; faceIdx < faceCount; faceIdx++) {
            insertFaceVertAttrsTriangles(vertices, verts, textureVerts, normalVerts, faces, faceIdx);
        }
    }
}

float* parseObjFile(int& size, string filename, bool isQuad) {
    vector<string> objFileContent;
    vector<float> verts;
    vector<float> textureVerts;
    vector<float> normalVerts;
    vector<int> faces;

    float* vertices;

    readFile(filename, objFileContent);
    int faceCount = sepAttrFromObjContent(objFileContent, verts, textureVerts, normalVerts, faces);
    if (isQuad) {
        vertices = (float*)malloc(sizeof(float) * faceCount * 9 * 6);
        size = faceCount * 9 * 6;
    }
    else {
        vertices = (float*)malloc(sizeof(float) * faceCount * 9 * 3);
        size = faceCount * 9 * 3;
    }

    assembleTriangles(vertices, verts, textureVerts, normalVerts, faces, faceCount, isQuad);

    //memory freeing
    vector<string>().swap(objFileContent);
    vector<float>().swap(verts);
    vector<float>().swap(textureVerts);
    vector<float>().swap(normalVerts);
    vector<int>().swap(faces);

    return vertices;
}