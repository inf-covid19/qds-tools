#!/bin/sh

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "count" -f "twitter-small" > twitter-small-count.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/twitter-tile.csv   -a "count" -f "twitter-small" > twitter-small-count-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/twitter-region.csv -a "count" -f "twitter-small" > twitter-small-count-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv              -a "count" -f "brightkite" > brightkite-count.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/brightkite-tile.csv   -a "count" -f "brightkite" > brightkite-count-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/brightkite-region.csv -a "count" -f "brightkite" > brightkite-count-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "count" -f "gowalla" > gowalla-count.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/gowalla-tile.csv   -a "count" -f "gowalla" > gowalla-count-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/gowalla-region.csv -a "count" -f "gowalla" > gowalla-count-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "count" -f "on_time_performance" > on_time_performance-count.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-tile.csv   -a "count" -f "on_time_performance" > on_time_performance-count-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-region.csv -a "count" -f "on_time_performance" > on_time_performance-count-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "quantile" -o "dep_delay_t.(0.001)" -f "on_time_performance" > on_time_performance-quantile-0_001.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-tile.csv   -a "quantile" -o "dep_delay_t.(0.001)" -f "on_time_performance" > on_time_performance-quantile-0_001-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-region.csv -a "quantile" -o "dep_delay_t.(0.001)" -f "on_time_performance" > on_time_performance-quantile-0_001-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "quantile" -o "dep_delay_t.(0.010)" -f "on_time_performance" > on_time_performance-quantile-0_010.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-tile.csv   -a "quantile" -o "dep_delay_t.(0.010)" -f "on_time_performance" > on_time_performance-quantile-0_010-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-region.csv -a "quantile" -o "dep_delay_t.(0.010)" -f "on_time_performance" > on_time_performance-quantile-0_010-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "quantile" -o "dep_delay_t.(0.100)" -f "on_time_performance" > on_time_performance-quantile-0_100.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-tile.csv   -a "quantile" -o "dep_delay_t.(0.100)" -f "on_time_performance" > on_time_performance-quantile-0_100-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-region.csv -a "quantile" -o "dep_delay_t.(0.100)" -f "on_time_performance" > on_time_performance-quantile-0_100-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "quantile" -o "dep_delay_t.(0.500)" -f "on_time_performance" > on_time_performance-quantile-0_500.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-tile.csv   -a "quantile" -o "dep_delay_t.(0.500)" -f "on_time_performance" > on_time_performance-quantile-0_500-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-region.csv -a "quantile" -o "dep_delay_t.(0.500)" -f "on_time_performance" > on_time_performance-quantile-0_500-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "quantile" -o "dep_delay_t.(0.900)" -f "on_time_performance" > on_time_performance-quantile-0_900.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-tile.csv   -a "quantile" -o "dep_delay_t.(0.900)" -f "on_time_performance" > on_time_performance-quantile-0_900-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-region.csv -a "quantile" -o "dep_delay_t.(0.900)" -f "on_time_performance" > on_time_performance-quantile-0_900-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "quantile" -o "dep_delay_t.(0.990)" -f "on_time_performance" > on_time_performance-quantile-0_990.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-tile.csv   -a "quantile" -o "dep_delay_t.(0.990)" -f "on_time_performance" > on_time_performance-quantile-0_990-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-region.csv -a "quantile" -o "dep_delay_t.(0.990)" -f "on_time_performance" > on_time_performance-quantile-0_990-region.log

./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/data.csv           -a "quantile" -o "dep_delay_t.(0.999)" -f "on_time_performance" > on_time_performance-quantile-0_999.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-tile.csv   -a "quantile" -o "dep_delay_t.(0.999)" -f "on_time_performance" > on_time_performance-quantile-0_999-tile.log
./nc2nds -i /home/cicerolp/git/nds-data/logs-nanocubes/flights-region.csv -a "quantile" -o "dep_delay_t.(0.999)" -f "on_time_performance" > on_time_performance-quantile-0_999-region.log
