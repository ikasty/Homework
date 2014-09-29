/*********************
 * Algorithm Alalysis
 * HW1
 * Kang, Daeyoun
 *********************/

// filename
#define INPFILENAME "input.txt"
#define OTPFILENAME "output.txt"

// debug mode
// decomment to turn on debug mode
//#define DEBUG 4
#define IMAGE_PRINT

////////////////////////////
// include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minesweep.lib.h"
#include "minesweep.h"


////////////////////////////
// value of this point
#define MINE(pnt) minesweep[(pnt).x][(pnt).y]

////////////////////////////
int main () {
	int i, j, k;

	PDEBUG(1, "DEBUG mode enabled, level %d\n", DEBUG);

	fileopen(ifp, INPFILENAME, "r");
	fileopen(ofp, OTPFILENAME, "w");
	if (!ifp || !ofp) {
		PDEBUG(0, "ERROR OPEN FILE! FORCE TERMINATED\n");
		return -1;
	}

	fscanf(ifp, "%d\n", &maxcase);
	while (maxcase > (curr_case++)) {
	// --- solve each case start ---
		PDEBUG(1, "###  Current case: #%d  ###\n", curr_case);

		// -- read --
		fscanf(ifp, "%d %d\n", &sizeX, &sizeY);
		PDEBUG(3, "map size: %dX%d\n", sizeX, sizeY);

		for (i = 0; i < sizeX; i++) {
			for (j = 0; j < sizeY; j++) {
				PNT *pnt;
				pnt = minesweep[i][j] = (PNT *)malloc(sizeof(PNT));

				memset(pnt, 0, sizeof(PNT));
				pnt->x = i; pnt->y = j;

				fscanf(ifp, "%d", &pnt->value);

				#ifdef IMAGE_PRINT
				backup[i][j] = pnt->value;
				#endif
			}
		}
		// -- end read --

		// -- preprocessing --
		PDEBUG(1, "start preprocessing...\n");
		for (i = 0; i < sizeX; i++) {
			for (j = 0; j < sizeY; j++) {
				PNT *pnt, *chkpnt = 0;
				CLUSTER *cluster;

				pnt = minesweep[i][j];
				PDEBUG(4, "add cluster in %d, %d, value %d\n", pnt->x, pnt->y, pnt->value);

				// cluster list init
				list_init(&pnt->clusters);

				// add new cluster
				cluster = (CLUSTER *)malloc(sizeof(CLUSTER));
				memset(cluster, 0, sizeof(CLUSTER));

				cluster->parent = pnt;
				list_add(&cluster->index, &pnt->clusters);
				cluster->value = pnt->value;				

				// find points arount here
				list_init(&cluster->points);
				for (k = 0; k < DIRLENGTH; k++) {
					if (!chkpnt) {
						chkpnt = (PNT *)malloc(sizeof(PNT));
						memset(chkpnt, 0, sizeof(PNT));
					}
					SET_PNT_DIR(chkpnt, pnt, k);
					if (!VALIDBOUND(*chkpnt)) continue;

					// check if these tiles are different type
					if (MINE(*chkpnt)->value >= NUMBER  && pnt->value >= NUMBER ||
						MINE(*chkpnt)->value <= COVERED && pnt->value <= COVERED)
						continue;

					// add this tile to cluster
					list_add_tail(&chkpnt->cluster_index, &cluster->points);
					PDEBUG(4, "add point %d, %d to cluster\n", chkpnt->x, chkpnt->y);
					chkpnt = 0;
				}
				if (chkpnt) free(chkpnt);

				// add find list
				if (pnt->value >= NUMBER) add_search_list(cluster);
			}
		}
		PDEBUG(1, "finish preprocessing\n");
		// -- end preprocessing --

		// -- solve --
		PDEBUG(1, "search queue start\n");
		while (!list_isempty(&search_lists)) {
			CLUSTER *current = 0;
			PNT *checker;
			list_t *list_checker;

			PAUSE;
			current = list_first_or_null(&search_lists, CLUSTER, search);
			PDEBUG(2, "current cluster: (%d %d), value %d\n", current->parent->x, current->parent->y, current->value);

			// now, count every valid try in clusters

			// reset count
			list_foreach(checker, &current->points, PNT, cluster_index)
				checker->count = 0;

			// count this cluster
			PDEBUG(3, "new) cluster valid check\n");
			current->count = check_valid(current, list_first_or_null(&current->points, PNT, cluster_index), 0);
			PDEBUG(3, "cluster valid counts = %d\n", current->count);

			// and check valid things
			list_for(list_checker, &current->points) {
				int get_new_item = 0;

				checker = list_entry(list_checker, PNT, cluster_index);
				PDEBUG(3, "point (%d, %d) valid is %d\n", checker->x, checker->y, checker->count);

				// if is mine
				if (checker->count == current->count && current->count) {
					PDEBUG(2, " @find mine at (%d, %d)\n", checker->x, checker->y);
					get_new_item = ISMINE;
				} else
				// if is NOT mine
				if (checker->count == 0) {
					PDEBUG(2, " +find NOT mine at (%d, %d)\n", checker->x, checker->y);
					get_new_item = ISNOTMINE;
				}

				if (get_new_item) {

					// set around this point
					set_mine_point(checker, get_new_item);

					// delete this from around clusters
					delete_mine_point(checker, get_new_item);

					list_checker = list_checker->prev;
					list_delete(&checker->cluster_index);
					free(checker);
				}
			} // end list_foreach

			// check if current cluster is empty
			if (list_isempty(&current->points)) {
				delete_cluster(current);
				continue;
			} else {
				PDEBUG(3, "split near this cluster -\n");
				split_near_cluster(current);
			}

			list_delete(&current->search);
		}
		PDEBUG(1, "finish checking.\n");

		// -- print --
		fprintf(ofp, "#%d\n", curr_case);
		for (i = 0; i < sizeX; i++)
			for (j = 0; j < sizeY; j++)
				if (minesweep[i][j]->value == ISMINE) fprintf(ofp, "%d %d ", i, j);
		fprintf(ofp, "\n");

		for (i = 0; i < sizeX; i++)
			for (j = 0; j < sizeY; j++)
				if (minesweep[i][j]->value == ISNOTMINE) fprintf(ofp, "%d %d ", i, j);
		fprintf(ofp, "\n");

		#ifdef IMAGE_PRINT
			printf("       case #%d image print: \n", curr_case);
			for (i = 0; i < sizeX; i++) {
				printf("\t\t");
				for (j = 0; j < sizeY; j++) {
					char c;
					PNT p = {i, j};

					if 		(MINE(p)->value == COVERED)		c = 'L';
					else if (MINE(p)->value == ISMINE)		c = '@';
					else if (MINE(p)->value == ISNOTMINE)	c = '+';
					else if (backup[p.x][p.y] == 0)			c = '.';
					else									c = backup[p.x][p.y] + '0';

					printf("%c ", c);
				}
				printf("\n");
			}
		#endif
		// -- end print --

		// -- post-processing --
		for (i = 0; i < sizeX; i++) {
			for (j = 0; j < sizeY; j++) {
				PNT p = {i, j};
				list_t *cluster_iter, pnt_iter;

				while (!list_isempty( &MINE(p)->clusters )) {
					CLUSTER *cluster;
					cluster = list_first_or_null(&MINE(p)->clusters, CLUSTER, index);

					while (!list_isempty(&cluster->points)) {
						PNT *pnt;
						pnt = list_first_or_null(&cluster->points, PNT, cluster_index);
						list_delete(&pnt->cluster_index);
						free(pnt);
					}
					list_delete(&cluster->index);
					free(cluster);
				}

				free(MINE(p));
			}
		}

	// --- end solve each case ---
	}
	PDEBUG(1, "Everything finish!\n");

	fclose(ifp);
	fclose(ofp);
	PAUSE;

	return 0;
}

