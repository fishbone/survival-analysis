#! /bin/bash
LD_PRELOAD=/home/yicheng1/.local/lib/libtcmalloc.so.4.2.6 \
        ./main          --evaluations=eval_loglik \
        --models=user_constant_model \
        --train_start=20050214 \
        --train_end=20081231 \
        --test_start=20090101 \
        --test_end=20090619 \
        --config=./lastfm_user_param.info \
        --stay_data_template=./last-fm-daily/%s.daily \
        --app_data_template=./app/%s.app \
        --profile_data_template=./prof/%s.prof \
        --loadparam="p" \
        --article ./article_cat \
        --lastfm=true
#--models=user_constant_model \
        #--models=global_constant_model \
