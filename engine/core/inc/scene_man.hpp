#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "metal_device.h"
#include "glm/glm.hpp"

namespace JMD {
class ParsedEffect;
class ParsedActor;
class ParsedRenderPass;
namespace Core {
// Forward declarations
class CommandBuffer;
class RenderPass;
// Structs & classes
struct Effect {
    std::string frag_shader_name;
    std::string vert_shader_name;
};
class Mesh {
public:
    Mesh();
    Mesh(char* vertices,
           unsigned int num_vertices,
           unsigned int stride,
           char* indices,
           unsigned int num_indices,
           unsigned int num_indices_bytes);
    ~Mesh();
    void ReleaseData();
private:
    unsigned int    num_vertices_;
    unsigned int    stride_;
    unsigned int    num_indices_;
    unsigned int    num_indices_bytes_;
    char*           vertices_;
    char*           indices_;
};
struct Model {
public:
    Model();
    ~Model();
    void ReleaseData();
    void set_number_of_meshes(unsigned int number_of_meshes){number_of_meshes_=number_of_meshes;}
    unsigned int number_of_meshes(){return number_of_meshes_;}
    void set_mesh_array(Mesh* mesh_array){delete[] mesh_array_;mesh_array_=mesh_array;}
    Mesh* mesh_array(){return mesh_array_;}
private:
    unsigned int number_of_meshes_;
    Mesh* mesh_array_;
};
struct PhysicsBody {
    glm::vec4 position;
};
struct Actor {
    PhysicsBody body;
    Model* model_ptr;
    Effect* effect_ptr;
    Actor():model_ptr(nullptr),effect_ptr(nullptr){}
};
struct Pipeline {
    Effect* effect_ptr;
    RenderPass* render_pass_ptr;
    GFX::PipelineDesc pipeline_desc;
};
struct Draw {
    Actor* actor_ptr;
    Pipeline* pipeline_ptr;
    Draw():actor_ptr(nullptr),pipeline_ptr(nullptr){}
};
struct CommandBuffer {
    GFX::CommandBuffer cb;
    std::vector<Draw> draws;
};
struct RenderPass {
    std::string actor_regex;
    std::vector<Actor*> actor_ptrs;
    std::vector<GFX::RenderAttachmentDesc> colour_attachments;
    GFX::RenderAttachmentDesc depth_stencil_attachment;
    std::vector<CommandBuffer> command_buffers;
};
class SceneMan {
public:
    SceneMan():loaded_bitflags_(0){}
    ~SceneMan();
    void Load(const std::string& scene_json_name);
    std::vector<Actor*> GetActorPtrs(std::string regex_string);
    void Update();
    void Draw();
private:
    void LoadEffects(const std::vector<ParsedEffect>& parsed_effects);
    void LoadActors(const std::vector<ParsedActor>& parsed_actors);
    void LoadRenderPasses(const std::vector<ParsedRenderPass>& parsed_render_passes);
    void ReleaseData();
    void BuildPipelines();
    void BuildPipeline(Effect &effect, RenderPass &render_pass);
    Pipeline* FindPipeline(const Effect &effect, const RenderPass &render_pass);
    void BuildCommandBuffers(const std::string &name, RenderPass &render_pass);
    
    enum Loaded {
        EFFECTS         = 1<<0,
        ACTORS          = 1<<1,
        MODELS          = 1<<2,
        RENDER_PASSES   = 1<<3,
        PIPELINES       = 1<<4,
        EVERYTHING      = EFFECTS|ACTORS|MODELS|RENDER_PASSES|PIPELINES
    };
    int loaded_bitflags_;
    std::unordered_map<std::string,RenderPass> render_passes_;
    std::unordered_map<std::string,Effect> effects_;
    std::unordered_map<std::string,Model> models_;
    std::unordered_map<std::string,Actor> actors_;
    std::vector<Pipeline> pipelines_;
};
}}