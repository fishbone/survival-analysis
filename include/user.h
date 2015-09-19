#ifndef __USER_H__
#define __USER_H__
#include <vector>
#include <unordered_map>
#include "comm.h"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
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
    double hourOfDay() const {
        return (seconds() % (24 * 60 * 60)) / (60.0 * 60.0);
    }
    double hours() const {
        return seconds() / 60.0 / 60.0;
    }
    void setTime(long t){
        _time = boost::posix_time::from_time_t(t);        
    }
  private:
    long seconds() const{
        return (_time - _time_t_epoch).total_seconds();
    }
    boost::posix_time::ptime _time;
    static const boost::posix_time::ptime _time_t_epoch;
    friend class User;
};

struct Session {
    int binFromLastSession() const{
        if(lastSession == nullptr){
            return -1;
        }
        int bin = (start.hours() - lastSession->end.hours()) / BIN_WIDTH;
	if (bin < 0)
	    std::cout<<"user.h: "<<start.hours()<<" "<<lastSession->end.hours()<<std::endl;
        if(bin < NUM_BIN){
            return bin;    
        }else{
            return NUM_BIN - 1;    
        }
    }
    Session(Time s, Time e, Session *p):
            start(s),
            end(e),
            lastSession(p){}
    Time start;
    Time end;
    Session *lastSession;
};

class User {
  public:
    User(){}
    User(long uid):user_id(uid){}
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
            Session *p = _visit_session.size() == 0 ? nullptr : &_visit_session.back();
            _visit_session.push_back({Time(t), Time(t), p});
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
