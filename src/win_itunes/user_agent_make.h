#ifndef WIN_ITUNES_USER_AGENT_MAKE_H_
#define WIN_ITUNES_USER_AGENT_MAKE_H_

#include <vector>
#include <string>

namespace FromIOS10{
  class UserAgentMake
  {
  public:
    UserAgentMake();
    ~UserAgentMake();
    const std::string AppStored();
    const std::string AppStore();
    const std::string iTunesStored();
  private:
    void add(const std::string& agent_value);
    void add(const std::string& agent_key, const std::string& agent_value);
    void reset();
    const std::string Get(bool is_space_separate);
    std::vector<std::string> agent_vec_;
    std::vector<std::pair <std::string, std::string>> agent_map_;
  };
  const std::string AppStored();
  const std::string AppStore();
  const std::string iTunesStored();
}

#endif
