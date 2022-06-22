/* 
 * I use a free block organization design called Segregated Free List, 
 * which allocates a number of pointers on the heap equal to SEG_LEN, 
 * each pointer corresponds to a size class and points to a free block in a 
 * formal heap block, which is equivalent to a SEG_LEN chain table. Each time
 * malloc is executed, it only needs to look in the corresponding chain table,
 * which can theoretically improve the efficiency of malloc and realloc.
 * However, I can't solve some bugs in realloc, so realloc still takes the way in the book,
 * which leads to low space utilization.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "mm.h"
#include "memlib.h"

// #define REALLOC
// #define HEAP
// #define DEBUG
/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE 4 /* Word and header/footer size (bytes) */        // line:vm:mm:beginconst
#define DSIZE 8                                                  /* Double word size (bytes) */
#define CHUNKSIZE (512) /* Extend heap by this amount (bytes) */ // line:vm:mm:endconst
#define LIST_NUM 20
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ALIGN(size) (((size) + (DSIZE - 1)) & ~0x7)

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc)) // line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define SUCC_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE))) // line:vm:mm:nextblkp
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))   // line:vm:mm:prevblkp
#define PREV_LINK(bp) ((char *)(bp) + WSIZE)
#define SUCC_LINK(bp) ((char *)bp)
#define SUCC_LINK_BLKP(bp) (GET(SUCC_LINK(bp)))
#define PREV_LINK_BLKP(bp) (GET(PREV_LINK(bp)))
#define GET_PRE(bp) ((unsigned int *)(long)(GET(bp)))
#define GET_SUC(bp) ((unsigned int *)(long)(GET((unsigned int *)bp + 1)))


static char *heap_loc = NULL;
static char *heap_listp;   /* Pointer to first block */
static char *free_listp;   /* Pointer to first free list */
static void *free_listp0;  /* size < 2^4 */
static void *free_listp1;  /* 2^4 <= size < 2^5 */
static void *free_listp2;  /* 2^5 <= size < 2^6 */
static void *free_listp3;  /* 2^6 <= size < 2^7 */
static void *free_listp4;  /* 2^7 <= size < 2^8 */
static void *free_listp5;  /* 2^8 <= size < 2^9 */
static void *free_listp6;  /* 2^9 <= size < 2^10 */
static void *free_listp7;  /* 2^10 <= size < 2^11 */
static void *free_listp8;  /* 2^11 <= size < 2^12 */
static void *free_listp9;  /* 2^12 <= size < 2^13 */
static void *free_listp10; /* 2^13 <= size < 2^14 */
static void *free_listp11; /* 2^14 <= size < 2^15 */
static void *free_listp12; /* 2^15 <= size < 2^16 */
static void *free_listp13; /* 2^16 <= size < 2^17 */
static void *free_listp14; /* 2^17 <= size < 2^18  */
static void *free_listp15; /* 2^18 <= size */
static void *free_listp16; 
static void *free_listp17; 
static void *free_listp18; 
static void *free_listp19; 
static void *free_listp20; 

#ifdef NEXT_FI6
static char *rover; /* Next fit rover */
#endif
/* Fun10tion prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkheap(int verbose);
static void checkblock(void *bp);
void *findList(size_t size);

/*
 * mm_init - Initialize the memory manager
 */

int mm_init(void){
    heap_loc = (char *)mem_heap_lo();
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) // line:vm:mm:begininit
        return -1;

    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += 2 * WSIZE;

    init_free_list();

#ifdef NEXT_FIT
    rover = heap_listp;
#endif
    /* $begin mminit */

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}
/* $end mminit */

/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size){
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;
    
#ifdef REALLOC
    printf("before mallocing, size is %d\n", size);
    mm_checkheap(1);
    // checkAllBlocks();
#endif

    if (heap_listp == 0){
        mm_init();
    }
    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = ALIGN(size) + DSIZE;

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
#ifdef REALLOC
        printf("after mallocing, size is %d\n", size);
        mm_checkheap(1);
#endif
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);

    return bp;
}
/* $end mmmalloc */

/*
 * find_fit - Find a fit for a block with asize bytes
 */
/* $begin mmfirstfit */
/* $begin mmfirstfit-proto */
static void *find_fit(size_t asize)
/* $end mmfirstfit-proto */{
    /* First-fit search */
    if (asize <= 0)
        return NULL;
    void **findP = findList(asize);
    size_t judgeSize = asize;
    while (findP)
    {
        void *bp = *findP;
        while (bp && bp > heap_listp)
        {
            if (asize <= GET_SIZE(HDRP(bp)))
            {
                return (void *)bp;
            }
            bp = GET_SUC(bp);
        }
        // TODO: 添加边界判断
        if (judgeSize <= (1 << 24))
        {
            findP = findList(judgeSize * 2);
            judgeSize *= 2;
        }
        else
        {
            return NULL;
        }
    }
    return NULL; 
}


