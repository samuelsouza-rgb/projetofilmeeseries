#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

/* ------------------------------------------------------------------------------
   STRUCT PRINCIPAL — FILME / SÉRIE
------------------------------------------------------------------------------ */
typedef struct {
    char titulo[50];
    char genero[30];
    int ano;
    char status; 
    /*
      status = ' '  -> registro ativo
      status = '*'  -> registro excluído (exclusão lógica)
    */
} Filme;

/* PROTÓTIPOS */
void configurar_locale(void);
void limpaBuffer(void);
void ler_string(char *s, int tam);
int tamanho(FILE *arq);
void cadastrar(FILE *arq);
void consultar(FILE *arq);
void excluir(FILE *arq);
void gerar_arquivo_texto(FILE *arq);

/* ------------------------------------------------------------------------------
   limpaBuffer
------------------------------------------------------------------------------ */
void limpaBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ------------------------------------------------------------------------------
   Leitura segura de strings
------------------------------------------------------------------------------ */
void ler_string(char *s, int tam) {
    fgets(s, tam, stdin);
    s[strcspn(s, "\n")] = '\0';
}

/* ------------------------------------------------------------------------------
   tamanho — retorna total de registros
------------------------------------------------------------------------------ */
int tamanho(FILE *arq) {
    long pos = ftell(arq);
    fseek(arq, 0, SEEK_END);
    long fim = ftell(arq);
    fseek(arq, pos, SEEK_SET);
    return (int)(fim / sizeof(Filme));
}

/* ------------------------------------------------------------------------------
   CADASTRAR FILME
------------------------------------------------------------------------------ */
void cadastrar(FILE *arq) {
    Filme f;
    char confirma;

    f.status = ' ';

    printf("\n=== CADASTRAR FILME/SÉRIE ===\n");
    printf("Registro: %d\n", tamanho(arq) + 1);

    printf("Título: ");
    ler_string(f.titulo, sizeof(f.titulo));

    printf("Gênero: ");
    ler_string(f.genero, sizeof(f.genero));

    printf("Ano: ");
    scanf("%d", &f.ano);
    limpaBuffer();

    printf("Confirmar cadastro (s/n)? ");
    scanf("%c", &confirma);
    limpaBuffer();

    if (toupper(confirma) == 'S') {
        fseek(arq, 0, SEEK_END);
        fwrite(&f, sizeof(Filme), 1, arq);
        fflush(arq);
        printf("Filme/Série cadastrado com sucesso!\n");
    } else {
        printf("Cadastro cancelado.\n");
    }
}

/* ------------------------------------------------------------------------------
   CONSULTAR FILME POR ÍNDICE
------------------------------------------------------------------------------ */
void consultar(FILE *arq) {
    int nr;
    Filme f;

    printf("\nInforme o código do filme/série: ");
    scanf("%d", &nr);
    limpaBuffer();

    int total = tamanho(arq);

    if (nr <= 0 || nr > total) {
        printf("Código inválido! Total: %d\n", total);
        return;
    }

    long pos = (long)(nr - 1) * sizeof(Filme);

    if (fseek(arq, pos, SEEK_SET) != 0) {
        printf("Erro ao posicionar ponteiro.\n");
        return;
    }

    fread(&f, sizeof(Filme), 1, arq);

    printf("\n=== FILME/SÉRIE (%d) ===\n", nr);

    if (f.status == '*')
        printf("Status: EXCLUÍDO LOGICAMENTE\n");

    printf("Título: %s\n", f.titulo);
    printf("Gênero: %s\n", f.genero);
    printf("Ano...: %d\n", f.ano);
}

