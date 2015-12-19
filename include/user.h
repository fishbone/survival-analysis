#ifndef __USER_H__
#define __USER_H__
#include <vector>
#include <unordered_map>
#include "feature.h"
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
  boost::gregorian::date date() const{
    return _time.date();
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
        return seconds() / 60.0 / 60.0 / 24.0;
    }
    double hourOfDay() const {
        return (seconds() % (24 * 60 * 60)) / (60.0 * 60.0);
    }
    double hours() const {
        double r = seconds();
        r /= 3600.0;
        return r;
    }
    void setTime(long t){
        _time = boost::posix_time::from_time_t(t);        
    }
    long seconds() const{
        return (_time - _time_t_epoch).total_seconds();
    }

  private:
    boost::posix_time::ptime _time;
    static const boost::posix_time::ptime _time_t_epoch;
    friend class User;
};

struct Session {
    int binFromLastSession() const{
        int bin = -1;
        if(idx != 0){
            bin = ((*session_list)[idx].start.hours() -
                   (*session_list)[idx - 1].end.hours()) / BIN_WIDTH;
        }
        
        if(bin < NUM_BIN){
            return bin;    
        }else{
            return NUM_BIN - 1;    
        }
    }

    Session(Time s,
            Time e,
            std::vector<Session> *sess_list,
            std::vector<Feature> *u):
            start(s),
            end(e),
            session_list(sess_list){
        day_features = u;
        idx = sess_list->size();
    }

    Session *lastSession(){
        if(idx == 0) return nullptr;
        return &(*session_list)[idx];
    }
            
    Time start;
    Time end;
    size_t idx;
    std::vector<Session> *session_list;
    std::vector<Feature> session_features;
    std::vector<Feature> *day_features;
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
    void append_session(const Session& session){
      _visit_session.push_back(session);
    }

    Session &add_session(long id,
                         long start,
                         long end,
                         const char *session_date){
        Session *p = _visit_session.size() == 0 ? nullptr : &_visit_session.back();
        std::vector<Feature> *u = &(_user_info[session_date]);
        _visit_session.push_back({
                Time(start),
                        Time(end),
                        &_visit_session,
                        u});
        if(_visit_session.back().binFromLastSession() < -1){
            std::cerr<<id<<" "<<start<<std::endl;
            assert(false);
        }
        return _visit_session.back();
    }

    void add_feature(const char*date, Feature u){
        _user_info[date].push_back(u);
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
    std::unordered_map<std::string, std::vector<Feature> > _user_info;
};
typedef std::unordered_map<long, User> UserContainer;
#endif
