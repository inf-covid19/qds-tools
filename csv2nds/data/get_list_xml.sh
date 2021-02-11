cat $1 | cut --delimiter=";" -f$3 > tmp1.txt &&
tail -n +2 tmp1.txt > tmp2.txt &&
sort -u tmp2.txt > tmp1.txt &&
sed 's/^/<bin>/' tmp1.txt > tmp2.txt &&
sed 's/$/<\/bin>/' tmp2.txt > tmp1.txt &&
mv tmp1.txt $2.xml &&
sed -i -e "s/\"//g" $2.xml &&
rm tmp2.txt
