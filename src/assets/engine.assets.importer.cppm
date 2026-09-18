module;

#include <format>
#include <initializer_list>
#include <iostream>
#include <string_view>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "entt/entt.hpp"

export module engine.assets.importer;

import engine.ecs;

/**\brief a class to import 3D models using the Assimp library
 */
export class Importer {
public:
  const std::vector<entt::entity>
  m_import(std::initializer_list<std::string_view> paths, ECS &ecs);

private:
  Assimp::Importer m_importer;

  void m_process_node(const aiNode *node, const aiScene *scene,
                      std::vector<aiMesh *> &meshes);
};

/**\brief reads every given file and processes the nodes for each of them
 */
const std::vector<entt::entity>
Importer::m_import(std::initializer_list<std::string_view> paths, ECS &ecs) {
  std::vector<entt::entity> entities;

  // read the files
  for (const auto &path : paths) {
    const aiScene *scene = m_importer.ReadFile(
        path.data(),
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes);
    if (!scene || !scene->mRootNode ||
        scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
      std::cerr << std::format("error in {}: {}\n", path,
                               m_importer.GetErrorString());
      //---------------------------------------------------------------
      // VERY BAD!!!!!!!! FIX THIS AT SOME POINT!!!!!!!!!!!
      //---------------------------------------------------------------
      std::exit(-1);
    }

    // store the meshes inside this vector
    std::vector<aiMesh *> meshes;
    m_process_node(scene->mRootNode, scene, meshes);

    for (const auto &mesh : meshes) {
      // storage for the vertices and indices
      std::vector<float> vertices;
      std::vector<unsigned int> indices;

      // add the vertices
      for (size_t i = 0; i < mesh->mNumVertices; ++i) {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
      }

      // add the indices
      for (size_t i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace &face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j) {
          indices.push_back(face.mIndices[j]);
        }
      }

      //---------------------------------------------------------------------------------------
      // JUST TESTING MODEL IMPORTING; MESHES SHOULD BE SENT BACK AS SEPARATE
      // ENTITIES THO LATER
      //---------------------------------------------------------------------------------------

      // register the meshes
      entt::entity entity =
          ecs.m_add_entity_with_component<Position3D>(std::move(vertices));
      ecs.m_add_component_to_entity<Indices>(entity, std::move(indices));
      entities.push_back(entity);

      //---------------------------------------------------------------------------------------
      // ADD MATERIAL PROCESSING AT SOME POINT HERE
      //---------------------------------------------------------------------------------------
    }
  }

  return entities;
}

/**\brief recursively process the meshes for each of the nodes and their
 * children
 */
void Importer::m_process_node(const aiNode *node, const aiScene *scene,
                              std::vector<aiMesh *> &meshes) {
  for (size_t i = 0; i < node->mNumMeshes; ++i) {
    meshes.push_back(scene->mMeshes[node->mMeshes[i]]);
  }

  for (size_t i = 0; i < node->mNumChildren; ++i) {
    m_process_node(node->mChildren[i], scene, meshes);
  }
}
