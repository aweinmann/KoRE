/*
  Copyright © 2012 The KoRE Project

  This file is part of KoRE.

  KoRE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  KoRE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with KoRE.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "KoRE/Log.h"
#include "KoRE/Loader/MeshLoader.h"
#include "KoRE/Components/Transform.h"
#include "KoRE/Components/MeshComponent.h"
#include "KoRE/Mesh.h"

using namespace kore;

kore::MeshLoader* kore::MeshLoader::getInstance() {
  static MeshLoader clInstance;
  return &clInstance;
}

kore::MeshLoader::MeshLoader() {
}

kore::MeshLoader::~MeshLoader() {
}


kore::Mesh*
    kore::MeshLoader::loadMesh(const aiScene* pAiScene,
                               const uint uMeshIdx) {
    kore::Mesh* pMesh = new kore::Mesh;
  
    aiMesh* pAiMesh = pAiScene->mMeshes[uMeshIdx];
    pMesh->_numVertices = pAiMesh->mNumVertices;

    // TODO(dlazarek): Make more flexible here:
    pMesh->_primitiveType = GL_TRIANGLES;

    pMesh->_name = getMeshName(uMeshIdx, pAiScene);

    if (pAiMesh->HasPositions()) {
        loadVertexPositions(pAiMesh, pMesh);
    }

    if (pAiMesh->HasNormals()) {
        loadVertexNormals(pAiMesh, pMesh);
    }

    if (pAiMesh->HasTangentsAndBitangents()) {
        loadVertexTangents(pAiMesh, pMesh);
    }

    // Load all texture coord-sets
    unsigned int iUVset = 0;
    while (pAiMesh->HasTextureCoords(iUVset)) {
        loadVertexTextureCoords(pAiMesh, pMesh, iUVset);
        ++iUVset;
    }

    // Load all vertex color sets
    unsigned int iColorSet = 0;
    while (pAiMesh->HasVertexColors(iColorSet)) {
        loadVertexColors(pAiMesh, pMesh, iColorSet);
        ++iColorSet;
    }

    if (pAiMesh->HasFaces()) {
        loadFaceIndices(pAiMesh, pMesh);
    }

    pMesh->createAttributeBuffers(BUFFERTYPE_INTERLEAVED);
    return pMesh;
}

aiNode* findNodeWithMesh(uint meshSceneIdx, aiNode* node, const aiScene* scene) {
  for (uint i = 0; i < node->mNumMeshes; ++i) {
    if (node->mMeshes[i] == meshSceneIdx) {
      return node;
    }
  }

  for (uint i = 0; i < node->mNumChildren; ++i) {
    aiNode* childNode = findNodeWithMesh(meshSceneIdx, node->mChildren[i], scene);
    if (childNode != NULL) {
      return childNode;
    }
  }

  return NULL;
}

std::string kore::MeshLoader::getMeshName(uint meshSceneIdx,
                                          const aiScene* paiScene) {
   aiMesh* paiMesh = paiScene->mMeshes[meshSceneIdx];
   std::string returnName;
   if (paiMesh->mName.length > 0) {
    returnName = std::string(paiMesh->mName.C_Str());
  } else {
    aiNode* node = findNodeWithMesh(meshSceneIdx, paiScene->mRootNode, paiScene);

    if (node != NULL) {
      uint internalMeshIdx = 0;
      for (uint i = 0; i < node->mNumMeshes; ++i) {
        if (node->mMeshes[i] == meshSceneIdx) {
          internalMeshIdx = i;
        }
      }

      char buf[100];
      sprintf(buf, "%s_mesh_%i", node->mName.C_Str(), internalMeshIdx);
      returnName = std::string(buf);
    }
  }
  return returnName;
}


void kore::MeshLoader::
    loadVertexPositions(const aiMesh* pAiMesh,
                         kore::Mesh* pMesh ) {
    unsigned int allocSize = pAiMesh->mNumVertices * 3 * 4;
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mVertices,
      allocSize);

    kore::MeshAttributeArray att;
    att.name = "v_position";
    att.numValues = pAiMesh->mNumVertices * 3;
    att.numComponents = 3;
    att.type = GL_FLOAT_VEC3;
    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

void kore::MeshLoader::
    loadVertexNormals(const aiMesh* pAiMesh,
                       kore::Mesh* pMesh) {
    unsigned int allocSize = pAiMesh->mNumVertices * 3 * 4;
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mNormals,
           allocSize);

    kore::MeshAttributeArray att;
    att.name = "v_normal";
    att.numValues = pAiMesh->mNumVertices * 3;
    att.numComponents = 3;
    att.type = GL_FLOAT_VEC3;
    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

void kore::MeshLoader::
    loadVertexTangents(const aiMesh* pAiMesh,
                       kore::Mesh* pMesh) {
    unsigned int allocSize = pAiMesh->mNumVertices * 3 * 4;
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mTangents,
           allocSize);

    kore::MeshAttributeArray att;
    att.name = "v_tangent";
    att.numValues = pAiMesh->mNumVertices * 3;
    att.numComponents = 3;
    att.type = GL_FLOAT_VEC3;
    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

void kore::MeshLoader::
    loadFaceIndices(const aiMesh* pAiMesh,
                     kore::Mesh* pMesh ) {
    for (unsigned int iFace = 0; iFace < pAiMesh->mNumFaces; ++iFace) {
        aiFace& rAiFace = pAiMesh->mFaces[iFace];
        for (unsigned int iIndex = 0; iIndex < rAiFace.mNumIndices; ++iIndex) {
            pMesh->_indices.push_back(rAiFace.mIndices[iIndex]);
        }
    }
}

void kore::MeshLoader::
    loadVertexColors(const aiMesh* pAiMesh,
                      kore::Mesh* pMesh,
                      unsigned int iColorSet) {
    unsigned int allocSize =
        pAiMesh->mNumVertices * 4 * pAiMesh->GetNumColorChannels();
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mColors[iColorSet], allocSize);

    kore::MeshAttributeArray att;
    char szNameBuf[20];
    sprintf(szNameBuf, "v_color%i", iColorSet);
    att.name = std::string(&szNameBuf[0]);
    att.numValues = pAiMesh->mNumVertices * pAiMesh->GetNumColorChannels();
    att.numComponents = pAiMesh->GetNumColorChannels();

    if (pAiMesh->GetNumColorChannels() == 2) {
        att.type = GL_FLOAT_VEC2;
    } else if (pAiMesh->GetNumColorChannels() == 3) {
        att.type = GL_FLOAT_VEC3;
    } else if (pAiMesh->GetNumColorChannels() == 4) {
        att.type = GL_FLOAT_VEC4;
    } else {
        Log::getInstance()->write("[WARNING] Mesh %s has an"
                                  "unsupported number of color channels: %i",
                                  pMesh->getName().c_str());
        free(pVertexData);
        return;
    }

    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

void kore::MeshLoader::
  loadVertexTextureCoords(const aiMesh* pAiMesh,
                           kore::Mesh* pMesh,
                           unsigned int iUVset) {
  // Note(dospelt) assimp imports always vec3 texcoords
  // TODO(dospelt) check which coordinates are used with
  // pAiMesh->mNumUVComponents[i] and adapt the MeshAttributeArray
  unsigned int allocSize =
    pAiMesh->mNumVertices *12; //size of vec3
  void* pVertexData = malloc(allocSize);
  memcpy(pVertexData, pAiMesh->mTextureCoords[iUVset], allocSize);

  kore::MeshAttributeArray att;
  char szNameBuf[20];
  sprintf(szNameBuf, "v_uv%i", iUVset);
  att.name = std::string(&szNameBuf[0]);
  att.numValues = pAiMesh->mNumVertices * 3;
  att.numComponents = 3;

  /*if (pAiMesh->GetNumUVChannels() == 2) {
    att.type = GL_FLOAT_VEC2;
  } else if (pAiMesh->GetNumUVChannels() == 3) {
      att.type = GL_FLOAT_VEC3;
  } else {
    Log::getInstance()->write("[WARNING] Mesh %s has an unsupported"
                              "number of UV channels: %i",
                              pMesh->getName().c_str());
    free(pVertexData);
    return;
  }*/
  att.type = GL_FLOAT_VEC3;
  att.componentType = GL_FLOAT;
  att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
  att.data = pVertexData;
  pMesh->_attributes.push_back(att);
}
