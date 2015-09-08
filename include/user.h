#ifndef __USER_H__
#define __USER_H__
#include <vector>
struct Session{
    double start;
    double end;
};

class User {
  public:
    const vector<Session> &get_sessions() const{
        return _visit_session;
    }
    const long id() const{
        return user_id;
    }
  private:
    long user_id;
    vector<Session> _visit_session;
};
typedef std::vector<user> UserArray;
#endif
