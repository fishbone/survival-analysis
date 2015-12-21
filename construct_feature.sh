#! /bin/bash
./main          --evaluations=eval_loglik \
                --models=construct_feature_model \
                --train_start=20150601 \
                --train_end=20150610 \
                --test_start=20150611 \
                --test_end=20150615 \
                --feature_model_only=0 \
                --config=./construct_feature.info \
                --stay_data_template=./daily/%s.daily \
                --app_data_template=./app/%s.app \
                --profile_data_template=./prof/%s.prof \
