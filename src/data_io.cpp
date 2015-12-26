#include "data_io.h"
#include "feature.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <utility>
#include <chrono>
#include <ctime>
#include <iostream>
#include <set>
#include <sstream>
#include <fstream>
#include <algorithm>
typedef bool (*read_handle)(std::istream &is, UserContainer &data, bool l);

// 1	20080109	1199870335	1199872791	7		639027	939178	957722	385049	607918	361210	640514
// 1	20080109	1199874607	1199878848	10		480941	24897	646083	623254	67779	432876	639027	939178	957722	385049
bool stay_handle_lastfm(std::istream &is,
			UserContainer &data,
			bool l){
    long uid, start_time, end_time;
    int arts;
    char read_date[128];
    is>>uid>>read_date>>start_time>>end_time>>arts;

    if(is.eof())
        return true;
    
    if(!data.count(uid)){
        data.insert(std::make_pair(uid, User(uid)));
    }

    Session &sess = data[uid].add_session(uid,
                                          start_time,
                                          end_time,
                                          read_date);
    auto& ref = getArticleCat(uid);
    for(int i : ref){
      sess.session_features.push_back({i, 1});
    }

    std::set<long> adup;
    std::set<std::string> sdup;
    int offset;
    for(int i = 0; i < arts; ++i){
      std::string str_id;
      long aid;
      is>>str_id>>aid;
      adup.insert(aid);
      sdup.insert(str_id);
      if(l){
	std::string tmp = str_id + "_read";
	offset = getFeatureOffset(tmp);
	sess.session_features.push_back({offset, 1});
      }
    }
    
    offset = getFeatureOffset("unique_song");
    sess.session_features.push_back({offset, log(1.0+ sdup.size())});

    offset = getFeatureOffset("unique_artist");
    sess.session_features.push_back({offset, log(1.0 + adup.size())});

    offset = getFeatureOffset("song_num");
    sess.session_features.push_back({offset, log(1.0 + arts)});
    int dw = sess.start.dayOfWeek();
    assert(dw >= 0 && dw <= 6);
    std::string dw_id;
    dw_id.push_back(dw + '0');
    dw_id += "_week";
    offset = getFeatureOffset(dw_id);
    sess.session_features.push_back({offset, 1});

    int dd = (int)(sess.start.hourOfDay() / 6.0);
    assert(dd >= 0 && dd <= 3);
    std::string dd_id;
    dd_id.push_back(dd + '0');
    dd_id += "_daystatus";
    offset = getFeatureOffset(dd_id);
    sess.session_features.push_back({offset, 1});

    {
        std::stringstream tmp_ss;
        tmp_ss<<(int)sess.start.hourOfDay();
        std::string t_id = tmp_ss.str();
        t_id += "_timeofday";
        offset = getFeatureOffset(t_id);
        sess.session_features.push_back({offset, 1});
    }
    return true;
}

/*
  4612525700	20150628	1435644003	1435645012	4	133961	35
  4612525700	20150628	1435670188	1435670188	0
*/
bool stay_handle(std::istream &is,
                 UserContainer &data,
                 bool l){
    long uid, start_time, end_time;
    int arts;
    char read_date[128];
    is>>uid>>read_date>>start_time>>end_time>>arts;

    if(is.eof())
        return true;
    // UTC -> Chine time
    assert(start_time <= end_time);
    start_time += 8 * 60 * 60;
    end_time += 8 * 60 * 60;
    
    if(!data.count(uid)){
        data.insert(std::make_pair(uid, User(uid)));
    }
    
    Session &sess = data[uid].add_session(uid,
                                          start_time,
                                          end_time,
                                          read_date);
    std::map<int, int> art_cnt;
    int offset;
    for(int i = 0; i < arts; ++i){
        std::string str_id;
        int stay;
        is>>str_id>>stay;

        long aid = atol(str_id.c_str());
        std::vector<int> &ref = getArticleCat(aid);
        for(int i : ref){
            art_cnt[i]++;
        }
        
        if(l){
            std::string tmp = str_id + "_read";
            offset = getFeatureOffset(tmp);
            sess.session_features.push_back({offset, 1});
            tmp = str_id + "_stay";
            offset = getFeatureOffset(tmp);
            sess.session_features.push_back({offset, stay/(double)3600.0}); // stay in hours
        }
    }
    // Put the article category into the feature
    {
        for(auto ite = art_cnt.begin(); ite != art_cnt.end(); ++ite){
            sess.session_features.push_back({ite->first, (double)ite->second});
        }
    }

    int dw = sess.start.dayOfWeek();
    assert(dw >= 0 && dw <= 6);
    std::string dw_id;
    dw_id.push_back(dw + '0');
    dw_id += "_week";
    offset = getFeatureOffset(dw_id);
    sess.session_features.push_back({offset, 1});

    int dd = (int)(sess.start.hourOfDay() / 6.0);
    assert(dd >= 0 && dd <= 3);
    std::string dd_id;
    dd_id.push_back(dd + '0');
    dd_id += "_daystatus";
    offset = getFeatureOffset(dd_id);
    sess.session_features.push_back({offset, 1});

    {
        std::stringstream tmp_ss;
        tmp_ss<<(int)sess.start.hourOfDay();
        std::string t_id = tmp_ss.str();
        t_id += "_timeofday";
        offset = getFeatureOffset(t_id);
        sess.session_features.push_back({offset, 1});
    }

    return true;
}

