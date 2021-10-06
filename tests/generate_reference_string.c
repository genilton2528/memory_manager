#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Converte string para int
int parse(const char *opt)
{
    char *remainder;
    int return_val = strtol(opt, &remainder, 10);
    if (strcmp(remainder, opt) == 0)
    {
        printf("Error parsing: %s\n", opt);
        exit(1);
    }
    return return_val;
}

// Somatorio de todas as probabilidades
int sum(int vet[], int len) {
    int sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += vet[i];
    }
    return sum;
}

// Recebe um numero aleatorio e retorna a pagina que se encontra no intervalo de probabilidade
int page(int vet[], int len, int random) {
    int start = 0;
    int end = 0;
    for (int i = 0; i < len; i++)
    {
        start = end;
        end += vet[i];        
        if ( random >= start && random < end) {
            return i;
        }        
    }
    return -1;
}

// Inicia o vetor de probabilidades
void init(int prob[], int len) {
    for (int i = 0; i < len; i++)
    {
        prob[i] = 1;
    }
}

// Incrementa a probabilidade da pagina referenciada e de seus vizinhos
void incProb(int range, int len, int pos, int prob[]) {
    int start = (pos - range) < 0 ? 0 : (pos - range);
    int end = (pos + range) > (len-1) ? len : (pos + range);

    for (int i = 0; i < len; i++)
    {
        if (prob[i] <= 1)
        {
            continue;
        }        
        prob[i]--;
    }
    
    for (int i = start; i <= end; i++)
    {
        prob[i] += 3;
    }
    prob[pos] += 2;
}

// Retorna um tipo de acesso aleatorio
char type_access(int c){
    return ( c%2 == 0 )? 'w':'r';
}

int main(int argc, char const *argv[])
{
    int pages;
    int requests;
    int range;
    int random;
    int* prob;    
    int sumProb;
    
    if (argc < 4)
    {
        printf("program <pages> <requests> <range>");
        exit(0);
    }
    pages = parse(argv[1]);
    requests = parse(argv[2]);
    range = parse(argv[3]);    

    prob = malloc(pages * sizeof(int));
    init(prob, pages);

    srand(time(NULL));

    for (int i = 0; i < requests; i++)
    {                  
        sumProb = sum(prob, pages);
        random = rand() % sumProb;        
        printf("%d %c\n", page( prob, pages, random), type_access(rand()));
        incProb( range, pages, random, prob);
    }

    return 0;
}