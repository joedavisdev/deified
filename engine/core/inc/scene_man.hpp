#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "glm/glm.hpp"

namespace JMD {
struct RenderPass {};
struct SMPipeline {};   // TEMP: SM prefix to avoid name clashes
class SMMesh {         // TEMP: SM prefix to avoid name clashes
public:
    unsigned int    num_vertices_;
    unsigned int    stride_;
    unsigned int    num_indices_;
    unsigned int    num_indices_bytes_;
    char*           vertices_;
    char*           indices_;
    
    SMMesh();
    SMMesh(char* vertices,
           unsigned int num_vertices,
           unsigned int stride,
           char* indices,
           unsigned int num_indices,
           unsigned int num_indices_bytes);
    ~SMMesh();
    void ReleaseData();
};
class Model {
public:
    unsigned int number_of_meshes;
    SMMesh* mesh_array;
    Model();
    ~Model();
    void ReleaseData();
};
struct PhysicsBody {
    glm::vec4 position;
};
struct Actor {
    std::string name;
    PhysicsBody body;   // World space position etc.
    Model* model;       // A renderable collection of meshes
};
class SceneMan {
public:
    ~SceneMan();
    void Load(const std::string& scene_json_name);
    std::vector<Actor*> GetActorPtrs(std::string regex_string);
    void Update();
    void Draw();
private:
    std::vector<RenderPass> render_passes_;
    std::vector<SMPipeline> pipelines_;
    std::unordered_map<std::string,Model> models_;
    std::unordered_map<std::string,Actor> actors_;
    
    void ReleaseData();
    void BakeRenderPass(unsigned int index);
};
}