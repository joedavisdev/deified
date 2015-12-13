#include "scene_man.hpp"

#include <cassert>
#include <regex>

namespace JMD {
#pragma mark Public functions
    void SceneMan::Load(std::string scene_json) {
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
    void BakeRenderPass(unsigned int index) {
        assert(0);
    }
}