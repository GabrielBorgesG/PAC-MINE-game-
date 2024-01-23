/*
JOGO PAC-MINE

MAPA 1: E=4; O=5; T=5; A=5
MAPA 2: E=8; O=10; T=10; A=3
MAPA 3: E=12; O=15; T=15; A=2
*/

//BIBLIOTECAS
#include "raylib.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

//DEFINIÇÕES
#define FPS 8
#define LARGURA 880
#define ALTURA (520+(FONTE*6)) //Este aumento da tela é para o rodapé
#define VEL 20
#define ARESTA_BLOCO 20
#define TOUPEIRA_MAX 15
#define LIMITE_PASSOS_TOUPEIRAS 5
#define NUM_BLOC_LARGURA_MAPA 44
#define NUM_BLOC_ALTURA_MAPA 26
#define FONTE 20 //Fonte da letra
#define VOLUME 0.2 //Vai de 0.0 a 1.0
#define VISIBILIDADE 4 //Quanto o jogador consegue enxergar
#define TEMPO_VISAO_GLOBAL 25 //Tempo que o poder de enxergar tudo dura
#define RAIO_TIRO 3 //Tamanho do tiro
#define ESMERALDAS_NIVEL_1 4
#define ESMERALDAS_NIVEL_2 8
#define ESMERALDAS_NIVEL_3 12
#define PONTOS_INIMIGO 200
#define PONTOS_ESMERALDA 100
#define PONTOS_OURO 50

typedef struct POSICAO{
    int posX, posY;
}POSICAO;

typedef struct TOUPEIRA{
    int TposX, TposY, TdesX, TdesY, Tpassos;
    bool vida;
}TOUPEIRA;

typedef struct PONTOS{
    int vida, contador_esmeralda, contador_ouro, inimigos_eliminados;
}PONTOS;

typedef struct ESTADO{
    POSICAO jogador;
    int nivel;
    PONTOS pontos;
}ESTADO;

void SetupWindow(){
    InitWindow(LARGURA, ALTURA, "PAC-MINE");
    SetTargetFPS(FPS);
    HideCursor();
    InitAudioDevice();
    //ToggleFullscreen();
}

//VERIFICA SE O JOGADOR PODE SE MOVER (BLOCOS # e S)
int podeMoverJ (int posX, int posY, int desX, int desY, char mapa[][NUM_BLOC_LARGURA_MAPA]){
    int resultado = 0;
    int m1, m2;
    char bloco;

    if (((posX + desX) < LARGURA) && ((posX + desX) >= 0) && (posY + desY < ALTURA) && (posY + desY >= 0)) resultado = 1;

    for(m1=0; m1 < NUM_BLOC_LARGURA_MAPA; m1++){
            for(m2=0; m2 < NUM_BLOC_ALTURA_MAPA; m2++){
                bloco = mapa[m2][m1];
                switch (bloco){
                    case'#':
                        if(colisao(posX+desX, posY+desY, m1*ARESTA_BLOCO, m2*ARESTA_BLOCO)) resultado = 0;
                        break;
                    case'S':
                        if(colisao(posX+desX, posY+desY, m1*ARESTA_BLOCO, m2*ARESTA_BLOCO)) resultado = 0;
                        break;
                    default: break;
                }
            }
         }
    return resultado;
}

//VERIFICA SE AS TOUPEIRAS PODEM SE MOVER (BLOCOS #)
int podeMoverT (int posX, int posY, int desX, int desY, char mapa[][NUM_BLOC_LARGURA_MAPA]){
    int resultado;
    int m1, m2;
    char bloco;

    if (((posX + desX) < LARGURA) && ((posX + desX) >= 0) && (posY + desY < ALTURA) && (posY + desY >= 0)) resultado = 1;
    for(m1=0; m1 < NUM_BLOC_LARGURA_MAPA; m1++){
            for(m2=0; m2 < NUM_BLOC_ALTURA_MAPA; m2++){
                bloco = mapa[m2][m1];
                switch (bloco){
                    case'#':
                        if(colisao(posX+desX, posY+desY, m1*ARESTA_BLOCO, m2*ARESTA_BLOCO)) resultado = 0;
                        break;
                    default: break;
                }
            }
         }
    return resultado;
}

