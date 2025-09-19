#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

// --- Constantes Globais -----------------------------------------------------
#define MAX_STR        48
#define NUM_TERRITORIOS 5      // mapa compacto p/ demo (América, Europa, Ásia, África, Oceania)
#define NUM_MISSOES     2

// Cores possíveis (dono do território)
typedef enum { COR_AZUL=0, COR_VERMELHO=1, COR_VERDE=2, COR_AMARELO=3, COR_BRANCO=4 } Cor;

// --- Estrutura de Dados -----------------------------------------------------
typedef struct {
    char nome[MAX_STR];
    Cor  dono;      // cor do exército que domina o território
    int  tropas;    // número de tropas no território
} Territorio;

// --- Protótipos das Funções -------------------------------------------------
// Setup/memória
Territorio* alocarMapa(size_t n);
void inicializarTerritorios(Territorio* mapa, size_t n);
void liberarMemoria(Territorio* mapa);

// Interface do usuário
void exibirMenuPrincipal(void);
void exibirMapa(const Territorio* mapa, size_t n);
void exibirMissao(int missaoId);

// Lógica principal
void faseDeAtaque(Territorio* mapa, size_t n, Cor corJogador);
int  simularAtaque(Territorio* mapa, size_t n, int idOrigem, int idDestino, Cor corJogador);

// Missões
int  sortearMissao(void);
int  verificarVitoria(const Territorio* mapa, size_t n, Cor corJogador, int missaoId);

// Utilitárias
void limparBufferEntrada(void);
int  lerInt(const char* prompt, int min, int max);
const char* nomeCor(Cor c);

// --- Função Principal (main) -----------------------------------------------
int main(void) {
    // 1. Configuração Inicial (Setup)
    setlocale(LC_ALL, ""); // PT-BR quando disponível
    srand((unsigned)time(NULL));

    Territorio* mapa = alocarMapa(NUM_TERRITORIOS);
    if (!mapa) { fprintf(stderr, "Falha ao alocar mapa.\n"); return 1; }
    inicializarTerritorios(mapa, NUM_TERRITORIOS);

    Cor corJogador = COR_AZUL;         // jogador é AZUL nesta demo
    int missaoId   = sortearMissao();  // missão secreta

    int venceu = 0, opcao;
    // 2. Laço Principal do Jogo (Game Loop)
    do {
        puts("\n====================== ESTADO ATUAL ======================");
        exibirMapa(mapa, NUM_TERRITORIOS);
        exibirMissao(missaoId);

        exibirMenuPrincipal();
        opcao = lerInt("> ", 0, 2);

        switch (opcao) {
            case 1: // Fase de Ataque
                faseDeAtaque(mapa, NUM_TERRITORIOS, corJogador);
                break;
            case 2: // Verificar Vitória
                if (verificarVitoria(mapa, NUM_TERRITORIOS, corJogador, missaoId)) {
                    puts("\n>>> MISSÃO CUMPRIDA! Você venceu. <<<");
                    venceu = 1;
                } else {
                    puts("\nAinda não cumpriu a missão. Continue tentando!");
                }
                break;
            case 0:
                puts("Encerrando o jogo...");
                break;
        }

        if (!venceu && opcao != 0) {
            puts("\n(Pressione ENTER para continuar)");
            limparBufferEntrada();
        }
    } while (!venceu && opcao != 0);

    // 3. Limpeza
    liberarMemoria(mapa);
    return 0;
}

// --- Implementação das Funções ---------------------------------------------

// alocarMapa(): calloc para zero-inicializar
Territorio* alocarMapa(size_t n) {
    return (Territorio*)calloc(n, sizeof(Territorio));
}

// inicializarTerritorios(): preenche dados iniciais
void inicializarTerritorios(Territorio* mapa, size_t n) {
    // Mapa do "nível novato" citado: 5 territórios
    static const char* NOMES[NUM_TERRITORIOS] = {
        "América", "Europa", "Ásia", "África", "Oceania"
    };

    for (size_t i = 0; i < n; i++) {
        strncpy(mapa[i].nome, NOMES[i], MAX_STR-1);
        mapa[i].nome[MAX_STR-1] = '\0';

        // Distribui donos e tropas iniciais (exemplo reproduzível e simples)
        // Jogador (AZUL) começa com 2 territórios; o resto com VERMELHO.
        if (i < 2) { // América, Europa
            mapa[i].dono   = COR_AZUL;
            mapa[i].tropas = 3;
        } else if (i == 2) { // Ásia
            mapa[i].dono   = COR_VERMELHO;
            mapa[i].tropas = 5;
        } else if (i == 3) { // África
            mapa[i].dono   = COR_AMARELO;
            mapa[i].tropas = 4;
        } else {             // Oceania
            mapa[i].dono   = COR_BRANCO;
            mapa[i].tropas = 2;
        }
    }
}

void liberarMemoria(Territorio* mapa) {
    free(mapa);
}

// Interface: menu, mapa, missão
void exibirMenuPrincipal(void) {
    puts("\n======================== MENU ============================");
    puts("1) Atacar");
    puts("2) Verificar missoes");
    puts("0) Sair");
}

void exibirMapa(const Territorio* mapa, size_t n) {
    puts("ID | Território         | Dono       | Tropas");
    puts("---+---------------------+------------+-------");
    for (size_t i = 0; i < n; i++) {
        printf("%2zu | %-19s | %-10s | %5d\n",
               i, mapa[i].nome, nomeCor(mapa[i].dono), mapa[i].tropas);
    }
}

