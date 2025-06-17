
// 1- Modifiquei o tamanho dos registros e a parte da distruibuicao dos arquivos
// para que aceite o int e o float.

// 2- Implementei o merge sort interno,falta implementar depois o merge multi way.

// 3- Terminei de implementar o Merge multi way, falta a adicao/remocao/imprimir
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <chrono> // so para ver o tempo de execucao do programa
using namespace std;
using namespace chrono;

// Constantes globais
	const int numPartes = 16;
	const string nomeBaseAqr = "run_ordenada_";
	const int k = 4;
	
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
		umRegistro.data_value = campo.empty() ? -1.0f : stof(campo); // verifica se esta vazio, transforma string em int

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
	ifstream arqEntradaCSV("dados.csv");
    if (!arqEntradaCSV.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo!");
    }else{
		cout << "Iniciando a divisao dos arquivos...\n";
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


// ====================================================================
//	Segunda Parte: Ordenar esses pequenos arquivos na memoria
// ====================================================================

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
	cout << "Iniciando a ordenacao interna dos arquivos...\n";

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

// ====================================================================
//	Terceira Parte: Intercalar Externamente
// ====================================================================

void intercalar_grupo_de_arquivos(string* nomesArquivosEntrada, int numArquivos, const string& nomeArquivoSaida)
{
    if (numArquivos <= 0){
		throw runtime_error("Nenhum arquivo chamado para intercalacaO!");
	}

    // 1. alocar e abrir os arquivos de entrada
    ifstream* arqsEntrada = new ifstream[numArquivos];
    for (int i = 0; i < numArquivos; ++i) {
        arqsEntrada[i].open(nomesArquivosEntrada[i], ios::binary);
        if (!arqsEntrada[i].is_open()) {
            delete[] arqsEntrada;
			throw runtime_error("Nao foi possivel abrir o arquivo de entrada para intercalacao!");
        }
    }

    // 2. abrir o arquivo de saída
    ofstream arqSaida(nomeArquivoSaida, ios::binary | ios::trunc);
    if (!arqSaida.is_open()) {
        delete[] arqsEntrada;
        throw runtime_error("Nao foi possivel criar o arquivo de saida da intercalacao!");
    }

    // 3. buffers para os registros e controle dos arquivos
    Registro* buffer = new Registro[numArquivos];
    bool* arquivoAtivo = new bool[numArquivos];
    int arquivosRestantes = 0;

    // 4. ler o primeiro registro de cada arquivo e inicializar o controle
    for (int i = 0; i < numArquivos; ++i) {
        if (arqsEntrada[i].read(reinterpret_cast<char*>(&buffer[i]), sizeof(Registro))) {
            arquivoAtivo[i] = true;
            arquivosRestantes++; // Arquivo tem dados, incrementa o contador
        } else {
            arquivoAtivo[i] = false; // Arquivo já estava vazio
        }
    }

    // 5. loop principal de intercalação, controlado pelo contador
    while (arquivosRestantes > 0) {
        int indiceMenor = -1;
        Registro menorRegistro;

        // encontra o menor registro entre os que estão no buffer
        for (int i = 0; i < numArquivos; ++i) {
            if (arquivoAtivo[i]) {
                if (indiceMenor == -1 || strcmp(buffer[i].series_reference, menorRegistro.series_reference) < 0) {
                    indiceMenor = i;
                    menorRegistro = buffer[i];
                }
            }
        }

        arqSaida.write(reinterpret_cast<const char*>(&menorRegistro), sizeof(Registro));

        // le o próximo registro do arquivo de onde o menor veio
        if (!arqsEntrada[indiceMenor].read(reinterpret_cast<char*>(&buffer[indiceMenor]), sizeof(Registro))) {
            arquivoAtivo[indiceMenor] = false; // Arquivo acabou
            arquivosRestantes--; // Decrementa o contador, pois um arquivo terminou
        }
    }

    // 6. fecha todos os arquivos e libera a memoria
    arqSaida.close();
    for (int i = 0; i < numArquivos; ++i) {
        arqsEntrada[i].close();
    }
    
    delete[] arqsEntrada;
    delete[] buffer;
    delete[] arquivoAtivo;
}

void intercalacao_multi_caminhos() // distribuidor
{
    cout << "Iniciando intercalacao com " << k << " f...\n";

	// 1. cria uma lista de string com o nome dos meus arquivos
    int numArquivosFonte = numPartes;
    string* arquivosFonte = new string[numArquivosFonte];
    for (int i = 0; i < numArquivosFonte; i++) {
        arquivosFonte[i] = nomeBaseAqr + to_string(i) + ".bin";
    }

    int passo = 1;
    // 2. intercala os arquivos ate houver apenas 1 arquivo
    while (numArquivosFonte > 1) {
        int numArquivosDestino = (numArquivosFonte + k - 1) / k; // Cálculo do teto da divisão
        string* arquivosDestino = new string[numArquivosDestino];
        int contadorArquivoSaida = 0;

        // itera sobre os arquivos fonte em grupos de 'k'
        for (int i = 0; i < numArquivosFonte; i += k) {
            int tamanhoGrupo = min(k, numArquivosFonte - i);
            
            // O grupo a ser intercalado é um sub-array de arquivosFonte
            // apontando para arquivosFonte[i]
            
            // Define o nome do arquivo de saída para este grupo
            string nomeSaida = "temp_passo" + to_string(passo) + "_" + to_string(contadorArquivoSaida) + ".bin";
            
            // Chama a função que faz a intercalação do grupo
            intercalar_grupo_de_arquivos(&arquivosFonte[i], tamanhoGrupo, nomeSaida);
            
            // Adiciona o novo arquivo gerado à lista de destino
            arquivosDestino[contadorArquivoSaida] = nomeSaida;
            contadorArquivoSaida++;
        }

        // limpa os arquivos do passo anterior
        for (int i = 0; i < numArquivosFonte; i++) {
            remove(arquivosFonte[i].c_str());
        }
        delete[] arquivosFonte; // Libera memória do array de nomes antigo

        // os arquivos de destino agora são a fonte para o próximo passo
        arquivosFonte = arquivosDestino;
        numArquivosFonte = numArquivosDestino;
        passo++;
    }

    // ao final, sobrará apenas um arquivo na lista, que é o resultado final
    if (numArquivosFonte == 1) {
        string nomeFinal = "dados_ordenados.bin";
        rename(arquivosFonte[0].c_str(), nomeFinal.c_str());
        cout << "Intercalacao concluida!\n";
    } else {
        throw runtime_error("Nenhum arquivo final gerado!");
    }
    
    // Libera a memória do último array de nomes de arquivo
    if (numArquivosFonte > 0) {
        delete[] arquivosFonte;
    }
}

int main ()
{
	auto start = steady_clock::now();
	
	try{
	// Primeiro Passo: dividir o csv em arquivos binarios nao ordenados
		dividir_CSV_em_PartesBinarias();
		
	// Segundo Passo: ordenar cada arquivo em binario
		ordenar_partes_internamente();
		
	// Terceiro Passo: Merge Multi-way
		intercalacao_multi_caminhos();
	} catch (exception& e) {
		cout << "Erro: "<< e.what() << endl;
	}
	
	
	auto end = steady_clock::now();
	auto elapsed = end - start;
	cout << duration<double>{elapsed}.count() << "s\n";
return 0;
}
