/*============================================================================*/
/* DETECTA SENSOR BAR DO WII E ENCONTRA ANGULO ENTRE ELES                     */
/*----------------------------------------------------------------------------*/
/* Autores: David Segalle - Viviane Ruotolo                                   */
/*============================================================================*/
/** Recebe uma matriz de char, a qual representa uma imagem e busca nelas o
 * centro de 2 círculos, com o intuito de encontrar o ângulo entre eles e a
 * coordenada de cada centro. */
/*============================================================================*/

#include "projeto5.h"
#include <stdlib.h>
#include <math.h>



#define PRETO 0
#define BRANCO -1

//Define até qual claridade é considerado ruído.
#define MEDIA 85



typedef struct{
    unsigned long altura;
    unsigned long largura;
    int** imagem;
}ImagemInt;



/*===============================Funções Internas=============================*/
/*----------------------------------------------------------------------------*/
ImagemInt* alocaStruct(Imagem1C* img);
void removeRuido(Imagem1C* img, ImagemInt *copia, int media);
int localizarRuidos(ImagemInt* img, int *conta_ruidos);
int preencheLabirinto(ImagemInt* img, int linha, int coluna, int setando);
void buscaMaioresGrupos(int* cont_valores, int setando, int* maior_valor_1, int* maior_valor_2);
void encontraCentro(ImagemInt* img, int* cont_valores, int maior_valor_1, int maior_valor_2, Coordenada *l, Coordenada *r);
double calculaAngulo(Coordenada* l, Coordenada* r);
void destroiCopia (ImagemInt* img);
int* alocaVetorInt (ImagemInt* img);
/*----------------------------------------------------------------------------*/


/**função principal
-------------------------------------------------------------
 *Parâmetros:
 *   Imagem1c* img: Struct com resolução e uma matriz com valores que representam a imagem.
 *   Coordenada* l: Struct onde coloca-se a coordenada do centro da bola a esquerda.
 *   Coordenada* r: Struct onde coloca-se a coordenada do centro da bola a direita.
 *
 *   Retorna o ângulo Entre os 2 círculos.  */

double detectaSensorBar (Imagem1C* img, Coordenada* l, Coordenada* r){

    int quant_ruidos, *conta_ruidos, cor_agrupamento1, cor_agrupamento2;
    ImagemInt *copia;

    copia = alocaStruct(img);

    //Preenche a matriz cópia.
    removeRuido(img, copia, MEDIA);

    conta_ruidos = alocaVetorInt(copia);

    //Identifica cada grupo de ruídos na cópia e as coordenadas dos 2 maiores.
    quant_ruidos = localizarRuidos(copia, conta_ruidos);
    buscaMaioresGrupos(conta_ruidos, quant_ruidos, &cor_agrupamento1, &cor_agrupamento2);
    encontraCentro(copia, conta_ruidos, cor_agrupamento1, cor_agrupamento2, l, r);

    free(conta_ruidos);
    destroiCopia(copia);

    //Retorna o angulo entre o círculo da esquerda e da direita.
    return calculaAngulo(l, r);
}


/*----------------------------------------------------------------------------*/
/** Aloca uma Struct similar a Imagem1C, porém, com valores de int.
 *
 *  Parâmetros: Imagem1C* img: imagem a copiar.
 *
 *  Retorna a Struct criada. */

ImagemInt* alocaStruct(Imagem1C* img){

    int i, x, y;
    int **linhas;
    ImagemInt *copia;

    copia = (ImagemInt*) malloc (sizeof (ImagemInt));

    //Por algum motivo isso requer uma variável intermediaria????
    x = img->largura;
    y = img->altura;
    copia->largura = x;
    copia->altura = y;

    linhas = (int** ) malloc (img->altura * sizeof (int* ));

    for(i = 0; i < img->altura; i++)
        linhas[i] = (int* ) malloc (img->largura * sizeof (int));

    copia->imagem = linhas;

    return copia;
}


/*----------------------------------------------------------------------------*/
/** Copia o conteúdo de uma imagem para outra usando um valor para
 *  tudo acima do limiar e 0 para o que estiver abaixo.
 *
 * Parâmetros: Imagem1C* img: imagem a copiar.
 *             ImagemInt* copia: imagem de destino. Pressupomos que tem o mesmo
 *                               tamanho da imagem de entrada.
 *
 * Valor de retorno: nenhum. */

void removeRuido(Imagem1C* img, ImagemInt *copia, int media){

    int i, j;


    for(i = 0; i < img->altura; i++)
        for(j = 0; j < img->largura; j++){
            if(img->dados[i][j] > media)
                copia->imagem[i][j] = BRANCO;
            else
                copia->imagem[i][j] = PRETO;
        }
}


