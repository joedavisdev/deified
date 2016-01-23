#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

#include "gfx_device.h"
#include "glm/glm.hpp"

namespace JMD {
class ParsedEffect;
class ParsedActor;
class ParsedRenderPass;
namespace Core {
static const unsigned int g_circular_buffer_size = 2;
// Forward declarations
class CommandBuffer;
class RenderPass;
// Structs & classes
struct Effect {
    std::string frag_shader_name;
    std::string vert_shader_name;
    GFX::Effect gfx_effect;
    std::vector<std::string> uniform_block_names;
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
    bool InitializeGFX();
    void ReleaseData();
private:
    unsigned int    num_vertices_;
    unsigned int    stride_;
    unsigned int    num_indices_;
    unsigned int    num_indices_bytes_;
    char*           vertices_;
    char*           indices_;
    GFX::Buffer     vertex_buffer_;
    GFX::Buffer     index_buffer_;
    bool            local_data_active_;
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
    glm::vec4 rotation;
};
struct Actor {
    PhysicsBody body;
    Model* model_ptr;
    Effect* effect_ptr;
    Actor():model_ptr(nullptr),effect_ptr(nullptr){}
};
typedef std::shared_ptr<Actor> ActorSPtr;
typedef std::weak_ptr<Actor> ActorWPtr;
struct Pipeline {
    Effect* effect_ptr;
    RenderPass* render_pass_ptr;
    GFX::PipelineState gfx_pipeline;
};
struct Draw {
    struct CircularGFXBuffer{
        GFX::Buffer buffer[g_circular_buffer_size];
    };
    ActorSPtr actor_sp;
    Pipeline* pipeline_ptr;
    std::vector<CircularGFXBuffer> uniform_buffers;
    Draw():pipeline_ptr(nullptr){}
};
struct CommandBuffer {
    GFX::CommandBuffer cb;
    std::vector<Draw> draws;
};
struct Camera {
    glm::mat4x4 view;
    glm::mat4x4 projection;
};
struct RenderPass {
    RenderPass():sample_count(1){}
    std::string actor_regex;
    std::vector<ActorSPtr> actor_ptrs;
    Camera camera; // TODO: Add a mechanism for the app to update these matrices
    unsigned int sample_count;
    std::vector<GFX::PixelFormat> colour_formats;
    GFX::PixelFormat depth_stencil_formats;
    std::vector<CommandBuffer> command_buffers;
};
// std::function declarations
typedef std::function<void(
    const std::string& block_name,
    const Camera& camera,
    const PhysicsBody& body,
    GFX::Buffer& gfx_buffer)>
UniformUpdateCallback;
    
class SceneMan {
public:
    SceneMan();
    ~SceneMan();
    void Load(const std::string& scene_json_name);
    std::vector<ActorWPtr> GetActorPtrs(std::string regex_string);
    void SetUniformUpdateFn(UniformUpdateCallback callback);
    void Bake();
    void Update(const unsigned int circular_buffer_index);
    void Draw();
private:
    void LoadEffects(const std::vector<ParsedEffect>& parsed_effects);
    void LoadActors(const std::vector<ParsedActor>& parsed_actors);
    void LoadRenderPasses(const std::vector<ParsedRenderPass>& parsed_render_passes);
    void ReleaseData();
    void BuildPipelines();
    void BuildPipeline(Effect &effect, RenderPass &render_pass);
    Pipeline* FindPipeline(const Effect &effect, const RenderPass &render_pass);
    void BuildCommandBuffers(RenderPass &render_pass);
    void BakeEffects();
    void BakePipelines();
    void BakeCommandBuffers();
    
    enum Stage {
        EFFECTS         = 1<<0,
        ACTORS          = 1<<1,
        MODELS          = 1<<2,
        RENDER_PASSES   = 1<<3,
        PIPELINES       = 1<<4,
        COMMAND_BUFFERS = 1<<5,
        ALL_LOADED      = EFFECTS|ACTORS|MODELS|RENDER_PASSES|PIPELINES,
        ALL_BAKED       = EFFECTS|PIPELINES|COMMAND_BUFFERS
    };
    int loaded_bitflags_;
    int baked_bitflags_;
    std::unordered_map<std::string,RenderPass> render_passes_;
    std::unordered_map<std::string,Effect> effects_;
    std::unordered_map<std::string,Model> models_;
    std::unordered_map<std::string,ActorSPtr> actors_;
    std::vector<Pipeline> pipelines_;
    GFX::Library gfx_default_library_;
    UniformUpdateCallback UniformUpdateFn;
};
}}