/*
  2597120	20150601	37	66852	162	15392	98693	29	68	62360	23	22	20	55184	143	70540	76823	108	6	133	153	82281	118	36590	492	20457	69030	109797	96344	59602	336	16964	318	88884	50	19632	35741	39	44806	37
  8314842	20150601	55	18933	127988	467	54514	73362	100844	20457	16230	104550	108411	143	77658	89	96341	33876	83	71633	17101	75	458	713	109896	68	13579	109630	30002	19633	46	16685	43	37	59043	69666	287	29	28	11035	792	151	22	12334	18	76696	15	14	13	23	9	66952	75399	6	74373	84355	74491	250
*/
bool app_handle(std::istream &s,
                UserContainer &data,
                bool l){
    long uid;
    char date[128];
    int apps;
    s>>uid>>date>>apps;
    if(s.eof())
        return false;

    std::string str_id;
    for(int i = 0; i < apps; ++i){
        s>>str_id;
        if(!data.count(uid)){
            continue;
        }
        std::string tmp = str_id + "_app";
        int offset = getFeatureOffset(tmp);
        data[uid].add_feature(date, {offset, 1});
    }
}

// 1       m       Japan
// 2       f       Peru
// 3       m       United States
// 4       f       n
int load_lastfm_profile(const char* filename){
  std::ifstream ifs(filename);
  std::cerr<<"Reading lastfm profile"<<std::endl;
  long uid;
  std::string s, c;
  int cnt = 0;
  while(ifs){
    ifs>>uid>>s>>c;
    if(s != "n"){
      setArtCat(uid, s + "_sex");
    }
    if(c != "n"){
      setArtCat(uid, c + "_cny");
    }
    ++cnt;
  }
  return cnt;
}

bool profile_handle(std::istream &s,
                    UserContainer &data,
                    bool l){
    long uid;
    char date[128];
    int apps;
    s>>uid>>date>>apps;
    if(s.eof())
        return false;


    std::string str_id;
    for(int i = 0; i < apps; ++i){
        s>>str_id;
        if(!data.count(uid)){
            continue;
        }
        std::string tmp = str_id + "_prof";
        int offset = getFeatureOffset(tmp);
        data[uid].add_feature(date, {offset, 1});
    }
}


static int read_data_from_file(
    const char* filename,
    UserContainer &data,
    read_handle handle,
    bool l){
    std::ifstream ifs(filename);
    if(!ifs.is_open()) return 0;
    int count = 0;
    while(!ifs.eof()){
        if(count % 1000 == 0){
            std::cerr<<"\rReading: "<<filename<<
                    "\tLoadFactor:"<<data.load_factor()<<
                    "\t"<<count;
        }
        handle(ifs, data, l);
        ++count;
    }

    std::cerr<<"\rReading: "<<filename
             <<"\tLoadFactor:"<<data.load_factor()
             <<"\t"<<count<<std::endl;

    return 0;
}

bool isIn(std::string &s, char c){
    for(auto x : s){
        if(x == c)
            return true;
    }
    return false;
}

int load_article_information(const char* filename){
    std::ifstream ifs(filename);
    
    long aid;
    int cnt = 0;
    while(ifs){
        if(cnt % 1000 == 0)
            std::cerr<<"Num: "<<cnt<<"\r";
        int n;
        ifs>>aid>>n;
        ++cnt;
        for(int i = 0; i != n; ++i){
            std::string c;
            ifs>>c;
            setArtCat(aid, c);
        }
    }
    std::cerr<<std::endl;
    return cnt;
}

int read_data(bool lastfm,
    std::string feature,
    const char* stay_dirtemp,
    const char* app_dirtemp,
    const char* profile_dirtemp,
    const char* start_day,
    const char* end_day,
    UserContainer &data){
    using namespace boost::gregorian;
    std::string ud(start_day);
    date start(from_undelimited_string(ud));
    ud = end_day;
    date end(from_undelimited_string(ud));
    date_duration inc_date(1);
    int count = 0;
    char filename[256];
    std::cerr<<"Reading stay data"<<std::endl;
    for(date d = start; d <= end; d = d + inc_date){    
      std::string cur_date = to_iso_string(d);
      snprintf(filename,
      	       sizeof(filename),
      	       stay_dirtemp,
      	       cur_date.c_str());
      count += read_data_from_file(filename,
      				   data,
      				   lastfm ? stay_handle_lastfm : stay_handle,
      				   isIn(feature, 'd'));
      
    }
    
    if(!lastfm && isIn(feature, 'p')){
      std::cerr<<"Reading profile data"<<std::endl;
      for(date d = start; d <= end; d = d + inc_date){    
	std::string cur_date = to_iso_string(d);
	snprintf(filename,
		 sizeof(filename),
		 profile_dirtemp,
		 cur_date.c_str());
	count += read_data_from_file(filename,
				     data,
				     profile_handle,
				     isIn(feature, 'p'));
      }
    }

    if(!lastfm && isIn(feature, 'a')){
        std::cerr<<"Reading app data"<<std::endl;
        for(date d = start; d <= end; d = d + inc_date){    
            std::string cur_date = to_iso_string(d);
            snprintf(filename,
                     sizeof(filename),
                     app_dirtemp,
                     cur_date.c_str());
            count += read_data_from_file(filename,
                                         data,
                                         app_handle,
                                         isIn(feature, 'a'));
        }
    }
    return count;
}