/*----------------------------------------------------------------------------*/
/** Localiza ruídos e utiliza outra função para identificar o tamanho do ruído
 *  ou do círculo e dar a ele um identificador.
 *
 * Parâmetros: ImagemInt* img: imagem onde os ruídos serão buscados.
 *             int* conta_ruídos: vetor na qual é marcado o tamanho do ruído em
 *                                cada identificador.
 *
 * Valor de retorno: Quantidade de grupos criados, para apenas considerar a parte
 *                   do vetor utilizada. */

int localizarRuidos(ImagemInt* img, int *conta_ruidos){
    int i, j, setando = 0;

    setando = 1;

    //Cada BRANCO encontrado inicia o preenchimento da região que pode ser ruído
    //ou círculo.
    for(i = 0; i < img->altura; i++)
        for(j = 0; j < img->largura; j++)
            if(img->imagem[i][j] == BRANCO){
                img->imagem[i][j] = setando;
                conta_ruidos[setando] = preencheLabirinto(img, i, j, setando);
                setando++;
            }

    return setando;
}


/*----------------------------------------------------------------------------*/
/** A partir de um ponto de ruído busca mais ruído ao seu redor e marca todo o
 *  grupo com um valor
 *
 * Parâmetros: ImagemInt* img: Imagem a ser marcada.
 *             int linha: Marca o ponto inicial de onde fazer a busca para não
 *                        percorrer a matriz toda.
 *             int coluna: Marca o ponto inicial de onde fazer a busca para não
 *                         percorrer a matriz toda.
 *             int setando: Identificador de qual grupo de ruído deve ser marcado.
 *
 * Valor de retorno: Quantidade de pixels marcados com i identificador. */

int preencheLabirinto(ImagemInt* img, int linha, int coluna, int setando){

    int i, j;
    int max_i, max_j, min_i, min_j;
    int flag, quant_setados;

    quant_setados = 0;

    min_i = linha - 1;
    min_j = coluna - 1;
    max_i = linha + 1;
    max_j = coluna + 1;

    flag = 1;
    while(flag){
        flag = 0;
        i = min_i;
        while(i < max_i){
            j = min_j;
            while(j < max_j){
                //Lógica de preenchimento.
                if(i - 1 >= 0 && j - 1 >= 0 && i + 1 < img->altura && j + 1 < img->largura && img->imagem[i][j] == setando){
                    if(img->imagem[i - 1][j] == BRANCO){
                        img->imagem[i - 1][j] = setando;
                        flag = 1;
                        //Não verificar se min_i da overflow evita usar outros ifs.
                        min_i--;
                        quant_setados++;
                    }
                    if(img->imagem[i][j - 1] == BRANCO){
                        img->imagem[i][j - 1] = setando;
                        flag = 1;
                        min_j--;
                        quant_setados++;
                    }
                    if(img->imagem[i + 1][j] == BRANCO){
                        img->imagem[i + 1][j] = setando;
                        flag = 1;
                        max_i++;
                        quant_setados++;
                    }
                    if(img->imagem[i][j + 1] == BRANCO){
                        img->imagem[i][j + 1] = setando;
                        flag = 1;
                        max_j++;
                        quant_setados++;
                    }

                }
                j++;
            }
            i++;
        }
    }
    return quant_setados + 1;
}


/*----------------------------------------------------------------------------*/
/** Busca os maiores grupos e coloca a posição deles no vetor.
 *
 * Parâmetros: int* cont_valores: Vetor com o tamanho de cada grupo de ruídos
 *             int setando: último valor do vetor a ser considerado.
 *             int* maior_valor_1: maior grupo de ruído.
 *             int* maior_valor_2: segundo maior grupo de ruído
 *
 * Valor de retorno: nenhum. */

void buscaMaioresGrupos(int* cont_valores, int setando, int* maior_valor_1, int* maior_valor_2){

    int i, maior_grupo_1, maior_grupo_2;

    maior_grupo_1 = -1;
    maior_grupo_2 = -1;

    //Altera os ponteiros para salvar a posição no vetor dos 2 maiores
    //conjunto de números.
    //Começa em 1.
    for (i = 1; i < setando; i++){
        if (cont_valores[i] > maior_grupo_1){
            if(maior_grupo_1 > maior_grupo_2){
                maior_grupo_2 = maior_grupo_1;
                (*maior_valor_2) = (*maior_valor_1);
            }
            maior_grupo_1 = cont_valores[i];
            (*maior_valor_1) = i;
        } else if (cont_valores[i] > maior_grupo_2){
            maior_grupo_2 = cont_valores[i];
            (*maior_valor_2) = i;
        }
    }
}