//ATUALIZA A POSIÇÃO DO JOGADOR
void move(int desX, int desY, int *posX, int *posY){
    *posX = *posX + desX;
    *posY = *posY + desY;
}

//ATUALIZA A POSIÇÃO DAS TOUPEIRAS
void Tmove(TOUPEIRA *Tponteiro){
    Tponteiro->TposX = Tponteiro->TposX + Tponteiro->TdesX;
    Tponteiro->TposY = Tponteiro->TposY + Tponteiro->TdesY;
}

//COPIA O MAPA DO ARQUIVO PARA UMA MATRIZ DENTRO DO PROGRAMA
int upload_mapa(char nome_arq_mapa[], char matriz[][NUM_BLOC_LARGURA_MAPA], POSICAO *posicao_do_jogador, TOUPEIRA *toupeiras){
    int ctrl = 0, i, j, Tcont = 0;
    FILE *mapa;

    if((mapa = fopen(nome_arq_mapa, "r")) != NULL){
        while(!feof(mapa)){
            for(i=0; i<NUM_BLOC_ALTURA_MAPA; i++){
                for(j=0; j<NUM_BLOC_LARGURA_MAPA; j++){
                    matriz[i][j] = getc(mapa);
                    if(matriz[i][j] == 'J'){
                        posicao_do_jogador->posX = j;
                        posicao_do_jogador->posY = i;
                    }
                    if(matriz[i][j] == 'T'){
                        toupeiras[Tcont].TposX = j;
                        toupeiras[Tcont].TposY = i;
                        Tcont++;
                    }
                }
                getc(mapa);
            }
        }
        ctrl = 1;
    }
    fclose(mapa);
    return ctrl;
}

//SALVA A POSIÇÃO DO JOGADOR
int save(char nome_arq_mapa[], ESTADO jogador_estado){
    FILE *mapa;
    int ctrl=0;

    if((mapa = fopen("Save_PAC-MINE", "w")) != NULL){
        fprintf(mapa, "%d, %d, %d, %d, %d, %d\n", jogador_estado.jogador.posX, jogador_estado.jogador.posY, jogador_estado.nivel, jogador_estado.pontos.contador_esmeralda, jogador_estado.pontos.contador_ouro, jogador_estado.pontos.vida);
        ctrl = 1;
    }
    return ctrl;
}

//VERIFICA COLISÃO ENTRE BLOCOS
int colisao(int JposX, int JposY, int posX_bloco, int posY_bloco){
    int ctrl = 0;

        if(JposX == posX_bloco && JposY == posY_bloco) ctrl =1;
    return ctrl;
}

//SOM
void AmbientSound(Sound Ambiente){
    if(!IsSoundPlaying(Ambiente))
    {
        PlaySound(Ambiente);
    }
}

//MENU
void abreMenu(bool *pause, ESTADO jogador_estado, bool *sair_jogo, char matriz[][NUM_BLOC_LARGURA_MAPA], POSICAO *posicao_do_jogador, TOUPEIRA *toupeiras){
    char texto_menu[5][20] = {"(N) Novo jogo", "(C) Carregar jogo", "(S) Salvar jogo", "(Q) Sair do jogo", "(V) Voltar ao jogo"};
    char oper;
    int ctrl, i;

    for (int i=0; i<4;i++){
            DrawText(texto_menu[i], (LARGURA - MeasureText(texto_menu[i], FONTE))/2, (ALTURA-FONTE)/2, FONTE, WHITE);
    }

    do{
        ctrl = 1;
        getchar();
        scanf(" %c", &oper);
        switch(oper){
            case 'V':
                *pause = false;
                break;
            case 'S':
                save("Save_PAC-MINE", jogador_estado);
                break;
            case 'N':
                if((upload_mapa("Mapa_1.txt", matriz, &posicao_do_jogador, &toupeiras[0])!= 1)){
                    printf("Erro na leitura do mapa 1\n");
                }
                *pause = !(*pause);
                break;
            case 'C':
                if((upload_mapa("Save_PAC-MINE", matriz, &posicao_do_jogador, &toupeiras[0])!= 1)){
                    printf("Erro na leitura do mapa 1\n");
                }
                break;
            case 'Q':
                sair_jogo = true;
                break;
            default:
                printf("Opcao invalida!");
                ctrl = 0;
        }
    }while(ctrl == 0);
}

