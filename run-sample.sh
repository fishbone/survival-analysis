#! /bin/bash
./main --models=model_test \
       --models=global_constant_model \
       --models=user_constant_model \
       --evaluations=eval_loglik \
       --train_start=20150628 \
       --train_end=20150629 \
       --test_start=20150701 \
       --test_end=20150701 \
       --stay_data_template=./daily/%s.daily \
       --app_data_template=./app/%s.app \
       --profile_data_template=./prof/%s.prof \
       --config=param.info
#       --models=hawkes_process_model \
#       --models=piecewise_constant_model \
