echo "Start"

count=`head -n 1 *dados-ac.csv | tr ';' '\n' | wc -l`

for i in `seq 2 $count`
do
	echo "Process field #$i/$count" &&
	`./get_list_of_field.sh $i`
done

echo "Finished"
