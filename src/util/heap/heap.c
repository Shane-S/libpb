#include <pb/util/heap/heap.h>
#include <pb/util/hashmap/hash_utils.h>
#include <stdlib.h>

pb_heap* pb_heap_create(size_t init_cap) {
    pb_heap* heap = malloc(sizeof(pb_heap));
    if(!heap) {
        return NULL;
    }
    
    /* Allocate the list to hold heap entries */
    if (pb_vector_init(&heap->items, sizeof(pb_heap_entry), init_cap ? init_cap : PB_HEAP_DEFAULT_CAP) == -1) {
        free(heap);
        return NULL;
    }

    /* Allocate the map to hold indices to each entry */
    heap->index_map = pb_hashmap_create(pb_pointer_hash, pb_pointer_eq);
    if (!heap->index_map) {
        pb_vector_free(&heap->items);
        free(heap);
        return NULL;
    }
    
    return heap;
}

void pb_heap_free(pb_heap* heap) {
    pb_vector_free(&heap->items);
    pb_hashmap_free(heap->index_map);
    free(heap);
}

/**
 * Shifts the element at the given position in the heap down until the heap property is satisfied.
 * @param heap The heap in which to shift elements down.
 * @param hole The position of the element to shift down.
 */
static void percolate_down(pb_heap* heap, int hole) {
    pb_heap_entry* entries = (pb_heap_entry*)heap->items.items;
    pb_heap_entry tmp = entries[hole]; 
    int child;
    
    for(; (hole * 2) + 1 < heap->items.size; hole = child) {
        child = (hole * 2) + 1;
        
        /* Check whether the child to the right is smaller and use that child as the parent if so */
        if(child != heap->items.size - 1 && entries[child + 1].priority < entries[child].priority) {
            child++;
        }
        
        if(entries[child].priority < tmp.priority) {
            entries[hole] = entries[child];
            pb_hashmap_put(heap->index_map, entries[hole].data, (void*)hole);
        } else {
            break;
        }
    }
    entries[hole] = tmp;
    pb_hashmap_put(heap->index_map, tmp.data, (void*)hole);
}

void percolate_up(pb_heap* heap, int hole) {
    pb_heap_entry* entries = (pb_heap_entry*)heap->items.items;
    pb_heap_entry tmp = entries[hole];

    for (; hole > 0 && entries[(hole - 1) / 2].priority > tmp.priority; hole = (hole - 1) / 2) {
        entries[hole] = entries[(hole - 1) / 2];
        pb_hashmap_put(heap->index_map, entries[hole].data, (void*)hole);
    }

    entries[hole] = tmp;
    pb_hashmap_put(heap->index_map, tmp.data, hole);
}

int pb_heap_insert(pb_heap* heap, void* item, float priority) {
    int hole = heap->items.size;
    pb_heap_entry* entries = (pb_heap_entry*)heap->items.items;
    pb_heap_entry e = { item, priority };

    /* Push the newest entry to the vector to adjust size/realloc if necessary */
    if (pb_vector_push_back(&heap->items, &e) == -1) {
        return -1;
    }

    if (pb_hashmap_put(heap->index_map, e.data, NULL) == -1) {
        return -1;
    }

    percolate_up(heap, hole);
    return 0;
}

void pb_heap_decrease_key(pb_heap* heap, void* item, float new_priority) {
    void* hole_v;
    int hole;
    pb_hashmap_get(heap->index_map, item, &hole_v);

    hole = (int)hole_v; /* void* may be bigger than int, so we need to store it in hole_v first */
    ((pb_heap_entry*)heap->items.items)[hole].priority = new_priority;
    percolate_up(heap, hole);
}

/**
 * Retrieves the item with the highest priority in the heap but does not remove it.
 *
 * @param heap The heap from which to retrieve the item.
 * @return The item with the highest priority or NULL if the heap is empty.
 */
void* pb_heap_peek_min(pb_heap* heap) {
    return heap->items.size != 0 ? ((pb_heap_entry*)heap->items.items)[0].data : NULL;
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
    pb_heap_entry* entries = (pb_heap_entry*)heap->items.items;
    if(heap->items.size == 0) {
        return NULL;
    }
    
    min_item = entries[0].data;
    
    entries[0] = entries[--heap->items.size];
    percolate_down(heap, 0);
    
    return min_item;
}