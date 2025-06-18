#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <chrono> // so para ver o tempo de execucao do programa
#include <cmath> // tamanho dos arquivos na rodada
using namespace std;
using namespace chrono;

// Constantes globais
	const int numPartes = 16;
	const string nomeBaseAqrF = "f_";
	const string nomeBaseAqrS = "s_";
struct Registro
{
		char series_reference[11];
		float period;
		int data_value;
		char status[11];
		char units[11];
		int magnitude;
		char subject[51];
		char group [51];
		char series_title_1[51];
		char series_title_2[51];
		char series_title_3[51];
		char series_title_4[51];
		char series_title_5[51];
};

// ====================================================================
//	Primeira Parte: Didivir em n arquivos e transformar em binario
// ====================================================================

//le linha e joga no determinado determinado campo do Registrador
void ler_linha(stringstream& streamLinha, char* destinoReg,int tamC_max, char delimitador = ',')
{
	string campo;
	getline(streamLinha, campo, delimitador);
	strncpy(destinoReg, campo.c_str(), tamC_max - 1);
    destinoReg[tamC_max - 1] = '\0'; // garantir que o último caractere seja nulo
}

//distribui cada linha do texto para um campo do registrador
void distribuidor_TCG(ifstream& arqEntradaCSV,ofstream* arqSaidaBin,int numPartes)
{
	long long contadorLinhas = 0;
	long long totalBytsGravados = 0;
	string linha;
	
	while(getline(arqEntradaCSV, linha)){
		string campo; // para campos que nao char

		 // cria um "arquivo" com a variavel linha
		 // para não ocorrer um erro se a linha faltar dado,
		 // e tem funcao de marcador
		stringstream streamLinha(linha);
		 // inicializa a struct com zeros para segurança, antes de preencher
		 Registro umRegistro = {};
		 
		 // campo char: series_reference
        ler_linha(streamLinha, umRegistro.series_reference,sizeof(umRegistro.series_reference));

		//  campo float: period
        getline(streamLinha, campo, ',');
		umRegistro.period = campo.empty() ? -1.0f : stof(campo); // verifica se esta vazio, transforma string em float

		// campo int: data_value
		getline(streamLinha, campo, ',');
		umRegistro.data_value = campo.empty() ? -1.0f : stof(campo); // verifica se esta vazio, transforma string em float

        ler_linha(streamLinha, umRegistro.status,sizeof(umRegistro.status));
        ler_linha(streamLinha, umRegistro.units,sizeof(umRegistro.units));

		// campo int: magnitude
		getline(streamLinha, campo, ',');
		umRegistro.magnitude = campo.empty() ? -1 : stoi(campo); // verifica se esta vazio, transforma string em int

        ler_linha(streamLinha, umRegistro.subject,sizeof(umRegistro.subject));
        ler_linha(streamLinha, umRegistro.group,sizeof(umRegistro.group));
        ler_linha(streamLinha, umRegistro.series_title_1,sizeof(umRegistro.series_title_1));
        ler_linha(streamLinha, umRegistro.series_title_2,sizeof(umRegistro.series_title_2));
        ler_linha(streamLinha, umRegistro.series_title_3,sizeof(umRegistro.series_title_3));
        ler_linha(streamLinha, umRegistro.series_title_4,sizeof(umRegistro.series_title_4));
		ler_linha(streamLinha, umRegistro.series_title_5,sizeof(umRegistro.series_title_5), '\n'); // o último campo não tem vírgula

        int indiceArquivo = contadorLinhas % numPartes;
        arqSaidaBin[indiceArquivo].write(reinterpret_cast<const char*>(&umRegistro), sizeof(Registro));

        contadorLinhas++;
		totalBytsGravados += sizeof(Registro);
    }
    ofstream arqInfo("Tamanho_total_bytes.txt"); // vai ser util na hora da intercalacao
    arqInfo << totalBytsGravados; 
}

