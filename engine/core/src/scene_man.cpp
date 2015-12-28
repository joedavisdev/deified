#include "scene_man.hpp"

#include <cassert>
#include <regex>
#include <unordered_map>

#include "scene_parser.hpp"
#include "PVRTResourceFile.h"
#include "PVRTModelPOD.h"

namespace JMD {
#pragma mark SMMesh: Public functions
SMMesh::SMMesh()
:
num_vertices_(0),
stride_(0),
num_indices_(0),
num_indices_bytes_(0),
vertices_(nullptr),
indices_(nullptr)
{}
SMMesh::SMMesh(char* vertices,
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
}
SMMesh::~SMMesh() {
}
void SMMesh::ReleaseData() {
    delete[] vertices_;vertices_=nullptr;
    delete[] indices_;indices_=nullptr;
}
#pragma mark Model: Public functions
Model::Model()
:
number_of_meshes(0),
mesh_array(nullptr)
{}
Model::~Model(){
}
void Model::ReleaseData() {
    for(unsigned int index=0;index<number_of_meshes;index++) {
        mesh_array[index].ReleaseData();
    }
    delete []mesh_array;
}
#pragma mark SceneMan: Public functions
SceneMan::~SceneMan(){
    this->ReleaseData();
}
void SceneMan::Load(const std::string& scene_json_name) {
    this->ReleaseData();
    std::unordered_map<std::string,CPVRTModelPOD> pod_map;
    CPVRTResourceFile scene_json(scene_json_name.c_str());
    SceneParser parser;parser.Parse(std::string((char *)scene_json.DataPtr()));
    // Load PODs
    for(const auto &parsed_actor: parser.actors) {
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
        model.number_of_meshes = pod.nNumMesh;
        model.mesh_array = new SMMesh[model.number_of_meshes];
        for(unsigned int index=0;index < pod.nNumMesh; index++){
            SPODMesh &pod_mesh(pod.pMesh[index]);
            SMMesh &mesh(model.mesh_array[index]);
            mesh = SMMesh((char*)pod_mesh.pInterleaved,
                          pod_mesh.nNumVertex,
                          pod_mesh.sVertex.nStride,
                          (char*)pod_mesh.sFaces.pData,
                          PVRTModelPODCountIndices(pod_mesh),
                          pod_mesh.sFaces.nStride);
        }
        models_.insert({pod_key_value.first,std::move(model)});
    }
    // Load actors
    for(const auto &parsed_actor: parser.actors) {
        Actor actor;
        auto map_model(models_.find(parsed_actor.model_name));
        if(map_model == models_.end()) {
            assert(0); // Model should exist
        }
        actor.model = &map_model->second;
        actor.body.position = parsed_actor.world_position;
        actors_.insert({parsed_actor.name,std::move(actor)});
    }
    // Load render passes
    for(const auto &parsed_render_pass: parser.render_passes) {
        RenderPass render_pass;
        render_pass.actor_regex_ = parsed_render_pass.actor_regex;
        render_pass.actor_ptrs_ = this->GetActorPtrs(render_pass.actor_regex_);
        for(const auto &attachment_type: parsed_render_pass.colour_formats) {
            RenderAttachment colour_attachment;
            if(colour_attachment.set_pixel_format(attachment_type)) {
                render_pass.colour_attachments_.push_back(std::move(colour_attachment));
            }else{
                printf("TODO: Render pass %s:\n\tLoad colour attachment\n",parsed_render_pass.name.c_str());
//                assert(0);
            }
        }
        RenderAttachment depth_stencil_attachment;
        if(depth_stencil_attachment.set_pixel_format(parsed_render_pass.depth_stencil_formats)) {
//            render_pass.depth_stencil_attachment_ = std::move(depth_stencil_attachment);
        }else{
            printf("TODO: Render pass %s:\n\tLoad depth+stencil attachment\n",parsed_render_pass.name.c_str());
//            assert(0);
        }
        render_passes_.insert({parsed_render_pass.name,std::move(render_pass)});
    }
    assert(0);
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
}
void SceneMan::BakeRenderPass(unsigned int index) {
    assert(0);
}
}