void remove_do_mapa(char mapa[][NUM_BLOC_LARGURA_MAPA], int posX, int posY){
    mapa[posY][posX] = ' ';
}

//PROGRAMA PRINCIPAL
int main(void){

//DEFINIÇÕES
    int posX, posY, i = 0, t = 0, ultimo_move, ctrl_nivel = 0, pontos_totais = 0;
    int timer = 0, fim_do_jogo = 0;
    char mapa[NUM_BLOC_ALTURA_MAPA][NUM_BLOC_LARGURA_MAPA] = {0}, bloco, m1 = 0, m2 = 0;
    char status[6][25] = {"Vida: ", "Esmeraldas: ", "Ouro: ", "Inimigos eliminados: ", "Pontos totais: ", "Esmeraldas no nível: "};
    bool Byakugan = false, pause = false, sair_jogo = 0;

    FILE *ponteiro_mapa1, *ponteiro_mapa2, *ponteiro_mapa3;
    TOUPEIRA toupeiras[TOUPEIRA_MAX];
    PONTOS jogador = {jogador.vida = 3, jogador.contador_esmeralda = 0, jogador.contador_ouro = 0, jogador.inimigos_eliminados = 0};
    PONTOS *Jponteiro = &jogador;
    ESTADO jogador_estado;
    jogador_estado.nivel = 1;
    POSICAO posicao_do_jogador;
    Rectangle parede;
    Rectangle parede2;
    Rectangle toupeira_rec;

//INICIALIZAÇÕES
    SetupWindow();

//CARREGAMENTO DA MÚSICA DE FUNDO
    Music Ambiente;
    Ambiente = LoadMusicStream("./Som/fundo.mp3");
    PlayMusicStream(Ambiente);
    SetMusicVolume(Ambiente, VOLUME);

//TEXTURAS
    Texture2D Orochimaru = LoadTexture("./Texturas/Orochi.png");
    Texture2D OrochimaruPower = LoadTexture("./Texturas/OrochimaruPower.png");
    Texture2D Sharingan = LoadTexture("./Texturas/Sharingan.png");
    Texture2D ByakuganT = LoadTexture("./Texturas/Byakugan.png");
    Texture2D Marca_da_maldicao = LoadTexture("./Texturas/Marca_da_maldicao.png");
    Texture2D Bloco_fundo = LoadTexture("./Texturas/Bloco_fundo.png");
    Texture2D Bloco_pedra = LoadTexture("./Texturas/Bloco_pedra.png");
    Texture2D Inimigo = LoadTexture("./Texturas/Naruto.png");

//LEITURA DO MAPA
    if((upload_mapa("Mapa_1.txt", mapa, &posicao_do_jogador, &toupeiras[0])!= 1)){
       printf("Erro na leitura do mapa 1\n");
    }

//INICIALIZAÇÃO DA POSIÇÃO DO JOGADOR
    posX = posicao_do_jogador.posX*ARESTA_BLOCO;
    posY = posicao_do_jogador.posY*ARESTA_BLOCO;

//INICIALIZAÇÃO DA POSIÇÃO DAS TOUPEIRAS CONFORME O MAPA E DESLOXAMENTO ALEATÓRIO
    for (t=0; t < TOUPEIRA_MAX; t++){ //Faz a passagem por todas as posições do vetor
                toupeiras[t].TposX = toupeiras[t].TposX * ARESTA_BLOCO;
                toupeiras[t].TposY = toupeiras[t].TposY * ARESTA_BLOCO;
                toupeiras[t].vida = 1;
    }
    do{
        toupeiras[t].TdesX = GetRandomValue(-1, 1) * ARESTA_BLOCO; //Coloca um deslocamento aleatório para cada inimigo
        toupeiras[t].TdesY = GetRandomValue(-1, 1) * ARESTA_BLOCO;
    }while(toupeiras[t].TdesX == 0 && toupeiras[t].TdesY == 0); //Até que pelo menos um dos deslocamentos (X ou Y) seja diferente de zero (inimigo parado)

//INICIALIZAÇÃO DO PROJÉTIL (TIRO)
    Vector2 Tiro_pos = {0,0}; //Coloca o projétil na posição (0, 0)
    bool Tiro_ativo = false; //Coloca o projétil como inativo

//LAÇO PRINCIPAL DO JOGO
    while (!WindowShouldClose() && !sair_jogo/* && jogador.vida > 0*/){ //O jogo continua até que: o usuário feche a janela, o jogador selecione a opção Q no menu ou morra

//PASSAGEM DE NÍVEL
        if((jogador.contador_esmeralda == ESMERALDAS_NIVEL_1) && ctrl_nivel == 0){ //Tem 4 esmeraldas no primeiro nível e assim que ele atinge essa pontuação, passa de fase
            jogador_estado.nivel = 2;
            ctrl_nivel = 1;
        }

        if((jogador_estado.nivel == 2) && (ctrl_nivel = 1)){ //Se o jogador atingiu a fase 2, carrega o segundo mapa
            ctrl_nivel = 2;
            if((upload_mapa("Mapa_2.txt", mapa, &posicao_do_jogador, toupeiras)!= 1)) printf("Erro na leitura do mapa 2\n");
            posX = posicao_do_jogador.posX*ARESTA_BLOCO;
            posY = posicao_do_jogador.posY*ARESTA_BLOCO;
        }

        if((jogador.contador_esmeralda == ESMERALDAS_NIVEL_1+ESMERALDAS_NIVEL_2) && (ctrl_nivel == 2)){  //Tem 8 esmeraldas no nível 2
            jogador_estado.nivel = 3;
            ctrl_nivel = 3;
        }

        if((jogador_estado.nivel == 3) && (ctrl_nivel = 3)){ //Se o jogador atingiu a fase 3, carrega o terceiro mapa
            ctrl_nivel = 4;
            if((upload_mapa("Mapa_3.txt", mapa, &posicao_do_jogador, toupeiras)!= 1)) printf("Erro na leitura do mapa 3\n");
            posX = posicao_do_jogador.posX*ARESTA_BLOCO;
            posY = posicao_do_jogador.posY*ARESTA_BLOCO;
        }

//SE ATINGIR O NÚMERO MÁXIMO DE ESMERALDAS
        if(jogador.contador_esmeralda == ESMERALDAS_NIVEL_1+ESMERALDAS_NIVEL_2+ESMERALDAS_NIVEL_2){
            fim_do_jogo = 1;
        }

//MENU DO JOGO E PAUSE
        if(IsKeyPressed(KEY_TAB)){
            pause = true; //Fica em pause
            do{
                abreMenu(&pause, jogador_estado, &sair_jogo, mapa, &posicao_do_jogador, &toupeiras[0]);
            }while(pause == true); //Enquanto pause não for alterado pela função abreMenu, fica em loop
        }

//MÚSICA DE FUNDO
        UpdateMusicStream(Ambiente);

//PARA USAR A TEXTURA
        Rectangle Jogador_Rec; //Cria um retângulo para lidar com o jogador
        Jogador_Rec.x = posX;
        Jogador_Rec.y = posY;
        Jogador_Rec.width = ARESTA_BLOCO;
        Jogador_Rec.height = ARESTA_BLOCO;

        Vector2 Jogador_Vec; //Cria um vetor para lidar com o jogador
        Jogador_Vec.x = posX;
        Jogador_Vec.y = posY;

        Vector2 Inimigo_vec;

//TIRO
            if (Tiro_ativo){ //Se o tiro estiver ativo, ele se movimenta conforme a última tentativa de movimentação do jogador
                //Escolha da direção do tiro
                switch(ultimo_move){
                case 1:
                    Tiro_pos.x += GetFrameTime() * 200;
                    break;
                case 2:
                    Tiro_pos.x -= GetFrameTime() * 200;
                    break;
                case 3:
                    Tiro_pos.y -= GetFrameTime() * 200;
                    break;
                case 4:
                    Tiro_pos.y += GetFrameTime() * 200;
                    break;
                }
                //Fica ativo até sair da janela
                if(Tiro_pos.x > LARGURA || Tiro_pos.x < 0 || Tiro_pos.y > ALTURA || Tiro_pos.y < 0)
                    Tiro_ativo = false;

                //Fica ativo até sofrer colisão com algum bloco
                for(m1=0; m1 < NUM_BLOC_LARGURA_MAPA; m1++){
                    for(m2=0; m2 < NUM_BLOC_ALTURA_MAPA; m2++){
                        bloco = mapa[m2][m1];
                        switch (bloco){
                            case'#':
                                parede.x = m1*ARESTA_BLOCO;
                                parede.y = m2*ARESTA_BLOCO;
                                parede.width = ARESTA_BLOCO;
                                parede.height = ARESTA_BLOCO;
                                if(CheckCollisionCircleRec(Tiro_pos, RAIO_TIRO, parede))
                                    Tiro_ativo = false;
                                break;
                            case'S':
                                parede2.x = m1*ARESTA_BLOCO;
                                parede2.y = m2*ARESTA_BLOCO;
                                parede2.width = ARESTA_BLOCO;
                                parede2.height = ARESTA_BLOCO;
                                if(CheckCollisionCircleRec(Tiro_pos, RAIO_TIRO, parede2)){
                                    remove_do_mapa(mapa, m1, m2);
                                    Tiro_ativo = false;
                                }
                                break;
                            default: break;
                        }
                    }
                 }
            }
            //Caso o projétil esteja inativo e o jogador apertar G, atira
            else if(IsKeyDown(KEY_G)){
                Tiro_ativo = true;
                Tiro_pos = (Vector2){posX+(ARESTA_BLOCO/2),posY+(ARESTA_BLOCO/2)}; //Surge no meio do jogador
            }

//TRATA ENTRADA DO USUÁRIO E ATUALIZA A POSIÇÃO DO JOGADOR E SUA MIRA
            if (IsKeyDown(KEY_RIGHT)){ //Vê se o usuário apertou a tecla
                ultimo_move = 1; //Salva a tentativa de movimentação
                if (podeMoverJ(posX, posY, VEL, 0, mapa)) //Vê se é possível se mover nesta direção
                    move(VEL, 0, &posX, &posY); //Se for possível, se movimenta
            }
            if (IsKeyDown(KEY_LEFT)){
                ultimo_move = 2;
                if (podeMoverJ(posX, posY, -VEL, 0, mapa))
                    move(-VEL, 0, &posX, &posY);
            }
            if (IsKeyDown(KEY_UP)){
                ultimo_move = 3;
                if (podeMoverJ(posX, posY, 0, -VEL, mapa))
                    move(0, -VEL, &posX, &posY);
            }
            if (IsKeyDown(KEY_DOWN)){
                ultimo_move = 4;
                if (podeMoverJ(posX, posY, 0, VEL, mapa))
                    move(0, VEL, &posX, &posY);
            }

//MOVIMENTAÇÃO DAS TOUPEIRAS
            for(t=0; t < TOUPEIRA_MAX; t++){ //Passa por todo o vetor que armaena inimigos
                if(podeMoverT(toupeiras[t].TposX,toupeiras[t].TposY,toupeiras[t].TdesX, toupeiras[t].TdesY, mapa) && toupeiras[t].Tpassos <= LIMITE_PASSOS_TOUPEIRAS){ //Confere se a movimentação é possível e se o contador já atingiu o limite máximo
                    Tmove(&toupeiras[t]); //Realiza a movimentação
                    toupeiras[t].Tpassos++; //Aumenta o contador de passos
                }
                else{
                    toupeiras[t].Tpassos = 0; //Reinicia o número de passos
                    do{
                        //Dá um novo deslocamento para as toupeiras
                        toupeiras[t].TdesX = GetRandomValue(-1, 1) * ARESTA_BLOCO;
                        toupeiras[t].TdesY = GetRandomValue(-1, 1) * ARESTA_BLOCO;
                    }while(toupeiras[t].TdesX == 0 && toupeiras[t].TdesY == 0);
                }
            }

//SE O JOGADOR TOCAR AS TOUPEIRAS, ATUALIZA PONTOS DE VIDA -- SE O TIRO ATINGIR UMA TOUPEIRA, ELA MORRE
            for(t=0; t < TOUPEIRA_MAX; t++){
                toupeira_rec.x = toupeiras[t].TposX;
                toupeira_rec.y = toupeiras[t].TposY;
                toupeira_rec.width = ARESTA_BLOCO;
                toupeira_rec.height = ARESTA_BLOCO;
                if(CheckCollisionCircleRec(Tiro_pos, RAIO_TIRO, toupeira_rec)){
                    toupeiras[t].vida = false;
                    jogador.inimigos_eliminados += 1;
                }
                if(colisao(posX, posY, toupeiras[t].TposX, toupeiras[t].TposY)){
                    Jponteiro->vida = Jponteiro->vida - 1;
                    posX = posicao_do_jogador.posX*ARESTA_BLOCO;
                    posY = posicao_do_jogador.posY*ARESTA_BLOCO;
                }
            }

            if(jogador.vida < 1){
                fim_do_jogo = 2;
            }

//----------------------------------------------------------------------------------
//ATUALIZA A REPRESENTAÇÃO VISUAL DO JOGO
//----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

//DESENHA MAPA E VERIFICA SE PEGOU OURO OU ESMERALDA
        Vector2 bloco_textura; //Para usar a textura nos blocos

        for(m1=0; m1 < NUM_BLOC_LARGURA_MAPA; m1++){
            for(m2=0; m2 < NUM_BLOC_ALTURA_MAPA; m2++){
                bloco = mapa[m2][m1];
                switch (bloco){
                    case'#':
                        DrawRectangle(m1*ARESTA_BLOCO, m2*ARESTA_BLOCO, ARESTA_BLOCO, ARESTA_BLOCO, BLACK);
                        break;
                    case'S':
                        DrawRectangle(m1*ARESTA_BLOCO, m2*ARESTA_BLOCO, ARESTA_BLOCO, ARESTA_BLOCO, DARKBROWN);
                        bloco_textura = (Vector2){m1*ARESTA_BLOCO, m2*ARESTA_BLOCO};
                        DrawTextureEx(Bloco_pedra, bloco_textura, 0, 0.05, WHITE);
                        break;
                    case'O':
                        DrawRectangle(m1*ARESTA_BLOCO, m2*ARESTA_BLOCO, ARESTA_BLOCO, ARESTA_BLOCO, GOLD);
                        bloco_textura = (Vector2){m1*ARESTA_BLOCO, m2*ARESTA_BLOCO};
                        DrawTextureEx(Marca_da_maldicao, bloco_textura, 0, 0.04, WHITE);
                        if(posX == m1*ARESTA_BLOCO && posY == m2*ARESTA_BLOCO){
                            Jponteiro->contador_ouro = Jponteiro->contador_ouro + 1;
                            remove_do_mapa(mapa, m1, m2);
                        }
                        break;
                    case'E':
                        DrawRectangle(m1*ARESTA_BLOCO, m2*ARESTA_BLOCO, ARESTA_BLOCO, ARESTA_BLOCO, GREEN);
                        bloco_textura = (Vector2){m1*ARESTA_BLOCO, m2*ARESTA_BLOCO};
                        DrawTextureEx(Sharingan, bloco_textura, 0, 0.04, WHITE);
                        if(posX == m1*ARESTA_BLOCO && posY == m2*ARESTA_BLOCO){
                            Jponteiro->contador_esmeralda = Jponteiro->contador_esmeralda + 1;
                            remove_do_mapa(mapa, m1, m2);
                        }
                        break;
                    case'A':
                        DrawRectangle(m1*ARESTA_BLOCO, m2*ARESTA_BLOCO, ARESTA_BLOCO, ARESTA_BLOCO, PURPLE);
                        bloco_textura = (Vector2){m1*ARESTA_BLOCO, m2*ARESTA_BLOCO};
                        DrawTextureEx(ByakuganT, bloco_textura, 0, 0.017, WHITE);
                        if(posX == m1*ARESTA_BLOCO && posY == m2*ARESTA_BLOCO){
                            Byakugan = true;
                            remove_do_mapa(mapa, m1, m2);
                        }
                        break;
                    case' ':
                        DrawRectangle(m1*ARESTA_BLOCO, m2*ARESTA_BLOCO, ARESTA_BLOCO, ARESTA_BLOCO, BROWN);
                        bloco_textura = (Vector2){m1*ARESTA_BLOCO, m2*ARESTA_BLOCO};
                        DrawTextureEx(Bloco_fundo, bloco_textura, 0, 0.04, WHITE);
                        break;
                    case'T':
                        DrawRectangle(m1*ARESTA_BLOCO, m2*ARESTA_BLOCO, ARESTA_BLOCO, ARESTA_BLOCO, BROWN);
                        bloco_textura = (Vector2){m1*ARESTA_BLOCO, m2*ARESTA_BLOCO};
                        DrawTextureEx(Bloco_fundo, bloco_textura, 0, 0.04, WHITE);
                        break;
                    case'J':
                        DrawRectangle(m1*ARESTA_BLOCO, m2*ARESTA_BLOCO, ARESTA_BLOCO, ARESTA_BLOCO, BROWN);
                        bloco_textura = (Vector2){m1*ARESTA_BLOCO, m2*ARESTA_BLOCO};
                        DrawTextureEx(Bloco_fundo, bloco_textura, 0, 0.04, WHITE);
                        break;
                }
            }
         }

//CONTROLE DO PODER DA VISÃO GLOBAL
        if(Byakugan) //Se o poder visual estiver ativo, o timer começa a contar/aumentar
           timer++;

        if(timer > TEMPO_VISAO_GLOBAL){ //Quando o timer atinge o limite, o poder fica inativo e o timer zera
           Byakugan = false;
           timer = 0;
        }

//ESCREVE STATUS DO JOGADOR
        sprintf(status[0], "Vida: %d", jogador.vida);
        sprintf(status[1], "Sharingans: %d", jogador.contador_esmeralda); //Análogo à esmeralda
        sprintf(status[2], "Marcas da maldicao: %d", jogador.contador_ouro); //Análogo ao ouro
        sprintf(status[3], "Inimigos eliminados: %d", jogador.inimigos_eliminados); // Permite que a string seja atualizada e salva em Texto[0]
        sprintf(status[4], "Pontos totais: %d", (jogador.contador_esmeralda*PONTOS_ESMERALDA)+(jogador.contador_ouro*PONTOS_OURO)+(jogador.inimigos_eliminados*PONTOS_INIMIGO));
        if(jogador_estado.nivel == 1)  sprintf(status[5], "Numero total de esmeraldas no nivel: %d", ESMERALDAS_NIVEL_1);
        else if(jogador_estado.nivel == 2)  sprintf(status[5], "Numero total de esmeraldas no nivel: %d", ESMERALDAS_NIVEL_2);
        else if(jogador_estado.nivel == 3)  sprintf(status[5], "Numero total de esmeraldas no nivel: %d", ESMERALDAS_NIVEL_3);

        for (int i=0; i<6;i++){
           DrawText(status[i], (LARGURA - MeasureText(status[i], FONTE))/2, ((FONTE)*i)+510, FONTE, WHITE);
        }

//DESENHA JOGADOR
        if(!Byakugan) DrawTextureEx(Orochimaru, Jogador_Vec, 0, 0.01, WHITE); //Sem o poder visual
        else DrawTextureEx(OrochimaruPower, Jogador_Vec, 0, 0.045, WHITE); //Com o poder visual

//DESENHA TOUPEIRAS
        for(t=0; t < TOUPEIRA_MAX; t++){
            if(toupeiras[t].vida){
                Inimigo_vec.x = toupeiras[t].TposX;
                Inimigo_vec.y = toupeiras[t].TposY;
                DrawTextureEx(Inimigo, Inimigo_vec, 0, 0.035, WHITE);
                //DrawRectangle(toupeiras[t].TposX, toupeiras[t].TposY, ARESTA_BLOCO, ARESTA_BLOCO, RED);
            }
        }

//DESENHA O TIRO
        if (Tiro_ativo)
            DrawCircle(Tiro_pos.x, Tiro_pos.y, RAIO_TIRO, WHITE);

//DESENHA MAPA ESCURO
        if(!Byakugan){
            DrawRectangle(0, 0, LARGURA, posY-(VISIBILIDADE*ARESTA_BLOCO), BLACK); //PARTE SUPERIOR
            DrawRectangle(0, posY+(VISIBILIDADE*ARESTA_BLOCO), LARGURA, ALTURA-posY-VISIBILIDADE-210, BLACK); //PARTE INFERIOR
            DrawRectangle(0, 0, posX-(5*ARESTA_BLOCO), 510, BLACK); //DIREITA
            DrawRectangle(posX+(5*ARESTA_BLOCO), 0, (LARGURA -posX-(VISIBILIDADE*ARESTA_BLOCO)), 510, BLACK); //ESQUERDA
        }

//DEFINE O FIM DE JOGO
        //Vitória
        if(fim_do_jogo == 1){
            DrawRectangle(0, 0, LARGURA, ALTURA, BLACK);
            DrawText("VOCE VENCEU!!!", (LARGURA - MeasureText("VOCE VENCEU!!!", FONTE*2))/2, (ALTURA-(FONTE*2))/2, FONTE*2, WHITE);
            DrawText(status[4], (LARGURA - MeasureText(status[4], FONTE*2))/2, (ALTURA+200-(FONTE*2))/2, FONTE*2, WHITE);
        //Derrota
        }else if(fim_do_jogo == 2){
            DrawRectangle(0, 0, LARGURA, ALTURA, BLACK);
            DrawText("VOCE PERDEU...", (LARGURA - MeasureText("VOCE PERDEU...", FONTE*2))/2, (ALTURA-(FONTE*2))/2, FONTE*2, WHITE);
            DrawText(status[4], (LARGURA - MeasureText(status[4], FONTE*2))/2, (ALTURA+200-(FONTE*2))/2, FONTE*2, WHITE);
        }

        EndDrawing();
     }
     UnloadMusicStream(Ambiente);
     UnloadTexture(Orochimaru);
     UnloadTexture(OrochimaruPower);
     UnloadTexture(Inimigo);
     UnloadTexture(Sharingan);
     UnloadTexture(ByakuganT);
     UnloadTexture(Marca_da_maldicao);
     UnloadTexture(Bloco_fundo);
     UnloadTexture(Bloco_pedra);

     CloseAudioDevice();
     CloseWindow();
     return 0;
}
