#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>


#include "ai.h"
#include "utils.h"
#include "priority_queue.h"

extern unsigned num_generated_nodes;
extern unsigned num_expanded_nodes; 
extern unsigned num_max_depth; 


struct heap h;

float get_reward( node_t* n );

/**
 * Function called by pacman.c
*/
void initialize_ai(){
	heap_init(&h);
}

/**
 * function to copy a src into a dst state
*/
void copy_state(state_t* dst, state_t* src){
	//Location of Ghosts and Pacman
	memcpy( dst->Loc, src->Loc, 5*2*sizeof(int) );

    //Direction of Ghosts and Pacman
	memcpy( dst->Dir, src->Dir, 5*2*sizeof(int) );

    //Default location in case Pacman/Ghosts die
	memcpy( dst->StartingPoints, src->StartingPoints, 5*2*sizeof(int) );

    //Check for invincibility
    dst->Invincible = src->Invincible;
    
    //Number of pellets left in level
    dst->Food = src->Food;
    
    //Main level array
	memcpy( dst->Level, src->Level, 29*28*sizeof(int) );

    //What level number are we on?
    dst->LevelNumber = src->LevelNumber;
    
    //Keep track of how many points to give for eating ghosts
    dst->GhostsInARow = src->GhostsInARow;

    //How long left for invincibility
    dst->tleft = src->tleft;

    //Initial points
    dst->Points = src->Points;

    //Remiaining Lives
    dst->Lives = src->Lives;   

}

node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->priority = 0;
	new_n->depth = 0;
	new_n->num_childs = 0;
	copy_state(&(new_n->state), init_state);
	new_n->acc_reward =  get_reward( new_n );
	return new_n;
	
}

/*Calculate the value of h(n) */
float heuristic( node_t* n ){
	float h = 0;
	
	//FILL IN MISSING CODE
	
	float i=0, l=0, g=0;

	/* check if the current node is the root node */
	if(n->parent==NULL){
		if(n->state.Invincible == 1){
			i = 10;
		}
		// if it is, it hasn't lost any lives
		l = 0;
		
	}else{
		/* check if this node eat the fruit not its parent */
		if(n->state.Invincible == 1 && n->parent->state.Invincible != 1){
			i = 10;
		}
		if(n->state.Lives < n->parent->state.Lives){
			l = 10;
		}
	}
	
	if(n->state.Lives<0){
		g = 100;
	}
	
	h = i - l - g;
	
	return h;
}