// check valid count, using backtracking
int check_valid(CLUSTER *cluster, PNT *pnt, int count) {
	int valid = 0;

	// valid check
	if (!pnt) {
		PNT *iter;
		if (count != cluster->value) return 0;

		// this is valid
		PDEBUG(4, "arrive success, set valid\n");

		// increase valid count
		list_foreach(iter, &cluster->points, PNT, cluster_index)
			if (MINE(*iter)->value == ISMINE) iter->count++;

		return 1;
	}

	if (check_around_point(pnt) == 1) {
		set_mine_point(pnt, ISMINE);
		valid += check_valid(cluster, list_next_or_null(&cluster->points, pnt, PNT, cluster_index), count + 1);
		set_mine_point(pnt, COVERED);
	}
	
	valid += check_valid(cluster, list_next_or_null(&cluster->points, pnt, PNT, cluster_index), count);

	return valid;
}

// if this point can be mine?
int check_around_point(PNT *curr_pnt) {
	PNT setpnt;
	int dir;

	for (dir = 0; dir < DIRLENGTH; dir++) {
		SET_PNT_DIR(&setpnt, curr_pnt, dir);
		if (!VALIDBOUND(setpnt)) continue;
		if (MINE(setpnt)->value == 0) return 0;
	}
	return 1;
}