/*
 *  insert - insert new block to list header
 */
//  Gethead   ((unsigned int *)(long)(GET(heap_list + WSIZE * num)))
void insert(void *bp){

    size_t size = GET_SIZE(HDRP(bp));
    void **findP = findList(size);
    // free, insert directly
    if (!(*findP)){
#ifdef DEBUG
        printf("prev link is %p\n", PREV_LINK(bp));
        printf("succ link is %p \n", SUCC_LINK(bp));
#endif
        PUT(PREV_LINK(bp), 0);
        PUT((SUCC_LINK(bp)), 0);
    }
    else{
        PUT(SUCC_LINK(bp), *findP);
        PUT(PREV_LINK(*findP), bp);
        PUT(PREV_LINK(bp), 0);
    }
    *findP = bp;
}

void delete (void *bp){

    size_t size = GET_SIZE(HDRP(bp));
    void **findP = findList(size);
    if (SUCC_LINK_BLKP(bp) && PREV_LINK_BLKP(bp) && (PREV_LINK_BLKP(bp) > heap_listp && SUCC_LINK_BLKP(bp) > heap_listp)){
        PUT(SUCC_LINK(PREV_LINK_BLKP(bp)), SUCC_LINK_BLKP(bp));
        PUT(PREV_LINK(SUCC_LINK_BLKP(bp)), PREV_LINK_BLKP(bp));
    }
    else if (PREV_LINK_BLKP(bp) && PREV_LINK_BLKP(bp) > heap_listp){
        PUT(SUCC_LINK(PREV_LINK_BLKP(bp)), NULL);
    }
    else if (SUCC_LINK_BLKP(bp) && SUCC_LINK_BLKP(bp) > heap_listp){
        PUT(PREV_LINK(SUCC_LINK_BLKP(bp)), NULL);
        if (bp == *findP){
            *findP == SUCC_LINK_BLKP(bp);
        }
    }
    else{
        if (bp == *findP){
            *findP = NULL;
        }
    }
}

/*
 * mm_free - Free a block
 */
/* $begin mmfree */
void mm_free(void *bp)
{
    /* $end mmfree */
    if (bp == 0)
        return;

    /* $begin mmfree */
    size_t size = GET_SIZE(HDRP(bp));
    /* $end mmfree */
    if (heap_listp == 0){
        mm_init();
    }
    /* $begin mmfree */
#ifdef REALLOC
    printf("before free, size is %d\n", size);
    mm_checkheap(1);

    // checkAllBlocks();
#endif
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
#ifdef REALLOC
    printf("after free, size is %d\n", size);
    mm_checkheap(1);
    // checkAllBlocks();
#endif
}

