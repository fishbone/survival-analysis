#ifndef __USER_H__
#define __USER_H__
#include <vector>
#include <unordered_map>
#include "comm.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
struct Time {
  public:
    Time(long t):_time(t){}
    long day() const{
        return _time / 60 / 60 / 24;
    }
    long seconds() const{
        return _time;
    }
  private:
    long _time;
    friend class User;
};
struct Session {
    Time start;
    Time end;
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
            _visit_session.back().end._time = t;
        }else{
            _visit_session.push_back({Time(t), Time(t)});
        }
    }
  private:
    bool same_session(long t){
        if(_visit_session.empty())
            return false;
        if(_visit_session.back().end._time + SESSION_MAX_STOP > t){
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