// set this point certain value (like mine, not mine, etc.)
void set_mine_point(PNT *pnt, int value) {
	PNT setpnt;
	int dir;

	MINE(*pnt)->value = value;

	// find each point
	for (dir = 0; dir < DIRLENGTH; dir++) {
		SET_PNT_DIR(&setpnt, pnt, dir);

		if (!VALIDBOUND(setpnt)) continue;
		if (MINE(setpnt)->value < NUMBER) continue;

		if (value == ISMINE) MINE(setpnt)->value--;
		else if (value == COVERED) MINE(setpnt)->value++;
	}
}

// delete this point from clusters
void delete_mine_point(PNT *pnt, int value) {
	CLUSTER *search_list, *cluster;
	PNT* setpnt;
	PDEBUG(4, "try to delete (%d, %d) from cluster\n", pnt->x, pnt->y);

	search_list = list_first_or_null(&MINE(*pnt)->clusters, CLUSTER, index);

	list_foreach(setpnt, &search_list->points, PNT, cluster_index) {
		PDEBUG(4, "find around (%d, %d)...\n", setpnt->x, setpnt->y);

		list_foreach(cluster, &MINE(*setpnt)->clusters, CLUSTER, index) {
			PNT *pnt_iter;
			int is_find = 0;

			// find point in each cluster
			list_foreach(pnt_iter, &cluster->points, PNT, cluster_index) {

				if (SAMEPNT(*pnt_iter, *pnt)) {
					PDEBUG(4, "find it\n");

					if (value == ISMINE) cluster->value--;

					// check it later if parent == cluster
					if (pnt_iter == pnt) continue;

					// delete point from list
					list_delete(&pnt_iter->cluster_index);
					free(pnt_iter);

					if (list_isempty(&cluster->points)) {
						PDEBUG(4, "erase empty cluster\n");
						delete_cluster(cluster);
					} else {
						// add search list
						add_search_list(cluster);
					}

					is_find = 1;
					break ;
				}
			}

			if (is_find) break;
			PDEBUG(4, "change cluster\n");
		} // end clusters

	} // end search_list
} // end func

