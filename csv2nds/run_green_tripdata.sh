for year in {2013..2016}; do
   echo ./csv2nds -i ./schema/green_tripdata_$year.xml
    ./csv2nds -i ./schema/green_tripdata_$year.xml &
done

echo Processing ...

wait

echo Done!
