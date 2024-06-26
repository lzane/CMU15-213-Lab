/* 
 * Explicit, 32-bit and 64-bit clean allocator based on explicit free
 * lists, first-fit placement, and boundary tag coalescing. 
 * Blocks must be aligned to doubleword (8 byte) 
 * boundaries. Minimum block size is 24 bytes. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/*
 * If NEXT_FIT defined use next fit search, else use first-fit search 
 */
#define NEXT_FITx

/**
 * Checkheap Flag
*/
// #define checkheap(x) mm_checkheap(x);
#define checkheap(x)

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */ 
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) 

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                   
#define GET_ALLOC(p) (GET(p) & 0x1)                    

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                      
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) 

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) 
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) 

/* Free list node */
#define PREV_FREE(bp)       ((char *)(bp)) 
#define NEXT_FREE(bp)       ((char *)(bp) + DSIZE)
#define GETD(p)       (*(void **)(p))    
#define PUTD(p, val)  (*(void **)(p) = (val))

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */
#ifdef NEXT_FIT
static char *rover;           /* Next fit rover */
#endif

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void remove_free_node(void *a);
static void append_free_node(void *a, void *b);

/* 
 * mm_init - Initialize the memory manager 
 */
int mm_init(void) 
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(8*WSIZE)) == (void *)-1) 
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(3*DSIZE, 1)); /* Prologue header */
    PUTD(heap_listp + (2*WSIZE), NULL);             /* free list root node prev*/
    PUTD(heap_listp + (4*WSIZE), NULL);             /* free list root node next*/
    PUT(heap_listp + (6*WSIZE), PACK(3*DSIZE, 1)); /* Prologue footer */ 
    PUT(heap_listp + (7*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2*WSIZE);
    checkheap(__LINE__);

#ifdef NEXT_FIT
    rover = heap_listp;
#endif

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) 
        return -1;

    checkheap(__LINE__);
    return 0;
}

/* 
 * malloc - Allocate a block with at least size bytes of payload 
 */
void *malloc(size_t size) 
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;      

    if (heap_listp == 0){
        mm_init();
    }
    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= 2*DSIZE)                                          
        asize = 3*DSIZE;                                        
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE); 

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {  
        place(bp, asize);                  
        checkheap(__LINE__); 
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);                 
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)  
        return NULL;                                  
    place(bp, asize);       
    checkheap(__LINE__);                           
    return bp;
} 

/* 
 * free - Free a block 
 */
