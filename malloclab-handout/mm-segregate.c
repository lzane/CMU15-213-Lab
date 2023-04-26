/* 
 * 32-bit and 64-bit clean allocator based on segregated free
 * lists, first-fit placement, and boundary tag coalescing. 
 * Blocks must be aligned to double word (8 byte) 
 * boundaries. Minimum block size is 24 bytes. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
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

/**
 * Checkheap Flag
*/
// #define checkheap(x) mm_checkheap(x);
#define checkheap(x)

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */ 
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<8)  /* Extend heap by this amount (bytes) */  
#define FREELISTCOUNT 12    /* How many free list */

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, prev_alloc, alloc)  ((size) | (prev_alloc<<1) | (alloc)) 

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                   
#define GET_ALLOC(p) (GET(p) & 0x1)                    
#define GET_PREV_ALLOC(p) ((GET(p)>>1) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                      
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) 

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) 
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) 

/* Free list node */
#define PREV_FREE(bp)       ((char *)(bp)) 
#define NEXT_FREE(bp)       ((char *)(bp) + WSIZE)
#define NULL_PTR            heap_start
#define GET_PTR(p)             ((char *)((unsigned long)(*(unsigned int *)(p))+(unsigned long)NULL_PTR))
#define PUT_PTR(p, val)        (*(unsigned int *)(p) = ((unsigned int)((char *)(val) - NULL_PTR))) 
#define ROOT_BY_BLOCK(block_num) ((char *)heap_listp + (block_num)*2*WSIZE)

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */
static char *heap_free_list_end = 0; /* Pointer to free list end */
static char *heap_start = 0; /* Pointer to heap start */

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void remove_free_node(void *a);
static void add_free_node(void *a);
static void *root_by_asize(size_t asize);

/* 
 * mm_init - Initialize the memory manager 
 */
int mm_init(void) 
{
    int reserve_size = FREELISTCOUNT*2*WSIZE;
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(DSIZE+reserve_size+DSIZE)) == (void *)-1) 
        return -1;
    heap_start = heap_listp;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + WSIZE, PACK(DSIZE+reserve_size, 0, 1)); /* Prologue header */
    heap_listp += (2*WSIZE);
    for (int i = 0; i < FREELISTCOUNT; i++)
    {
        PUT_PTR(ROOT_BY_BLOCK(i), NULL_PTR);             /* free list root node prev*/
        PUT_PTR(ROOT_BY_BLOCK(i) + WSIZE, NULL_PTR);             /* free list root node next*/
    }
    heap_free_list_end = ROOT_BY_BLOCK(FREELISTCOUNT);
    PUT(heap_free_list_end,PACK(DSIZE+reserve_size, 0, 1)); /* Prologue footer */ 
    PUT(heap_free_list_end+WSIZE, PACK(0, 1, 1));     /* Epilogue header */
    checkheap(__LINE__);

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
    if (size <= DSIZE+WSIZE)                                          
        asize = 2*DSIZE;                                        
    else
        asize = DSIZE * ((size + WSIZE + (DSIZE-1)) / DSIZE);

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

    unsigned int is_prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    PUT(HDRP(bp), PACK(size, is_prev_alloc, 0));
    PUT(FTRP(bp), PACK(size, is_prev_alloc, 0));
    
    // reset the next block prev alloc flag
    char *next_blk_header = HDRP(NEXT_BLKP(bp));
    PUT(next_blk_header, PACK(GET_SIZE(next_blk_header), 0, GET_ALLOC(next_blk_header))); 

    bp = coalesce(bp);
    add_free_node(bp);

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
    
    checkheap(__LINE__);
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

    checkheap(__LINE__);
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
                printf("[%d] heapsize not equal, my: %lu, mem_heapsize: %lu\n", lineno, (size_t)(bp-heap_listp)+DSIZE, mem_heapsize());
                abort();
            }
            break;
        }

        // free block AND header and footer not equal
        if(!GET_ALLOC(HDRP(bp)) && GET(HDRP(bp))!=GET(FTRP(bp))){
            printf("[%d] header and footer not equal\n", lineno);
            abort();
        }

        // block alloc AND next block alloc tag mismatch
        if(GET_ALLOC(HDRP(bp)) != GET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)))){
            printf("[%d] block's alloc and next block's prev alloc mismatch\n", lineno);
            abort(); 
        }

        // base pointer not align to 8
        if((uintptr_t)bp % 8 != 0){
            printf("[%d] bp not align to 8\n", lineno);
            abort();
        }

        // no consecutive free block
        if(!GET_ALLOC(HDRP(bp))&&!GET_ALLOC(HDRP(NEXT_BLKP(bp)))){
            printf("[%d] consecutive free block\n", lineno); 
            abort();
        }

        if(!GET_ALLOC(HDRP(bp))){
            freeBlockCountByHeap++;
        }

        // check next bp
        bp += GET_SIZE(HDRP(bp));
    }

    // check the free list
    for (int i = 0; i < FREELISTCOUNT; i++)
    {
        bp = ROOT_BY_BLOCK(i);
        while (1)
        {
            // reach end
            if (GET_PTR(NEXT_FREE(bp)) == NULL_PTR)
            {
                break;
            }

            void *next = GET_PTR(NEXT_FREE(bp));
            void *prev = GET_PTR(PREV_FREE(bp));

            // double link list
            if (GET_PTR(PREV_FREE(next)) != bp)
            {
                printf("[%d] a's next and b's prev not equal\n", lineno);
                abort();
            }

            // within bound
            if (next != NULL_PTR && (next < mem_heap_lo() || next > mem_heap_hi()))
            {
                printf("[%d] a's next exceed boundary\n", lineno);
                abort();
            }
            if (prev != NULL_PTR && (prev < mem_heap_lo() || prev > mem_heap_hi()))
            {
                printf("[%d] a's prev exceed boundary\n", lineno);
                abort();
            }

            // block size fall within the range
            int size = GET_SIZE(HDRP(bp));
            if(bp!=ROOT_BY_BLOCK(i) && root_by_asize(size)!=ROOT_BY_BLOCK(i)){
                printf("[%d] block size %d not within link list %d\n", lineno, size, i);
                abort();
            }

            freeBlockCountByList++;
            bp = next;
        }
    }

    if(freeBlockCountByHeap != freeBlockCountByList){
        printf("[%d] freeBlockCountByHeap(%d) != freeBlockCountByList(%d)\n", lineno, freeBlockCountByHeap, freeBlockCountByList);
        abort(); 
    }
}

