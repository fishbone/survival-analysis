#! /bin/bash
./main --models=adhoc_statistics_model \
       --loadparam="a" \
       --evaluations=eval_loglik \
       --train_start=20150601 \
       --train_end=20150815 \
       --test_start=20150816 \
       --test_end=20150823 \
       --stay_data_template=./daily/%s.daily \
       --app_data_template=./app/%s.app \
       --profile_data_template=./prof/%s.prof \
       --config=param.info \
       #--models=feature_based_model \
       #--models=feature_based_model \
      # --models=user_constant_model \
