#include "user_agent_make.h"

namespace FromIOS10{
  static UserAgentMake make;
  UserAgentMake::UserAgentMake(){
    reset();
  }
  UserAgentMake::~UserAgentMake(){
    reset();
  }
  const std::string UserAgentMake::AppStored(){
    reset();
    add("com.apple.appstored", "1.0");
    add("iOS", "10.2");
    add("model", "iPhone6,1");
    add("hwp", "s5l8960x");
    add("build", "14C92");
    add("(6; dt:89)");
    return Get(true);
  }
  const std::string UserAgentMake::AppStore(){
    reset();
    add("AppStore", "2.0");
    add("iOS", "10.2");
    add("model", "iPhone6,1");
    add("hwp", "s5l8960x");
    add("build", "14C92");
    add("(6; dt:89)");
    return Get(true);
  }
  const std::string UserAgentMake::iTunesStored(){
    reset();
    add("itunesstored", "1.0");
    add("iOS", "10.2");
    add("model", "iPhone6,1");
    add("hwp", "s5l8960x");
    add("build", "14C92");
    add("(6; dt:89)");
    return Get(true);
  }
  void UserAgentMake::add(const std::string& agent_value){
    agent_vec_.push_back(agent_value);
  }
  void UserAgentMake::add(const std::string& agent_key, const std::string& agent_value){
    agent_map_.push_back(std::make_pair(agent_key, agent_value));
  }
  void UserAgentMake::reset(){
    agent_vec_.resize(0);
    agent_map_.clear();
  }
  const std::string UserAgentMake::Get(bool is_space_separate){
    std::string result;
    std::vector<std::pair <std::string, std::string>>::iterator it_map = agent_map_.begin();
    std::vector<std::string>::iterator it_vec = agent_vec_.begin();
    for (it_map = agent_map_.begin(); it_map != agent_map_.end(); it_map++){
      if (it_map->first.size() && it_map->second.size()){
        result.append(it_map->first);
        result.append("/");
        result.append(it_map->second);
        if (is_space_separate){
          result.append(" ");
        }
      }
    }
    for (it_vec = agent_vec_.begin(); it_vec != agent_vec_.end(); it_vec++){
      if (it_vec->size()){
        result.append(*it_vec);
        if (is_space_separate){
          result.append(" ");
        }
      }
    }
    return result;
  }
  const std::string AppStored(){
    std::string result = make.AppStored();
    return result;
  }
  const std::string AppStore(){
    std::string result = make.AppStore();
    return result;
  }
  const std::string iTunesStored(){
    std::string result = make.iTunesStored();
    return result;
  }
}