/* $end mmfree */
/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
/* $begin mmfree */
static void *coalesce(void *bp)
{
    void *prev = PREV_BLKP(bp);
    void *next = SUCC_BLKP(bp);
    size_t getSize = GET_SIZE(((char *)(bp)-DSIZE));
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(SUCC_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));


    if (prev_alloc && next_alloc){ /* Case 0 */
        insert(bp);
        return bp;
    }

    else if (prev_alloc && !next_alloc){ /* Case 1 */
        size += GET_SIZE(HDRP(SUCC_BLKP(bp)));
        delete (SUCC_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(PREV_LINK(bp), NULL);
        PUT(SUCC_LINK(bp), NULL);
        insert(bp);
        return bp;
    }

    else if (!prev_alloc && next_alloc){ /* Case 2 */

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete (PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        PUT(PREV_LINK(bp), NULL);
        PUT(SUCC_LINK(bp), NULL);
        insert(bp);
        return bp;
    }

    else{ /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(SUCC_BLKP(bp)));
        delete (PREV_BLKP(bp));
        delete (SUCC_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(SUCC_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        PUT(PREV_LINK(bp), NULL);
        PUT(SUCC_LINK(bp), NULL);
        insert(bp);
        return bp;
    }

}
/* $end mmfree */



void *mm_realloc(void *ptr, size_t size){
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

/*
 * mm_realloc - Naive implementation of realloc
 */


// void *mm_realloc(void *ptr, size_t size)
// {

//     printf("before realloc\n");
//     printf("ptr is %p, size is %d", ptr, size);
//     mm_checkheap(1);
//     // 如果 ptr == NULL 直接分配
//     if(ptr == NULL)    
//         return mm_malloc(size);
//     // 如果 size == 0 就释放
//     else if(size == 0){
//         mm_free(ptr);
//         return NULL;
//     }
//     size_t asize = ALIGN(size), old_size = GET_SIZE(HDRP(ptr));
//     size_t mv_size = MIN(asize, old_size);
//     char *oldptr = ptr;
//     char *newptr;


//     if(old_size == asize)
//         return ptr;

//     size_t prev_alloc =  GET_ALLOC(FTRP(PREV_BLKP(ptr)));
//     size_t next_alloc =  GET_ALLOC(HDRP(SUCC_BLKP(ptr)));
//     // printf("succ_blkp: %p\n", SUCC_BLKP(ptr));
//     size_t next_size = GET_SIZE(HDRP(SUCC_BLKP(ptr)));
//     // printf("nextsize is %d\n", next_size);
//     next_size = GET_ALLOC(HDRP(SUCC_BLKP(ptr))) ? 0 : next_size;
//     char *next_bp = SUCC_BLKP(ptr);
//     size_t total_size = old_size;

//     if(asize < old_size){
//         // printf("case 1\n");
//         PUT(HDRP(ptr), PACK(asize, 1));
//         PUT(FTRP(ptr), PACK(asize, 1));
//         PUT(HDRP(SUCC_BLKP(ptr)), PACK(old_size - asize, 0));
//         PUT(FTRP(SUCC_BLKP(ptr)), PACK(old_size - asize, 0));
//         insert(SUCC_BLKP(ptr));
//         coalesce(SUCC_BLKP(ptr));
//             printf("after realloc\n");
//     mm_checkheap(1);
//         return ptr;
//     }
//         else{
//             // printf("next size is %d\n", next_size);
//         int enough_size = next_size >= (asize - old_size + DSIZE * 2); /* remain size is enough or not */
//         int can_expand = (next_size && !GET_SIZE(HDRP(SUCC_BLKP(SUCC_BLKP(ptr))))) || !GET_SIZE(HDRP(SUCC_BLKP(ptr))); /* the block can be expanded or not */
//         // printf("enough_size = %d cam_expand = %d\n", enough_size, can_expand);
//         /* subCase: remain size is not enough but can be simply expanded as it is at the end of heap */
//         if(!enough_size && can_expand){
//                     // printf("case 2\n");

//             extend_heap(MAX(asize - old_size, CHUNKSIZE)/WSIZE);
//             next_size = GET_SIZE(HDRP(SUCC_BLKP(ptr)));
//         }
//         /* subCase: remain size is enough */
//         if(enough_size || can_expand){
//                     // printf("case 3\n");

//             delete(SUCC_BLKP(ptr));
//             // mm_checkheap(1);
//             PUT(HDRP(ptr), PACK(asize, 1));
//             PUT(FTRP(ptr), PACK(asize, 1));
//             if(next_size != asize - old_size){
//                 printf("FTRP(SUCC_BLKP(ptr)) = %p \n", FTRP(SUCC_BLKP(ptr)));
//                 PUT(HDRP(SUCC_BLKP(ptr)), PACK(old_size + next_size - asize, 0));
//                 PUT(FTRP(SUCC_BLKP(ptr)), PACK(old_size + next_size - asize, 0));
//                 insert(SUCC_BLKP(ptr));
//                 // latest_freeblkp = SUCC_BLKP(ptr);
//             }
//                 printf("after realloc\n");
//                 mm_checkheap(1);
//                 return ptr;
//         }
//         /* subCase: have to malloc new one */
//         else{
//                     // printf("case 4\n");

//             void *newptr = mm_malloc(size);
//             if (newptr == NULL)
//                 return NULL;
//             memcpy(newptr, ptr, old_size - DSIZE);
//             mm_free(ptr);
//             printf("after realloc\n");
//             mm_checkheap(1);
//             return newptr;
//         }    
//     }

//     if(prev_alloc && !next_alloc && (old_size + next_size >= asize)){    // 后空闲  
//         // printf("case 1  next size is %d\n", next_size);
//         total_size += next_size;
//         delete(next_bp);
//         PUT(HDRP(ptr), PACK(total_size, 1));
//         PUT(FTRP(ptr), PACK(total_size, 1));
//         place(ptr, total_size);
//     }
//     else if(!next_size && asize >= old_size){   // 如果后部是结尾块，则直接 extend_heap
//             // printf("case 2  next size is %d\n", next_size);
        
//         size_t extend_size = asize - old_size;
//         if((long)(mem_sbrk(extend_size)) == -1)
//             return NULL; 
        
//         PUT(HDRP(ptr), PACK(total_size + extend_size, 1));
//         PUT(FTRP(ptr), PACK(total_size + extend_size, 1));
//         PUT(HDRP(SUCC_BLKP(ptr)), PACK(0, 1)); 
//         place(ptr, asize);
//     }
//     else{   // 直接分配
//         //  printf("case 3  next size is %d\n", next_size);
//         newptr = mm_malloc(asize);
//         if(newptr == NULL)
//             return NULL;
//         memcpy(newptr, ptr, MIN(old_size, size));
//         mm_free(ptr);
//         // printf("after realloc\n");
//         mm_checkheap(1);
//         return newptr;
//     }
//     // printf("after realloc\n");
//     mm_checkheap(1);
//     return ptr;
// }


/*
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose){
    checkheap(verbose);
}


/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL; // line:vm:mm:endextend

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); /* Free block header */
    PUT(FTRP(bp), PACK(size, 0)); /* Free block footer */
    // insert(bp);
    PUT(HDRP(SUCC_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */

static void place(void *bp, size_t asize){

    size_t csize = GET_SIZE(HDRP(bp));

#ifdef DEBUG
    printf("placing : cisze is %d, asize is %d \n", csize, asize);
#endif
    if (!GET_ALLOC(HDRP(bp)))
        delete (bp);
    if ((csize - asize) >= (2 * DSIZE)){
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = SUCC_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        coalesce(bp);
    }
    else{
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
/* $end mmplace */

static void printblock(void *bp)
{
    size_t hsize, halloc, fsize, falloc;

    checkheap(0);
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0){
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp,
           hsize, (halloc ? 'a' : 'f'),
           fsize, (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp){
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp))){
        printf("Error: header does not match footer\n");
        printf("header: %d footer %d\n", GET(HDRP(bp)),GET(FTRP(bp)));
    }
       
}

/*
 * checkheap - Minimal check of the heap for consistency
 */
void checkheap(int verbose){
    char *bp = heap_listp;
    // printf("listp: %p", bp);
    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    checkblock(heap_listp);

    // printf("HDRP is %p, GETSIZE is = %d \n", HDRP(bp), GET_SIZE(HDRP(bp)));
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = SUCC_BLKP(bp)){
        if (verbose)
            printblock(bp);
        checkblock(bp);
    }

    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}

void checkAllBlocks()
{
    char *header = heap_listp;
    while (GET_SIZE(HDRP(header)) > 0){
        printf("%d/%d -> ", GET_SIZE(HDRP(header)), GET_ALLOC(HDRP(header)));
        header = SUCC_BLKP(header);
    }
    printf(" end \n");
}

void checkAllList(){
    size_t size = 16;
    void **findP;
    int index = 0;
    while (size < (1 << 20)){
        printf("list %d: ", index);
        findP = findList(size);
        while ((*findP)){
            printf("current addr %p ", *findP);
            if (index != 15)
                *findP = SUCC_LINK_BLKP(*findP);
        }
        size *= 2;
        index++;
    }
}

/**
 * findList - a quick solution to get list addr by index
 */
void *findList(size_t size){
    size_t r, shift;
    r = (size > 0xFFFF) << 4;
    size >>= r;
    shift = (size > 0xFF) << 3;
    size >>= shift;
    r |= shift;
    shift = (size > 0xF) << 2;
    size >>= shift;
    r |= shift;
    shift = (size > 0x3) << 1;
    size >>= shift;
    r |= shift;
    r |= (size >> 1);
    // from 2^4  (minest free block: 16 bytes)
    int x = (int)r - 4;
    if (x < 0)
        x = -1;
    if (x >= LIST_NUM)
        x = LIST_NUM ;
    // int x = (int)log2(size) - 3;
    x = x + 1;
    if(x < 0) x = 0;
    switch (x)
    {
    case 0:
        return &free_listp0;
    case 1:
        return &free_listp1;
    case 2:
        return &free_listp2;
    case 3:
        return &free_listp3;
    case 4:
        return &free_listp4;
    case 5:
        return &free_listp5;
    case 6:
        return &free_listp6;
    case 7:
        return &free_listp7;
    case 8:
        return &free_listp8;
    case 9:
        return &free_listp9;
    case 10:
        return &free_listp10;
    case 11:
        return &free_listp11;
    case 12:
        return &free_listp12;
    case 13:
        return &free_listp13;
    case 14:
        return &free_listp14;
    case 15:
        return &free_listp15;
    case 16:
        return &free_listp16;
    case 17:
        return &free_listp17;
    case 18:
        return &free_listp18;
    case 19:
        return &free_listp19;
    default:
        return &free_listp20;
    }
}

/**
 *  init_free_list - allocate all free_list as NULL
 */
void init_free_list()
{

    free_listp0 = NULL;
    free_listp1 = NULL;
    free_listp2 = NULL;
    free_listp3 = NULL;
    free_listp4 = NULL;
    free_listp5 = NULL;
    free_listp6 = NULL;
    free_listp7 = NULL;
    free_listp8 = NULL;
    free_listp9 = NULL;
    free_listp10 = NULL;
    free_listp11 = NULL;
    free_listp12 = NULL;
    free_listp13 = NULL;
    free_listp14 = NULL;
    free_listp15 = NULL;
    free_listp16 = NULL;
    free_listp17 = NULL;
    free_listp18 = NULL;
    free_listp19 = NULL;
    free_listp20 = NULL;
}
