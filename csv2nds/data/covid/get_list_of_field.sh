echo "Process the files:" &&

dir="dimentions" &&
name="no_name" &&

touch tmp2.txt &&
mkdir -p $dir

for i in *dados-*
do
	echo "-$i" &&
	cat $i | cut --delimiter=";" -f$1 > tmp1.txt &&
	tail -n +2 tmp1.txt >> tmp2.txt &&
	name=`head -n 1 $i | cut --delimiter=";" -f$1`
done

echo "Processing data to $dir/$name.xml" &&

sort -u tmp2.txt > tmp1.txt &&
sed 's/^/<bin>/' tmp1.txt > tmp2.txt &&
sed 's/$/<\/bin>/' tmp2.txt > tmp1.txt &&
mv tmp1.txt $dir/$name.xml &&
sed -i -e "s/\"//g" $dir/$name.xml &&
rm tmp2.txt &&

echo "Finished the processing"
