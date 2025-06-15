
// 1- modifiquei o tamanho dos registros e a parte da distruibuicao dos arquivos
// para que aceite o int e o float.

// 2- Implementei o merge sort interno,falta implementar depois o merge multi way.

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
using namespace std;

	// Definicao do numero de arquivos divididos  e nomes dos arquivos
	const int numPartes = 13;
	const string nomeBaseAqr = "parte_F";
struct Registro
{
		char series_reference[10];
		float period;
		int data_value;
		char status[10];
		char units[10];
		int magnitude;
		char subject[50];
		char group [50];
		char series_title_1[50];
		char series_title_2[50];
		char series_title_3[50];
		char series_title_4[50];
		char series_title_5[50];
};

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
		umRegistro.data_value = campo.empty() ? -1 : stoi(campo); // verifica se esta vazio, transforma string em int

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
    }
}

void dividir_CSV_em_PartesBinarias()
{	

	// 1. verificar se o arquivo esta na mesma pasta do codigo
	ifstream arqEntradaCSV("dados_trabalho.csv");
    if (!arqEntradaCSV.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo!");
    }else{
		cout << "Iniciando a divisao dos arquivos!\n";
	}
	
	// 2. criar as partes divididas
	ofstream* arqSaidaBin = new ofstream[numPartes];
	for(int i=0; i<numPartes; i++){
		string nomeArq = nomeBaseAqr +  to_string(i) + ".bin";
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

long long tamanho_Registro(const string& nomeArquivo)
{
    ifstream arquivo(nomeArquivo, ios::binary | ios::ate); // vai ate o final do arquivo
    if (!arquivo.is_open()){
		throw runtime_error("Nao foi possivel abrir o arquivo!");
	}
    long long tamanho = arquivo.tellg(); // diz o tamanho
    return (tamanho / sizeof(Registro));
}

void intercala(Registro v[], long p, long q, long r)
{
    long i = p, j = q;
    long tamanho = r - p + 1;
    Registro* aux = new Registro[tamanho]; // vetor auxiliar
    long k = 0;
    while((i < q) and (j <= r)){                
		// verifica se v[i] - v[j] > 0, se sim v[i] > v[j]
       if(strcmp(v[i].series_reference,v[j].series_reference) > 0){
           aux[k++] = v[i++];
		}else{ 
			aux[k++] = v[j++];
		}
	} 
// terminou um dos vetores, agora copia o outro
	while (i < q) {
   		aux[k++] = v[i++];
	}   
    while (j < r) {
       aux[k++] = v[j++]; 
    }
    // agora copiamos do vetor auxiliar aux[] em v[p:r]
	for (long m = 0; m < tamanho; m++){ 
		v[p + m] = aux[m];
    }
	delete [] aux;
}

void mergeSortIterativo(Registro v[], long tam)
{
    long p, r, b = 1;
    while (b < tam) {
        p = 0;
        while (p + b < tam) {
            r = p + 2*b - 1;
            if (r >= tam) r = tam - 1;
            intercala(v, p, p+b-1, r);
            p = p + 2*b;
        }
        b = 2*b; // dobra o tamanho do bloco
	}
}

void ordenar_partes_internamente()
{
	cout << "Iniciando a ordenacao interna dos arquivos!\n";

	// 1. alocar na memoria um array com os mesmos numeros de registros
	for(int i=0; i<numPartes; i++){
		string nomeArq = nomeBaseAqr +  to_string(i) + ".bin";
		long numRegistro = tamanho_Registro(nomeArq);
		Registro* umRegistroInterno = new Registro[numRegistro];

	// 2. abrir o arquivo e ler, colocando os dados em binario no umRegistroInterno
		ifstream arqEntrada(nomeArq, ios::binary);
		arqEntrada.read(reinterpret_cast<char*>(umRegistroInterno), numRegistro*sizeof(Registro));
		arqEntrada.close();

	// 3. ordenar o array internamente
		mergeSortIterativo(umRegistroInterno,numRegistro);

	// 4. jogar os arrays ordenados nesse meu arquivo, mas antes deletar oq estava escrito
		ofstream arqSaida(nomeArq, ios::binary | ios::trunc); // deleta com o trunc
		arqSaida.write(reinterpret_cast<char*>(umRegistroInterno), numRegistro * sizeof(Registro));

    // 5. fechar todos os arquivos
    arqSaida.close();

    // 6. liberar a memória alocada para o array de arquivos
    delete[] umRegistroInterno;
	}

	cout << "Ordenacao interna concluida!\n";
}


int main ()
{
	try{
	dividir_CSV_em_PartesBinarias();
	ordenar_partes_internamente();
	} catch (exception& e) {
		cout << "Erro: "<< e.what() << endl;
	}
	
	
return 0;
}
