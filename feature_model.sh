#! /bin/bash
./main          --evaluations=eval_loglik \
                --train_start=20150601 \
                --models=feature_based_model \
                --feature_model_only=0 \
                --train_end=20150610 \
                --test_start=20150611 \
                --test_end=20150615 \
                --config=./feature_model_param.info \
                --stay_data_template=./daily/%s.daily \
                --app_data_template=./app/%s.app \
                --profile_data_template=./prof/%s.prof \
