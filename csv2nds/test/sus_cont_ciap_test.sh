array=( cod_ibge equipe cod_uf qtd_gestacao_previa qtd_partos idade qtd_ciaps semanas_gestacional tipo_atendimento versao sexo turno tipo_origem cod_reg)
dataset="sus_cont_ciap"

echo "BEGIN" > "${dataset}_test.js" &&
for i in "${array[@]}"
do
	echo -e "\n$i" >> "${dataset}_test.js" &&
	curl "http://localhost:7000/api/query/dataset=${dataset}/aggr=count/const=$i.values.(all)/group=$i" >> "${dataset}_test.js"
done
echo -e "\nEND" >> "${dataset}_test.js"