void dividir_CSV_em_PartesBinarias()
{	

	// 1. verificar se o arquivo esta na mesma pasta do codigo
	ifstream arqEntradaCSV("dados.csv");
    if (!arqEntradaCSV.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo!");
    }else{
		cout << "Iniciando a divisao dos arquivos...\n";
	}
	
	// 2. criar as partes divididas
	ofstream* arqSaidaBin = new ofstream[numPartes];
	for(int i=0; i<numPartes; i++){
		string nomeArq = nomeBaseAqrF +  to_string(i) + ".bin";
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
    
    cout << "Arquivo dividido em binario com sucesso!\n";

}

// ====================================================================
//	Segunda  Parte: Intercalar Externamente
// ====================================================================

bool procura_maior(ifstream arqF[],ofstream arqS[],int rodada = 0)
{
	Registro buffer[numPartes]; // armazena o registro atual de cada arquivo
	bool comValor[numPartes];	// marca quais arquivos ainda tem registros
 	int QteArqComValor = 0;
 	int tamIncrementado = 0;
	
	// 1. le o primeiro registro de cada arquivo
	for(int i=0; i< numPartes; i++){
		if(arqF[i].read(reinterpret_cast<char*>(&buffer[i]),sizeof(Registro))){
			comValor[i] = true;
			QteArqComValor++;
		}else{
			comValor[i] = false;
		}
	}	
	// 2. enquanto houver arquivos com registros
	while((QteArqComValor > 0) && (tamIncrementado != tamFinal)){
		int posMaior = -1;
		
		// encontra o maior entre os buffers
		for (int i=0; i < numPartes; i++){
			if((comValor[i])){
				if(posMaior == -1 || strcmp(buffer[i].series_reference, buffer[posMaior].series_reference) >= 0){
					posMaior = i;
				}
			}
		}
	// 3. escreve o maior na saida	
		arqS[0].write(reinterpret_cast<char*>(&buffer[posMaior]),sizeof(Registro));
		if(!arqF[posMaior].read(reinterpret_cast<char*>(&buffer[posMaior]),sizeof(Registro))){
			comValor[posMaior] = false;
			QteArqComValor--;
		}
		tamIncrementado++;
	}
	// Fechar arquivos
	for (int i = 0; i < numPartes; i++) {
		arqF[i].close();
		arqS[i].close();
	}
	
	ifstream testeFinalF((nomeBaseAqrF + "0.bin"), ios::binary | ios::ate);
	ifstream testeFinalS((nomeBaseAqrS + "0.bin"), ios::binary | ios::ate);
    long long tamF_F = testeFinalF.tellg();
    long long tamF_S = testeFinalS.tellg();

	testeFinalF.close();
	testeFinalS.close();
	
	ifstream arqInfo("Tamanho_total_bytes.txt");
	long long tamanhoEsperado;
	info >> tamanhoEsperado;
	info.close();
   // se o arquivo 0 contém todos os registros (exemplo: conhecido total)
   if (tamF_F == tamanhoEsperado || tamF_S == tamanhoEsperado){
		ordenado = true;
		cout << "Arquivo ordenado final gerado!\n";
	}
}

void intercalacao_multi_caminhos() // distribuidor
{
	cout << "Iniciando intercalacao...\n";
	
	int rodada = 0;
	bool ordenado = false;
	
	while(!ordenado){
		// 1. alternar os papeis dos arquivos entre as rodadas por meio do prefixo das chamadas
		string papel_f = (rodada % 2 == 0) ? nomeBaseAqrF : nomeBaseAqrS;
		string papel_s = (rodada % 2 == 0) ? nomeBaseAqrS : nomeBaseAqrF;
		
		// 2. abrir para leitura das partes divididas
		ifstream* arqLeitura = new ifstream[numPartes];
		for(int i=0; i<numPartes; i++){
			string nome = papel_f +  to_string(i) + ".bin";
			arqLeitura[i].open(nome, ios::binary);
		}
		
		// 3. abrir para escrita das partes divididas
		ofstream* arqEntrada = new ofstream[numPartes];
		for(int i=0; i<numPartes; i++){
			string nome = papel_s +  to_string(i) + ".bin";
			arqEntrada[i].open(nome, ios::binary | ios::trunc);
		}
		
		
		// 4. realizar a intercalacao
		ordenado = procura_maior(arqLeitura, arqEntrada,rodada);
		// 5. liberar memoria
		for(int i=0; i<numPartes; i++){
			arqLeitura[i].close();
			arqEntrada[i].close();
		}
		
		delete [] arqLeitura;
		delete [] arqEntrada;
	
		rodada++;
	}
	cout << "Intercalacao realizada com sucesso!\n";
}

int main ()
{
	auto start = steady_clock::now();
	
	try{
	// Primeiro Passo: dividir o csv em arquivos binarios nao ordenados
		dividir_CSV_em_PartesBinarias();

	// Segundo: Multi-way merging
		intercalacao_multi_caminhos();
	} catch (exception& e) {
		cout << "Erro: "<< e.what() << endl;
	}
	
	
	auto end = steady_clock::now();
	auto elapsed = end - start;
	cout << duration<double>{elapsed}.count() << "s\n";
return 0;
}
