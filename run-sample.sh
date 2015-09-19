#! /bin/bash
./main --models=piecewise_constant_model \
       --models=user_constant_model \
       --models=model_test \
       --evaluations=eval_loglik \
       --train_start=20150628 \
       --train_end=20150701 \
       --test_start=20150702 \
       --test_end=20150702 \
       --data_template=$HOME/survival-analysis/data/user_survive/daily/show_read_stay.%s \
       --config=param.info
