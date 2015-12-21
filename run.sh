#! /bin/bash
LD_PRELOAD=/home/yicheng1/.local/lib/libtcmalloc.so.4.2.6 \
          ./main          --evaluations=eval_loglik \
          --models=feature_based_model \
          --train_start=20150601 \
          --train_end=201508289 \
          --test_start=20090501 \
          --test_end=20090619 \
          --config=./integrated_param.info \
          --stay_data_template=./daily/%s.daily \
          --app_data_template=./app/%s.app \
	  --profile_data_template=./prof/%s.prof \
          --loadparam="p" \
          --article ./article_cat \
	  --dump="toutiao-lstm"

          #--models=user_constant_model \
          #--models=global_constant_model \
