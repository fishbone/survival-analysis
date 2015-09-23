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
    Time(){}
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
    int days() const {
        return seconds() / 60 / 60 / 24;
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
struct ReadInfo {
    ReadInfo(long id, long t):articleId(id),
                              stayTime(t){}
    long articleId;
    long stayTime;
};
struct Apps{
    double date;
    int app_id;
};

struct UserFeature {
    typedef std::pair<int, int> Feature;
    std::vector<Feature> features;
};

struct Session {
    int binFromLastSession() const{
        if(bin < NUM_BIN){
            return bin;    
        }else{
            return NUM_BIN - 1;    
        }
    }
    Session(Time s, Time e, Session *p, UserFeature *u):
            start(s),
            end(e){
        if(p == nullptr)
            bin = -1;
        else
            bin = (start.hours() - p->end.hours()) / BIN_WIDTH;
        user_info = u;
    }
    Time start;
    Time end;
    int bin;
    std::vector<ReadInfo> articles;
    UserFeature *user_info;
};

class User {
  public:
    User(){}
    User(long id):_user_id(id){}
    const std::vector<Session> &get_sessions() const{
        return _visit_session;
    }
    const long id() const{
        return _user_id;
    }

    Session &add_session(long id,
                         long start,
                         long end,
                         const char *session_date){
        // For last session
        Session *p = _visit_session.size() == 0 ? nullptr : &_visit_session.back();
        UserFeature *u = _user_info.count(session_date) == 0 ?
                nullptr :
                _user_info[session_date];
        _visit_session.push_back({Time(start), Time(end), p, u});
        return _visit_session.back();
    }

    void add_userinfo(const char*date, UserFeature *u){
        _user_info[date] = u;
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
    long _user_id;
    std::vector<Session> _visit_session;
    std::unordered_map<std::string, UserFeature*> _user_info;
};
typedef std::unordered_map<long, User> UserContainer;
#endif