void free(void *bp)
{
    if (bp == 0) 
        return;

    size_t size = GET_SIZE(HDRP(bp));
    if (heap_listp == 0){
        mm_init();
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    
    bp = coalesce(bp);
    append_free_node(heap_listp, bp);

    checkheap(__LINE__); 
}

/*
 * realloc - Naive implementation of realloc
 */
void *realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

/**
 * calloc simple implementation
*/
void *calloc (size_t nmemb, size_t size){
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    memset(newptr, 0, bytes);

    return newptr;
}

/* 
 * mm_checkheap - Check the heap for correctness. Helpful hint: You
 *                can call this function using mm_checkheap(__LINE__);
 *                to identify the line number of the call site.
 */
void mm_checkheap(int lineno)  
{ 
    char *bp;
    int freeBlockCountByHeap = 0;
    int freeBlockCountByList = 0;

    // check the heap
    bp = heap_listp;    
    while (1)
    {
        // reach end
        if(GET_SIZE(HDRP(bp))==0 && GET_ALLOC(HDRP(bp))==1){
            if((size_t)(bp-heap_listp)+DSIZE != mem_heapsize()){
                printf("[%d] heapsize not equal\n", lineno);
                exit(1); 
            }
            break;
        }

        // header and footer not equal
        if(GET(HDRP(bp))!=GET(FTRP(bp))){
            printf("[%d] header and footer not equal\n", lineno);
            exit(1);
        }

        // base pointer not align to 8
        if((uintptr_t)bp % 8 != 0){
            printf("[%d] bp not align to 8\n", lineno);
            exit(1); 
        }

        // no consecutive free block
        if(bp!=heap_listp && !GET_ALLOC(HDRP(PREV_BLKP(bp))) && !GET_ALLOC(HDRP(bp))){
            printf("[%d] consecutive free block\n", lineno); 
            exit(1); 
        }

        if(!GET_ALLOC(HDRP(bp))){
            freeBlockCountByHeap++;
        }

        // check next bp
        bp += GET_SIZE(HDRP(bp));
    }

    // check the free list 
    bp = heap_listp;
    while (1)
    {
        // reach end
        if(GETD(NEXT_FREE(bp))==NULL){
            break;
        }

        void *next = GETD(NEXT_FREE(bp));
        void *prev = GETD(PREV_FREE(bp));
        
        // double link list
        if(GETD(PREV_FREE(next))!=bp){
            printf("[%d] a's next and b's prev not equal\n", lineno);
            exit(1);
        }

        // within bound
        if(next!=NULL && (next < mem_heap_lo() || next>mem_heap_hi())){
           printf("[%d] a's next exceed boundary\n", lineno);
           exit(1); 
        }
        if(prev!=NULL && (prev < mem_heap_lo() || prev>mem_heap_hi())){
           printf("[%d] a's prev exceed boundary\n", lineno);
           exit(1); 
        }

        freeBlockCountByList++;
        bp = next;
    }

    if(freeBlockCountByHeap != freeBlockCountByList){
        printf("[%d] freeBlockCountByHeap(%d) != freeBlockCountByList(%d)\n", lineno, freeBlockCountByHeap, freeBlockCountByList);
        exit(1);  
    }
}

/* 
 * The remaining routines are internal helper routines 
 */

/**
 * append node b after node a
*/
static void append_free_node(void *a, void *b){
    void *c = GETD(NEXT_FREE(a));
    PUTD(NEXT_FREE(a), b);
    PUTD(PREV_FREE(b), a);

    // reset b's next
    PUTD(NEXT_FREE(b), NULL);

    if(c!=NULL){
        PUTD(NEXT_FREE(b), c);
        PUTD(PREV_FREE(c), b);
    }    
}

/**
 * remove node a from free list
*/
static void remove_free_node(void *a){
    void *prevNode = GETD(PREV_FREE(a));
    void *nextNode = GETD(NEXT_FREE(a));

    if(prevNode!=NULL){
        PUTD(NEXT_FREE(prevNode), nextNode);
    }

    if(nextNode!=NULL){
        PUTD(PREV_FREE(nextNode), prevNode);
    }
}

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; 
    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                                        

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */   
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ 
    
    /* Coalesce if the previous block was free */
    bp = coalesce(bp);

    append_free_node(heap_listp, bp);
    return bp;             
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) 
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        remove_free_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        remove_free_node(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else {                                     /* Case 4 */
        remove_free_node(NEXT_BLKP(bp));
        remove_free_node(PREV_BLKP(bp)); 
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
#ifdef NEXT_FIT
    /* Make sure the rover isn't pointing into the free block */
    /* that we just coalesced */
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp))) 
        rover = bp;
#endif

    return bp;
}

/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize)
{   
    size_t csize = GET_SIZE(HDRP(bp));    
    if ((csize - asize) >= (3*DSIZE)) { 
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        // use the remain room to create a new block
        // then add it to the free list
        PUT(HDRP(NEXT_BLKP(bp)), PACK(csize-asize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(csize-asize, 0));
        remove_free_node(bp);
        append_free_node(heap_listp, NEXT_BLKP(bp));
    }
    else { 
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        remove_free_node(bp);
    }

    checkheap(__LINE__);
}

/* 
 * find_fit - Find a fit for a block with asize bytes 
 */
static void *find_fit(size_t asize)
{
#ifdef NEXT_FIT 
    /* Next fit search */
    char *oldrover = rover;

    /* Search from the rover to the end of list */
    for ( ; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    /* search from start of list to old rover */
    for (rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    return NULL;  /* no fit found */
#else 
    /* First-fit search */
    void *bp;

    for (bp = GETD(NEXT_FREE(heap_listp));  bp!= NULL; bp = GETD(NEXT_FREE(bp))) {
        if ( asize <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    return NULL; /* No fit */
#endif
}

