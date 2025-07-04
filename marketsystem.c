#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <time.h>

// --- Define o nome do arquivo de produtos ---
#define ARQPRODUTOS "produtos.txt"                   
// --- Define o nome do arquivo do caixa ---
#define ARQCAIXA "caixa.txt"                         
// --- Define o nome do arquivo de vendas detalhadas ---
#define ARQ_VDETALHADAS "vendas_detalhadas.txt" 
// --- Define o nome do arquivo de log do sistema ---
#define ARQLOG "log.txt"                             
// --- Define o tamanho máximo para strings ---
#define MAX_TAMANHO 100                              

float totalpendente = 0; 
int caixa_aberto = 0;    
float saldo_caixa = 0;   

// --- Estrutura que representa um produto ---
typedef struct produto
{
    char categoria[MAX_TAMANHO];
    int codigo;
    char nome[MAX_TAMANHO];
    float preco;
    int quantidade;
} produto;

// --- Estrutura que representa um nó da árvore binária ---
typedef struct no
{
    produto p;
    struct no *esquerda;
    struct no *direita;
} no;

// --- Estrutura que representa um item do carrinho de compras (lista encadeada) ---
typedef struct item_carrinho
{
    produto p_vendido;
    int quantidade_vendida;
    struct item_carrinho *proximo;
} item_carrinho;

typedef struct
{
    char data_hora[20];
    int codigo;
    char nome[MAX_TAMANHO];
    int quantidade;
    float preco_unitario;
} venda_detalhada;

item_carrinho *carrinho_atual = NULL;


// --- Função para limpar o buffer do teclado ---
void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- Função para registrar eventos no arquivo de log ---
void registrar_log(const char *mensagem) {
    FILE *arquivo = fopen(ARQLOG, "a");
    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo de log"); 
        return;
    }

    
    time_t tempo_atual;
    char buffer_tempo[26];
    struct tm *info_tempo;

    time(&tempo_atual);
    info_tempo = localtime(&tempo_atual);
    strftime(buffer_tempo, sizeof(buffer_tempo), "%Y-%m-%d %H:%M:%S", info_tempo);

    fprintf(arquivo, "[%s] %s\n", buffer_tempo, mensagem); 
    fclose(arquivo);
}

// --- Função que cria um novo nó para a árvore binária ---
no *criar_no(produto p) {
    no *novo_no = (no *)malloc(sizeof(no)); 
    if (novo_no == NULL) {
        registrar_log("ERRO: Falha ao alocar memória para novo nó da árvore."); 
        return NULL; 
    }

    novo_no->p = p; 
    novo_no->esquerda = NULL; 
    novo_no->direita = NULL; 

    return novo_no; 
}

// --- Função que insere um produto na árvore binária ---
no *inserir_na_arvore(no *raiz, produto p) {
    if (raiz == NULL) { 
        return criar_no(p);
    }

    
    if (p.codigo < raiz->p.codigo) { 
        raiz->esquerda = inserir_na_arvore(raiz->esquerda, p); 
    }
    else if (p.codigo > raiz->p.codigo) { 
        raiz->direita = inserir_na_arvore(raiz->direita, p); 
    }

    return raiz; 
}

// --- Função que busca um produto na árvore binária pelo código ---
no *buscar_na_arvore(no *raiz, int codigo) {
    if (raiz == NULL || raiz->p.codigo == codigo) {
        return raiz;
    }

    
    if (codigo < raiz->p.codigo) {
        return buscar_na_arvore(raiz->esquerda, codigo);
    }
    else {
        return buscar_na_arvore(raiz->direita, codigo);
    }
}

// --- Função recursiva para salvar os nós da árvore em arquivo ---
void salvar_arvore_recursivo(FILE *arq, no *raiz) {
    if (raiz != NULL) { 
        salvar_arvore_recursivo(arq, raiz->esquerda); 
        fprintf(arq, "%s;%d;%s;%.2f;%d\n", raiz->p.categoria, raiz->p.codigo, raiz->p.nome, raiz->p.preco, raiz->p.quantidade); 
        salvar_arvore_recursivo(arq, raiz->direita); 
    } 
}

// --- Função que salva todos os produtos da árvore em arquivo ---
void salvar_produtos(const char *nomeArquivo, no *raiz) {
    FILE *arquivo = fopen(nomeArquivo, "w"); 
    if (arquivo == NULL) {
        registrar_log("ERRO: Falha ao abrir arquivo de produtos para escrita.");
        return;
    }
    salvar_arvore_recursivo(arquivo, raiz); 
    fclose(arquivo);
}

