#!/usr/bin/env bash

set +e

INPUT_DIR=tripdata
DATASET=green

for year in {2013..2017}; do
    for month in {01..12}; do
        echo ./csv2nds -i ./schema/${DATASET}_tripdata_${year}_${month}.xml

        mkdir -p ${DATASET}_tripdata_${year}

        rm -R ${DATASET}_tripdata_${year}

        mkdir -p ${DATASET}_tripdata_${year}

        # move files
        cp "${INPUT_DIR}/${DATASET}_tripdata_${year}-${month}.csv" ${DATASET}_tripdata_${year}/

        # run
        ./csv2nds -i ./schema/${DATASET}_tripdata_${year}.xml

        # rename files
        mv output/${DATASET}_tripdata_${year}.xml output/${DATASET}_tripdata_${year}_${month}.xml
        mv output/${DATASET}_tripdata_${year}.nds output/${DATASET}_tripdata_${year}_${month}.nds
    done
done

DATASET=yellow

for year in {2009..2017}; do
    for month in {01..12}; do
        echo ./csv2nds -i ./schema/${DATASET}_tripdata_${year}_${month}.xml

        mkdir -p ${DATASET}_tripdata_${year}

        rm -R ${DATASET}_tripdata_${year}

        mkdir -p ${DATASET}_tripdata_${year}

        # move files
        cp "${INPUT_DIR}/${DATASET}_tripdata_${year}-${month}.csv" ${DATASET}_tripdata_${year}/

        # run
        ./csv2nds -i ./schema/${DATASET}_tripdata_${year}.xml

        # rename files
        mv output/${DATASET}_tripdata_${year}.xml output/${DATASET}_tripdata_${year}_${month}.xml
        mv output/${DATASET}_tripdata_${year}.nds output/${DATASET}_tripdata_${year}_${month}.nds
    done
done