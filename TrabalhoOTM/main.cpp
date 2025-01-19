#include "ilcplex\cplex.h"
#include "ilcplex\ilocplex.h"
#include "gurobi_c++.h"
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <ctime>
#include "head.h"
#define TEMP_EXECUCAO 3600 // em segundos
#define PATH_MODEL "..\\model\\"
#define PATH_INSTANCES "..\\instancias\\"
#define PATH_SOLUTIONS_CPLEX "..\\solucoes\\cplex\\"
#define PATH_SOLUTIONS_GUROBI "..\\solucoes\\gurobi\\"
#define FILE_LP ".lp"
#define FILE_TXT ".txt"
#define FILE_SOL ".sol"

#define INSTANCE "dificil-PSC5-C5-100"


int main() {

    const char* path_instance = PATH_INSTANCES INSTANCE FILE_TXT;
    ler_instancia_arquivo(path_instance);
    //escrever_instancia();

    const char* path_model = PATH_MODEL INSTANCE FILE_LP;
    criar_modelo_cplex(path_model);

    cplex(path_model);
    gurobi(path_model);
    
    return 0;
}

void cplex(const char* path_model) {
    
    IloEnv env;
    IloModel model(env);
    IloCplex cplex(env);


    IloObjective obj(env);
    IloNumVarArray vars(env);
    IloRangeArray constraints(env);

    cplex.importModel(model, path_model, obj, vars, constraints);
    cplex.extract(model);

    cplex.setParam(IloCplex::Param::TimeLimit, TEMP_EXECUCAO);

    clock_t inicio, fim;
    double tempo_decorrido;

    inicio = clock();
    tempo_decorrido = 0;

    if (cplex.solve()) {
        fim = clock() - inicio;
        tempo_decorrido = (double)fim / CLOCKS_PER_SEC;

        printf("Solucao encontrada\n");
        printf("FO: %f\n", cplex.getObjValue());
        printf("Lower bound: %f\n", cplex.getBestObjValue());
        printf("Gap relativo (%): %f\n", cplex.getMIPRelativeGap() * 100);
        printf("Tempo de execucao (s): %f\n", tempo_decorrido);



        const char* path_solution = PATH_SOLUTIONS_CPLEX INSTANCE FILE_SOL;
        cplex.writeSolution(path_solution);
        printf("Solucao salva em %s", path_solution);

        const char* path_resultados = PATH_SOLUTIONS_CPLEX INSTANCE FILE_TXT;
        escrever_solucao_arquivo(path_resultados, cplex.getBestObjValue(), cplex.getObjValue(), cplex.getMIPRelativeGap() * 100, tempo_decorrido);

        printf("Resultado salvo em %s", path_resultados);

    }
    else {
        printf("Não foi possivel encontrar uma solucao\n");
    }

    env.end();

}

void gurobi(const char* path_model) {

    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "gurobi.log");
    env.start();

    GRBModel model = GRBModel(env, path_model);

    model.set(GRB_DoubleParam_TimeLimit, TEMP_EXECUCAO);

    clock_t inicio, fim;
    double tempo_decorrido;
    inicio = clock();
    tempo_decorrido = 0;

    model.optimize();

    fim = clock() - inicio;
    tempo_decorrido = (double)fim / CLOCKS_PER_SEC;
    printf("Solucao encontrada\n");
    printf("FO: %f\n", model.get(GRB_DoubleAttr_ObjVal));
    printf("Lower bound: %f\n", model.get(GRB_DoubleAttr_ObjBound));
    printf("Gap relativo (%): %f\n", model.get(GRB_DoubleAttr_MIPGap) * 100);
    printf("Tempo de execucao (s): %f\n", tempo_decorrido);

    const char* path_solution = PATH_SOLUTIONS_GUROBI INSTANCE FILE_SOL;
    model.write(path_solution);
    printf("Solucao salva em %s", path_solution);

    const char* path_resultados = PATH_SOLUTIONS_GUROBI INSTANCE FILE_TXT;
    escrever_solucao_arquivo(path_resultados, model.get(GRB_DoubleAttr_ObjBound), model.get(GRB_DoubleAttr_ObjVal), model.get(GRB_DoubleAttr_MIPGap) * 100, tempo_decorrido);

    printf("Resultado salvo em %s", path_resultados);
}

