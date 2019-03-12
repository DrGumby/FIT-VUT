/**
 * @file proj3.h
 * @author Kamil Vojanec (xvojan00)
 * @date 6.12.2017
 * @brief Kostra hlavickoveho souboru 3. projekt IZP 2017/18 a pro dokumentaci Javadoc.
 */

/** @defgroup Structs Group of structs
    Structs used in program
    @{
*/

/**
 * @brief Object structure. Has ID and x,y coordinates
 *
 * Used by struct cluster_t. Specifies one object.
*/
struct obj_t {
    int id; /**< Identification number of object */
    float x; /**< X coordinate of object */
    float y; /**< Y coordinate of object */
};

/**
 * @brief Cluster structure. Has size, capacity and array of struct obj_t
*/
struct cluster_t {
    int size; /**< Size of cluster */
    int capacity; /**< Maximum capacity of cluster */
    struct obj_t *obj; /**< Array of struct obj_t */
};

/** @} */ //End of group Structs

/** @defgroup Single Single cluster functions
    Functions and variables that operate over a single cluster
    @{
*/

/**
 * @brief Initializes a cluster
 * @param *c Pointer to cluster that should be initialized
 * @param cap Capacity of given cluster
 * @pre c != NULL
 * @pre cap >= 0
 * @post c->size = 0
 * @post c->capacity = cap
 * @post c->obj allocated
*/
void init_cluster(struct cluster_t *c, int cap);

/**
 * @brief Frees array of obj_t and initializes the cluster to empty
 * @param *c Pointer to cluster to be cleared
 * @pre c != NULL
 * @post c->size = 0
 * @post c->capacity = 0
 * @post c->obj = NULL
*/
void clear_cluster(struct cluster_t *c);

/**
 * @brief Value recommended for reallocation
*/
extern const int CLUSTER_CHUNK;

/**
 * @brief Resizes cluster to new capacity
 * @param *c Pointer to cluster that should be resized
 * @param new_cap New Capacity to which the cluster is to be resized
 * @return Pointer to a resized cluster
 * @pre c
 * @pre c->capacity >= 0
 * @pre new_cap >= 0
 * @post c->capacity = new_cap
*/
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap);

/**
 * @brief Appends an element of obj_t to cluster
 * @param *c Pointer to cluster to which an object should be appended
 * @param obj Object which should be appended
 * @post c->size += 1
*/
void append_cluster(struct cluster_t *c, struct obj_t obj);

/**
 * @brief Merges two clusters together
 *
 * Takes objects from *c2 and appends them into *c1. Then sorts the array of objects.
 * Cluster *c2 is not changed.
 * @param *c1 Pointer to cluster to which objects are added
 * @param *c2 Pointer to cluster from which objects are added to c1
 * @pre c1 != NULL
 * @pre c2 != NULL
 * @post Cluster c1 has objects from cluster c2 and c1 is sorted. Cluster c2 is not changed.
*/
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2);

/** @} */ //End of group Single

/** @defgroup Array Array Functions
    Functions that operate over an array of clusters
    @{
*/

/**
 * @brief Removes cluster from array
 * @param *carr Array of struct cluster_t
 * @param narr Number of objects in *carr
 * @param idx Index of element that should be removed
 * @return Number of elements in *carr after an object has been removed
 * @pre idx < narr
 * @pre narr > 0
 * @post narr -1
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx);

/** @} */ //Temporary end of array

/**
    @defgroup Calculation Calculation Functions
    Functions that are used to calculate
    @{
*/

/**
 * @brief Calculates the distance between two objects
 * @param *o1 Pointer to first object
 * @param *o2 Pointer to second object
 * @pre o1 != NULL
 * @pre o2 != NULL
 * @return Distance between objects
*/
float obj_distance(struct obj_t *o1, struct obj_t *o2);

/**
 * @brief Calculates the distance between two clusters
 * @param *c1 Pointer to first cluster
 * @param *c2 Pointer to second cluster
 * @pre c1 != NULL
 * @pre c1->size > 0
 * @pre c2 != NULL
 * @pre c2->size > 0
 * @return Distance between clusters
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2);

/**
 * @brief Finds the two closest clusters
 * @param *carr Array of clusters where neighbours are to be found
 * @param narr Number of elements in *carr
 * @param *c1 Pointer to the index where one of the elements has been found
 * @param *c2 Pointer to the index where the other element has been found
 * @pre narr > 0
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2);
/** @} */ //End of Calculation


/** @addtogroup Single
    @{
*/

/**
 * @brief Sorts all objects in a cluster_t
 * @param *c Pointer to cluster where all objects should be sorted
 * @pre Unsorted array of clusters
 * @post Sorted array of clusters
*/
void sort_cluster(struct cluster_t *c);

/**
 * @brief Prints all objects in cluster
 * @param *c Pointer to cluster whose object should be printed
*/
void print_cluster(struct cluster_t *c);

/** @} */ //End of Single

/** @addtogroup Array
    @{
*/

/**
 * @brief Loads objects from a file into an array of clusters
 * @param *filename Name of a file from which objects should be loaded
 * @param **arr Pointer to array of clusters into which the objects should be loaded
 * @return Number of objects loaded into clusters
 * @pre arr != NULL
 * @post Pointer *arr is initialized to hold clusters. Objects are loaded into clusters and clusters are added to *arr
 *
 * Function loads clusters from a file. Every object is appended to a cluster. Every cluster has one object in it.
*/
int load_clusters(char *filename, struct cluster_t **arr);

/**
 * @brief Prints objects from all clusters in array
 * @param *carr Array that sholud have its clusters printed
 * @param narr Number of elements to be printed
*/
void print_clusters(struct cluster_t *carr, int narr);

/** @} */ //End of Array
