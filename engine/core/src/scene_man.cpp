#include "scene_man.hpp"

#include <cassert>
#include <regex>
#include <unordered_map>

#include "scene_parser.hpp"
#include "PVRTResourceFile.h"
#include "PVRTModelPOD.h"

namespace JMD {
#pragma mark Public functions
    SceneMan::~SceneMan(){
        ReleaseData();
    }
    void SceneMan::Load(const std::string& scene_json_name) {
        ReleaseData();
        std::unordered_map<std::string,CPVRTModelPOD> pod_map;
        CPVRTResourceFile scene_json(scene_json_name.c_str());
        SceneParser parser;parser.Parse(std::string((char *)scene_json.DataPtr()));
        // Load PODs
        for(const auto &parsed_actor: parser.actors) {
            // Search the POD cache
            CPVRTModelPOD* pod_ptr(nullptr);
            for(auto &pod_key_value: pod_map) {
                if(parsed_actor.model_name.find(pod_key_value.first) != std::string::npos) {
                    pod_ptr = &pod_key_value.second;
                    break;
                }
            }
            // POD isn't cached, so load it!
            if(pod_ptr == nullptr) {
                pod_map.insert({parsed_actor.model_name,CPVRTModelPOD()});
                // TODO: Opimize. There must be a more efficient way to do this!
                for(auto &pod_key_value: pod_map) {
                    if(parsed_actor.model_name.find(pod_key_value.first) != std::string::npos) {
                        CPVRTModelPOD& pod(pod_key_value.second);
                        pod.ReadFromFile(parsed_actor.model_name.c_str());
                        break;
                    }
                }
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
            models_.push_back(std::move(model));
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
        
        for(auto &actor : actors_) {
            if(std::regex_match(actor.name,expression)) {
                actor_matches.push_back(&actor);
            }
        }
        return actor_matches;
    }
#pragma mark Private functions
void SceneMan::ReleaseData() {
    for(auto &model: models_){
        for(unsigned int index=0;index<models_.size();index++) {
            SMMesh &mesh(model.mesh_array[index]);
            mesh.ReleaseData();
        }
    }
}
void SceneMan::BakeRenderPass(unsigned int index) {
    assert(0);
}
}