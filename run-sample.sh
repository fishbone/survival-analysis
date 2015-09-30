#! /bin/bash
./main --models=construct_feature_model \
       --evaluations=eval_loglik \
       --train_start=20150601 \
       --train_end=20150602 \
       --test_start=20150801 \
       --test_end=20150723 \
       --stay_data_template=./daily/%s.daily \
       --app_data_template=./app/%s.app \
       --profile_data_template=./prof/%s.prof \
       --config=param.info
#       --models=hawkes_process_model \
