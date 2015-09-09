#ifndef __USER_H__
#define __USER_H__
#include <vector>
#include <unordered_map>
#include "comm.h"
struct Session {
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
    void add_impr(long t){
        if(same_session(t)){
            _visit_session.back().end = t;
        }else{
            _visit_session.push_back({t, t});
        }
    }
  private:
    bool same_session(long t){
        if(_visit_session.empty())
            return false;
        if(_visit_session.back().end + SESSION_MAX_STOP > t){
            return true;
        }
        return false;
    }
  private:
    long user_id;
    std::vector<Session> _visit_session;

};
typedef std::unordered_map<long, User> UserContainer;
#endif
