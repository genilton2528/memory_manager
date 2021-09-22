#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// Campos da tabela de páginas
#define PT_FIELDS 6           // quantidade de campos na tabela
#define PT_FRAMEID 0          // Endereço da memória física
#define PT_MAPPED 1           // Endereço presente na tabela
#define PT_DIRTY 2            // Página dirty
#define PT_REFERENCE_BIT 3    // Bit de referencia
#define PT_REFERENCE_MODE 4   // Tipo de acesso, converter para char
#define PT_AGING_COUNTER 5    // Contador para aging

// Tipos de acesso
#define READ 'r'
#define WRITE 'w'

// Define a função que simula o algoritmo da política de subst.
typedef int (*eviction_function)(signed char**, int, int, int, int, int);

typedef struct {
    char *name;
    void *function;
} paging_policy_table;

// Codifique as reposições a partir daqui!
// Cada método abaixo retorna uma página para ser trocada. Note também
// que cada algoritmo recebe:
// - A tabela de páginas
// - O tamanho da mesma
// - A última página acessada
// - A primeira moldura acessada (para fifo)
// - O número de molduras
// - Se a última instrução gerou um ciclo de clock
//
// Adicione mais parâmetros caso ache necessário

int fifo( 
    signed char** page_table, int number_pages, int previous_page, 
    int fifo_first_frame, int number_frames, int clock ) 
{
    return -1;
}

int second_chance( 
    signed char** page_table, int number_pages, int previous_page,
    int fifo_first_frame, int number_frames, int clock ) 
{
    return -1;
}

int nru(
    signed char** page_table, int number_pages, int previous_page,
    int fifo_first_frame, int number_frames, int clock ) 
{
    return -1;
}

int aging(
    signed char** page_table, int number_pages, int previous_page,
    int fifo_first_frame, int number_frames, int clock ) 
{
    return -1;
}

int random_page(
    signed char** page_table, int number_pages, int previous_page,
    int fifo_first_frame, int number_frames, int clock ) 
{
    int page = rand() % number_pages;
    while ( page_table[page][PT_MAPPED] == 0 ) // Encontra página mapeada
    page = rand() % number_pages;
    return page;
}

// Simulador a partir daqui

int find_next_frame(
    int *physical_memory, int *number_free_frames,
    int number_frames, int *previous_free ) 
{
    if ( *number_free_frames == 0 ) {
        return -1;
    }

    // Procura por um frame livre de forma circula na memória.
    // Não é muito eficiente, mas fazer um hash em C seria mais custoso.
    do {
        *previous_free = (*previous_free + 1) % number_frames;
    } while (physical_memory[*previous_free] == 1);

    return *previous_free;
}

int simulate(
    signed char **page_table, int number_pages, int *previous_page, 
    int *fifo_first_frame, int *physical_memory, int *number_free_frames, 
    int number_frames, int *previous_free, int virtual_address, 
    char access_type, eviction_function evict, int clock ) 
{
    if ( virtual_address >= number_pages || virtual_address < 0 ) {
        printf("Invalid access \n");
        exit(1);
    }

    if ( page_table[virtual_address][PT_MAPPED] == 1 ) {
        page_table[virtual_address][PT_REFERENCE_BIT] = 1;
        return 0; // Not Page Fault!
    }

    int next_frame_address;
    if ((*number_free_frames) > 0) { // Ainda temos memória física livre!
        next_frame_address = 
            find_next_frame(physical_memory, number_free_frames, number_frames, previous_free);
        if (*fifo_first_frame == -1)
            *fifo_first_frame = next_frame_address;
        *number_free_frames = *number_free_frames - 1;
    } else { // Precisamos liberar a memória!
        assert(*number_free_frames == 0);
        int to_free = 
            evict(page_table, number_pages, *previous_page, *fifo_first_frame, number_frames, clock);
        assert(to_free >= 0);
        assert(to_free < number_pages);
        assert(page_table[to_free][PT_MAPPED] != 0);

        next_frame_address = page_table[to_free][PT_FRAMEID];
        *fifo_first_frame = (*fifo_first_frame + 1) % number_frames;
        // Libera pagina antiga
        page_table[to_free][PT_FRAMEID] = -1;
        page_table[to_free][PT_MAPPED] = 0;
        page_table[to_free][PT_DIRTY] = 0;
        page_table[to_free][PT_REFERENCE_BIT] = 0;
        page_table[to_free][PT_REFERENCE_MODE] = 0;
        page_table[to_free][PT_AGING_COUNTER] = 0;
    }

    // Coloca endereço físico na tabela de páginas!
    signed char *page_table_data = page_table[virtual_address];
    page_table_data[PT_FRAMEID] = next_frame_address;
    page_table_data[PT_MAPPED] = 1;
    if (access_type == WRITE) {
        page_table_data[PT_DIRTY] = 1;
    }
    page_table_data[PT_REFERENCE_BIT] = 1;
    page_table_data[PT_REFERENCE_MODE] = (signed char) access_type;
    *previous_page = virtual_address;

    if (clock == 1) {
        for (int i = 0; i < number_pages; i++)
            page_table[i][PT_REFERENCE_BIT] = 0;
    }

    return 1; // Page Fault!
}