// split near cluster
inline void split_near_cluster(CLUSTER *this_cluster) {
	LIST_HEAD(container);
	CLUSTER *new_cluster = 0, *iter_cluster;
	PNT *iter_pnt;

	PDEBUG(3, "try to find subset cluster of (%d, %d)\n", this_cluster->parent->x, this_cluster->parent->y);
	// find clusters around this cluster
	{
		CLUSTER *around_cluster;
		PNT *search;

		search = list_first_or_null(&this_cluster->points, PNT, cluster_index);
		if (!search) return ;
		PDEBUG(3, "cluster contains (%d, %d)\n", search->x, search->y);

		// this is 'the number cell list' around single 'covered cell'
		// we only check a single covered cell, because the cluster must contains
		// every single covered cell
		around_cluster = list_first_or_null(&MINE(*search)->clusters, CLUSTER, index);
		if (!around_cluster) return ;

		list_foreach(iter_pnt, &around_cluster->points, PNT, cluster_index) {
			if (SAMEPNT(*iter_pnt, *this_cluster->parent)) continue;

			// make list of 'containers of the number cells'
			list_foreach(iter_cluster, &MINE(*iter_pnt)->clusters, CLUSTER, index)
				list_add_tail(&iter_cluster->container, &container);
		}
	}

	if (list_isempty(&container)) return ;

	// now, check which cluster is containes 'this_cluster' and split
	list_foreach(iter_cluster, &container, CLUSTER, container) {
		list_t *list;

		if (!is_cluster_subset(this_cluster, iter_cluster)) continue;

		PDEBUG(4, "find subset at %d, %d\n", iter_cluster->parent->x, iter_cluster->parent->y);

		if (!new_cluster) {
			new_cluster = (CLUSTER *)malloc(sizeof(CLUSTER));
			memset(new_cluster, 0, sizeof(CLUSTER));
			list_init(&new_cluster->points);
		}

		// split!
		list_for(list, &iter_cluster->points) {
			iter_pnt = list_entry(list, PNT, cluster_index);

			if (!is_cluster_contains_pnt(this_cluster, iter_pnt)) {
				list = list->prev;
				list_delete(&iter_pnt->cluster_index);
				list_add_tail(&iter_pnt->cluster_index, &new_cluster->points);
			}
		}

		// split value
		new_cluster->value = iter_cluster->value - this_cluster->value;
		iter_cluster->value = this_cluster->value;
		PDEBUG(4, "new_cluster value=%d, iter_cluster value=%d\n", new_cluster->value, iter_cluster->value);

		// if new cluster is NOT empty, add it
		if (!list_isempty(&new_cluster->points)) {
			// to parent
			list_add_tail(&new_cluster->index, &iter_cluster->parent->clusters);
			new_cluster->parent = iter_cluster->parent;

			// and, search list
			add_search_list(new_cluster);

			new_cluster = 0;
		}
	} // end iter_cluster
	if (new_cluster) free(new_cluster);
} // end func

int is_cluster_subset(CLUSTER *small, CLUSTER *big) {
	PNT *iter;

	list_foreach(iter, &small->points, PNT, cluster_index)
		if (!is_cluster_contains_pnt(big, iter)) return 0;

	return 1;
}

int is_cluster_contains_pnt(CLUSTER *cluster, PNT *pnt) {
	PNT *iter;

	list_foreach(iter, &cluster->points, PNT, cluster_index)
		if (SAMEPNT(*iter, *pnt)) return 1;

	return 0;
}

void add_search_list(CLUSTER *cluster) {
	if (list_isempty(&cluster->points)) return ;
	if (list_islisted(&cluster->search)) return ;

	PDEBUG(3, "add (%d, %d) to search queue\n", cluster->parent->x, cluster->parent->y);
	list_add_tail(&cluster->search, &search_lists);

	return ;
}

void delete_cluster(CLUSTER *cluster) {
	list_delete(&cluster->index);
	if (list_islisted(&cluster->search)) {
		PDEBUG(4, "delete cluster (%d, %d) from search queue\n", cluster->parent->x, cluster->parent->y);
		list_delete(&cluster->search);
	}
	free(cluster);
}