// --- Função que carrega os produtos do arquivo para a árvore ---
no *carregar_produtos(no *raiz) {
    FILE *arquivo = fopen(ARQPRODUTOS, "r");
    if (arquivo == NULL) {
        arquivo = fopen(ARQPRODUTOS, "w");
        if (arquivo != NULL) {
            fclose(arquivo);
            registrar_log("Arquivo de produtos não encontrado. Um novo foi criado.");
        }
        return NULL;
    }

    char linha[MAX_TAMANHO * 2];
    produto prod;

    while (fgets(linha, sizeof(linha), arquivo)) {
        linha[strcspn(linha, "\n")] = 0;

        if (sscanf(linha, "%[^;];%d;%[^;];%f;%d", prod.categoria, &prod.codigo, prod.nome, &prod.preco, &prod.quantidade) == 5) {
            raiz = inserir_na_arvore(raiz, prod); 
        }
    }
    fclose(arquivo);
    registrar_log("Produtos carregados do arquivo para a árvore.");
    return raiz;
}

// --- Função que libera a memória da árvore binária ---
void liberar_arvore(no *raiz) {
    if (raiz != NULL) {
        liberar_arvore(raiz->esquerda);
        liberar_arvore(raiz->direita);
        free(raiz);
    }
}

// --- Função que adiciona item ao carrinho ---
void carrinho_add_item(produto p, int quantidade) {
    item_carrinho *novo_item = (item_carrinho *)malloc(sizeof(item_carrinho)); 
    
    novo_item->p_vendido = p; 
    novo_item->quantidade_vendida = quantidade; 
    novo_item->proximo = carrinho_atual; 
    carrinho_atual = novo_item; 
}