/*----------------------------------------------------------------------------*/
/** Encontra o centro de cada círculo.
 *
 * Parâmetros: ImagemInt* img: imagem.
 *             int* cont_valores: vetor com a quantidade de valores em cada grupo
 *             de ruído.
 *             int maior_valor_1: posição do vetor correspondente ao maior ruído.
 *             int maior_valor_2: posição do vetor correspondente ao segundo maior
 *             ruído.
 *             Coordenada* l: Recebe as coordenadas do círculo mais a esquerda.
 *             Coordenada* r: Recebe as coordenadas do círculo mais a direita.
 *
 * Valor de retorno: nenhum. */

void encontraCentro(ImagemInt* img, int* cont_valores, int maior_valor_1, int maior_valor_2, Coordenada *l, Coordenada *r){

    int i, j;
    int altura_soma_1, largura_soma_1;
    int altura_soma_2, largura_soma_2;
    double altura_media_1, largura_media_1, altura_media_2, largura_media_2;
    Coordenada maior1, maior2;


    altura_soma_1 = 0;
    largura_soma_1 = 0;
    altura_soma_2 = 0;
    largura_soma_2 = 0;


    //Correr o vetor somando pilha de i e pilha de j.
    for(i = 0; i < img->altura; i++){
        for(j = 0; j < img->largura; j++){
            if(img->imagem[i][j] == maior_valor_1){
                altura_soma_1 += i;
                largura_soma_1 += j;
            }
            else if(img->imagem[i][j] == maior_valor_2){
                altura_soma_2 += i;
                largura_soma_2 += j;
            }
        }
    }


    //Calcula a media para achar o centro do eixo x e y.
    altura_media_1 = (altura_soma_1 / (float) cont_valores[maior_valor_1]) + 0.5;
    altura_media_2 = (altura_soma_2 / (float) cont_valores[maior_valor_2]) + 0.5;

    largura_media_1 = (largura_soma_1 / (float) cont_valores[maior_valor_1]) + 0.5;
    largura_media_2 = (largura_soma_2 / (float) cont_valores[maior_valor_2]) + 0.5;


    maior1.y = (int) altura_media_1;
    maior1.x = (int) largura_media_1;

    maior2.y = (int) altura_media_2;
    maior2.x = (int) largura_media_2;


    //Descobre qual fica na esquerda e qual na direita.
    if(maior1.x < maior2.x){
        l->x = maior1.x;
        l->y = maior1.y;
        r->x = maior2.x;
        r->y = maior2.y;
    }
    else{
        l->x = maior2.x;
        l->y = maior2.y;
        r->x = maior1.x;
        r->y = maior1.y;
    }
}


/*----------------------------------------------------------------------------*/
/** Calcula o ângulo entre os 2 círculos.
 *
 * Parâmetros: Coordenada* l: coordenada do círculo da esquerda.
 *             Coordenada* r: coordenada do círculo da direita.
 *
 * Valor de retorno: ângulo calculado. */

double calculaAngulo(Coordenada* l, Coordenada* r){

    double deltaX, deltaY, tangente, angulo;


    deltaY = abs(l->y - r->y);
    deltaX = abs(l->x - r->x);

    tangente = deltaY/deltaX;

    angulo = atan (tangente);

    return angulo;
}



/*----------------------------------------------------------------------------*/
/** desaloca a Struct de int criada dentro da função.
 *
 * Parâmetros: ImagemInt* img: imagem a desalocar
 *
 * Valor de retorno: nenhum. */

void destroiCopia (ImagemInt* img)
{
    int i;

    for (i = 0; i < img->altura; i++)
        free (img->imagem [i]);
    free (img->imagem);
    free (img);
}


/*----------------------------------------------------------------------------*/
/** Cria vetor onde serão guardadas as quantidades de pixels de cada grupo de
 *  ruído.
 *
 * Parâmetros: ImagemInt* img: As coordenadas da struct ditam o tamanho do vetor.
 *
 * Valor de retorno: posição onde o vetor está guardado */

int* alocaVetorInt (ImagemInt* img){

    int tamanho;
    int *vetor;

    //Vetor possui o tamanho = quantidade de pixel, porém, a sua maioria não
    //será usada.
    tamanho = img->altura * img->largura;

    vetor = (int* ) malloc (tamanho * sizeof (int));

    return vetor;
}