void run(signed char **page_table, int number_pages, int *previous_page, int *fifo_first_frame,
         int *physical_memory, int *number_free_frames, int number_frames,
         int *previous_free, eviction_function evict, int clock_freq) {
    int virtual_address;
    char access_type;
    int i = 0;
    int clock = 0;
    int faults = 0;
    while (scanf("%d", &virtual_address) == 1) {
        getchar();
        scanf("%c", &access_type);
        clock = ((i+1) % clock_freq) == 0;
        faults += simulate(page_table, number_pages, previous_page, fifo_first_frame,
                           physical_memory, number_free_frames, number_frames, previous_free,
                           virtual_address, access_type, evict, clock);
        i++;
    }
    printf("%d\n", faults);
}

int parse(char *opt) {
    char* remainder;
    int return_val = strtol(opt, &remainder, 10);
    if (strcmp(remainder, opt) == 0) {
        printf("Error parsing: %s\n", opt);
        exit(1);
    }
    return return_val;
}

void read_header(int *number_pages, int *number_frames) {
    scanf("%d %d\n", number_pages, number_frames);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage %s <algorithm> <clock_freq>\n", argv[0]);
        exit(1);
    }

    char *algorithm = argv[1];
    int clock_freq = parse(argv[2]);
    int number_pages;
    int number_frames;
    read_header(&number_pages, &number_frames);

    // Aponta para cada função que realmente roda a política de parse
    paging_policy_table policies[] = {
            {"fifo", *fifo},
            {"second_chance", *second_chance},
            {"nru", *nru},
            {"aging", *aging},
            {"random", *random_page}
    };

    int n_policies = sizeof(policies) / sizeof(policies[0]);
    eviction_function evict = NULL;
    for (int i = 0; i < n_policies; i++) {
        if (strcmp(policies[i].name, algorithm) == 0) {
            evict = policies[i].function;
            break;
        }
    }

    if (evict == NULL) {
        printf("Please pass a valid paging algorithm.\n");
        exit(1);
    }

    // Aloca tabela de páginas
    signed char **page_table = (signed char **) malloc(number_pages * sizeof(signed char*));
    for (int i = 0; i < number_pages; i++) {
        page_table[i] = (signed char *) malloc(PT_FIELDS * sizeof(signed char));
        page_table[i][PT_FRAMEID] = -1;
        page_table[i][PT_MAPPED] = 0;
        page_table[i][PT_DIRTY] = 0;
        page_table[i][PT_REFERENCE_BIT] = 0;
        page_table[i][PT_REFERENCE_MODE] = 0;
        page_table[i][PT_AGING_COUNTER] = 0;
    }

    // Memória Real é apenas uma tabela de bits (na verdade uso ints) indicando
    // quais frames/molduras estão livre. 0 == livre!
    int *physical_memory = (int *) malloc(number_frames * sizeof(int));
    for (int i = 0; i < number_frames; i++) {
        physical_memory[i] = 0;
    }
    int number_free_frames = number_frames;
    int previous_free = -1;
    int previous_page = -1;
    int fifo_first_frame = -1;

    // Roda o simulador
    srand(time(NULL));
    run(page_table, number_pages, &previous_page, &fifo_first_frame, physical_memory,
        &number_free_frames, number_frames, &previous_free, evict, clock_freq);

    // Liberando os mallocs
    for (int i = 0; i < number_pages; i++) {
        free(page_table[i]);
    }
    free(page_table);
    free(physical_memory);
}