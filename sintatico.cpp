#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>

#include "lexico.hpp"

// Change this below to disable/enable debug messages
//#define dprint(message)
#define dprint(message) cout << linha << " " << #message << endl;

using namespace std;

void programa();
void dc_c(vector<string>);
void dc_v(vector<string>, vector<string>);
void tipo_var(vector<string>, vector<string>);
void variaveis(vector<string>, vector<string>);
void dc_p(vector<string>);
void corpo_p(vector<string>, vector<string>);
void argumentos(vector<string>, vector<string>);
void comandos(vector<string>, vector<string>);
void cmd(vector<string>, vector<string>);
void condicao(vector<string>, vector<string>);
void expressao(vector<string>, vector<string>);
void termo(vector<string>);

void erro();
void erro(string);
int  erro(string, vector<string>);
int  erro(string, vector<string>, vector<string>);

ifstream ler;
ofstream saida;
string simb;	 	//Guarda símbolo lido
string id;		 	//Guarda nome do identificador
int linha;			//Conta linha
int num_erros = 0;	//Conta qtde de erros

//Guarda comandos (muito utilizado para sincronizacao no errro)
vector<string>cmds = {"simb_begin", "simb_read", "simb_write", "simb_for", "simb_if", "simb_while", "id"};

int main(int argc, char** argv) {
	
	if (argc != 3) {
		perror("Parametros incorretos");
		exit(1);
	}
	
	//Abre arquivos de lerada e saída do analisador sintático 
	ler.open(argv[1], ios::in);
	saida.open(argv[2], ios::out);
	
	if (!ler.is_open() || !saida.is_open()) {
		cout << "Impossivel abrir/criar arquivo(s)" << endl;
		exit(0);
	}
	
	iniciar_lexico();
	
	//Inicia análise sintática
	simb = obter_simbolo();
	programa();
	if (ler.eof()) {
		cout << "Analise sintatica finalizada" << endl;
	} else {
		erro("Comandos depois do fim do programa");
	}
	
	if (num_erros == 0) cout << "Compilacao bem-sucedida" << endl;
	cout << num_erros << " erros detectados";
	
	ler.close();
	saida.close();
	
	return 0;
}

