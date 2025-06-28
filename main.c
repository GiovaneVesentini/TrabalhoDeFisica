#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <raylib.h> // Inclui a biblioteca Raylib

// --- PARÂMETROS DE SIMULAÇÃO DEFINIDOS POR MACROS ---
#define NUMERO_ESFERAS 10         // Número de esferas
#define LARGURA_JANELA 550    // Largura da janela da simulação
#define ALTURA_JANELA 550    // Altura da janela da simulação
#define VELOCIDADE_MEDIA 10.0    // Velocidade média inicial das esferas
#define DT_FISICA 0.005       // Intervalo de tempo para os cálculos físicos (em segundos)
#define RAIO_ESFERA 20.0     // Raio de cada esfera
#define MASSA_ESFERA 1.0        // Massa de cada esfera (para colisões realistas)
#define FPS_ALVO           // Taxa de quadros desejada para renderização

// --- Nome do arquivo de imagem de fundo ---
#define CAMINHO_IMAGEM_FUNDO "c:bom-dia-60559x8jFVj3gp.jpg" // Caminho para sua imagem de fundo

// --- Estrutura para representar uma esfera ---
typedef struct {
    double x, y;    // Posição (double para precisão)
    double vx, vy;  // Velocidade
    double raio;  // Raio
    double massa;    // Massa da esfera
    Color cor;    // Cor para visualização com Raylib
} Esfera; // Estrutura renomeada para Esfera

// --- Funções de Ajuda ---

// Gera um número aleatório double entre min e max
double obterDoubleAleatorio(double min, double max) {
    return min + (double)rand() / RAND_MAX * (max - min);
}

// Inicializa as esferas com posições e velocidades aleatórias
void inicializarEsferas(Esfera esferas[], int numeroEsferas, double larguraJanela, double alturaJanela, double velocidadeMedia, double raioEsfera, double massaEsfera) {
    srand(time(NULL)); // Inicializa a semente do gerador de números aleatórios

    for (int i = 0; i < numeroEsferas; i++) {
        esferas[i].raio = raioEsfera;
        esferas[i].massa = massaEsfera;

        // Cores aleatórias para as esferas
        esferas[i].cor = (Color){ GetRandomValue(50, 250), GetRandomValue(50, 250), GetRandomValue(50, 250), 255 };

        // Posição aleatória dentro dos limites da janela, evitando sobreposição inicial
        int colisaoDetectada;
        do {
            colisaoDetectada = 0;
            // Garante que a esfera nasça dentro da janela e não sobrepondo as bordas
            esferas[i].x = obterDoubleAleatorio(raioEsfera, larguraJanela - raioEsfera);
            esferas[i].y = obterDoubleAleatorio(raioEsfera, alturaJanela - raioEsfera);

            // Verifica sobreposição com outras esferas já inicializadas
            for (int j = 0; j < i; j++) {
                double dx = esferas[i].x - esferas[j].x;
                double dy = esferas[i].y - esferas[j].y;
                double distancia = sqrt(dx * dx + dy * dy);
                if (distancia < (esferas[i].raio + esferas[j].raio)) {
                    colisaoDetectada = 1;
                    break;
                }
            }
        } while (colisaoDetectada);

        // Velocidade aleatória com base na velocidade média
        double angulo = obterDoubleAleatorio(0, 2 * M_PI); // Ângulo aleatório em radianos
        esferas[i].vx = velocidadeMedia * cos(angulo);
        esferas[i].vy = velocidadeMedia * sin(angulo);
    }
}

// Atualiza as posições das esferas
void atualizarPosicoes(Esfera esferas[], int numeroEsferas, double dt) {
    for (int i = 0; i < numeroEsferas; i++) {
        esferas[i].x += esferas[i].vx * dt;
        esferas[i].y += esferas[i].vy * dt;
    }
}

// Trata colisões com as paredes
void tratarColisoesParede(Esfera *esfera, double larguraJanela, double alturaJanela) {
    // Colisão com parede esquerda/direita
    if (esfera->x - esfera->raio < 0) {
        esfera->x = esfera->raio; // Ajusta posição
        esfera->vx *= -1;          // Inverte a velocidade na direção X
    } else if (esfera->x + esfera->raio > larguraJanela) {
        esfera->x = larguraJanela - esfera->raio; // Ajusta posição
        esfera->vx *= -1;                         // Inverte a velocidade na direção X
    }

    // Colisão com parede superior/inferior
    if (esfera->y - esfera->raio < 0) {
        esfera->y = esfera->raio; // Ajusta posição
        esfera->vy *= -1;          // Inverte a velocidade na direção Y
    } else if (esfera->y + esfera->raio > alturaJanela) {
        esfera->y = alturaJanela - esfera->raio; // Ajusta posição
        esfera->y = alturaJanela - esfera->raio;
        esfera->vy *= -1;                          // Inverte a velocidade na direção Y
    }
}

