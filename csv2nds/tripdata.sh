#!/usr/bin/env bash

INPUT_DIR=tripdata
DATASET=green

for year in {2013..2017}; do
    mkdir ${DATASET}_tripdata_${year}

    for month in {1..12}; do
        echo ./csv2nds -i ./schema/${DATASET}_tripdata_${year}_${month}.xml

        rm -R mkdir ${DATASET}_tripdata_${year}

        # move files
        cp "${INPUT_DIR}/*_${year}_${month}.csv" ${DATASET}_tripdata_${year}/

        # run
        ./csv2nds -i ./schema/${DATASET}_tripdata_${year}.xml

        # rename files
        mv output/${DATASET}_tripdata_${year}.xml output/${DATASET}_tripdata_${year}_${month}.xml
        mv output/${DATASET}_tripdata_${year}.nds output/${DATASET}_tripdata_${year}_${month}.nds
    done
done

DATASET=yellow

for year in {2009..2017}; do
    mkdir ${DATASET}_tripdata_${year}

    for month in {1..12}; do
        echo ./csv2nds -i ./schema/${DATASET}_tripdata_${year}_${month}.xml

        rm -R mkdir ${DATASET}_tripdata_${year}

        # move files
        cp "${INPUT_DIR}/*_${year}_${month}.csv" ${DATASET}_tripdata_${year}/

        # run
        ./csv2nds -i ./schema/${DATASET}_tripdata_${year}.xml

        # rename files
        mv output/${DATASET}_tripdata_${year}.xml output/${DATASET}_tripdata_${year}_${month}.xml
        mv output/${DATASET}_tripdata_${year}.nds output/${DATASET}_tripdata_${year}_${month}.nds
    done
done