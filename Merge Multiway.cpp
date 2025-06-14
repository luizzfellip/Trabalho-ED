#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
using namespace std;
const int TAM_MAX = 100;
struct Registro
{
		char series_reference[TAM_MAX];
		char period[TAM_MAX];
		char data_value[TAM_MAX];
		char status[TAM_MAX];
		char units[TAM_MAX];
		char subject[TAM_MAX];
		char group [TAM_MAX];
		char series_title_1[TAM_MAX];
		char series_title_2[TAM_MAX];
		char series_title_3[TAM_MAX];
		char series_title_4[TAM_MAX];
		char series_title_5[TAM_MAX];
};

void ler_linha(stringstream& streamLinha, char* destinoReg, char delimitador = ',')
{
	string campo;
	getline(streamLinha, campo, delimitador);
	strncpy(destinoReg, campo.c_str(), TAM_MAX - 1);
    destinoReg[TAM_MAX - 1] = '\0'; // garantir que o último caractere seja nulo
}

//distribui cada linha do texto para um campo do registrador
void distribuidor_TCG(ifstream& arqEntradaCSV,ofstream* arqSaidaBin,int numPartes)
{
	long long contadorLinhas = 0;
	string linha;
	
	while(getline(arqEntradaCSV, linha)){
		 // cria um arquivo com a variavel linha
		 // para não ocorrer um erro se a linha faltar dado,
		 // e tem funcao de marcador
		stringstream streamLinha(linha);
		 // inicializa a struct com zeros para segurança, antes de preencher
		 Registro umRegistro = {};
		 
        ler_linha(streamLinha, umRegistro.series_reference);
        ler_linha(streamLinha, umRegistro.period);
		ler_linha(streamLinha, umRegistro.data_value);
        ler_linha(streamLinha, umRegistro.status);
        ler_linha(streamLinha, umRegistro.units);
        ler_linha(streamLinha, umRegistro.subject);
        ler_linha(streamLinha, umRegistro.group);
        ler_linha(streamLinha, umRegistro.series_title_1);
        ler_linha(streamLinha, umRegistro.series_title_2);
        ler_linha(streamLinha, umRegistro.series_title_3);
        ler_linha(streamLinha, umRegistro.series_title_4);
		ler_linha(streamLinha, umRegistro.series_title_5, '\n'); // o último campo não tem vírgula

        int indiceArquivo = contadorLinhas % numPartes;
        arqSaidaBin[indiceArquivo].write(reinterpret_cast<const char*>(&umRegistro), sizeof(Registro));

        contadorLinhas++;
    }
}

void dividir_CSV_em_PartesBinarias()
{	
	
	// 1. definir partes divididas e abrir arquivo
	const int numPartes = 11;
	const string nomeBase = "parte_F";
	
	ifstream arqEntradaCSV("dados.csv");
    if (!arqEntradaCSV.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo!");
    }
	
	// 2. criar as partes divididas
	ofstream* arqSaidaBin = new ofstream[numPartes];
	for(int i=0; i<numPartes; i++){
		string nomeArq = nomeBase +  to_string(i) + ".bin";
		arqSaidaBin[i].open(nomeArq, ios::binary | ios::trunc);
		
		if(!arqSaidaBin[i].is_open()){
			delete[] arqSaidaBin;
			throw runtime_error("Nao foi possivel criar os arquivos de saida!");
		}
	}
	
	// 3. ignorar cabecalho
	string cabecalho;
	getline(arqEntradaCSV, cabecalho);
    
    // 4. dividir e jogar nos campos desejados
    distribuidor_TCG(arqEntradaCSV, arqSaidaBin, numPartes);
    
    // 5. fechar todos os arquivos
    arqEntradaCSV.close();
    for (int i = 0; i < numPartes; ++i) {
        arqSaidaBin[i].close();
    }
    
    // 6. liberar a memória alocada para o array de arquivos
    delete[] arqSaidaBin;
    
    cout << "Arquivo dividido com sucesso!\n";

}




int main ()
{
	try{
	dividir_CSV_em_PartesBinarias();
	} catch (exception& e) {
		cout << "Erro: "<< e.what() << endl;
	}
	
	
return 0;
};
