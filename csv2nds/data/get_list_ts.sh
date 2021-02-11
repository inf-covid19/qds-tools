cp $1.xml $1.ts &&
sed -i -e "s/<bin>/'/g" $1.ts &&
sed -i -e "s/<\/bin>/', /g" $1.ts &&
sed -i -e ':a;N;$!ba;s/\n//g' $1.ts &&
sed -i -e 's/.$//' $1.ts &&
sed -i -e 's/.$//' $1.ts