void exibirMissao(int missaoId) {
    puts("\n*** Sua missão secreta ***");
    switch (missaoId) {
        case 1:
            puts("- Destruir o exército VERMELHO (todos os territórios dele).");
            break;
        case 2:
            puts("- Controlar pelo menos 3 territorios no total.");
            break;
        default:
            puts("- [Missão desconhecida]");
            break;
    }
}

// Ataque (UI da fase + simulação)
void faseDeAtaque(Territorio* mapa, size_t n, Cor corJogador) {
    puts("\n=== Fase de Ataque ===");
    int origem  = lerInt("ID do território de ORIGEM: ", 0, (int)n-1);
    int destino = lerInt("ID do território de DESTINO: ", 0, (int)n-1);

    int conquistou = simularAtaque(mapa, n, origem, destino, corJogador);
    if (conquistou) puts(">> Território CONQUISTADO!");
}

// Lógica do duelo (dados 3x2, estilo War simplificado)
int simularAtaque(Territorio* mapa, size_t n, int idOrigem, int idDestino, Cor corJogador) {
    if (idOrigem == idDestino) { puts("Origem e destino iguais."); return 0; }
    if (idOrigem < 0 || idOrigem >= (int)n || idDestino < 0 || idDestino >= (int)n) {
        puts("IDs invalidos."); return 0;
    }

    Territorio *A = &mapa[idOrigem], *D = &mapa[idDestino];

    if (A->dono != corJogador) { puts("Você só pode atacar a partir de um território seu."); return 0; }
    if (D->dono == corJogador) { puts("Destino também é seu. Ataque inválido."); return 0; }
    if (A->tropas < 2)         { puts("É preciso ao menos 2 tropas para atacar."); return 0; }

    // Dados do atacante (até 3, limitado por tropas-1) e do defensor (até 2, limitado por tropas)
    int dadosAtk = (A->tropas - 1 >= 3) ? 3 : (A->tropas - 1);
    int dadosDef = (D->tropas >= 2) ? 2 : D->tropas;

    int ra[3]={0}, rd[2]={0};
    for (int i=0;i<dadosAtk;i++) ra[i] = (rand()%6)+1;
    for (int i=0;i<dadosDef;i++) rd[i] = (rand()%6)+1;

    // Ordena desc para comparar na ordem (maiores contra maiores)
    for (int i=0;i<dadosAtk-1;i++)
        for (int j=i+1;j<dadosAtk;j++)
            if (ra[j] > ra[i]) { int t=ra[i]; ra[i]=ra[j]; ra[j]=t; }
    for (int i=0;i<dadosDef-1;i++)
        for (int j=i+1;j<dadosDef;j++)
            if (rd[j] > rd[i]) { int t=rd[i]; rd[i]=rd[j]; rd[j]=t; }

    printf("Rolagens  Atk:"); for (int i=0;i<dadosAtk;i++) printf(" %d", ra[i]);
    printf("  Def:");       for (int i=0;i<dadosDef;i++) printf(" %d", rd[i]); puts("");

    int comps = (dadosAtk < dadosDef ? dadosAtk : dadosDef);
    for (int i=0;i<comps;i++) {
        if (ra[i] > rd[i]) D->tropas--;
        else               A->tropas--;
    }

    printf("Após batalha -> Origem(%s): %d | Destino(%s): %d\n",
           A->nome, A->tropas, D->nome, D->tropas);

    if (D->tropas <= 0) {
        // Conquistou: transfere 1 tropa (mínimo) para ocupar
        int mover = (A->tropas > 1) ? 1 : 0;
        if (mover == 0) { // garante pelo menos 1
            puts("Sem tropas suficientes para ocupar. Ajustando para mover 1.");
            mover = 1;
        }
        A->tropas -= mover;
        D->tropas  = mover;
        D->dono    = corJogador;
        return 1;
    }
    return 0;
}

// Missões
int sortearMissao(void) {
    // 1: destruir VERMELHO; 2: controlar >= 3 territórios
    return (rand() % NUM_MISSOES) + 1;
}

int verificarVitoria(const Territorio* mapa, size_t n, Cor corJogador, int missaoId) {
    switch (missaoId) {
        case 1: { // destruir VERMELHO
            int existeVermelho = 0;
            for (size_t i=0;i<n;i++) if (mapa[i].dono == COR_VERMELHO) { existeVermelho = 1; break; }
            return existeVermelho ? 0 : 1;
        }
        case 2: { // controlar >= 3
            int meus = 0;
            for (size_t i=0;i<n;i++) if (mapa[i].dono == corJogador) meus++;
            return (meus >= 3) ? 1 : 0;
        }
        default:
            return 0;
    }
}

// Utilitárias
void limparBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
}

int lerInt(const char* prompt, int min, int max) {
    int v;
    for (;;) {
        if (prompt) printf("%s", prompt);
        if (scanf("%d", &v) == 1 && v >= min && v <= max) { limparBufferEntrada(); return v; }
        limparBufferEntrada();
        printf("Valor invalido. Digite um inteiro entre %d e %d.\n", min, max);
    }
}

const char* nomeCor(Cor c) {
    switch (c) {
        case COR_AZUL:     return "Azul";
        case COR_VERMELHO: return "Vermelho";
        case COR_VERDE:    return "Verde";
        case COR_AMARELO:  return "Amarelo";
        case COR_BRANCO:   return "Branco";
        default:           return "?";
    }
}