/* ------------------------------------------------------------------------------
   EXCLUSÃO LÓGICA
------------------------------------------------------------------------------ */
void excluir(FILE *arq) {
    int nr;
    char confirma;
    Filme f;

    printf("\nInforme o código para excluir: ");
    scanf("%d", &nr);
    limpaBuffer();

    int total = tamanho(arq);

    if (nr <= 0 || nr > total) {
        printf("Código inválido.\n");
        return;
    }

    long pos_byte = (long)(nr - 1) * sizeof(Filme);

    fseek(arq, pos_byte, SEEK_SET);
    fread(&f, sizeof(Filme), 1, arq);

    if (f.status == '*') {
        printf("Registro já está excluído.\n");
        return;
    }

    printf("Confirmar exclusão (s/n)? ");
    scanf("%c", &confirma);
    limpaBuffer();

    if (toupper(confirma) == 'S') {
        f.status = '*';
        fseek(arq, pos_byte, SEEK_SET);
        fwrite(&f, sizeof(Filme), 1, arq);
        fflush(arq);
        printf("Registro excluído com sucesso!\n");
    } else {
        printf("Exclusão cancelada.\n");
    }
}

/* ------------------------------------------------------------------------------
   GERAR ARQUIVO TEXTO
------------------------------------------------------------------------------ */
void gerar_arquivo_texto(FILE *arq) {
    char nomearq[80];
    int total, i;
    Filme f;
    char status_str[12];

    printf("\nNome do arquivo (sem extensão): ");
    ler_string(nomearq, sizeof(nomearq));
    strcat(nomearq, ".txt");

    FILE *arqtxt = fopen(nomearq, "w");

    if (!arqtxt) {
        printf("Erro ao criar arquivo texto.\n");
        return;
    }

    fprintf(arqtxt, "RELATÓRIO DE FILMES/SÉRIES\n\n");
    fprintf(arqtxt, "COD  %-30s %-20s %-5s STATUS\n",
            "TÍTULO", "GÊNERO", "ANO");
    fprintf(arqtxt, "--------------------------------------------------------------------------\n");

    total = tamanho(arq);

    for (i = 0; i < total; i++) {
        fseek(arq, i * sizeof(Filme), SEEK_SET);
        fread(&f, sizeof(Filme), 1, arq);

        if (f.status == '*')
            strcpy(status_str, "EXCLUIDO");
        else
            strcpy(status_str, "ATIVO");

        fprintf(arqtxt, "%03d %-30s %-20s %-5d %s\n",
                i + 1, f.titulo, f.genero, f.ano, status_str);
    }

    fclose(arqtxt);

    printf("Arquivo '%s' gerado com sucesso!\n", nomearq);
}

/* ------------------------------------------------------------------------------
   CONFIGURAR LOCALE
------------------------------------------------------------------------------ */
void configurar_locale(void) {
#if defined(_WIN32)
    system("chcp 65001 > nul");
#endif

    const char *locais[] = {
        "pt_BR.UTF-8",
        "pt_BR.utf8",
        "Portuguese_Brazil.1252",
        "Portuguese",
        ""
    };

    int i;
    for (i = 0; i < 5; i++) {
        const char *r = setlocale(LC_ALL, locais[i]);
        if (r != NULL) {
            return;
        }
    }
}

/* ------------------------------------------------------------------------------
   MAIN
------------------------------------------------------------------------------ */
int main(void) {
    configurar_locale();

    FILE *arq = fopen("filmes.dat", "r+b");

    if (!arq) {
        arq = fopen("filmes.dat", "w+b");
        if (!arq) {
            printf("Erro ao abrir/criar arquivo.\n");
            return 1;
        }
    }

    int op;

    do {
        printf("\n=========== COLEÇÃO DE FILMES/SÉRIES ===========\n");
        printf("1. Cadastrar\n");
        printf("2. Consultar\n");
        printf("3. Excluir\n");
        printf("4. Gerar arquivo texto\n");
        printf("5. Sair\n");
        printf("-----------------------------------------------\n");
        printf("Total de registros: %d\n", tamanho(arq));
        printf("Opção: ");

        if (scanf("%d", &op) != 1) {
            printf("Digite um número válido.\n");
            limpaBuffer();
            continue;
        }
        limpaBuffer();

        switch (op) {
            case 1: cadastrar(arq); break;
            case 2: consultar(arq); break;
            case 3: excluir(arq); break;
            case 4: gerar_arquivo_texto(arq); break;
            case 5: printf("Saindo...\n"); break;
            default: printf("Opção inválida!\n");
        }

    } while (op != 5);

    fclose(arq);
    return 0;
}

