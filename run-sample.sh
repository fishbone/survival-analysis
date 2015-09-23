#! /bin/bash
./main --models=piecewise_constant_model \
       --models=global_constant_model \
       --models=user_constant_model \
       --models=hawkes_process_model \
       --models=model_test \
       --evaluations=eval_loglik \
       --train_start=20150601 \
       --train_end=20150610 \
       --test_start=20150611 \
       --test_end=20150613 \
       --stay_data_template=./daily/%s.daily \
       --profile_data_template=/home/yicheng1/survival-analysis/processed_data/%s.profile \
       --app_data_template=/home/yicheng1/survival-analysis/processed_data/%s.app \
       --config=param.info
