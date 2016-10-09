#include <pb/util/pb_heap.h>
#include <stdlib.h>

pb_heap* pb_heap_create(pb_heap_cmp cmp, size_t init_cap) {
    pb_heap* heap = malloc(sizeof(pb_heap));
    if(!heap) {
        return NULL;
    }
    
    heap->cap = init_cap ? init_cap : PB_HEAP_DEFAULT_CAP;
    heap->items = malloc(sizeof(void*) * heap->cap);
    if(!heap->items) {
        free(heap);
        return NULL;
    }
    
    heap->cmp = cmp;
    heap->size = 0;
    
    return heap;
}

void pb_heap_free(pb_heap* heap) {
    free(heap->items);
    free(heap);
}

/**
 * Shifts the element at the given position in the heap down until the heap property is satisfied.
 * @param heap The heap in which to shift elements down.
 * @param hole The position of the element to shift down.
 */
static void percolate_down(pb_heap* heap, int hole) {
    void* tmp = heap->items[hole];
    int child;
    
    for(; (hole * 2) + 1 < heap->size; hole = child) {
        child = (hole * 2) + 1;
        
        /* Check whether the child to the right is smaller and use that child as the parent if so */
        if(child != heap->size - 1 && heap->cmp(heap->items[child + 1], heap->items[child]) < 0) {
            child++;
        }
        
        if(heap->cmp(heap->items[child], tmp) < 0) {
            heap->items[hole] = heap->items[child];
        } else {
            break;
        }
    }
    heap->items[hole] = tmp;
}

int pb_heap_insert(pb_heap* heap, void* item) {
    int hole = heap->size;
    
    if(heap->size == heap->cap) {
        void* items = realloc(heap->items, heap->cap * 2 * sizeof(void*));
        if(!items)
            return 0;
        heap->items = items;
        heap->cap *= 2;
    }
    
    heap->size++;
    for (; hole > 0 && heap->cmp(heap->items[(hole - 1) / 2], item) > 0; hole = (hole - 1) / 2) {
        heap->items[hole] = heap->items[(hole - 1) / 2];
    }
    
    heap->items[hole] = item;
    return 1;
}

/**
 * Retrieves the item with the highest priority in the heap but does not remove it.
 *
 * @param heap The heap from which to retrieve the item.
 * @return The item with the highest priority or NULL if the heap is empty.
 */
void* pb_heap_peek_min(pb_heap* heap) {
    return heap->size != 0 ? heap->items[0] : NULL;
}

/**
 * Retrieves the item with the highest priority from the heap and removes it,
 * causing the next-highest-priority item to take its place.
 *
 * @param heap The heap from which to remove the item.
 * @return The item with the highest priority or NULL if the heap is empty.
 */
void* pb_heap_get_min(pb_heap* heap) {
    void* min_item;
    if(heap->size == 0) {
        return NULL;
    }
    
    min_item = heap->items[0];
    heap->items[0] = heap->items[--heap->size];
    percolate_down(heap, 0);
    
    return min_item;
}