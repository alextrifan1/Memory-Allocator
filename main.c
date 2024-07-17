#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

typedef char ALIGN[16];

/// vream ca header-ul nostru sa se potriveasca pe adrese de memorie de 16 bytes
/// dimensiunea pt union este dimensiunea membrului mai mare din el
union header {
    struct {
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub;
};
typedef union header header_t;

header_t *head = NULL, *tail = NULL;

pthread_mutex_t global_malloc_lock;

/// traverseaza lista inlantuita pana gaseste un bloc de memorie care e free si are o marime potrivita
/// @param size nr de biti pe care vrem sa ii alocam
/// @return blocul de memorie disponibil sau NULL
header_t *get_free_block(size_t size) {
    header_t *curr = head;
    while(curr) {
        if (curr->s.is_free && curr->s.size >= size)
                return curr;
        curr = curr->s.next;
    }
    return NULL;
}

/// implementare malloc
/// @param size nr de biti pe care vrem sa ii alocam
/// @return pointer catre inceputul sp. de memorie alocat sau NULL
void *malloc(size_t size) {
    size_t total_size;
    void *block;                /// pointer catre memoria pe care o alocam
    header_t *header;
    if (!size)
        return NULL;
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size);
    if (header) {
        head->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }
    //tratam cazul in care nu gasim un bloc de memorie disponibil
    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);         /// sbrk incrementeaza spatiul de memorie cu size nr biti
    if (block == (void*) -1) {
        /// verifica daca s a putut face alocarea
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if (!head)
        head = header;
    if (tail)
        tail->s.next = header;
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header+1);
}

void free(void *block) {
    header_t *header, *tmp;
    void *programbreak;

    if (!block)
        return;
    pthread_mutex_lock((&global_malloc_lock));
    header = (header_t*)block - 1;

    programbreak = sbrk(0);
    if ((char*)block + header->s.size == programbreak) {
        if (head == tail) {
            head = NULL;
            tail = NULL;
        }
        else {
            tmp = head;
            while (tmp) {
                if (tmp->s.next == tail) {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}

///
/// @param num
/// @param nsize
/// @return
void *calloc(size_t num, size_t nsize) {
    size_t size;
    void *block;
    if (!num || !nsize)
        return NULL;
    size = num * nsize;
    if (nsize != size/num)
        return NULL;
    block = malloc(size);
    if (!block)
        return NULL;
    memset(block, 0, size);
    return block;
}

void *realloc(void *block, size_t size) {
    header_t *header;
    void *ret;
    if (!block || !size)
        return malloc(size);
    header = (header_t*)block - 1;
    if (header->s.size >= size)
        return block;
    ret = malloc(size);
    if (ret) {
        memcpy(ret, block, header->s.size);
        free(block);
    }
    return ret;
}