float get_reward ( node_t* n ){
	float reward = 0;
	
	//FILL IN MISSING CODE
	
	float heuristic_n = heuristic(n);
	float score_n = n -> state.Points;
	float score_parent;
	
	/* find its parent' s score */
	if(n->parent == NULL){
		score_parent = score_n;
	}else{
		score_parent = n->parent->state.Points;
	}
	
	reward = heuristic_n + score_n - score_parent;

	float discount = pow(0.99,n->depth);
	
   	
	return discount * reward;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
bool applyAction(node_t* n, node_t** new_node, move_t action ){

	bool changed_dir = false;
	
	*new_node = (node_t*) malloc(sizeof(node_t));
	
	/* copy a the state of n into new_node' s state */
	copy_state(&((*new_node)->state), &(n->state));
	
    //FILL IN MISSING CODE
	
	/* check whether the action can be appllied */
    changed_dir = execute_move_t( &((*new_node)->state), action );
	
	/* update the node after applying the action */
	(*new_node)->parent = n;
	(*new_node)->depth = n->depth + 1;
	(*new_node)->priority = -((*new_node)->depth);
	(*new_node)->num_childs = 0;
	(*new_node)->acc_reward = get_reward(*new_node);
	
	/* if the depth is 1 means this is the first node,
	 * so the move of it will be the current action 
	 */
	if((*new_node)->depth == 1){
		(*new_node)->move = action;
	}else{
		/* if the depth greater than 1, then find its 
		*  parent's move as its 
		*/
		(*new_node)->move = (*new_node)->parent->move;
	}
	

	return changed_dir;

}


/**
 * Find best action by building all possible paths up to budget
 * and back propagate using either max or avg
 */

move_t get_next_move( state_t init_state, int budget, propagation_t propagation, char* stats ){
	move_t best_action = rand() % 4;

	float best_action_score[4];
	for(unsigned i = 0; i < 4; i++)
	    best_action_score[i] = INT_MIN;

	unsigned generated_nodes = 0;
	unsigned expanded_nodes = 0;
	unsigned max_depth = 0;
	
	//Add the initial node
	node_t* n = create_init_node( &init_state );
	
	// create empty array to store the node
	node_t** explored = malloc(sizeof(node_t*)*(budget*4));

	explored[generated_nodes] = n;
	
	//Use the max heap API provided in priority_queue.h
	heap_push(&h,n);
	
	//FILL IN THE GRAPH ALGORITHM
	
	while(h.count>0){
		/* get the node which has the highest priority */
		node_t* priority_node = heap_delete(&h);
		
		if(expanded_nodes<budget){
			expanded_nodes ++;
			for(int action = 0; action < 4; action++){
				
				node_t* new_node = NULL;
				/* create a new node and apply the action */
				bool change_dir = applyAction(priority_node, &new_node, action);
				/* check if the action can be applied */
				if(change_dir){
					
					/* if action applicable, the new node generated */
					priority_node->num_childs++;
					generated_nodes++;
					
					/* find the max depth */
					if(new_node->depth > max_depth){
						max_depth = new_node->depth;
					}
					
					/* add the new generated node into array */
					explored[generated_nodes] = new_node;
					
					/* check if lives lost, if lost create a new node */
					if(new_node->state.Lives < priority_node->state.Lives){
						// do nothing
					}else{
						heap_push(&h, new_node);
					}
				}else{
					free(new_node);
					continue;
				}
			}
		}
	}
	
	
	
	
	/* calculate the accumulated rewards */
	for(int i = generated_nodes-1; i > -1; i--){
		node_t* temp = explored[i];
		if(temp->num_childs == 0){
			float reward = 0;
			reward += temp->acc_reward;
			node_t* parent_temp = temp->parent;
			// counte the number of nodes
			int num = 1;
			do{ 
				/* calculate the total accumulated rewards of every nodes*/
				reward += parent_temp->acc_reward;
				parent_temp = parent_temp->parent;
				num++;
			}while(parent_temp!=NULL);
			if(propagation == max){
				// calculate the max reward
				if(reward > best_action_score[temp->move]){
					best_action_score[temp->move] = reward;
				}
			}else if(propagation == avg){
				// calculate the average reward
				if(reward/num > best_action_score[temp->move]){
					best_action_score[temp->move] = reward/num;
				}
			}
		}
	}
	
	/* free each node in explored array first */
	for(int i = 0; i < generated_nodes; i++){
		if(explored[i]!=NULL){
			free(explored[i]);
		}
	}
	
	/* free the whole array */
	free(explored);
	/* free the heap */
	emptyPQ(&h);
	
	float best_score = INT_MIN;
	
	/* first find the max score */
	for(int i = 0; i < 4; i++){
		if(best_action_score[i]>=best_score){
			best_score = best_action_score[i];
		}
	}
	
	/* randomly choose the action until find the action has the best score */
	while(best_action_score[best_action]<best_score){
		best_action = rand()%4;
	}
	
	/* calculate the total generated nodes and expanded nodes */
	num_generated_nodes += generated_nodes;
	num_expanded_nodes += expanded_nodes;
	
	/* find the max depth */
	if(num_max_depth<max_depth){
		num_max_depth = max_depth;
	}
	
	
	sprintf(stats, "Max Depth: %d Expanded nodes: %d  Generated nodes: %d\n",max_depth,expanded_nodes,generated_nodes);
	
	if(best_action == left)
		sprintf(stats, "%sSelected action: Left\n",stats);
	if(best_action == right)
		sprintf(stats, "%sSelected action: Right\n",stats);
	if(best_action == up)
		sprintf(stats, "%sSelected action: Up\n",stats);
	if(best_action == down)
		sprintf(stats, "%sSelected action: Down\n",stats);

	sprintf(stats, "%sScore Left %f Right %f Up %f Down %f",stats,best_action_score[left],best_action_score[right],best_action_score[up],best_action_score[down]);
	
	
	return best_action;
}