void programa() {
	if (simb.compare("simb_program") == 0) {
		dprint(Em: <programa>: simb_program lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		erro("program", {"id"});
	}
	
	if (simb.compare("id") == 0) {
		dprint(Em: <programa>: id lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		erro("identificador", {"simb_pv"});
	}
	
	if (simb.compare("simb_pv") == 0) {
		dprint(Em: <programa>: simb_pv lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		erro("\';\'", {"simb_const", "simb_var", "simb_procedure", "simb_begin"});
	}
	
	dc_c({"simb_begin", "simb_var", "simb_procedure"});
	dc_v({"simb_begin", "simb_procedure"}, {});
	dc_p({"simb_begin"});
	
	if (simb.compare("simb_begin") == 0) {
		dprint(Em: <programa>: simb_begin lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		erro("begin", {"simb_end"});
	}
	
	comandos({"simb_end"}, {});
	
	if (simb.compare("simb_end") == 0) {
		dprint(Em: <programa>: simb_end lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		cout <<"aqui" <<endl;
		erro("end", {"simb_p"});
	}
	
	if (simb.compare("simb_p") == 0) {
		dprint(Em: <programa>: simb_p lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		erro("\'.\'");
	}
}

void dc_c(vector<string> S){
	
	// const
	while(simb.compare("simb_const") == 0){
		int ret;
		dprint(Em: <dc_c>: simb_const lido pelo sintatico)
		simb = obter_simbolo();
		
		// ident
		if (simb.compare("id") == 0) {
			dprint(Em: <dc_c>: id lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("identificador", {"simb_igual", "simb_integer", "simb_real"}, S);
			if (ret <= 0) return;
		}
		
		// =
		if (simb.compare("simb_igual") == 0) {
			dprint(Em: <dc_c>: simb_igual lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\'=\'", {"simb_integer", "simb_real"}, S);
			if (ret <= 0) return;
		}
		
		// <numero>
		if (simb.compare("NUM_REAL") == 0 || simb.compare("NUM_INT") == 0) {
			dprint(Em: <dc_c>: <numero> lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("Numero", {"simb_pv"}, S);
			if (ret <= 0) return;
		}
		
		// ;
		if (simb.compare("simb_pv") == 0) {
			dprint(Em: <dc_c>: simb_pv lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\';\'", {"simb_const"}, S);
			if (ret <= 0) return;
		}
	}
}

void dc_v(vector<string> local, vector<string> S){
	int ret;
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));
	// var
	while(simb.compare("simb_var") == 0){
		dprint(Em: <dc_v>: simb_var lido pelo sintatico)
		simb = obter_simbolo();
		
		// <variaveis>
		variaveis({"simb_procedure", "simb_begin"}, S);
		
		// :
		if (simb.compare("simb_dp") == 0) {
			dprint(Em: <dc_v>: simb_dp lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\':\'", {"simb_real", "simb_integer", "simb_pv"}, S);
			if (ret <= 0) return;
		}
		
		// <tipo_var>
		tipo_var({"simb_procedure", "simb_begin"}, S);
		
		// ;
		if (simb.compare("simb_pv") == 0) {
			dprint(Em: <dc_v>: simb_pv lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\';\'", {"simb_var", "simb_procedure", "simb_begin"}, S);
			if (ret <= 0) return;
		}
	}
}

void tipo_var(vector<string> local, vector<string> S){
	int ret;
	
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));
	
	// "real" ou "integer"
	if (simb.compare("simb_real") == 0) {
		dprint(Em: <tipo_var>: simb_real lido pelo sintatico)
		simb = obter_simbolo();
	} else if(simb.compare("simb_integer") == 0){
		dprint(Em: <tipo_var>: simb_integer lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		ret = erro("Numero", V);
		if (ret <= 0) return;
	}
}

void variaveis(vector<string> local, vector<string> S){
	int continua, ret;
	
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));

	do {
		continua = 0;
		// ident
		if (simb.compare("id") == 0) {
			dprint(Em: <variaveis>: id lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("identificador", {"simb_v", "id"}, V);
			if (ret <= 0) return;
		}
		
		// ',' ou 'lambda'
		if (simb.compare("simb_v") == 0) {
			dprint(Em: <variaveis>: simb_v lido pelo sintatico)
			simb = obter_simbolo();
			continua = 1;
		}
	} while(continua);
}

void dc_p(vector<string> S){
	int ret;
	// "procedure" ou 'lambda'
	while (simb.compare("simb_procedure") == 0) {
		dprint(Em: <dc_p>: simb_procedure lido pelo sintatico)
		simb = obter_simbolo();
		
		// ident
		if (simb.compare("id") == 0) {
			dprint(Em: <dc_p>: id lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("identificador", {"simb_apar", "id", "simb_dp"}, S);
			if (ret <= 0) return;
		}
		
		// '(' ou 'lambda'
		if (simb.compare("simb_apar") == 0) {
			dprint(Em: <dc_p>: simb_apar lido pelo sintatico)
			simb = obter_simbolo();
			
			while(1){
				// <variaveis>
				variaveis({}, {});
				
				// :
				if (simb.compare("simb_dp") == 0) {
					dprint(Em: <dc_p>: simb_dp lido pelo sintatico)
					simb = obter_simbolo();
				} else {
					erro();
				}
			
				// <tipo_var>
				tipo_var({}, {});
				
				// ';' ou ')'
				if (simb.compare("simb_pv") == 0) {
					dprint(Em: <dc_p>: simb_pv lido pelo sintatico)
					simb = obter_simbolo();
					continue;
				} else if(simb.compare("simb_fpar") == 0){
					dprint(Em: <dc_p>: simb_fpar lido pelo sintatico)
					simb = obter_simbolo();
					break;
				} else {
					erro();
					break;
				}
			}
			
		}
		
		// ;
		if (simb.compare("simb_pv") == 0) {
			dprint(Em: <dc_p>: simb_pv lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\';\'", {"simb_var", "simb_begin"}, S);
			if (ret <= 0) return;
		}
		
		// <corpo_p>
		corpo_p({"simb_procedure", "simb_begin"}, S);
	}
}

void corpo_p(vector<string> local, vector<string> S){
	int ret;
	
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));

	// <dc_v>
	dc_v(cmds, V);
	
	// begin
	if (simb.compare("simb_begin") == 0) {
		dprint(Em: <corpo_p>: simb_begin lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		ret = erro("begin", cmds, V);
		if (ret <= 0) return;
	}
	
	// <comandos>
	comandos({"simb_pv"}, V);
	
	// end
	if (simb.compare("simb_end") == 0) {
		dprint(Em: <corpo_p>: simb_end lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		ret = erro("end", {"simb_pv"}, V);
		if (ret <= 0) return;
	}
	
	// ;
	if (simb.compare("simb_pv") == 0) {
		dprint(Em: <corpo_p>: simb_pv lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		erro("\';\'");
	}
}

void argumentos(vector<string> local, vector<string> S){
	int ret, continua;
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));
	
	do {
		continua = 0;
		// ident
		if (simb.compare("id") == 0) {
			dprint(Em: <argumentos>: id lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("identificador", {"simb_pv"}, S);
			if (ret <= 0) return;
		}
		
		// ';' ou 'lambda'
		if (simb.compare("simb_pv") == 0) {
			dprint(Em: <argumentos>: simb_pv lido pelo sintatico)
			simb = obter_simbolo();
			continua = 1;
		}
	} while (continua);
}

void comandos(vector<string> local, vector<string> S){
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));
	while(	simb.compare("simb_read")	== 0 ||
			simb.compare("simb_write")	== 0 ||
			simb.compare("simb_while")	== 0 ||
			simb.compare("simb_if")		== 0 ||
			simb.compare("id")			== 0 ||
			simb.compare("simb_begin")	== 0 ||
			simb.compare("simb_for")	== 0 )
	{
		dprint(Em: <comandos>: comando valido lido pelo sintatico)		
		cmd({"simb_pv"}, S);
		
		if (simb.compare("simb_pv") == 0) {
			dprint(Em: <comandos>: simb_pv lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			erro("\';\'");
		}
	}
}

void cmd(vector<string> local, vector<string> S){
	int ret;	
	
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));
	
	if(simb.compare("simb_read") == 0 || simb.compare("simb_write") == 0){
		dprint(Em: <cmd>: read-write lido pelo sintatico)
		simb = obter_simbolo();
		
		// (
		if (simb.compare("simb_apar") == 0) {
			dprint(Em: <cmd>: simb_apar lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\'(\'", {"id"}, V);
			if (ret <= 0) return;
		}
		
		// <variaveis>
		variaveis({"simb_fpar"}, V);
		
		// )
		if (simb.compare("simb_fpar") == 0) {
			dprint(Em: <cmd>: simb_fpar lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\')\'", {"id"}, V);
			if (ret <= 0) return;
		}
	}
	else if (simb.compare("simb_while") == 0){
		dprint(Em: <cmd>: simb_while lido pelo sintatico)
		simb = obter_simbolo();
		
		// (
		if (simb.compare("simb_apar") == 0) {
			dprint(Em: <cmd>: simb_apar lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\'(\'", {"id"}, V);
			if (ret <= 0) return;
		}
		
		// <condicao>
		condicao({"simb_fpar"}, V);
		
		// )
		if (simb.compare("simb_fpar") == 0) {
			dprint(Em: <cmd>: simb_fpar lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("\')\'", {"id"}, V);
			if (ret <= 0) return;
		}
		
		// do
		if (simb.compare("simb_do") == 0) {
			dprint(Em: <cmd>: simb_do lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("do", cmds, V);
			if (ret <= 0) return;
		}
		
		// <cmd>
		cmd(cmds, V);
	}
	else if (simb.compare("simb_if") == 0){
		dprint(Em: <cmd>: simb_if lido pelo sintatico)
		simb = obter_simbolo();
		
		// <condicao>
		condicao(cmds, V);
		
		// then
		if (simb.compare("simb_then") == 0) {
			dprint(Em: <cmd>: simb_then lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("then", cmds, V);
			if (ret <=0) return;
		}
		
		// <cmd>
		cmd(cmds, V);
		
		// else ou lambda
		if (simb.compare("simb_else") == 0) {
			dprint(Em: <cmd>: simb_else lido pelo sintatico)
			simb = obter_simbolo();
			
			// <cmd>
			cmd(cmds, V);
		}		
	}
	else if (simb.compare("id") == 0){
		dprint(Em: <cmd>: id lido pelo sintatico)
		simb = obter_simbolo();
		
		// ":=", '(' or 'lambda'
		if (simb.compare("simb_atrib") == 0) {
			dprint(Em: <cmd>: simb_atrib lido pelo sintatico)
			simb = obter_simbolo();
			
			// <expressao>
			expressao(cmds, V);
			
		} else if (simb.compare("simb_apar") == 0) {
			dprint(Em: <cmd>: simb_apar lido pelo sintatico)
			simb = obter_simbolo();
			
			// <argumentos>
			argumentos({"simb_fpar", "simb_begin", "simb_read", "simb_write", "simb_for", "simb_if", "simb_while", "id"}, V);
			
			// )
			if (simb.compare("simb_fpar") == 0) {
				dprint(Em: <cmd>: simb_fpar lido pelo sintatico)
				simb = obter_simbolo();
			} else {
				erro("\')\'", cmds, V);
			}
		}
	}
	else if (simb.compare("simb_begin") == 0){
		dprint(Em: <cmd>: simb_begin lido pelo sintatico)
		simb = obter_simbolo();
		
		// <comandos>
		comandos(V, {});
		
		// end
		if (simb.compare("simb_end") == 0) {
			dprint(Em: <cmd>: simb_end lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("end", cmds,V);
			if (ret <= 0);
		}
	}
	else if (simb.compare("simb_for") == 0){
		dprint(Em: <cmd>: simb_for lido pelo sintatico)
		simb = obter_simbolo();
		
		// ident
		if (simb.compare("id") == 0) {
			dprint(Em: <cmd>: id lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("identificador", {"simb_atrib"}, V);
			if (ret <= 0) return;
		}
		
		// :=
		if (simb.compare("simb_atrib") == 0) {
			dprint(Em: <cmd>: simb_atrib lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro(":=", {"simb_mais", "simb_menos", "simb_apar", "id", "NUM_INT", "NUM_REAL"}, V);
			if (ret <= 0) return;
		}
		
		// <expressao>
		expressao({"simb_to"}, V);
		
		// to
		if (simb.compare("simb_to") == 0) {
			dprint(Em: <cmd>: simb_to lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("to", {"simb_mais", "simb_menos", "simb_apar", "id", "NUM_INT", "NUM_REAL"}, V);
			if (ret <= 0) return;
		}
		
		// <expressao>
		expressao({"simb_do", "simb_begin", "simb_read", "simb_write", "simb_for", "simb_if", "simb_while", "id"}, V);
		
		// do
		if (simb.compare("simb_do") == 0) {
			dprint(Em: <cmd>: simb_do lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("do", cmds, V);
			if (ret <= 0) return;
		}
		
		// <cmd>
		cmd(cmds, V);
	}
	else {
		erro("Comando", {"simb_pv"});
	}
}

void condicao(vector<string> local, vector<string> S){
	int ret;	
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));
	
	expressao({"simb_igual", "simb_dif", "simb_maior_igual", "simb_menor_igual", "simb_maior", "simb_menor"}, V);
	
	if(	simb.compare("simb_igual")		== 0 ||
		simb.compare("simb_dif")		== 0 ||
		simb.compare("simb_maior_igual")== 0 ||
		simb.compare("simb_menor_igual")== 0 ||
		simb.compare("simb_maior")		== 0 ||
		simb.compare("simb_menor")		== 0 )
	{
		dprint(Em: <condicao>: simbolo de relacao condicional lido pelo sintatico)
		simb = obter_simbolo();
	} else {
		ret = erro("Simbolo de comparacao", {"simb_mais", "simb_menos", "simb_apar", "id", "NUM_INT", "NUM_REAL"}, V);
		if (ret <= 0) return;
	}	
	
	expressao({}, V);
}

void expressao(vector<string> local, vector<string> S){
	int ret;	
	vector<string> V(local);
	move(S.begin(), S.end(), back_inserter(V));
	
	termo(V);
	
	if(	simb.compare("simb_mais") == 0 || simb.compare("simb_menos") == 0 )
	{
		dprint(Em: <expressao>: simb_mais ou simb_menos lido pelo sintatico)
		simb = obter_simbolo();
		expressao({}, V);
	}
}

void termo(vector<string> S){
	int ret;
	// '+', '-', ou 'lambda'
	if(	simb.compare("simb_mais") == 0 ||simb.compare("simb_menos")	== 0 )
	{
		dprint(Em: <termo>: simb_mais ou simb_menos lido pelo sintatico)
		simb = obter_simbolo();
	}
	
	while(1){
		// <numero>, '(', ou ident
		if (simb.compare("simb_apar") == 0) {
			dprint(Em: <termo>: simb_apar lido pelo sintatico)
			simb = obter_simbolo();
			
			expressao({"simb_fpar"}, S);
			
			if (simb.compare("simb_fpar") == 0) {
				dprint(Em: <termo>: simb_fpar lido pelo sintatico)
				simb = obter_simbolo();
			} else {
				ret = erro("\')\'", {"simb_div", "simb_mult"}, S);
				if (ret <= 0) return;
			}
			
		} else if (	simb.compare("NUM_REAL") == 0 ||
					simb.compare("NUM_INT")  == 0 ||
					simb.compare("id") == 0)
		{
			dprint(Em: <termo>: <numero> ou id lido pelo sintatico)
			simb = obter_simbolo();
		} else {
			ret = erro("Numero ou identificador", {"simb_div", "simb_mult"}, S);
			if (ret <= 0) return;
		}
		
		// '*', '/', ou 'lambda'
		if (simb.compare("simb_mult") == 0 || simb.compare("simb_div") == 0) {
			dprint(Em: <termo>: simb_mult ou simb_div lido pelo sintatico)
			simb = obter_simbolo();
			continue;
		} else {
			break;
		}
	}
}


void erro() {
	num_erros++;
	saida << "Erro na linha " << linha << " : " << simb << endl;
}

void erro(string esp) {
	num_erros++;
		if (simb.compare("id") == 0) {
		saida << "Erro na linha " << linha << " : " << id << ". Esperado : " << esp << endl;
	} else {
		saida << "Erro na linha " << linha << " : " << simb << ". Esperado : " << esp << endl;
	}
}

/*Retorno == -1 -> Seguidor do pai encontrado
  Retorno == 0 ->  Simb. Sincr. não encontrado
  Retorno == 1 ->  Simb. Sincr. encontrado (não do pai)
*/
int erro(string esp, vector<string> seg, vector<string> segPai) {
	num_erros++;
	if (simb.compare("id") == 0) {
		saida << "Erro na linha " << linha << " : " << id << ". Esperado : " << esp << endl;
	} else {
		saida << "Erro na linha " << linha << " : " << simb << ". Esperado : " << esp << endl;
	}
	//Procura símbolos de sincronização próprios
	while (!ler.eof()) {
		if ( find(seg.begin(), seg.end(), simb) != seg.end() ) return 1;
		if ( find(segPai.begin(), segPai.end(), simb) != segPai.end() ) return -1;
		simb = obter_simbolo();
	}
	return 0;
}

//Não recebe seguidores do pai
int erro(string esp, vector<string> seg) {
	num_erros++;
	if (simb.compare("id") == 0) {
		saida << "Erro na linha " << linha << " : " << id << ". Esperado : " << esp << endl;
	} else {
		saida << "Erro na linha " << linha << " : " << simb << ". Esperado : " << esp << endl;
	}
	//Procura símbolos de sincronização próprios
	while (!ler.eof()) {
		if ( find(seg.begin(), seg.end(), simb) != seg.end() ) return 1;
		simb = obter_simbolo();
	}
	return 0;
}
