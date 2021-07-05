// Main structure declaration: binary search tree due to store the nodes info find on Monero peer-to-peer network
struct bst_ip {
	struct in_addr    nodip; // IPv4 of current node, type uint32_t s_addr
	int               port;  // port number of current node
	int               stat;  // stat of current node: found (0), request done (1), state to remove from map (2), remove beharra (3), waiting for transactions (4)
	float             lon;   // longitude of current node
	float             lat;   // latitude of current node
	int               tkop;  // number of transactions received from the current node (-1 if the process has not been initialized)
	pthread_mutex_t   lock;  // to ensure atomicity: mutex
//	pthread_cond_t	  cond;  // wait/signal not used
	struct bst_ip   * left;  // left sub-tree (lower IP)
	struct bst_ip   * right; // rigth sub-tree (higher IP)
};

// main data structure pointer (global)
struct bst_ip *root;

// Methods
int insert_elem(struct in_addr ip, int port);
void print_bst();
void print(struct bst_ip *current, int k, FILE *fp);
struct bst_ip * find(struct bst_ip *unek, struct in_addr ip);
int count_avail(struct bst_ip * current);
