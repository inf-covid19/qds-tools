echo "BEGIN" > test_cube.txt &&

echo -e "sus_cont_ciap_mini" >> test_cube.txt &&
date >> test_cube.txt &&
./csv2nds -i schema/sus_cont_ciap_mini.xml >> test_cube.txt &&
date >> test_cube.txt &&

echo -e "\nsus_cont_ciap_mini_inv" >> test_cube.txt &&
date >> test_cube.txt &&
./csv2nds -i schema/sus_cont_ciap_mini_inv.xml >> test_cube.txt &&
date >> test_cube.txt &&

echo -e "\nsus_cont_ciap" >> test_cube.txt &&
date >> test_cube.txt &&
./csv2nds -i schema/sus_cont_ciap.xml >> test_cube.txt &&
date >> test_cube.txt &&

echo -e "\nsus_cont_ciap_inv" >> test_cube.txt &&
date >> test_cube.txt &&
./csv2nds -i schema/sus_cont_ciap_inv.xml >> test_cube.txt &&
date >> test_cube.txt &&

echo "END" >> test_cube.txt
