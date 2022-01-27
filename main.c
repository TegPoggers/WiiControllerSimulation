/*============================================================================*/
/* TRABALHO: WII SENSOR BAR                                                   */
/*----------------------------------------------------------------------------*/
/* Autor: Bogdan T. Nassu - nassu@dainf.ct.utfpr.edu.br                       */
/*============================================================================*/

#include <stdio.h>
#include <time.h>
#include <math.h>
#include "gerador_de_testes.h"
#include "projeto5.h"

/*============================================================================*/
/* Um modificador para a semente usada pelo gerador de n�meros aleat�rios na
 * hora de criar imagens de teste. Todos os trabalhos usar�o as mesmas
 * imagens, ent�o o offset precisa ser igual para todos. Entretanto, usaremos
 * um valor diferente na hora de testar os trabalhos de fato. */

#define RANDOM_SEED_OFFSET 0

#define N_TESTES 1

#define SALVA_INTERMEDIARIOS 0 /* Flag que diz se devemos salvar as imagens de teste. Desative se for rodar muitos testes! */

/*============================================================================*/

#define MIN(a,b) ((a<b)? a : b)
#define MAX(a,b) ((a>b)? a : b)

/*============================================================================*/

int main ()
{
    int i;
    Imagem1C* img; /* Imagem de teste. */
    clock_t tempo_inicio, tempo_total = 0;
    double erro_l, erro_r;
    double erros [N_TESTES];
    double erro_medio, desvpad, erro_max;
    int pior_teste = -1;
    /* TODO: depois da entrega! int angulos_errados = 0; */
    Coordenada l_real, r_real, l_medido, r_medido;
    double diagonal, angulo_real, angulo_medido;

    /* Realiza N_TESTES testes. */
    for (i = 0; i < N_TESTES; i++)
    {
        printf ("----------------------------------------\n");
        printf ("Teste %d\n", i);

        /* Gera um caso de teste. */
        img = geraCasoDeTeste (i+RANDOM_SEED_OFFSET, &l_real, &r_real, &angulo_real);
        if (!img)
        {
            printf ("Erro gerando caso de teste (avisar o professor!!!).\n");
            return (1);
        }
        diagonal = sqrt (img->altura*img->altura + img->largura*img->largura);

        /* Salva (se necess�rio). */
        if (SALVA_INTERMEDIARIOS)
        {
            char foostring [64];
            sprintf (foostring, "teste%d.bmp", i);
            salvaImagem1C (img, foostring);
        }

        /* Invoca o testador. */
        tempo_inicio = clock ();
        angulo_medido = detectaSensorBar (img, &l_medido, &r_medido);
        tempo_total += clock () - tempo_inicio;

        printf ("Real: (%d,%d) e (%d,%d) (SEGREDO rad)\nDetectado: (%d,%d) e (%d,%d) (%.4f rad)\nErro: %.2f\n", l_real.x, l_real.y, r_real.x, r_real.y, l_medido.x, l_medido.y, r_medido.x, r_medido.y, angulo_medido, erros [i]);

        /* Joga fora as imagens geradas. */
        destroiImagem1C (img);
    }

    /* Calcula m�dia, m�ximo e desvio padr�o. */
    erro_medio = erros [0];
    erro_max = erros [0];
    pior_teste = 0;
    for (i = 1; i < N_TESTES; i++)
    {
        erro_medio += erros [i];
        if (erros [i] > erro_max)
        {
            erro_max = erros [i];
            pior_teste = i;
        }
    }

    erro_medio /= N_TESTES;

    desvpad = 0;
    for (i = 0; i < N_TESTES; i++)
        desvpad += (erros [i] - erro_medio)*(erros [i] - erro_medio);
    desvpad /= N_TESTES;
    desvpad = sqrt (desvpad);

    /* Mostra os resultados. */
    printf ("----------------------------------------\n");
    printf ("Erro medio: %.6f\n", erro_medio);
    printf ("Pior teste: %d (erro: %.6f)\n", pior_teste, erro_max);
    printf ("Desvio padrao: %.6f\n", desvpad);
    printf ("Score: %.6f\n", erro_medio + 1.5*desvpad);
    // TODO: depois da entrega! printf ("Angulos errados: %d\n", angulos_errados);
    printf ("Tempo total: %d\n", (int) tempo_total);

    return (0);
}

/*============================================================================*/
