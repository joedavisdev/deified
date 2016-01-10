#include "scene_man.hpp"

#include <cassert>
#include <regex>
#include <unordered_map>

#include "scene_parser.hpp"
#include "shader_blocks_map.hpp"
#include "PVRTResourceFile.h"
#include "PVRTModelPOD.h"

namespace JMD {
namespace Core {
#pragma mark Mesh: Public functions
Mesh::Mesh()
:
num_vertices_(0),
stride_(0),
num_indices_(0),
num_indices_bytes_(0),
vertices_(nullptr),
indices_(nullptr)
{}
Mesh::Mesh(char* vertices,
   unsigned int num_vertices,
   unsigned int stride,
   char* indices,
   unsigned int num_indices,
   unsigned int num_indices_bytes)
:
num_vertices_(num_vertices),
stride_(stride),
num_indices_(num_indices),
num_indices_bytes_(num_indices_bytes)
{
    vertices_ = new char[num_vertices];
    memcpy(vertices_,vertices,num_vertices);
    indices_ = new char[num_indices];
    memcpy(indices_,indices,num_indices);
    local_data_active_ = true;
}
bool Mesh::InitializeGFX() {
    if(!local_data_active_) return false;
    vertex_buffer_.Initialise(vertices_, num_vertices_);
    index_buffer_.Initialise(indices_, num_indices_);
    return true;
}
Mesh::~Mesh() {
}
void Mesh::ReleaseGFXData() {
    vertex_buffer_.Release();
    index_buffer_.Release();
}
void Mesh::ReleaseLocalData() {
    delete[] vertices_;vertices_=nullptr;
    delete[] indices_;indices_=nullptr;
    local_data_active_ = false;
}
void Mesh::ReleaseData() {
    this->ReleaseGFXData();
    this->ReleaseLocalData();
}
#pragma mark Model: Public functions
Model::Model()
:
number_of_meshes_(0),
mesh_array_(nullptr)
{}
Model::~Model(){
}
void Model::ReleaseData() {
    for(unsigned int index=0;index<number_of_meshes_;index++) {
        mesh_array_[index].ReleaseData();
    }
    delete []mesh_array_;
}
#pragma mark SceneMan: Public functions
SceneMan::SceneMan():
loaded_bitflags_(0),
baked_bitflags_(0){
    UniformUpdateFn = [](const std::string&, const Camera&, const PhysicsBody&, GFX::Buffer&){
        printf("ERROR: Please provide a custom function to update uniform blocks\n");assert(0);
    };
}
SceneMan::~SceneMan(){
    this->ReleaseData();
}
void SceneMan::SetUniformUpdateFn(UniformUpdateCallback callback) {
    this->UniformUpdateFn = callback;
}
void SceneMan::LoadEffects(const std::vector<ParsedEffect>& parsed_effects) {
    // Load effects
    for(const auto &parsed_effect: parsed_effects) {
        Effect effect;
        effect.frag_shader_name = parsed_effect.frag_shader_name;
        effect.vert_shader_name = parsed_effect.vert_shader_name;
        effect.uniform_block_names = parsed_effect.uniform_block_names;
        effects_.insert({parsed_effect.name,std::move(effect)});
    }
    loaded_bitflags_ |= Stage::EFFECTS;
}
void SceneMan::LoadActors(const std::vector<ParsedActor>& parsed_actors) {
    std::unordered_map<std::string,CPVRTModelPOD> pod_map;
    // Load PODs
    for(const auto &parsed_actor: parsed_actors) {
        // Search the POD map
        CPVRTModelPOD* pod_ptr(nullptr);
        for(auto &pod_key_value: pod_map) {
            if(parsed_actor.model_name.find(pod_key_value.first) != std::string::npos) {
                pod_ptr = &pod_key_value.second;
                break;
            }
        }
        // POD isn't cached, so load it!
        if(pod_ptr == nullptr) {
            auto pod_iter(pod_map.insert({parsed_actor.model_name,std::move(CPVRTModelPOD())}));
            CPVRTModelPOD &pod(pod_iter.first->second);
            pod.ReadFromFile(parsed_actor.model_name.c_str());
        }
    }
    // Load models
    for(const auto &pod_key_value: pod_map) {
        Model model;
        const CPVRTModelPOD& pod(pod_key_value.second);
        model.set_number_of_meshes(pod.nNumMesh);
        model.set_mesh_array(new Mesh[model.number_of_meshes()]);
        for(unsigned int index=0;index < pod.nNumMesh; index++){
            SPODMesh &pod_mesh(pod.pMesh[index]);
            Mesh &mesh(model.mesh_array()[index]);
            mesh = Mesh((char*)pod_mesh.pInterleaved,
                        pod_mesh.nNumVertex,
                        pod_mesh.sVertex.nStride,
                        (char*)pod_mesh.sFaces.pData,
                        PVRTModelPODCountIndices(pod_mesh),
                        pod_mesh.sFaces.nStride);
            mesh.InitializeGFX();
        }
        models_.insert({pod_key_value.first,std::move(model)});
    }
    loaded_bitflags_ |= Stage::MODELS;
    // Load actors
    for(const auto &parsed_actor: parsed_actors) {
        Actor actor;
        auto map_model(models_.find(parsed_actor.model_name));
        if(map_model == models_.end()) {
            assert(0); // Model should exist
        }
        actor.model_ptr = &map_model->second;
        auto map_effect(effects_.find(parsed_actor.effect_name));
        if(map_effect == effects_.end()) {
            assert(0); // Model should exist
        }
        actor.effect_ptr = &map_effect->second;
        actor.body.position = parsed_actor.world_position;
        actors_.insert({parsed_actor.name,std::move(actor)});
    }
    loaded_bitflags_ |= Stage::ACTORS;
}
void SceneMan::LoadRenderPasses(const std::vector<ParsedRenderPass>& parsed_render_passes) {
    // Load render passes
    for(const auto &parsed_render_pass: parsed_render_passes) { 
        RenderPass render_pass;
        render_pass.actor_regex = parsed_render_pass.actor_regex;
        render_pass.actor_ptrs = this->GetActorPtrs(render_pass.actor_regex);
        for(const auto &attachment_format: parsed_render_pass.colour_formats) {
            GFX::PixelFormat colour_attachment;
            if(colour_attachment.Initialize(attachment_format)) {
                render_pass.colour_formats.push_back(std::move(colour_attachment));
            }else{
                printf("ERROR: Failed to load colour_formats = %s\n", attachment_format.c_str());
                assert(0);
            }
        }
        GFX::PixelFormat depth_stencil_formats;
        if(depth_stencil_formats.Initialize(parsed_render_pass.depth_stencil_formats)) {
            render_pass.depth_stencil_formats = std::move(depth_stencil_formats);
        }else{
            printf("ERROR: Failed to load depth_stencil_formats = %s\n", parsed_render_pass.depth_stencil_formats.c_str());
            assert(0);
        }
        render_passes_.insert({parsed_render_pass.name,std::move(render_pass)});
    }
    loaded_bitflags_ |= Stage::RENDER_PASSES;
}
void SceneMan::Load(const std::string& scene_json_name) {
    this->ReleaseData();
    CPVRTResourceFile scene_json(scene_json_name.c_str());
    SceneParser parser;parser.Parse(std::string((char *)scene_json.DataPtr()));
    this->LoadEffects(parser.effects);
    this->LoadActors(parser.actors);
    this->LoadRenderPasses(parser.render_passes);
    
    this->BuildPipelines();
    for (auto &render_pass: render_passes_) {
        this->BuildCommandBuffers(render_pass.second);
    }
    gfx_default_library_.Initialize("");
}
void SceneMan::BakeEffects(){
    assert((loaded_bitflags_ & Stage::EFFECTS) != 0);
    for(auto& effect_iter:effects_) {
        Effect& effect(effect_iter.second);
        effect.gfx_effect.Initialize(gfx_default_library_, effect.vert_shader_name, effect.frag_shader_name);
    }
    baked_bitflags_ |= Stage::EFFECTS;
}
void SceneMan::BakePipelines(){
    assert((loaded_bitflags_ & Stage::PIPELINES) != 0);
    assert((baked_bitflags_ & Stage::EFFECTS) != 0);
    for(auto& pipeline:pipelines_) {
        GFX::PipelineDesc gfx_pipeline_desc;
        Effect& effect(*pipeline.effect_ptr);
        const RenderPass& render_pass(*pipeline.render_pass_ptr);
        gfx_pipeline_desc.Initialize(effect.gfx_effect,
                               render_pass.sample_count,
                               render_pass.colour_formats,
                               render_pass.depth_stencil_formats,
                               GFX::PixelFormat());
        pipeline.gfx_pipeline.Initialize(gfx_pipeline_desc);
    }
    baked_bitflags_ |= Stage::PIPELINES;
}
void SceneMan::BakeCommandBuffers() {
    for(auto& render_pass_iter:render_passes_){
        for(auto& comand_buffer:render_pass_iter.second.command_buffers) {
            GFX::CommandBuffer& cb(comand_buffer.cb);
            for(auto& draw:comand_buffer.draws){
            }
        }
    }
    assert(0);
    baked_bitflags_ |= Stage::COMMAND_BUFFERS;
}
void SceneMan::Bake(){
    assert(loaded_bitflags_ == Stage::ALL_LOADED);
    this->BakeEffects();
    this->BakePipelines();
    this->BakeCommandBuffers();
    assert(baked_bitflags_ == Stage::ALL_BAKED);
}
void SceneMan::Update() {
    assert(0);
}
void SceneMan::Draw() {
    assert(0);
}
std::vector<Actor*> SceneMan::GetActorPtrs(std::string regex_string) {
    std::regex expression(regex_string);
    std::vector<Actor*> actor_matches;
    
    for(auto &actor_iter : actors_) {
        if(std::regex_match(actor_iter.first,expression)) {
            actor_matches.push_back(&actor_iter.second);
        }
    }
    return actor_matches;
}
#pragma mark SceneMan: Private functions
void SceneMan::ReleaseData() {
    for(auto &model: models_){
        model.second.ReleaseData();
    }
    render_passes_.clear();
    for(auto& effect_iter: effects_) {
        effect_iter.second.gfx_effect.Release();
    }
    effects_.clear();
    models_.clear();
    actors_.clear();
    pipelines_.clear();
    gfx_default_library_.Release();
    baked_bitflags_ = loaded_bitflags_ = 0;
}
void SceneMan::BuildPipelines() {
    for(auto& render_pass_iter:render_passes_) {
        RenderPass& render_pass(render_pass_iter.second);
        for(auto actor_ptr:render_pass.actor_ptrs) {
            this->BuildPipeline(*actor_ptr->effect_ptr,render_pass);
        }
    }
    loaded_bitflags_ |= Stage::PIPELINES;
}
void SceneMan::BuildPipeline(Effect &effect, RenderPass &render_pass) {
    Pipeline* pipeline_ptr(this->FindPipeline(effect,render_pass));
    if(pipeline_ptr == nullptr) {
        Pipeline pipeline;
        pipeline.effect_ptr = &effect;
        pipeline.render_pass_ptr = &render_pass;
        pipelines_.push_back(std::move(pipeline));
    }
}
Pipeline* SceneMan::FindPipeline(const Effect &effect, const RenderPass &render_pass) {
    Pipeline* pipeline_ptr(nullptr);
    for(auto &pipeline:pipelines_) {
        if(pipeline.effect_ptr == &effect && pipeline.render_pass_ptr == &render_pass) {
            pipeline_ptr = &pipeline;
            break;
        }
    }
    return pipeline_ptr;
}
void SceneMan::BuildCommandBuffers(RenderPass &render_pass) {
    assert(loaded_bitflags_ == Stage::ALL_LOADED);
    CommandBuffer command_buffer; // NOTE: Currently limited to one command buffer per render pass
    // Create draws
    for(auto actor_ptr:render_pass.actor_ptrs) {
        assert(actor_ptr != nullptr);
        struct Draw draw;
        draw.actor_ptr = actor_ptr;
        // Find pipeline
        Effect& effect(*draw.actor_ptr->effect_ptr);
        Pipeline* pipeline_ptr(this->FindPipeline(effect, render_pass));
        assert(pipeline_ptr != nullptr);
        draw.pipeline_ptr = pipeline_ptr;
        for(const auto& block_name:effect.uniform_block_names) {
            const unsigned int uniform_block_size(uniform_block_sizes.find(block_name)->second);
            Draw::CircularGFXBuffer circular;
            for (unsigned int index = 0;index < g_num_uniform_buffers;index++){
                circular.buffer[index].Initialise(nullptr, uniform_block_size);
            }
            draw.uniform_buffers.push_back(std::move(circular));
        }
        command_buffer.draws.push_back(std::move(draw));
    }
    // TODO: Sort draws by pipeline
    render_pass.command_buffers.push_back(std::move(command_buffer));
}
}}