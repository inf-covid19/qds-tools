for year in {2009..2016}; do
   echo ./csv2nds -i ./schema/yellow_tripdata_$year.xml
    ./csv2nds -i ./schema/yellow_tripdata_$year.xml &
done

echo Processing ...

wait

echo Done!