void ler_instancia_arquivo(const char* path)
{
    FILE* f = nullptr;
    if (fopen_s(&f, path, "r") != 0) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", path);
        return;
    }

    fscanf_s(f, "%d %d %d", &QTD_FABRICAS, &QTD_DEPOSITOS, &QTD_CLIENTES);

    for (int i = 0; i < QTD_CLIENTES; i++)
    {
        fscanf_s(f, "%d", &DEMANDA_CLIENTE[i]);
    }

    for (int i = 0; i < QTD_FABRICAS; i++) {
        fscanf_s(f, "%d %d", &CAPACIDADE_FABRICA[i],&CUSTO_ABT_FABRICA[i]);
    }

    for (int i = 0; i < QTD_FABRICAS; i++)
    {
        for (int j = 0; j < QTD_DEPOSITOS; j++)
        {
            fscanf_s(f, "%d", &CUSTO_TRANSP_FABRICA_DEPOSITO[i][j]);
        }
    }

    for (int i = 0; i < QTD_DEPOSITOS; i++) {
        fscanf_s(f, "%d %d", &CAPACIDADE_DEPOSITO[i], &CUSTO_ABT_DEPOSITO[i]);
    }

    for (int j = 0; j < QTD_DEPOSITOS; j++)
    {
        for (int k = 0; k < QTD_CLIENTES; k++)
        {
            fscanf_s(f, "%d", &CUSTO_TRANSP_DEPOSITO_CLIENTE[j][k]);
        }
    }
}

void criar_modelo_cplex(const char* path) {
    FILE* f = nullptr;
    if (fopen_s(&f, path, "w") != 0) {
        printf("Erro ao abrir o arquivo: %s\n", path);
        return;
    }

    fprintf_s(f, "Minimize\n");
    fprintf(f, "obj:\n"); // -----------------

    for (int i = 0; i < QTD_FABRICAS; i++) {
        fprintf(f, "%d y_%d + ", CUSTO_ABT_FABRICA[i], i);
    }

    for (int j = 0; j < QTD_DEPOSITOS; j++) {
        fprintf(f, "%d z_%d + ", CUSTO_ABT_DEPOSITO[j], j);
    }

    for (int i = 0; i < QTD_FABRICAS; i++) {
        for (int j = 0; j < QTD_DEPOSITOS; j++) {
            fprintf(f, "%d x_%d_%d + ", CUSTO_TRANSP_FABRICA_DEPOSITO[i][j], i, j);
        }
    }

    for (int j = 0; j < QTD_DEPOSITOS; j++) {
        for (int k = 0; k < QTD_CLIENTES; k++) {
            if (k == QTD_CLIENTES - 1 && j == QTD_DEPOSITOS - 1) {
                fprintf(f, "%d s_%d_%d", CUSTO_TRANSP_DEPOSITO_CLIENTE[j][k], j, k);
            }
            else {
                fprintf(f, "%d s_%d_%d + ", CUSTO_TRANSP_DEPOSITO_CLIENTE[j][k], j, k);
            }
        }
    }
    fprintf(f, "\n\n");

    fclose(f);
    if (fopen_s(&f, path, "a") != 0) {
        printf("Erro ao abrir o arquivo: %s\n", path);
        return;
    }

    fprintf(f, "Subject To\n"); // -----------------
    for (int k = 0; k < QTD_CLIENTES; k++)
    {
        for (int j = 0; j < QTD_DEPOSITOS; j++) {
            if (j == QTD_DEPOSITOS - 1) {
                fprintf(f, "s_%d_%d >= %d\n", j, k, DEMANDA_CLIENTE[k]);
            }
            else {
                fprintf(f, "s_%d_%d + ", j, k);
            }
        }
        //fprintf(f, ">= %d\n", DEMANDA_CLIENTE[k]);
    }

    for (int j = 0; j < QTD_DEPOSITOS; j++) {
        for (int i = 0; i < QTD_FABRICAS; i++) {
            if (i == QTD_FABRICAS - 1) {
                fprintf(f, "x_%d_%d - ", i, j);
            }
            else {
                fprintf(f, "x_%d_%d + ", i, j);
            }
        }
        
        for (int k = 0; k < QTD_CLIENTES; k++) {
            if (k == QTD_CLIENTES - 1) {
                fprintf(f, "s_%d_%d >= 0\n", j, k);
            }
            else {
                fprintf(f, "s_%d_%d - ", j, k);
            }
        }
        //fprintf(f, ">= 0\n");
        //fprintf(f, "\n");
    }

    for (int i = 0; i < QTD_FABRICAS; i++) {
        for (int j = 0; j < QTD_DEPOSITOS; j++) {
            if (j == QTD_DEPOSITOS - 1) {
                fprintf(f, "x_%d_%d ", i, j);
            }
            else {
                fprintf(f, "x_%d_%d + ", i, j);
            }
        }
        fprintf(f, "- %d y_%d <= 0\n", CAPACIDADE_FABRICA[i], i);
    }

    for (int j = 0; j < QTD_DEPOSITOS; j++) {
        for (int k = 0; k < QTD_CLIENTES; k++) {
            if (k == QTD_CLIENTES - 1) {
                fprintf(f, "s_%d_%d ", j, k);
            }
            else {
                fprintf(f, "s_%d_%d + ", j, k);
            }
        }
        fprintf(f, "- %d z_%d <= 0\n", CAPACIDADE_DEPOSITO[j], j);
    }
    fprintf(f, "\n");

    fclose(f);
    if (fopen_s(&f, path, "a") != 0) {
        printf("Erro ao abrir o arquivo: %s\n", path);
        return;
    }

    fprintf(f, "Bounds\n"); // -----------------
    for (int i = 0; i < QTD_FABRICAS; i++) {
        for (int j = 0; j < QTD_DEPOSITOS; j++) {
            fprintf(f, "x_%d_%d > 0\n", i, j);
        }
    }

    for (int j = 0; j < QTD_DEPOSITOS; j++) {
        for (int k = 0; k < QTD_CLIENTES; k++) {
            fprintf(f, "s_%d_%d > 0\n", j, k);
        }
    }
    fprintf(f, "\n");

    fclose(f);
    if (fopen_s(&f, path, "a") != 0) {
        printf("Erro ao abrir o arquivo: %s\n", path);
        return;
    }

    fprintf(f, "Binaries\n"); // -----------------
    for (int i = 0; i < QTD_FABRICAS; i++) {
        fprintf(f, "y_%d\n", i);
    }

    for (int j = 0; j < QTD_DEPOSITOS; j++) {
        fprintf(f, "z_%d\n", j);
    }
    fprintf(f, "\n");

    fprintf(f, "End"); // -----------------

    fclose(f);

}

