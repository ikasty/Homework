/*********************
 * Algorithm Alalysis
 * HW1
 * Kang, Daeyoun
 *********************/

////////////////////////////
// define variables, etc.

// max input size
#define MAXSIZE 10
// tile types
#define NUMBER 0
#define COVERED -1
#define ISMINE -2
#define ISNOTMINE -3

// lists
LIST_HEAD(search_lists);

// point struct
typedef struct {
	int x, y;
	int value;
	
	// cluster head (master)
	list_head clusters;
	// cluster index (slave)
	list_t cluster_index;

	// check count
	int count;
} PNT;

// cluster struct
typedef struct {
	// search index
	list_t search;

	// index for main point (slave)
	list_t index;
	// parent point
	PNT *parent;

	// head of points in this cluster (master)
	list_head points;

	// value of this cluster
	int value;

	// valid count
	int count;

	// index for cluster_lists
	list_t container;
} CLUSTER;

// define direction
static PNT direction[8] = 
	{{-1, 0}, {-1,- 1}, {0,- 1}, {1,- 1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}};

enum list{
	TP, TL, LF, LD, DW, DR, RG, RT
	//top, topleft, left, leftdown, down, downright, right, righttop
	, DIRLENGTH
};

////////////////////////////
// useful macros

// direction
#define DIR_X(dir) direction[dir].x
#define DIR_Y(dir) direction[dir].y

// set point value
#define SET_PNT(set, orig) {(set)->x = (orig)->x; (set)->y = (orig)->y;}
#define SET_PNT_DIR(set, orig, dir) {							\
	(set)->x = (orig)->x + DIR_X(dir);							\
	(set)->y = (orig)->y + DIR_Y(dir);							\
}
// point valid check
#define VALIDBOUND(p) (((p).x >= 0 && (p).x < sizeX) && ((p).y >= 0 && (p).y < sizeY))
// point equivalence
#define SAMEPNT(p, q) ((p).x == (q).x && (p).y == (q).y)

////////////////////////////
// normal variables
int maxcase, curr_case;
int sizeX, sizeY;

// input container
static PNT* minesweep[MAXSIZE][MAXSIZE];
//static int lookup[MAXSIZE][MAXSIZE];


// file pointers
static FILE *ifp, *ofp;

// functions
int check_valid(CLUSTER *cluster, PNT *pnt, int count);
int check_around_point(PNT *curr_pnt);
void set_mine_point(PNT *pnt, int value);
void delete_mine_point(PNT *pnt, int value);
inline void split_near_cluster(CLUSTER *this_cluster);
int is_cluster_subset(CLUSTER *small, CLUSTER *big);
int is_cluster_contains_pnt(CLUSTER *cluster, PNT *pnt);
void add_search_list(CLUSTER *cluster);
void delete_cluster(CLUSTER *cluster);

// only for debug print
#ifdef IMAGE_PRINT
static int backup[MAXSIZE][MAXSIZE];
#endif