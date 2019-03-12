/*
* Kostra programu pro 3. projekt IZP 2017/18
 *
 * Jednoducha shlukova analyza
 * Unweighted pair-group average
 * https://is.muni.cz/th/172767/fi_b/5739129/web/web/usrov.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);


    c->size = 0;
    c->capacity = cap;
    if(c->capacity == 0)
    {
        c->obj = NULL;
    }
    else
    {
        c->obj = malloc(cap*sizeof(struct obj_t));
        if(c->obj == NULL)
            fprintf(stderr, "Memory allocation failed");
    }



}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c) {

    if(c != NULL)
    {
        c->size = 0;
        c->capacity = 0;

        free(c->obj);
        c->obj = NULL;
    }
    else
    {
        fprintf(stderr, "Trying to clear a NULL cluster. Skipping...\n");
    }
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{

    if(c->size >= c->capacity)
    {
        c = resize_cluster(c, c->capacity+CLUSTER_CHUNK);
    }
    if(c == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
    }
    else
    {
        c->obj[c->size] = obj;
        c->size++;
    }
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);


    for (int i = 0; i < c2->size; ++i) {
        append_cluster(c1, c2->obj[i]);
        if(c1 == NULL)
        {
            break;
        }

    }
    sort_cluster(c1);

}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);




    clear_cluster(&carr[idx]);
    for (int i = idx; i < narr-1; ++i) {
        carr[i] = carr[i+1];

    }

    return narr-1;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);


    float xDist = o1->x - o2->x;
    float yDist = o1->y - o2->y;
    xDist = xDist * xDist;
    yDist = yDist * yDist;
    float euclDist = sqrtf(xDist + yDist);
    return euclDist;

}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);


    float tempSum = 0.f;
    for(int i = 0; i < c1->size; i++)
    {
        for(int j = 0; j < c2->size; j++)
        {
            tempSum += obj_distance(&c1->obj[i], &c2->obj[j]);
        }
    }
    float clusterDistance = tempSum/(c1->size*c2->size);
    return clusterDistance;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);


    float previousClosest = cluster_distance(&carr[0], &carr[1]);
    float currentClosest  = 0.f;
    int idxA = 0;
    int idxB = 1;
    for(int i = 0; i < narr; i++)
    {
        for(int j = i+1; j < narr; j++)
        {
            currentClosest = cluster_distance(&carr[i], &carr[j]);
            if(currentClosest < previousClosest)
            {
                previousClosest = currentClosest;
                idxA = i;
                idxB = j;
            }
        }
    }
    *c1 = idxA;
    *c2 = idxB;

}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

int checkDuplicateID(struct cluster_t *arr, int narr, int id)
{
    for (int i = 0; i < narr; ++i) {
        if(arr[i].obj->id == id)
        {
            fprintf(stderr, "Duplicate ID\n");
            return 0;
        }
    }
    return 1;
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr) {
    assert(arr != NULL);


    FILE *objectList = fopen(filename, "r");

    if (objectList == NULL) {
        fprintf(stderr, "No such file found.\n");
        *arr = NULL;
        return -1;

    }
    int count = 0;
    int objid = -1;
    int x = 0;
    int y = 0;
    int i = 0;
    if (fscanf(objectList, "count=%d", &count) != 1)
    {
        fclose(objectList);
        fprintf(stderr, "Invalid count entered. Exiting...\n");
        *arr =NULL;
        return -1;
    }
    *arr = malloc((count) *sizeof(struct cluster_t));
    if(*arr == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        *arr = NULL;
        return -1;
    }
    for (i = 0; i < count; ++i) {
        if(fscanf(objectList, "%d %d %d", &objid, &x, &y) == 3) {
            if(x < 0 || x > 1000 || y < 0 || y > 1000 || checkDuplicateID(*arr,i, objid) == 0)
            {
                fprintf(stderr, "Invalid object. Exiting...\n");
                for (int j = 0; j < i; ++j) {
                    clear_cluster(&(*arr)[j]);

                }
                free(*arr);
                fclose(objectList);
                *arr = NULL;
                return -1;
            }


            struct obj_t newObject;
            newObject.id = objid;
            newObject.x = x;
            newObject.y = y;//print_clusters(clusters, 5);

            init_cluster(&(*arr)[i], 1);
            append_cluster(&(*arr)[i], newObject);


        }
        else
        {
            fprintf(stderr, "File is invalid. Exiting...\n");
            for (int j = 0; j < i; ++j) {
                clear_cluster(&(*arr)[j]);

            }
            free(*arr);
            fclose(objectList);
            *arr = NULL;
            return -1;
        }

    }
    fclose(objectList);
    return i;


}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

int main(int argc, char *argv[])
{
    struct cluster_t *clusters;



    int targetClusters = 1;
    int clusterCountTotal = 0;
    if(argc == 2 || argc == 3)
    {

        clusterCountTotal = load_clusters(argv[1], &clusters);
        if(clusterCountTotal == -1)
        {
            return 1;
        }
        if(argc == 3)
        {
            char * ptr = '\0';
            targetClusters = (int)strtol(argv[2], &ptr, 0);
            if(*ptr != '\0' || targetClusters < 1)
            {
                fprintf(stderr, "Incorrect entry\n");
                for (int i = 0; i < clusterCountTotal; ++i) {
                    remove_cluster(&clusters[i], 1, 0);
                }
                free(clusters);
                return 1;
            }
        }
    }
    else
    {
        fprintf(stderr, "No parameter specified. Exiting...\n");
        return 1;
    }

    int nearestClusterIdxA = 0;
    int nearestClusterIdxB = 0;



    while(clusterCountTotal >  targetClusters)
    {
        find_neighbours(clusters, clusterCountTotal, &nearestClusterIdxA, &nearestClusterIdxB);
        merge_clusters(&clusters[nearestClusterIdxA], &clusters[nearestClusterIdxB]);
        if(&clusters[nearestClusterIdxA] == NULL)
        {
            for (int i = 0; i < clusterCountTotal; ++i) {
                remove_cluster(&clusters[i], 1, 0);
            }
            free(clusters);
            return 1;
        }

        clusterCountTotal = remove_cluster(clusters, clusterCountTotal, nearestClusterIdxB);


    }
    print_clusters(clusters, clusterCountTotal);
    for (int i = 0; i < clusterCountTotal; ++i) {
        remove_cluster(&clusters[i], 1,0);
    }

    free(clusters);


}
