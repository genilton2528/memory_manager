#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Campos da tabela de páginas
#define PT_FIELDS 6         // quantidade de campos na tabela
#define PT_FRAMEID 0        // Endereço da memória física
#define PT_MAPPED 1         // Endereço presente na tabela
#define PT_DIRTY 2          // Página dirty
#define PT_REFERENCE_BIT 3  // Bit de referencia
#define PT_REFERENCE_MODE 4 // Tipo de acesso, converter para char
#define PT_AGING_COUNTER 5  // Contador para aging

// Tipos de acesso
#define READ 'r'
#define WRITE 'w'

// Estruturas para o LRU
struct ListNode {
    int value;
    int key;
    struct ListNode *prev;
    struct ListNode *next;
};

struct LRUCache {
    int capacity;
    int len;
    struct ListNode **hashmap;
    // Cabeça e cauda
    struct ListNode *head;
    struct ListNode *tail;
};

// Define a função que simula o algoritmo da política de subst.
typedef int (*eviction_function)(unsigned char **, int, int, int *, int, int, struct LRUCache*, int);

typedef struct
{
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

// Printa o campo da tabela formatado
void formated_cell(int cell)
{
    if (cell == -1)
    {
        printf("  X   |");
    }
    else if (cell > 9)
    {
        printf("  %d  |", cell);
    }
    else
    {
        printf("  %d   |", cell);
    }
}

// Retorna uma linha da page_table, que e escolhido pelo id do frame
unsigned char *return_frame(unsigned char **page_table, int number_pages, int frame_id)
{
    for (int i = 0; i < number_pages; i++)
    {
        if (page_table[i][PT_FRAMEID] == frame_id)
        {
            return page_table[i];
        }
    }
    return NULL;
}

// Retorna o index da pagina de um determinado frame
int index_of(unsigned char **page_table, int number_pages, int frame_id)
{
    for (int i = 0; i < number_pages; i++)
    {
        if (page_table[i][PT_FRAMEID] == frame_id)
        {
            return i;
        }
    }
    return -1;
}

// converte a variavel aging para binario
void convert_byte(int aging) {    
    char byte[] = { '0', '0', '0', '0', '0', '0', '0', '0' };
    int decimal = aging;
    for (int i = 7; i >= 0; i--, aging /= 2)
    {
        if (aging%2 == 0)
        {
            byte[i] = '0';
        } else
        {
            byte[i] = '1';
        }        
    }
    printf(" %s -", byte);
    if ( decimal > 99)
    {
        printf(" %d |", decimal);
    } else if ( decimal > 9)
    {
        printf(" %d  |", decimal);
    }
    else
    {
        printf(" %d   |", decimal);
    }
}

// Imprimi uma tabela com os dados dos frames
void print_frames(unsigned char **page_table, int number_pages, int number_frames)
{

    unsigned char *frame = NULL;

    printf("║         | Addr | Pag  |  M   |  D   |  R   |  AT  |      Agin      |\n");
    for (int i = 0; i < number_frames; i++)
    {
        frame = return_frame(page_table, number_pages, i);
        printf("║         |");
        formated_cell(i);
        if (frame == NULL)
        {
            printf("  X   |  -   |  -   |  -   |  -   |       -        |\n");
        }
        else
        {
            formated_cell(index_of(page_table, number_pages, i)); // Pagina
            formated_cell(frame[PT_MAPPED]);                      // Endereço presente na tabela
            formated_cell(frame[PT_DIRTY]);                       // Página dirty
            formated_cell(frame[PT_REFERENCE_BIT]);               // Bit de referencia            
            if (frame[PT_REFERENCE_MODE] == WRITE || frame[PT_REFERENCE_MODE] == READ)
            {
                printf("  %c   |", frame[PT_REFERENCE_MODE]); // Tipo de acesso, converter para char
            }
            else
            {
                printf("  -   |");
            }
            convert_byte(frame[PT_AGING_COUNTER]);               // Contador para aging
            printf("\n");
        }
    }
    printf("║\n");
}

// Algoritimos
int fifo(
    unsigned char **page_table, int number_pages, int previous_page,
    int *fifo_first_frame, int number_frames, int virtual_address, struct LRUCache* cache, int clock )
{
    for (int page = 0; page < number_pages; page++) // Encontra página mapeada
    {
        if (page_table[page][PT_MAPPED] == 1 && page_table[page][PT_FRAMEID] == *fifo_first_frame)
        {
            return page;
        }
    }
    return -1;
}

int second_chance(
    unsigned char **page_table, int number_pages, int previous_page,
    int *fifo_first_frame, int number_frames, int virtual_address, struct LRUCache* cache, int clock )
{
    int page = 0;

    for ( ; page < number_pages; )
    {
        if (page_table[page][PT_MAPPED] == 1 && page_table[page][PT_FRAMEID] == *fifo_first_frame){
            if (page_table[page][PT_REFERENCE_BIT] == 1) {
                *fifo_first_frame = (*fifo_first_frame + 1) % number_frames;
                page_table[page][PT_REFERENCE_BIT] = 0;
            } else {
                break;
            }
        }
        page = ( page + 1 ) % number_pages;        
    }     
    
    return page;    
}

int nru(
    unsigned char **page_table, int number_pages, int previous_page,
    int *fifo_first_frame, int number_frames, int virtual_address, struct LRUCache* cache, int clock )
{
    int R = 0, M = 1;
    int class[4][2];
    class[0][R] = 0; class[0][M] = 0;
    class[1][R] = 0; class[1][M] = 1;
    class[2][R] = 1; class[2][M] = 0;
    class[3][R] = 1; class[3][M] = 1;

    // Percorre as classes
    for (int i = 0; i < 4; i++)
    {
        // Percorre as paginas
        for (int page = 0; page < number_pages; page++) // Encontra página mapeada
        {
            if (
                page_table[page][PT_MAPPED] == 1 && 
                page_table[page][PT_DIRTY] == class[i][M] &&
                page_table[page][PT_REFERENCE_BIT] == class[i][R]
            )
            {
                return page;
            }
        }
    }
    
}

// Decrementa o aging
void age(unsigned char **page_table, int number_pages){
    for (int page = 0; page < number_pages; page++)
    {
        page_table[page][PT_AGING_COUNTER] /= 2;
    }
}

int aging(
    unsigned char **page_table, int number_pages, int previous_page,
    int *fifo_first_frame, int number_frames, int virtual_address, struct LRUCache* cache, int clock )
{
    int smaller = index_of(page_table, number_pages, 0); // Retorna a pagina que esta alocada no FRAME_ID 0
    for (int page = 0; page < number_pages; page++) // Encontra página mapeada
    {
        if ( page_table[page][PT_MAPPED] == 1 )
        {               
            if ( page_table[page][PT_AGING_COUNTER] <= page_table[smaller][PT_AGING_COUNTER] )
            {                
                smaller = page;
            }
        }
    }
    return smaller;
}

// LRU
struct ListNode *initNode(int value, int key) {
    struct ListNode *node = (struct ListNode *)malloc(sizeof(struct ListNode));
    node->value = value;
    node->key = key;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

struct LRUCache initCache(int capacity) {
    struct LRUCache cache;
    cache.capacity = capacity;   
    cache.len = 0; 
    cache.head = initNode(-1, -1);
    cache.tail = initNode(-1, -1);
    // O início da lista de cadeia inicial é head <-> tail
    cache.head->next = cache.tail;
    cache.tail->prev = cache.head;
    // Inicializa hashmap
    cache.hashmap = malloc(capacity * sizeof(struct ListNode*));
    for (int i = 0; i < capacity; i++)
    {
        cache.hashmap[i] = NULL;
    }
    return cache;
}

// Como as operações Get e PUT podem precisar mover um nó na lista linear bidirecional para o final, defina um método
void move_node_to_tail(int key, struct LRUCache* cache) {
    // Primeiro, o nó que aponta para a tabela hash KEY é retirado e o nó é nomeado pela simplicidade.
    //      hashmap[key]                               hashmap[key]
    //           |                                          |
    //           V              -->                         V
    // prev <-> node <-> next         pre <-> next   ...   node
    struct ListNode* node = cache->hashmap[key];
    node->prev->next = node->next;
    node->next->prev = node->prev;
    // Depois de inserir o nó no nó da cauda
    //                 hashmap[key]                 hashmap[key]
    //                      |                            |
    //                      V        -->                 V
    // prev <-> tail  ...  node                prev <-> node <-> tail
    node->prev = cache->tail->prev;
    node->next = cache->tail;
    cache->tail->prev->next = node;
    cache->tail->prev = node;
}

int has(int value, struct LRUCache* cache) {
    int hashKey = value % cache->capacity;

    if( cache->hashmap[hashKey] != NULL &&  cache->hashmap[hashKey]->value == value ) {
        return hashKey;
    }
    
    for (int i = 0; i < cache->capacity; i++)
    {
        hashKey = (hashKey + 1) % cache->capacity;   
        if (cache->hashmap[hashKey] != NULL && cache->hashmap[hashKey]->value == value) {
            return hashKey;
        }
    }

    return -1;
}

int nextHashKey(int value, struct LRUCache* cache) {
    int key = value % cache->capacity;
    
    if (cache->len == cache->capacity)
    {
        return -1;
    }
    if( cache->hashmap[key] == NULL ) {
        return key;
    }
    while (cache->hashmap[key] != NULL)
    {
        key = (key + 1) % cache->capacity;
    }
    return key;
}

int put(int value, struct LRUCache* cache) {
    struct ListNode* removed = NULL;
    int removedValue = -1;
    int key;
    // Verifica se o valor ja esta na lista, caso true retorna o index, se false retorna -1
    int indexOf = has( value, cache );     
    if ( indexOf >= 0 ) {
        // Se a própria chave já estiver na tabela de hash, você não precisa adicionar novos nós na lista.
        // Mas precisa atualizar o valor do valor do valor do nó de valor correspondente.
        // Move o nó para o final
        move_node_to_tail( indexOf, cache);
    } else {
        if ( cache->len == cache->capacity ) {
            // remove
            key = cache->head->next->key;
            cache->hashmap[key] = NULL;
            removed = cache->head->next;
            removedValue = removed->value;
            // Remova o nó que não foi acessado por mais tempo, ou seja, o nó após o nó principal
            cache->head->next = cache->head->next->next;
            cache->head->next->prev = cache->head;
            cache->len--;
            free(removed);
        } else {
            key = nextHashKey( value, cache);
        }
        
        struct ListNode* newNode = initNode(value, key);
        cache->hashmap[key] = newNode;
        newNode->prev = cache->tail->prev;
        newNode->next = cache->tail;
        cache->tail->prev->next = newNode;
        cache->tail->prev = newNode;
        cache->len++;
        return removedValue;
    }
}

int get(int value, struct LRUCache* cache) {
    int indexOf = has( value, cache );
    if ( indexOf >= 0 ) {
        // Se você o moveu para o final da lista vinculada (torna-se o acesso mais recente)
        move_node_to_tail( indexOf, cache);
        return cache->hashmap[indexOf]->value;
    }
    return -1;
}

void printList(struct LRUCache* cache){
   if (cache->head == NULL) {
        return;
    }

    struct ListNode* current = cache->head->next;
    
    printf("║\n║         LRUCache: Head -> ");

    while( current->next != NULL ) {
        printf("{ %d }[%d] -> ", current->value, current->key);
        current = current->next;
    }
    
    printf("Tail\n║\n");
}

int lru(
    unsigned char **page_table, int number_pages, int previous_page,
    int *fifo_first_frame, int number_frames, int virtual_address, struct LRUCache* cache, int clock )
{        
    return put( virtual_address, cache);
}

int random_page(
    unsigned char **page_table, int number_pages, int previous_page,
    int *fifo_first_frame, int number_frames, int virtual_address, struct LRUCache* cache, int clock )
{
    int page = rand() % number_pages;
    while (page_table[page][PT_MAPPED] == 0)
    { // Encontra página mapeada
        page = rand() % number_pages;
    }
    return page;
}

// Simulador a partir daqui

int find_next_frame(
    int *physical_memory, int *number_free_frames,
    int number_frames, int *previous_free)
{
    if (*number_free_frames == 0)
    {
        return -1;
    }

    // Procura por um frame livre de forma circula na memória.
    // Não é muito eficiente, mas fazer um hash em C seria mais custoso.
    do
    {
        *previous_free = (*previous_free + 1) % number_frames;
    } while (physical_memory[*previous_free] == 1);

    return *previous_free;
}

// reseta os bits de referencia
void reset_reference_bit( unsigned char **page_table, int number_pages, int clock ){
    if (clock == 1)
    {
        for (int i = 0; i < number_pages; i++)
        {
            page_table[i][PT_REFERENCE_BIT] = 0;
        }
    }
}

int simulate(
    unsigned char **page_table, int number_pages, int *previous_page,
    int *fifo_first_frame, int *physical_memory, int *number_free_frames,
    int number_frames, int *previous_free, int virtual_address,
    char access_type, eviction_function evict, int clock, struct LRUCache* cache,int debug)
{
    if (virtual_address >= number_pages || virtual_address < 0)
    {
        printf("Invalid access: \n");
        exit(1);
    }

    if (page_table[virtual_address][PT_MAPPED] == 1)
    {
        if (debug == 1) {
            printf("║ ╚═══> Pagina mapeada na memoria fisica...\n║\n");
        }
        page_table[virtual_address][PT_REFERENCE_BIT] = 1; // Bit de referencia
        page_table[virtual_address][PT_AGING_COUNTER] += 128; // Contador aging
        get( virtual_address, cache); // Referencia na cache do LRU
        if (access_type == WRITE)
        {
            page_table[virtual_address][PT_DIRTY] = 1;
        }        
        return 0; // Not Page Fault!
    }

    if (debug == 1) {
        printf("║ ╚═╦═> Pagina não mapeada na memoria fisica\n");
    }
    int next_frame_address;
    if ((*number_free_frames) > 0)
    { // Ainda temos memória física livre!
        next_frame_address =
            find_next_frame(physical_memory, number_free_frames, number_frames, previous_free);
        if (debug == 1) {
            printf("║   ╚═══> Tem memória física livre, frame: %d\n║\n", next_frame_address);
        }
        if (*fifo_first_frame == -1)
            *fifo_first_frame = next_frame_address;
        *number_free_frames = *number_free_frames - 1;
        put( virtual_address, cache); // Adiciona a pagina na chache
    }
    else
    { // Precisamos liberar a memória!
        assert(*number_free_frames == 0);
        int to_free =
            evict(page_table, number_pages, *previous_page, fifo_first_frame, number_frames, virtual_address, cache, clock);
        next_frame_address = page_table[to_free][PT_FRAMEID];
        if (debug == 1) {
            printf("║   ╚═══> Precisamos liberar a memória, Page despejado: %d, Frame liberado: %d\n║\n", to_free, next_frame_address);
        }
        assert(to_free >= 0);
        assert(to_free < number_pages);
        assert(page_table[to_free][PT_MAPPED] != 0);

        // Incrementa o fifo
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
    unsigned char *page_table_data = page_table[virtual_address];
    page_table_data[PT_FRAMEID] = next_frame_address;
    page_table_data[PT_MAPPED] = 1;
    if (access_type == WRITE)
    {
        page_table_data[PT_DIRTY] = 1;
    }
    page_table_data[PT_REFERENCE_BIT] = 1;
    page_table[virtual_address][PT_AGING_COUNTER] += 128;
    page_table_data[PT_REFERENCE_MODE] = (unsigned char)access_type;
    *previous_page = virtual_address;

    reset_reference_bit(page_table, number_pages, clock);

    return 1; // Page Fault!
}

void run(
    unsigned char **page_table, int number_pages, int *previous_page, int *fifo_first_frame, int *physical_memory,
    int *number_free_frames, int number_frames, int *previous_free, eviction_function evict, 
    int clock_freq, struct LRUCache* cache, int debug)
{
    int virtual_address;
    char access_type;
    int i = 0;
    int clock = 0;
    int faults = 0;
    if (debug == 1) {
        printf("╔ Inicio da simulação...\n║\n");
    }
    while (scanf("%d", &virtual_address) == 1)
    {
        getchar();
        scanf("%c", &access_type);
        clock = ((i + 1) % clock_freq) == 0;
        age(page_table, number_pages);
        if (debug == 1) {
            printf("╠═╦═> Endereco virtual: %d, acesso: %c, clock: %d\n", virtual_address, access_type, clock);
        }
        faults += simulate(
            page_table, number_pages, previous_page, fifo_first_frame,
            physical_memory, number_free_frames, number_frames, previous_free,
            virtual_address, access_type, evict, clock, cache, debug);
        i++;
        if (debug == 1)
        {
            print_frames(page_table, number_pages, number_frames);
            printList(cache);
        }        
    }
    if (debug == 1) {
        printf("╚═══> Total de faltas: %d\n", faults);
    } else {
        printf("%d\n", faults);
    }
    
}

int parse(char *opt)
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

void read_header(int *number_pages, int *number_frames)
{
    scanf("%d %d\n", number_pages, number_frames);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage %s <algorithm> <clock_freq> <options>\n", argv[0]);
        printf("Algorithms: fifo, second_chance, nru, aging, lru, random\n");
        exit(1);
    }

    char *algorithm = argv[1];
    int clock_freq = parse(argv[2]);
    int number_pages;
    int number_frames;
    int debug = 0;
    read_header(&number_pages, &number_frames);

    // Verifica se esta no modo debug
    if (argc >= 4 && strcmp(argv[3], "debug") == 0){
        debug = 1;
    }

    // Aponta para cada função que realmente roda a política de parse
    paging_policy_table policies[] =
    {
        {"fifo", *fifo},
        {"second_chance", *second_chance},
        {"nru", *nru},
        {"aging", *aging},
        {"lru", *lru},
        {"random", *random_page}
    };

    // Seleciona o algoritimo escolhido
    int number_policies = sizeof(policies) / sizeof(policies[0]);
    eviction_function evict = NULL;
    for (int i = 0; i < number_policies; i++)
    {
        if (strcmp(policies[i].name, algorithm) == 0)
        {
            evict = policies[i].function;
            if (debug == 1) {
                printf("Algoritimo escolhido: %s...\n", algorithm);
            }            
            break;
        }
    }

    if (evict == NULL)
    {
        printf("Please pass a valid paging algorithm.\n");
        exit(1);
    }

    // Aloca tabela de páginas, onde cada celula e um inteiro de 1 Byte (8bits)
    unsigned char **page_table = (unsigned char **)malloc(number_pages * sizeof(unsigned char *));
    for (int i = 0; i < number_pages; i++)
    {
        page_table[i] = (unsigned char *)malloc(PT_FIELDS * sizeof(unsigned char));
        page_table[i][PT_FRAMEID] = -1;       // Endereço da memória física
        page_table[i][PT_MAPPED] = 0;         // Endereço presente na tabela
        page_table[i][PT_DIRTY] = 0;          // Página dirty
        page_table[i][PT_REFERENCE_BIT] = 0;  // Bit de referencia
        page_table[i][PT_REFERENCE_MODE] = 0; // Tipo de acesso, converter para char
        page_table[i][PT_AGING_COUNTER] = 0;  // Contador para aging
    }

    // Memória Real é apenas uma tabela de bits (na verdade uso ints) indicando
    // quais frames/molduras estão livre. 0 == livre!
    int *physical_memory = (int *)malloc(number_frames * sizeof(int));
    for (int i = 0; i < number_frames; i++)
    {
        physical_memory[i] = 0;
    }
    if (debug == 1)
    {
        printf("Memoria alocada com %d frames...\n", number_frames);
    }
    int number_free_frames = number_frames;
    int previous_free = -1;
    int previous_page = -1;
    int fifo_first_frame = -1;

    // Cache para o LRU
    struct LRUCache cache = initCache(number_frames);

    srand(time(NULL));
    // Roda o simulador
    run(
        page_table, number_pages, &previous_page, &fifo_first_frame, physical_memory,
        &number_free_frames, number_frames, &previous_free, evict, clock_freq, &cache, debug);

    // Liberando os mallocs
    for (int i = 0; i < number_pages; i++)
    {
        free(page_table[i]);
    }
    free(page_table);
    free(physical_memory);

    for (int i = 0; i < number_frames; i++)
    {
        free(cache.hashmap[i]);
    }

    return 0;
}