void escrever_instancia() {

    printf("Quantidade de Fabricas: %d\n", QTD_FABRICAS);
    printf("Quantidade de Depositos: %d\n", QTD_DEPOSITOS);
    printf("Quantidade de Clientes: %d\n\n", QTD_CLIENTES);

    printf("Demanda dos Clientes:\n");
    for (int i = 0; i < QTD_CLIENTES; i++) {
        printf("Cliente %d: %d\n", i + 1, DEMANDA_CLIENTE[i]);
    }
    printf("\n");

    printf("Custo de Abertura e Capacidade das Fabricas:\n");
    for (int i = 0; i < QTD_FABRICAS; i++) {
        printf("Fabrica %d - Custo: %d, Capacidade: %d\n", i + 1, CUSTO_ABT_FABRICA[i], CAPACIDADE_FABRICA[i]);
    }
    printf("\n");

    printf("Custo de Transporte entre Fabricas e Depositos:\n");
    for (int i = 0; i < QTD_FABRICAS; i++) {
        for (int j = 0; j < QTD_DEPOSITOS; j++) {
            printf("Fabrica %d -> Deposito %d: %d\n", i + 1, j + 1, CUSTO_TRANSP_FABRICA_DEPOSITO[i][j]);
        }
    }
    printf("\n");

    printf("Custo de Abertura e Capacidade dos Depositos:\n");
    for (int i = 0; i < QTD_DEPOSITOS; i++) {
        printf("Deposito %d - Custo: %d, Capacidade: %d\n", i + 1, CUSTO_ABT_DEPOSITO[i], CAPACIDADE_DEPOSITO[i]);
    }
    printf("\n");

    printf("Custo de Transporte entre Depositos e Clientes:\n");
    for (int j = 0; j < QTD_DEPOSITOS; j++) {
        for (int k = 0; k < QTD_CLIENTES; k++) {
            printf("Deposito %d -> Cliente %d: %d\n", j + 1, k + 1, CUSTO_TRANSP_DEPOSITO_CLIENTE[j][k]);
        }
    }
    printf("\n");
}

void escrever_solucao_arquivo(const char* path, double LB, double UB, double gap, double tempo) {
    
    FILE* f = nullptr;
    if (fopen_s(&f, path, "w") != 0) {
        printf("Erro ao abrir o arquivo: %s\n", path);
        return;
    }

    fprintf(f, "LB: %f\n", LB);
    fprintf(f, "UB: %f\n", UB);
    fprintf(f, "Gap (%): %f\n", gap);
    fprintf(f, "Tempo (s): %f\n", tempo);

    fclose(f);
}