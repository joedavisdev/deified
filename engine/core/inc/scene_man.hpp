#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "metal_device.h"
#include "glm/glm.hpp"

namespace JMD {
namespace Core {
// Structs & classes
class Mesh {
public:
    unsigned int    num_vertices_;
    unsigned int    stride_;
    unsigned int    num_indices_;
    unsigned int    num_indices_bytes_;
    char*           vertices_;
    char*           indices_;
    
    Mesh();
    Mesh(char* vertices,
           unsigned int num_vertices,
           unsigned int stride,
           char* indices,
           unsigned int num_indices,
           unsigned int num_indices_bytes);
    ~Mesh();
    void ReleaseData();
};
class Model {
public:
    unsigned int number_of_meshes;
    Mesh* mesh_array;
    Model();
    ~Model();
    void ReleaseData();
};
struct PhysicsBody {
    glm::vec4 position;
};
struct Actor {
    PhysicsBody body;   // World space position etc.
    Model* model;       // A renderable collection of meshes
};
struct Draw {
    Actor* actor_;
    GFX::Pipeline* pipeline_;
};
struct CommandBuffer {
    GFX::CommandBuffer cb_;
    std::vector<Draw> draws_;
};
struct RenderPass {
    std::string actor_regex_;
    std::vector<Actor*> actor_ptrs_;
    std::vector<GFX::RenderAttachmentDesc> colour_attachments_;
    GFX::RenderAttachmentDesc depth_stencil_attachment_;
    std::vector<CommandBuffer> command_buffers_;
};
class SceneMan {
public:
    ~SceneMan();
    void Load(const std::string& scene_json_name);
    std::vector<Actor*> GetActorPtrs(std::string regex_string);
    void Update();
    void Draw();
private:
    std::unordered_map<std::string,RenderPass> render_passes_;
    std::vector<GFX::Pipeline> pipelines_;
    std::unordered_map<std::string,Model> models_;
    std::unordered_map<std::string,Actor> actors_;
    
    void ReleaseData();
    void BakeRenderPass(const std::string &name, RenderPass &render_pass);
};
}}