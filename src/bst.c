#include "main.h"
#include "bst.h"

/* Method to insert nodes on a binary search tree, O(log n)
 * return 0 when element is inserted
 * return 1 when a error occurs (e.g. that IP,Port exists on the tree)
*/
int insert_elem(struct in_addr ip, int port) {

        struct bst_ip *current = root;
        struct bst_ip *new = malloc (sizeof *new);
        new->nodip.s_addr = ip.s_addr;
        new->port = port;
        new->stat = 0; // node is found, first state (0)
        new->lon = 0;
	new->lat = 0;
	new->tkop = -1;
	new->left = NULL;
        new->right = NULL;
	pthread_mutex_init(&(new->lock), NULL);
//	pthread_cond_init (&(new->cond), NULL); // not used
//      printf("%u and %u \n", ntohl(current->nodip.s_addr), ntohl(new->nodip.s_addr));
        // sort by IP value with ntohl()...
        while(current != NULL) {
                unsigned int insert = ntohl(new->nodip.s_addr);
                unsigned int exists = ntohl(current->nodip.s_addr);
                if(exists > insert) {     // insert on the left sub-tree
                        if(current->left == NULL) { // place it
                                current->left = new;
                                return 0;
                        }
                        else current = current->left; // continue
                }
                else if(exists < insert){ // insert on the right sub-tree
                        if(current->right == NULL) { // place it
                                current->right = new;
                                return 0;
                        }
                        else current = current->right; // continue
                }
		// if IP is equal.. check the port number
		else if(current->port > new->port) { // insert on the left sub-tree
			if(current->left == NULL) { // place it
                                current->left = new;
                                return 0;
                        }
                        else current = current->left; // continue
		}
		else if(current->port < new->port) { // insert on the right sub-tree
			if(current->right == NULL) { // place it
                                current->right = new;
                                return 0;
                        }
                        else current = current->right; // continue
		}
		else 	break; // if IP,Port exists terminate insertion
        }
        return 1;

}


/* Write binary search tree on 'logbst' file (info of nodes)
 * Sorted by IP (in-order)
*/
void print_bst() {
	FILE *fp = fopen("logbst", "w");
	if(fp==NULL) {
		printf("file error on print_bst function");
		exit(1);
	}
	fprintf(fp, "Dept.\t  IPv4 \t\tPort   Stat  Lon     \t   Lat    \t TrKop.\n");
	fflush(fp);

        if(root->left != NULL ) print(root->left, 1, fp);
        fprintf(fp, "     < %15.15s  %5d  %d  %11f\t%11f\t  %2d  >   \n",inet_ntoa(root->nodip), root->port, root->stat, root->lon, root->lat, root->tkop);
        if(root->right != NULL) print(root->right, 1, fp);
	fclose(fp);

}

/* Recursive method to print all nodes of the bst (in-order)
*/
void print(struct bst_ip *current, int k, FILE *fp) {
        if(current->left != NULL ) print(current->left, k+1, fp);
        fprintf(fp, " %2d  %15.15s \t%5d  %d  %11f\t%11f\t  %2d\n", k, inet_ntoa(current->nodip), current->port, current->stat, current->lon, current->lat, current->tkop);
        if(current->right != NULL) print(current->right, k+1, fp);
}


/* Method to find a element on binary search tree recursively
 * return element of the tree
 * or continue searching recursively (only one sub-tree, right or left)
 * or return NULL
*/
struct bst_ip * find(struct bst_ip *curr, struct in_addr x) {
	if(curr==NULL || ntohl(curr->nodip.s_addr) == ntohl(x.s_addr)) // elementua find da edo null (ez dago)
		return curr;
	else if(ntohl(x.s_addr) > ntohl(curr->nodip.s_addr)) // find beharrekoa (x) handiagoa da, eskuinean findko da
		return find(curr->right, x);
	else // find beharrekoa (x) txikiagoa da, ezkerrean findko da
		return find(curr->left, x);
}

/* Method to count available nodes on binary search tree
 * We consider a node is available when state is 0, 1 or 4
 * return 0 or positive int
*/
int count_avail(struct bst_ip *current) {

	if(current == NULL)                                     return 0;
	else if(current->stat < 2 || current->stat == 4)	return 1 + count_avail(current->left) + count_avail(current->right);
	else                                                    return 0 + count_avail(current->left) + count_avail(current->right);

}

