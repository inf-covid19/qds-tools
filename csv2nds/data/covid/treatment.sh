echo "Start treatment" &&
date &&

dir="treatments" &&

mkdir -p $dir

for i in dados-*
do
	echo "Original data: $i" &&
	head -n 2 $i &&
	wc -l $i &&
	echo "Selecting columns" &&
	awk -F ";" '{print $2 ";" $3 ";" $6 ";" $9 ";" $10 ";" $11 ";" $12 ";" $14 ";" $16 ";" $18 ";" $22 ";" $24 ";" $27 ";" $28 ";" $29 ";" $30}' $i > $dir/tmp_$i &&
	head -n 2 $dir/tmp_$i &&
	wc -l $dir/tmp_$i &&
	echo "Deleting pt characters" &&
	iconv -f latin1 -t ascii//TRANSLIT $dir/tmp_$i > $dir/tmp2_$i &&
	mv $dir/tmp2_$i $dir/tmp_$i &&
	head -n 2 $dir/tmp_$i &&
	wc -l $dir/tmp_$i &&
	echo "Treatment undefined" &&
	sed -e "s/null/undefined/g" $dir/tmp_$i > $dir/tmp2_$i &&
	sed -e "s/;$/;undefined/g" $dir/tmp2_$i > $dir/tmp_$i &&
	sed -e "s/^;/undefined;/g" $dir/tmp_$i > $dir/tmp2_$i &&
	cat $dir/tmp2_$i | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" | sed -e "s/;;/;undefined;/g" > $dir/tmp_$i &&
	head -n 2 $dir/tmp_$i &&
	wc -l $dir/tmp_$i &&
	echo "Clean files" &&
	rm $dir/tmp2_$i &&
	mv $dir/tmp_$i $dir/pro_$i &&
	head -n 2 $dir/pro_$i &&
	wc -l $dir/pro_$i 
done

echo "Finished treatment" &&
date
