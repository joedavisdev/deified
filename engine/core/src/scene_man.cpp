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
void Mesh::ReleaseData() {
    delete[] vertices_;vertices_=nullptr;
    delete[] indices_;indices_=nullptr;
    local_data_active_ = false;
}
#pragma mark Model: Public functions
Model::Model()
:
number_of_meshes_(0)
{}
void Model::ReleaseData() {
    for(auto& mesh_sp:mesh_array_) {
        mesh_sp->ReleaseData();
    }
    mesh_array_.clear();
}
void Model::InitializeMeshArray(const unsigned int size) {
    mesh_array_.clear();
    mesh_array_.resize(size);
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
        effects_.insert({parsed_effect.name,std::move(std::make_shared<Effect>(effect))});
    }
    loaded_bitflags_ |= Stage::EFFECTS;
}
void SceneMan::LoadActors(const std::vector<ParsedActor>& parsed_actors) {
    std::unordered_map<std::string,CPVRTModelPOD> pod_map;
    // Load PODs
    for(const auto &parsed_actor: parsed_actors) {
        // Search the POD map
        CPVRTModelPOD* pod_ptr(nullptr);
        for(auto& pod_key_value: pod_map) {
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
        model.InitializeMeshArray(pod.nNumMesh);
        for(unsigned int index=0;index < pod.nNumMesh; index++){
            SPODMesh &pod_mesh(pod.pMesh[index]);
            auto& mesh(model.mesh_array().at(index));
            mesh = std::make_shared<Mesh>(Mesh((char*)pod_mesh.pInterleaved,
                        pod_mesh.nNumVertex,
                        pod_mesh.sVertex.nStride,
                        (char*)pod_mesh.sFaces.pData,
                        PVRTModelPODCountIndices(pod_mesh),
                        pod_mesh.sFaces.nStride));
            mesh->InitializeGFX();
        }
        models_.insert({pod_key_value.first,std::move(std::make_shared<Model>(model))});
    }
    loaded_bitflags_ |= Stage::MODELS;
    // Load actors
    for(const auto &parsed_actor: parsed_actors) {
        Actor actor;
        auto map_model(models_.find(parsed_actor.model_name));
        if(map_model == models_.end()) {
            assert(0); // Model should exist
        }
        actor.model_sp = map_model->second;
        auto map_effect(effects_.find(parsed_actor.effect_name));
        if(map_effect == effects_.end()) {
            assert(0); // Model should exist
        }
        actor.effect_sp = map_effect->second;
        actor.body.position = parsed_actor.world_position;
        actors_.insert({parsed_actor.name,std::move(std::make_shared<Actor>(actor))});
    }
    loaded_bitflags_ |= Stage::ACTORS;
}
void SceneMan::LoadRenderPasses(const std::vector<ParsedRenderPass>& parsed_render_passes) {
    // Load render passes
    for(const auto &parsed_render_pass: parsed_render_passes) { 
        RenderPass render_pass;
        render_pass.actor_regex = parsed_render_pass.actor_regex;
        auto actor_wp_array(this->GetActorPtrs(render_pass.actor_regex));
        for(auto& actor_wp:actor_wp_array) {
            render_pass.actor_sp_array.push_back(actor_wp.lock());
        }
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
        render_passes_.insert({parsed_render_pass.name,std::move(std::make_shared<RenderPass>(render_pass))});
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
        EffectSPtr& effect_sp(effect_iter.second);
        effect_sp->gfx_effect.Initialize(gfx_default_library_, effect_sp->vert_shader_name, effect_sp->frag_shader_name);
    }
    baked_bitflags_ |= Stage::EFFECTS;
}
void SceneMan::BakePipelines(){
    assert((loaded_bitflags_ & Stage::PIPELINES) != 0);
    assert((baked_bitflags_ & Stage::EFFECTS) != 0);
    for(auto& pipeline:pipelines_) {
        GFX::PipelineDesc gfx_pipeline_desc;
        Effect& effect(*pipeline.effect_sp);
        const RenderPass& render_pass(*pipeline.render_pass_sp);
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
        for(auto& comand_buffer:render_pass_iter.second->command_buffers) {
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
void SceneMan::Update(const unsigned int circular_buffer_index) {
    assert(circular_buffer_index < g_circular_buffer_size);
    // Update uniform buffers
    for(auto& render_pass_iter:render_passes_) {
        RenderPass& render_pass(*render_pass_iter.second);
        for(auto& command_buffer:render_pass.command_buffers) {
            for(auto& draw:command_buffer.draws) {
                const Effect& effect(*draw.actor_sp->effect_sp);
                for(unsigned int index=0;index < draw.uniform_buffers.size();index++){
                    const std::string& block_name(effect.uniform_block_names[index]);
                    GFX::Buffer& buffer(draw.uniform_buffers[index].buffer[circular_buffer_index]);
                    
                    UniformUpdateFn(block_name, render_pass.camera, draw.actor_sp->body, buffer);
                }
            }
        }
    }
    assert(0);
}
void SceneMan::Draw() {
    assert(0);
}
std::vector<ActorWPtr> SceneMan::GetActorPtrs(std::string regex_string) {
    std::regex expression(regex_string);
    std::vector<ActorWPtr> actor_matches;
    
    for(auto& actor_iter:actors_) {
        if(std::regex_match(actor_iter.first,expression)) {
            actor_matches.push_back(actor_iter.second);
        }
    }
    return actor_matches;
}
#pragma mark SceneMan: Private functions
void SceneMan::ReleaseData() {
    for(auto& model: models_){
        model.second->ReleaseData();
    }
    render_passes_.clear();
    effects_.clear();
    models_.clear();
    actors_.clear();
    pipelines_.clear();
    baked_bitflags_ = loaded_bitflags_ = 0;
}
void SceneMan::BuildPipelines() {
    for(auto& render_pass_iter:render_passes_) {
        for(auto& actor_sp:render_pass_iter.second->actor_sp_array) {
            this->BuildPipeline(actor_sp->effect_sp,render_pass_iter.second);
        }
    }
    loaded_bitflags_ |= Stage::PIPELINES;
}
void SceneMan::BuildPipeline(const EffectSPtr& effect_sp, const RenderPassSPtr& render_pass_sp) {
    Pipeline* pipeline_ptr(this->FindPipeline(effect_sp,render_pass_sp));
    if(pipeline_ptr == nullptr) {
        Pipeline pipeline;
        pipeline.effect_sp = effect_sp;
        pipeline.render_pass_sp = render_pass_sp;
        pipelines_.push_back(std::move(pipeline));
    }
}
Pipeline* SceneMan::FindPipeline(const EffectSPtr& effect_sp, const RenderPassSPtr& render_pass_sp) {
    Pipeline* pipeline_ptr(nullptr);
    for(auto& pipeline:pipelines_) {
        if(pipeline.effect_sp == effect_sp && pipeline.render_pass_sp == render_pass_sp) {
            pipeline_ptr = &pipeline;
            break;
        }
    }
    return pipeline_ptr;
}
void SceneMan::BuildCommandBuffers(RenderPassSPtr& render_pass_sp) {
    assert(loaded_bitflags_ == Stage::ALL_LOADED);
    CommandBuffer command_buffer; // NOTE: Currently limited to one command buffer per render pass
    // Create draws
    for(auto& render_pass_actor_sp:render_pass_sp->actor_sp_array) {
        struct Draw draw;
        draw.actor_sp = render_pass_actor_sp;
        // Find pipeline
        const EffectSPtr& effect_sp(draw.actor_sp->effect_sp);
        Pipeline* pipeline_ptr(this->FindPipeline(effect_sp, render_pass_sp));
        assert(pipeline_ptr != nullptr);
        draw.pipeline_ptr = pipeline_ptr;
        for(const auto& block_name:effect_sp->uniform_block_names) {
            const unsigned int uniform_block_size(uniform_block_sizes.find(block_name)->second);
            Draw::CircularGFXBuffer circular;
            for (unsigned int index = 0;index < g_circular_buffer_size;index++){
                circular.buffer[index].Initialise(nullptr, uniform_block_size);
            }
            draw.uniform_buffers.push_back(std::move(circular));
        }
        command_buffer.draws.push_back(std::move(draw));
    }
    // TODO: Sort draws by pipeline
    render_pass_sp->command_buffers.push_back(std::move(command_buffer));
}
}}