/* 
 * The remaining routines are internal helper routines 
 */

/**
 * add node a to free list
*/
static void add_free_node(void *a){
    void *root = root_by_asize(GET_SIZE(HDRP(a)));
    void *next = GET_PTR(NEXT_FREE(root));

    PUT_PTR(NEXT_FREE(root), a);
    PUT_PTR(PREV_FREE(a), root);

    // reset b's next
    PUT_PTR(NEXT_FREE(a), NULL_PTR);

    if(next!=NULL_PTR){
        PUT_PTR(NEXT_FREE(a), next);
        PUT_PTR(PREV_FREE(next), a);
    }    
}

/**
 * remove node a from free list
*/
static void remove_free_node(void *a){
    void *prevNode = GET_PTR(PREV_FREE(a));
    void *nextNode = GET_PTR(NEXT_FREE(a));

    if(prevNode!=NULL_PTR){
        PUT_PTR(NEXT_FREE(prevNode), nextNode);
    }

    if(nextNode!=NULL_PTR){
        PUT_PTR(PREV_FREE(nextNode), prevNode);
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
    unsigned int is_prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    PUT(HDRP(bp), PACK(size, is_prev_alloc, 0));         /* Free block header */   
    PUT(FTRP(bp), PACK(size, is_prev_alloc, 0));         /* Free block footer */   
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 0, 1)); /* New epilogue header */ 
    
    /* Coalesce if the previous block was free */
    bp = coalesce(bp);

    add_free_node(bp);
    return bp;             
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) 
{
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        remove_free_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, prev_alloc, 0));
        PUT(FTRP(bp), PACK(size, prev_alloc, 0));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        remove_free_node(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        unsigned int prev_blk_prev_alloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, prev_blk_prev_alloc, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, prev_blk_prev_alloc, 0));
        bp = PREV_BLKP(bp);
    }

    else {                                     /* Case 4 */
        remove_free_node(NEXT_BLKP(bp));
        remove_free_node(PREV_BLKP(bp)); 
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        unsigned int prev_blk_prev_alloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, prev_blk_prev_alloc, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, prev_blk_prev_alloc, 0));
        bp = PREV_BLKP(bp);
    }

    return bp;
}

/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize)
{   
    size_t csize = GET_SIZE(HDRP(bp));    
    if ((csize - asize) >= (2*DSIZE)) { 
        PUT(HDRP(bp), PACK(asize, 1, 1));
        PUT(FTRP(bp), PACK(asize, 1, 1));
        // use the remain room to create a new block
        // then add it to the free list
        PUT(HDRP(NEXT_BLKP(bp)), PACK(csize-asize, 1, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(csize-asize, 1, 0));

        remove_free_node(bp);
        add_free_node(NEXT_BLKP(bp));
    }
    else { 
        PUT(HDRP(bp), PACK(csize, 1, 1));
        PUT(FTRP(bp), PACK(csize, 1, 1));
        // mark next block (allocated) prev alloc flag to tree
        char *next_blk_header = HDRP(NEXT_BLKP(bp));
        PUT(next_blk_header, PACK(GET_SIZE(next_blk_header), 1, GET_ALLOC(next_blk_header)));
        
        remove_free_node(bp);
    } 

    checkheap(__LINE__);
}

/* 
 * find_fit - Find a fit for a block with asize bytes 
 */
static void *find_fit(size_t asize)
{
    /* First-fit search */
    void *bp;
    char *root;

    /* first search the target free list */
    root = root_by_asize(asize);
    for (bp = GET_PTR(NEXT_FREE(root)); bp != NULL_PTR; bp = GET_PTR(NEXT_FREE(bp)))
    {
        if (asize <= GET_SIZE(HDRP(bp)))
        {
            return bp;
        }
    }

    /* search the larger free list */
    for (root += 2 * WSIZE; root != heap_free_list_end; root += 2 * WSIZE)
    {
        bp = GET_PTR(NEXT_FREE(root));
        if (bp != NULL_PTR)
        {
            return bp;
        }
    }

    return NULL; /* No fit */
}


/**
 * root return the root of the target link list
 * where the asize block should be
 * naive
*/
static void *root_by_asize(size_t asize){
    size_t block_size = asize/WSIZE;
    int block_num = log2(block_size)-2;
    if (block_num > FREELISTCOUNT-1) block_num = FREELISTCOUNT-1;
    
    return ROOT_BY_BLOCK(block_num); 
}