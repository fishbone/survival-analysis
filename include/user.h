#ifndef __USER_H__
#define __USER_H__
#include <vector>
#include <unordered_map>
struct Session{
    long start;
    long end;
};

class User {
  public:
    const std::vector<Session> &get_sessions() const{
        return _visit_session;
    }
    const long id() const{
        return user_id;
    }
  private:
    long user_id;
    std::vector<Session> _visit_session;
};
typedef std::unordered_map<long, User> UserArray;
#endif
