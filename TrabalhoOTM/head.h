#define MAX_FABRICAS 100
#define MAX_DEPOSITOS 200
#define MAX_CLIENTES 400

// VARI�VEIS DE LEITURA

int QTD_FABRICAS;
int QTD_DEPOSITOS;
int QTD_CLIENTES;
int DEMANDA_CLIENTE[MAX_CLIENTES];
int CUSTO_ABT_FABRICA[MAX_FABRICAS];
int CAPACIDADE_FABRICA[MAX_FABRICAS];
int CUSTO_ABT_DEPOSITO[MAX_DEPOSITOS];
int CAPACIDADE_DEPOSITO[MAX_DEPOSITOS];
int CUSTO_TRANSP_FABRICA_DEPOSITO[MAX_FABRICAS][MAX_DEPOSITOS];
int CUSTO_TRANSP_DEPOSITO_CLIENTE[MAX_DEPOSITOS][MAX_CLIENTES];

// FUN��ESSS

void ler_instancia_arquivo(const char* path);
void criar_modelo_cplex(const char* path);
void escrever_instancia();
void cplex(const char* path_model);
void gurobi(const char* path_model);
void escrever_solucao_arquivo(const char* path, double LB, double UB, double gap, double tempo);