// Trata colisões entre duas esferas (choque elástico, considerando massas diferentes)
void tratarColisaoEsfera(Esfera *e1, Esfera *e2) {
    double dx = e2->x - e1->x;
    double dy = e2->y - e1->y;
    double distancia = sqrt(dx * dx + dy * dy);

    // Verifica se houve colisão (sobreposição)
    if (distancia < (e1->raio + e2->raio) && distancia != 0) {
        // Vetor normalizado da colisão (do centro de e1 para o centro de e2)
        double nx = dx / distancia;
        double ny = dy / distancia;

        // Vetor tangente (perpendicular ao normal)
        double tx = -ny;
        double ty = nx;

        // Componentes da velocidade ao longo dos vetores normal e tangente para e1
        double v1n = e1->vx * nx + e1->vy * ny;
        double v1t = e1->vx * tx + e1->vy * ty;

        // Componentes da velocidade ao longo dos vetores normal e tangente para e2
        double v2n = e2->vx * nx + e2->vy * ny;
        double v2t = e2->vx * tx + e2->vy * ty;

        // Calcula as novas velocidades normais (conservação de momento e energia)
        double m1 = e1->massa;
        double m2 = e2->massa;

        double v1n_final = (v1n * (m1 - m2) + 2 * m2 * v2n) / (m1 + m2);
        double v2n_final = (v2n * (m2 - m1) + 2 * m1 * v1n) / (m1 + m2);

        // Atualiza as velocidades totais (normal + tangente) para e1
        e1->vx = v1n_final * nx + v1t * tx;
        e1->vy = v1n_final * ny + v1t * ty;

        // Atualiza as velocidades totais (normal + tangente) para e2
        e2->vx = v2n_final * nx + v2t * tx;
        e2->vy = v2n_final * ny + v2t * ty;

        // Ajusta as posições para evitar sobreposição persistente após a colisão
        double sobreposicao = (e1->raio + e2->raio) - distancia;
        e1->x -= 0.5 * sobreposicao * nx;
        e1->y -= 0.5 * sobreposicao * ny;
        e2->x += 0.5 * sobreposicao * nx;
        e2->y += 0.5 * sobreposicao * ny;
    }
}

// --- Função Principal ---
int main() {
    // --- Alocação de memória para as esferas ---
    Esfera *esferas = (Esfera *)malloc(NUMERO_ESFERAS * sizeof(Esfera));
    if (esferas == NULL) {
        perror("Erro ao alocar memória para as esferas");
        return 1;
    }

    inicializarEsferas(esferas, NUMERO_ESFERAS, LARGURA_JANELA, ALTURA_JANELA, VELOCIDADE_MEDIA, RAIO_ESFERA, MASSA_ESFERA);

    // --- Configuração da Janela Raylib ---
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, "Simulação de Colisões (Português) - Raylib");
    //SetTargetFPS(FPS_ALVO); // Define o FPS desejado para renderização

    // --- Carregar a imagem de fundo ---
    Texture2D texturaFundo = LoadTexture(CAMINHO_IMAGEM_FUNDO); // Carrega a textura da imagem
    //if (!IsTextureReady(texturaFundo)) { // Verifica se a textura foi carregada corretamente
       // TraceLog(LOG_WARNING, "IMAGEM: Não foi possível carregar a imagem de fundo em %s. Usando cor sólida.", CAMINHO_IMAGEM_FUNDO);
   // }


    // --- Loop Principal da Simulação/Renderização ---
    while (!WindowShouldClose()) { // Roda enquanto a janela não é fechada pelo usuário
        // Calcula o tempo do frame para a renderização
        double tempoFrame = GetFrameTime();

        // Determina quantos passos de física devem ocorrer por cada frame de renderização
        int passosFisicaPorFrame = (int)(tempoFrame / DT_FISICA);
        if (passosFisicaPorFrame == 0) passosFisicaPorFrame = 1; // Garante pelo menos 1 passo

        // --- Atualização da Lógica da Simulação (múltiplos passos de física por frame) ---
        for (int passo = 0; passo < passosFisicaPorFrame; passo++) {
            atualizarPosicoes(esferas, NUMERO_ESFERAS, DT_FISICA);

            // Trata colisões com as paredes
            for (int i = 0; i < NUMERO_ESFERAS; i++) {
                tratarColisoesParede(&esferas[i], LARGURA_JANELA, ALTURA_JANELA);
            }

            // Trata colisões entre as esferas (otimizado: cada par uma vez)
            for (int i = 0; i < NUMERO_ESFERAS; i++) {
                for (int j = i + 1; j < NUMERO_ESFERAS; j++) {
                    double dx = esferas[i].x - esferas[j].x;
                    double dy = esferas[i].y - esferas[j].y;
                    double distancia = sqrt(dx * dx + dy * dy);

                    // Se a distância for menor que a soma dos raios, houve colisão
                    if (distancia < (esferas[i].raio + esferas[j].raio)) {
                        tratarColisaoEsfera(&esferas[i], &esferas[j]);
                    }
                }
            }
        }

        // --- Desenho (Renderização) ---
        BeginDrawing();

        // Desenha a imagem de fundo
       // if (IsTextureReady(texturaFundo)) {
         DrawTexture(texturaFundo, 0, 0, WHITE); // Desenha a textura na posição (0,0) com filtro de cor WHITE (sem alteração)
       // } else {
           // ClearBackground(BLACK); // Se a imagem não carregar, limpa com preto
       // }

        // Desenha as esferas
        for (int i = 0; i < NUMERO_ESFERAS; i++) {
            DrawCircle((int)esferas[i].x, (int)esferas[i].y, (int)esferas[i].raio, esferas[i].cor);
            DrawCircleLines((int)esferas[i].x, (int)esferas[i].y, (int)esferas[i].raio, DARKGRAY);
        }

        // Desenha a borda da "mesa"
        DrawRectangleLines(0, 0, LARGURA_JANELA, ALTURA_JANELA, WHITE);
        DrawFPS(10,10);
        EndDrawing();
    }

    // --- Finalização da Raylib e Liberação de Recursos ---
    UnloadTexture(texturaFundo); // Descarrega a textura da memória
    CloseWindow(); // Fecha a janela e libera recursos da Raylib
    free(esferas); // Libera a memória alocada para as esferas

    return 0;
}