// --- Função que limpa o carrinho (libera a lista encadeada) ---
void limpar_carrinho() {
    item_carrinho *atual = carrinho_atual;
    item_carrinho *proximo;
    while (atual != NULL) {
        proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    carrinho_atual = NULL;
}

// --- Função que finaliza a compra e registra no sistema ---
void finalizar_compra(no *raiz) {
    if (!caixa_aberto) {
        printf("O caixa está fechado. Abra o caixa para finalizar a compra.\n");
        Sleep(1000);
        return;
    }
    if (totalpendente <= 0 || carrinho_atual == NULL) {
        printf("Nenhum item no carrinho para finalizar.\n");
        Sleep(1000);
        return;
    }

    int opc;
    printf("Forma de pagamento:\n1 - Dinheiro\n2 - Cartão\nOpção: ");
    if (scanf("%d", &opc) != 1 || (opc != 1 && opc != 2)) {
        limpar_buffer();
        printf("Forma de pagamento inválida.\n");
        Sleep(1000);
        return;
    }
    limpar_buffer();

    FILE *arquivo = fopen(ARQ_VDETALHADAS, "a"); 
    if (arquivo == NULL) {
        perror("Erro ao registrar a venda.");
        registrar_log("ERRO: Falha ao abrir arquivo de vendas detalhadas.");
        Sleep(1000);
        return;
    }

    
    time_t tempo_atual;
    char buffer_tempo[20];
    time(&tempo_atual);
    strftime(buffer_tempo, sizeof(buffer_tempo), "%Y-%m-%d %H:%M:%S", localtime(&tempo_atual));

    item_carrinho *item = carrinho_atual; 
    while (item != NULL) {
        fprintf(arquivo, "%s;%d;%s;%d;%.2f\n", buffer_tempo, item->p_vendido.codigo, item->p_vendido.nome, item->quantidade_vendida, item->p_vendido.preco);
        item = item->proximo;
    }
    fclose(arquivo);

    
    saldo_caixa += totalpendente;
    char log_msg[100];
    sprintf(log_msg, "Venda finalizada no valor de R$ %.2f.", totalpendente);
    registrar_log(log_msg);

    
    limpar_carrinho();
    totalpendente = 0;

    
    salvar_produtos(ARQPRODUTOS, raiz);
    printf("Compra finalizada com sucesso!\n");
    system("pause");
}

// --- Função de interface para adicionar produtos ao carrinho ---
void carrinho(no *raiz, char *categoria) {
    if (!caixa_aberto) {
        printf("O caixa está fechado. Abra o caixa para realizar compras.\n");
        Sleep(1000);
        return;
    }

    int opc, quantidade;
    printf("--------------------------------------------------------\n");
    printf("Insira o código do produto para adicionar ao carrinho, ou insira 0 para voltar: ");
    if (scanf("%d", &opc) != 1) {
        limpar_buffer();
        printf("Opção inválida.\n");
        Sleep(1000);
        return;
    }
    if (opc == 0) {
        return;
    }
    limpar_buffer();

    no *no_encontrado = buscar_na_arvore(raiz, opc); 

    if (no_encontrado != NULL && strcmp(no_encontrado->p.categoria, categoria) == 0) { 
        printf("Adicionando \"%s\" ao carrinho.\n", no_encontrado->p.nome);
        printf("Insira a quantidade: ");
        if (scanf("%d", &quantidade) != 1 || quantidade <= 0) { 
            limpar_buffer();
            printf("Quantidade inválida.\n");
            Sleep(1000);
            return;
        }

        if (quantidade > no_encontrado->p.quantidade) { 
            printf("Quantidade fora do estoque. Disponível: %d\n", no_encontrado->p.quantidade);
            Sleep(1000);
            return;
        }

        
        no_encontrado->p.quantidade -= quantidade; 
        totalpendente += quantidade * no_encontrado->p.preco; 
        carrinho_add_item(no_encontrado->p, quantidade); 

        printf("Foram adicionados %d %s ao carrinho. Total parcial: %.2f\n", quantidade, no_encontrado->p.nome, totalpendente);
        system("pause");
    } else {
        printf("Produto não encontrado ou não pertence a categoria %s.\n", categoria);
        Sleep(1000);
    }
}

// --- Função que zera o estoque da padaria ---
void zerar_estoque_padaria_arvore(no *raiz) {
    if (raiz != NULL) {
        zerar_estoque_padaria_arvore(raiz->esquerda);
        if (strcmp(raiz->p.categoria, "PADARIA") == 0) { 
            raiz->p.quantidade = 0;
        }
        zerar_estoque_padaria_arvore(raiz->direita);
    }
}

// --- Função que abre ou fecha o caixa e zera a padaria ---
void abrir_fechar_caixa(no *raiz) {
    if (!caixa_aberto) { 
        
        FILE *arquivo = fopen(ARQCAIXA, "r");

        if (arquivo != NULL) { 
            fscanf(arquivo, "%f", &saldo_caixa);
            fclose(arquivo);
        } else { 
            saldo_caixa = 0;
        }
        caixa_aberto = 1; 
        printf("Caixa aberto com saldo de R$ %.2f\n", saldo_caixa);
        registrar_log("Caixa aberto.");
    } else {
        
        FILE *arquivo = fopen(ARQCAIXA, "w");
        if (arquivo != NULL) {
            fprintf(arquivo, "%.2f\n", saldo_caixa); 
            fclose(arquivo);
        }
        caixa_aberto = 0; 
        printf("Caixa fechado. Saldo salvo em %s\n", ARQCAIXA);
        registrar_log("Caixa fechado.");
    }

    
    zerar_estoque_padaria_arvore(raiz);
    salvar_produtos(ARQPRODUTOS, raiz);
    printf("Estoque da padaria foi zerado.\n");
    registrar_log("Estoque da padaria zerado.");
    system("pause");
}

// --- Função de interface para adicionar novos produtos ---
void adicionar_produto(no **p_raiz) {
    produto novo;
    int opc;

    system("cls");
    printf("Escolha a categoria do novo produto:\n1 - Limpeza\n2 - Alimentos\n3 - Padaria\nOpção: ");
    if (scanf("%d", &opc) != 1) {
        limpar_buffer();
        printf("Opção inválida.\n");
        Sleep(1000);
        return;
    }

    switch (opc) {
    case 1:
        strcpy(novo.categoria, "LIMPEZA");
        break;
    case 2:
        strcpy(novo.categoria, "ALIMENTOS");
        break;
    case 3:
        strcpy(novo.categoria, "PADARIA");
        break;
    default:
        printf("Categoria inválida.\n");
        Sleep(1000);
        return;
    }
    limpar_buffer();

    printf("Insira o código do produto: ");
    if (scanf("%d", &novo.codigo) != 1) {
        limpar_buffer();
        printf("Código inválido.\n");
        Sleep(1000);
        return;
    }
    limpar_buffer();

    if (buscar_na_arvore(*p_raiz, novo.codigo) != NULL) { 
        printf("Erro: Código de produto já existente.\n");
        Sleep(1000);
        return;
    }

    printf("Insira o nome do produto: "); 
    fgets(novo.nome, sizeof(novo.nome), stdin);
    novo.nome[strcspn(novo.nome, "\n")] = 0; 

    printf("Insira o preço do produto: "); 
    if (scanf("%f", &novo.preco) != 1 || novo.preco <= 0) {
        limpar_buffer();
        printf("Preço inválido.\n");
        Sleep(1000);
        return;
    }
    limpar_buffer();

    printf("Insira a quantidade do produto: "); 
    if (scanf("%d", &novo.quantidade) != 1 || novo.quantidade < 0) {
        limpar_buffer();
        printf("Quantidade inválida.\n");
        Sleep(1000);
        return;
    }
    limpar_buffer();

    
    *p_raiz = inserir_na_arvore(*p_raiz, novo);
    salvar_produtos(ARQPRODUTOS, *p_raiz);

    system("cls");
    printf("Produto adicionado com sucesso.\n");
    char log_msg[150];
    sprintf(log_msg, "Produto adicionado: Cod %d, Nome %s, Categoria %s", novo.codigo, novo.nome, novo.categoria); 
    registrar_log(log_msg);
    printf("Categoria: %s\nCódigo: %d\nNome: %s\nPreço: %.2f\nQuantidade: %d\n", novo.categoria, novo.codigo, novo.nome, novo.preco, novo.quantidade);
    system("pause");
}

// --- Função que exibe produtos da árvore filtrados por categoria ---
void exibir_produtos_da_arvore(no *raiz, const char *categoria) {
    if (raiz != NULL) {
        exibir_produtos_da_arvore(raiz->esquerda, categoria);
        if (strcmp(raiz->p.categoria, categoria) == 0) { 
            printf("%-5d %-30s R$%-9.2f %-10d\n", raiz->p.codigo, raiz->p.nome, raiz->p.preco, raiz->p.quantidade);
        }
        exibir_produtos_da_arvore(raiz->direita, categoria);
    }
}

// --- Função de interface para exibir produtos e permitir compra ---
void exibir_produtos(no *raiz, char *categoria) {
    system("cls");
    printf("CATEGORIA: %s.\n", categoria);
    printf("%-5s %-30s %-10s %-10s\n", "ID", "NOME", "PREÇO", "QUANTIDADE");
    printf("---------------------------------------------------------------\n");
    exibir_produtos_da_arvore(raiz, categoria);
    
    carrinho(raiz, categoria);
}

// --- Função que atualiza o estoque da padaria recursivamente ---
void atualizar_estoque_padaria_recursivo(no *raiz) {
    if (raiz != NULL) {
        atualizar_estoque_padaria_recursivo(raiz->esquerda);
        
        if (strcmp(raiz->p.categoria, "PADARIA") == 0) { 
            int nova_quantidade;
            printf("Produto: %s | Qtd atual: %d | Nova quantidade: ", raiz->p.nome, raiz->p.quantidade);
            if (scanf("%d", &nova_quantidade) != 1 || nova_quantidade < 0) { 
                printf("Inválido. Pulando.\n");
                limpar_buffer();
            } else {
                raiz->p.quantidade = nova_quantidade; 
                limpar_buffer();
            }
        }
        atualizar_estoque_padaria_recursivo(raiz->direita);
    }
}

// --- Função de interface para atualizar estoque da padaria ---
void atualizar_estoque_padaria(no *raiz) {
    if (!caixa_aberto) {
        printf("O caixa está fechado. Abra o caixa para atualizar o estoque.\n");
        Sleep(1000);
        return;
    }
    printf("Atualizando o estoque da padaria...\n");
    atualizar_estoque_padaria_recursivo(raiz);
    salvar_produtos(ARQPRODUTOS, raiz);
    printf("Estoque da padaria atualizado com sucesso.\n");
    registrar_log("Estoque da padaria atualizado manualmente.");
    system("pause");
}

// --- Função que compara produtos por nome (para ordenação) ---
int cmp_prod_por_nome(const void *a, const void *b) {
    produto *pA = (produto *)a; 
    produto *pB = (produto *)b; 
    return strcmp(pA->nome, pB->nome); 
}

// --- Função que compara vendas por nome ---
int comparar_vendas_por_nome(const void *a, const void *b) {
    venda_detalhada *vA = (venda_detalhada *)a;
    venda_detalhada *vB = (venda_detalhada *)b;
    return strcmp(vA->nome, vB->nome); 
}

// --- Função que compara vendas por código ---
int comparar_vendas_por_codigo(const void *a, const void *b) {
    venda_detalhada *vA = (venda_detalhada *)a;
    venda_detalhada *vB = (venda_detalhada *)b;
    return vA->codigo - vB->codigo; 
}

// --- Função que converte a árvore em array ---
void arvore_para_array(no *raiz, produto **array, int *contador) {
    if (raiz == NULL) {
        return;
    }
    arvore_para_array(raiz->esquerda, array, contador);
    (*array)[*contador] = raiz->p; 
    (*contador)++; 
    arvore_para_array(raiz->direita, array, contador);
}

// --- Função que exibe produtos ordenados por código ---
void exibir_produtos_ordenados(no *raiz) {
    if (raiz != NULL) {
        exibir_produtos_ordenados(raiz->esquerda);
        printf("%-5d %-30s R$%-9.2f %-10d %-15s\n", raiz->p.codigo, raiz->p.nome, raiz->p.preco, raiz->p.quantidade, raiz->p.categoria);
        exibir_produtos_ordenados(raiz->direita);
    }
}

// --- Função que exibe relatório de produtos ---
void relatorio_produtos_por_ordem(no *raiz, int por_nome) {
    system("cls");
    if (por_nome) {
        printf("--- Relatório de Produtos (Ordem Alfabética) ---\n");
    } else {
        printf("--- Relatório de Produtos (Ordem de Código) ---\n");
    }
    printf("%-5s %-30s %-10s %-10s %-15s\n", "ID", "NOME", "PREÇO", "QTD", "CATEGORIA");
    printf("----------------------------------------------------------------------\n");

    if (por_nome) {

        produto *produtos_array = (produto *)malloc(sizeof(produto) * 1000); 
        int contador = 0;

        arvore_para_array(raiz, &produtos_array, &contador);
        qsort(produtos_array, contador, sizeof(produto), cmp_prod_por_nome);

        for (int i = 0; i < contador; i++) {
            printf("%-5d %-30s R$%-9.2f %-10d %-15s\n", produtos_array[i].codigo, produtos_array[i].nome, produtos_array[i].preco, produtos_array[i].quantidade, produtos_array[i].categoria);
        }
        free(produtos_array);
    } else {
        exibir_produtos_ordenados(raiz);
    }
    printf("----------------------------------------------------------------------\n");
    system("pause");
}

// --- Função que exibe relatório de vendas detalhadas ---
void relatorio_vendas(int por_nome) {
    system("cls");
    if (por_nome) {
        printf("--- Relatório Detalhado de Vendas (Ordenado por Nome do Produto) ---\n");
    } else {
        printf("--- Relatório Detalhado de Vendas (Ordenado por Código do Produto) ---\n");
    }

    
    printf("%-20s %-5s %-25s %-5s %-10s %-10s\n", "DATA/HORA", "ID", "NOME", "QTD", "PREÇO UN.", "SUBTOTAL");
    printf("====================================================================================\n");

    FILE *arq = fopen(ARQ_VDETALHADAS, "r"); 
    if (arq == NULL) { 
        printf("Nenhuma venda registrada ainda.\n"); 
        printf("====================================================================================\n");
        system("pause");
        return;
    }

    
    venda_detalhada *vendas = NULL;         
    int capacidade = 100;                   
    int n_vendas = 0;                       
    char linha[256];                        
    float total_geral_vendas = 0.0;         

    while (fgets(linha, sizeof(linha), arq)) { 

        
        sscanf(linha, "%19[^;];%d;%[^;];%d;%f", vendas[n_vendas].data_hora, &vendas[n_vendas].codigo, vendas[n_vendas].nome, &vendas[n_vendas].quantidade, &vendas[n_vendas].preco_unitario); 
        n_vendas++; 
    }
    fclose(arq); 

    if (n_vendas > 0) { 
        if (por_nome) {
            
            qsort(                         
                vendas,                    
                n_vendas,                  
                sizeof(venda_detalhada),   
                comparar_vendas_por_nome); 

        } else {
            qsort(vendas, n_vendas, sizeof(venda_detalhada), comparar_vendas_por_codigo); 
        }

        for (int i = 0; i < n_vendas; i++) { 

            float subtotal = vendas[i].quantidade * vendas[i].preco_unitario; 
            
            printf("%-20s %-5d %-25s %-5d R$%-9.2f R$%-9.2f\n", vendas[i].data_hora, vendas[i].codigo, vendas[i].nome, vendas[i].quantidade, vendas[i].preco_unitario, subtotal);
            total_geral_vendas += subtotal; 
        }
        free(vendas); 
    } else { 
        printf("Nenhuma venda registrada ainda.\n");
    }

    printf("====================================================================================\n");
    printf("TOTAL GERAL DAS VENDAS: R$ %.2f\n", total_geral_vendas); 
    printf("====================================================================================\n");
    system("pause");
}

// --- Função que exibe o menu de relatórios ---
void menu_relatorios(no *raiz) {
    int opc;
    do {
        system("cls");
        printf("--- MENU DE RELATÓRIOS ---\n");
        printf("=========================================\n");
        printf("Listagem de Produtos:\n");
        printf("  1 - Produtos (ordenado por nome)\n");
        printf("  2 - Produtos (ordenado por código)\n");
        printf("-----------------------------------------\n");
        printf("Listagem de Vendas:\n");
        printf("  3 - Vendas Detalhadas (ordenado por nome do produto)\n");
        printf("  4 - Vendas Detalhadas (ordenado por código do produto)\n");
        printf("=========================================\n");
        printf("  0 - Voltar ao menu principal\n");
        printf("=========================================\n");
        printf("Selecione uma opção: ");

        if (scanf("%d", &opc) != 1) {
            limpar_buffer();
            return;
        }
        limpar_buffer();

        switch (opc) {
        case 0: 
            break; 
        case 1:
            relatorio_produtos_por_ordem(raiz, 1); 
            break;
        case 2:
            relatorio_produtos_por_ordem(raiz, 0);  
            break;
        case 3:
            relatorio_vendas(1);  
            break;
        case 4:
            relatorio_vendas(0);  
            break;
        default:
            printf("Opção inválida.\n");
            Sleep(1000);
            break;
        }
    } while (opc != 0);
}

// --- Função principal do sistema ---
int main() {
    
    SetConsoleOutputCP(65001); 
    no *raiz = NULL;
    int opc;

    registrar_log("-----------------------------------------");
    registrar_log("Programa iniciado.");
    
    raiz = carregar_produtos(raiz); 

    
    do {
        system("cls");
        printf("Mercadinho\n");
        printf("===========================================================\n");
        printf("COMPRAR:                                   Total no carrinho: R$ %.2f\n", totalpendente);
        printf("  1 - Limpeza\n  2 - Alimentos\n  3 - Padaria\n");
        printf("===========================================================\n");
        printf("CAIXA:                                     Status: %s\n", caixa_aberto ? "ABERTO" : "FECHADO");
        printf("  4 - Finalizar compra\n  5 - Abrir/Fechar caixa\n");
        printf("===========================================================\n");
        printf("ADM:\n  6 - Adicionar produto\n  7 - Atualizar estoque da Padaria\n  8 - Relatórios\n  9 - Encerrar programa\n");
        printf("===========================================================\n");
        printf("Selecione uma opção: ");
       
        if (scanf("%d", &opc) != 1) { 
            limpar_buffer();
            printf("Opção inválida.\n");
            Sleep(1000);
            continue; 
        }

        switch (opc) {
        case 1:
            exibir_produtos(raiz, "LIMPEZA");
            break;
        case 2:
            exibir_produtos(raiz, "ALIMENTOS");
            break;
        case 3:
            exibir_produtos(raiz, "PADARIA");
            break;
        case 4:
            finalizar_compra(raiz);
            break;
        case 5:
            abrir_fechar_caixa(raiz);
            break;
        case 6:
            adicionar_produto(&raiz);
            break;
        case 7:
            atualizar_estoque_padaria(raiz);
            break;
        case 8:
            menu_relatorios(raiz);
            break;
        case 9:
            printf("Encerrando programa.\n");
            break;
        default:
            limpar_buffer();
            printf("Opção inválida.\n");
            Sleep(1000);
            break;
        }
    } while (opc != 9);
    
    salvar_produtos(ARQPRODUTOS, raiz);
    
    liberar_arvore(raiz);
    limpar_carrinho();
    raiz = NULL;

    registrar_log("Programa encerrado.");
    registrar_log("-----------------------------------------\n");

    return 0;
}