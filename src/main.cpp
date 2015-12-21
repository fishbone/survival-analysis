#include <iostream>
#include <boost/program_options.hpp>
#include "comm.h"
#include <ctime>
#include <string>
#include "data_io.h"
#include "model_base.h"
using namespace std;
using namespace boost::program_options;


void print_help(boost::program_options::options_description &desc){
    cerr<<"Usage: prog_name [options]"<<endl;
    cerr<<endl;
    cerr<<desc<<endl;
}

bool parse_param(int argc, char *argv[], variables_map &vm){
    options_description desc("Options");
    desc.add_options()
            ("models", value<std::vector<std::string> >()->multitoken()->required(), "The models to be trained with")
            ("evaluations", value<std::vector<std::string> >()->multitoken()->required(), "The evaluation algorithms to be calculated")
            ("train_start", value<std::string>()->required(), "The start date of the data for training")
            ("train_end", value<std::string>()->required(), "The end date of the data for training")
            ("test_start", value<std::string>()->required(), "The start date of the data for testing")
            ("test_end", value<std::string>()->required(), "The end date of the data for testing")
            ("stay_data_template", value<std::string>()->required(), "The template of the directory for the stay data")
            ("profile_data_template", value<std::string>()->required(), "The template of the directory for the profile data")
            ("app_data_template", value<std::string>()->required(), "The template of the directory for the app data")
            ("config", value<std::string>()->required(), "The configure file for parameters of the model")
            ("loadparam", value<std::string>()->required(), "The params to be loaded")
            ("article", value<std::string>()->required(), "article category")
            ("help", "produce help message");
    
    try{
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
    }catch(exception &e){
        if(vm.count("help")){
            print_help(desc);
        }else{
            cerr<<e.what()<<endl;
        }
        return false;
    }
    if(vm.count("help")){
        print_help(desc);
        return false;
    }
    return true;
}
void load_evals(const std::vector<std::string> &eval_name,
                std::vector<EvaluationBase*> &evals){
    for(auto i : eval_name){
        EvaluationBase *e = EvaluationBase::makeEval(i.c_str());
        if(nullptr == e){
            cerr<<"Evaluation: "<<i<<" doesn't exist!!!!!"<<endl;
        }else{
            evals.push_back(e);
            cout<<"Loaded eval "<<e->evalName()<<endl;
        }
    }
}
void load_models(const std::vector<std::string> &models_name,
                 std::vector<ModelBase*> &models){
    for(auto i : models_name){
        ModelBase *m = ModelBase::makeModel(i.c_str());
        if(nullptr == m){
            cerr<<"Model: "<<i<<" doesn't exist!!!!!"<<endl;
        }else{
            models.push_back(m);
            cout<<"Loaded model "<<m->modelName()<<endl;
        }
    }
}
void train_models(
    UserContainer *data,
    std::vector<ModelBase*>& models){
    for(auto i : models){
        auto start = time(nullptr);
        cerr<<"Training model: "<<i->modelName()<<endl;
        i->train(data);
        auto end = time(nullptr);
        cerr<<"Finish training model: "<<i->modelName()<<" CostTime:"<<(end - start)<<"s"<<endl;
    }
}
void test_models(
    UserContainer *data,
    std::vector<ModelBase*> &models,
    std::vector<EvaluationBase*> &evals){
    for(auto m : models){
        std::vector<std::tuple<long, ModelBase::PredictRes> > result;
        m->batchPredict(data, result);
        cout<<"DEBUG="<<result.size()<<endl;
        for(auto e : evals){
            cout<<"Model="
                <<m->modelName()
                <<"\tEval="<<e->evalName()
                <<"\tResult=["<<e->doEval(data, result)<<"]"<<endl;
        }
    }
}

int main(int argc, char *argv[]){
    variables_map vm;
    if(!parse_param(argc, argv, vm)){
        return -1;
    }
    if(!ModelBase::loadConfig(vm["config"].as<std::string>().c_str())){
        std::cerr<<"Load config file "<<vm["config"].as<std::string>().c_str()<<" failed"<<std::endl;
        return -1;
    }
    BIN_WIDTH = ModelBase::getConfig()["BIN_WIDTH"].as<double>();
    NUM_BIN = ModelBase::getConfig()["NUM_BIN"].as<int>();
    cerr<<"BIN_WIDTH="<<BIN_WIDTH<<endl;
    cerr<<"NUM_BIN="<<NUM_BIN<<endl;
    UserContainer train_data;
    train_data.reserve(10000000);
    {
        std::cerr<<"Read article category"<<std::endl;
        int cnt = load_article_information(vm["article"].as<std::string>().c_str());
        std::cerr<<"Read article category finished: "<<cnt<<std::endl;
    }

    cerr<<"Reading data for training dataset"<<endl;
    read_data(
        vm["loadparam"].as<std::string>(),
        vm["stay_data_template"].as<std::string>().c_str(),
        vm["app_data_template"].as<std::string>().c_str(),
        vm["profile_data_template"].as<std::string>().c_str(),
        vm["train_start"].as<std::string>().c_str(),
        vm["train_end"].as<std::string>().c_str(),
        train_data);

    UserContainer test_data;
    test_data.reserve(10000000);
    cerr<<"Reading data for testing dataset"<<endl;
    read_data(vm["loadparam"].as<std::string>(),
              vm["stay_data_template"].as<std::string>().c_str(),
              vm["app_data_template"].as<std::string>().c_str(),
              vm["profile_data_template"].as<std::string>().c_str(),
              vm["test_start"].as<std::string>().c_str(),
              vm["test_end"].as<std::string>().c_str(),
              test_data);
    std::vector<ModelBase*> models;
    load_models(vm["models"].as<std::vector<std::string> >(),
                models);
    if(0 == models.size()){
        cerr<<"Please give at least one model to do training"<<endl;
        return -1;
    }
    for(size_t i = 0; i != models.size(); ++i)
        models[i]->setData(&train_data, &test_data);
    std::vector<EvaluationBase*> evals;
    load_evals(vm["evaluations"].as<std::vector<std::string> >(),
               evals);

    if(0 == evals.size()){
        cerr<<"Please give at least one evaluation to do testing"<<endl;
        return -1;
    }

    cerr<<"Start to train models"<<endl;
    train_models(&train_data, models);
    cerr <<"Start to test models"<<endl;
    test_models(&test_data, models, evals);
    
    return 0;
}
