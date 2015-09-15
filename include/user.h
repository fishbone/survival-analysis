#ifndef __USER_H__
#define __USER_H__
#include <vector>
#include <unordered_map>
#include "comm.h"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
struct Time {
  public:
    Time(long t){
        setTime(t);
    }
    int dayOfYear() const {
        return _time.date().day_of_year();
    }
    int dayOfMonth() const {
        return _time.date().day();
    }

    int dayOfWeek() const {
        return _time.date().day_of_week();
    }
    
    long seconds() const{
        return (_time - _time_t_epoch).total_seconds();
    }
    double hours() const {
        return seconds() / 60.0 / 60.0;
    }
    void setTime(long t){
        _time = boost::posix_time::from_time_t(t);        
    }
  private:
    boost::posix_time::ptime _time;
    static const boost::posix_time::ptime _time_t_epoch;
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
            _visit_session.back().end.setTime(t);
        }else{
            _visit_session.push_back({Time(t), Time(t)});
        }
    }
  private:
    bool same_session(long t){
        if(_visit_session.empty())
            return false;
        if(_visit_session.back().end.seconds() + SESSION_MAX_STOP > t){
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
