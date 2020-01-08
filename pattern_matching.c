#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"pattern_matching.h"

pm_state_t* compute_fail_state(pm_state_t*, char);
void recursive_destroy(pm_state_t* state);
//--------------------------------------------------------------------------
// init the automata
int pm_init(pm_t *pm)
{
	if (pm == NULL)
		return -1;
	pm_state_t *pmState = malloc(sizeof(pm_state_t)); //malloc the S0 state
	if (pmState == NULL)
    {
	    perror("failed malloc state\n");
        return -1;
    }
	pmState->id = 0;
	pmState->depth = 0;
	pmState->output = NULL;
	pmState->fail = NULL;
	pmState->_transitions = NULL;
	pm->zerostate = pmState;
	pm->newstate = 1;
	return 0;
}
//--------------------------------------------------------------------------
//build the basic automata by the patterns
int pm_addstring(pm_t *pm, unsigned char *str, size_t n)
{
	if (pm == NULL)
		return -1;
	if (pm->newstate > PM_CHARACTERS) 
		return -1;
	pm_state_t* curr_state = pm->zerostate, *to_state;
	int i;
	for (i = 0; i<(int)n; i++) //pass on the pattern
	{
		to_state = pm_goto_get(curr_state, str[i]); //check the next state with this char
		if (to_state == NULL)
		{
			to_state = malloc(sizeof(pm_state_t)); //allocate new state 
			if (to_state == NULL)
            {
                perror("failed malloc state\n");
                return -1;
            }
			to_state->_transitions = NULL;
			to_state->depth = curr_state->depth + 1; 
			to_state->fail = NULL;
			to_state->id = pm->newstate;
			to_state->output = NULL;
			if (pm_goto_set(curr_state, str[i], to_state)<0) //create new state
				return -1;
			printf("Allocating state %d\n%d -> %c -> %d\n", pm->newstate, curr_state->id, str[i], to_state->id);
			pm->newstate++;
		}
		curr_state = to_state;
		if (i == n - 1) //make an output list in the last state of the pattern
		{
			to_state->output = malloc(sizeof(slist_t));
			if (to_state->output == NULL)
			{
                perror("failed malloc output list\n");
                return -1;
			}
			slist_init(to_state->output);
			slist_append(to_state->output, str);
		}
	}
	return 0;
}
//--------------------------------------------------------------------------
//create new state
int pm_goto_set(pm_state_t *from_state, unsigned char symbol, pm_state_t *to_state)
{
	if (from_state->_transitions == NULL)  //if now is the first transition
	{
		from_state->_transitions = malloc(sizeof(slist_t));
		if (from_state->_transitions == NULL)
		{
            perror("failed malloc transitions list\n");
            return -1;
		}
		slist_init(from_state->_transitions);
	}
	pm_labeled_edge_t *edge = malloc(sizeof(pm_labeled_edge_t)); //aloocate new edge
	if (edge == NULL)
	{
        perror("failed malloc edge\n");
        return -1;
    }
	edge->label = symbol;
	edge->state = to_state;
	if (slist_append(from_state->_transitions, edge)<0)
		return -1;
	return 0;
}
//--------------------------------------------------------------------------
//check if the edge exsist
pm_state_t* pm_goto_get(pm_state_t *state, unsigned char symbol)
{
	if (state->_transitions == NULL)
		return NULL;
	slist_node_t *temp;
	pm_labeled_edge_t *ed;
	for (temp = slist_head(state->_transitions); temp != NULL; temp = slist_next(temp)) //pass on the transitions
	{
		ed = (pm_labeled_edge_t*)(temp->data); //convert the data to edge
		if (ed->label == symbol)
		{
			return ed->state;
		}
	}
	return NULL; //the edge doesnt exist
}
//--------------------------------------------------------------------------
//define the failure transitions and the output lists
int pm_makeFSM(pm_t *pm)
{
    if(pm == NULL)
    {
        perror("automat is NULL\n");
        return -1;
    }
	pm_state_t *father, *temp_state, *toState_fail;
	slist_node_t *temp_trans;
	slist_t *queue = malloc(sizeof(slist_t));
	if (queue == NULL)
	{
        perror("failed malloc queue\n");
        return -1;
	}
	slist_init(queue);
	father = pm->zerostate;
	temp_trans = slist_head(father->_transitions);
	while (temp_trans) //enter the children of S0 to the queue
	{
		temp_state = ((pm_labeled_edge_t*)(temp_trans->data))->state;
		slist_append(queue, temp_state);
		temp_state->fail = pm->zerostate;
		temp_trans = slist_next(temp_trans);
	}

	while (slist_head(queue)) //while the queue is not empty
	{
		father = (pm_state_t*)slist_pop_first(queue); //take the first state in the queue
		if (father->_transitions == NULL)
			continue;
		temp_trans = slist_head(father->_transitions);
		while (temp_trans) //pass on the fatherws children
		{
			temp_state = ((pm_labeled_edge_t*)(temp_trans->data))->state;
			slist_append(queue, temp_state); //add the next state to the queue
			toState_fail = compute_fail_state(father,(char)(((pm_labeled_edge_t*)(temp_trans->data))->label));
			temp_state->fail = toState_fail; 
			if (toState_fail->output != NULL) //append the output list of the fail transition
			{
				if (temp_state->output == NULL)
				{
					temp_state->output = malloc(sizeof(slist_t));
					if(temp_state->output==NULL)
                    {
                        perror("failed malloc output list\n");
                        return -1;
                    }
					slist_init(temp_state->output);
				}
				slist_append_list(temp_state->output, toState_fail->output);
			}
			printf("Setting f(%d)=%d\n", temp_state->id, temp_state->fail->id);
			temp_trans = slist_next(temp_trans);
		}
	}
	free(queue);
	return 0;
}
//---------------------------------------------------------------------------------------
//compute the failure transition
pm_state_t* compute_fail_state(pm_state_t* father, char label)
{
	pm_state_t *temp_father=father->fail, *toState_fail;
	toState_fail = pm_goto_get(temp_father, label); //check if the next state exsist
	while (toState_fail == NULL)
	{
	    if (temp_father->depth == 0) //if S0 - we found the fail state
        {
            toState_fail = temp_father;
            break;
        }
		temp_father = temp_father->fail;
		toState_fail = pm_goto_get(temp_father, label);
	}
	return toState_fail;
}
//----------------------------------------------------------------------------------------
//search of matches in the text by the automata
slist_t* pm_fsm_search(pm_state_t *zero_state, unsigned char *str, size_t n)
{
	pm_match_t* new_match;
	pm_state_t* to_state, *state = zero_state;
	slist_node_t* temp;
	slist_t* match_list = malloc(sizeof(slist_t));
	slist_init(match_list);
	if (state == NULL)
	{
	    perror("failed malloc matches list\n");
	    return NULL;
	}
	int i;
	for (i = 0; i<(int)n; i++) //pass on the text
	{
		while ((to_state = pm_goto_get(state, str[i])) == NULL) //find the next state until S0
		{
			if (state->id == 0)
				break;
			state = state->fail;
		}
		if (state->id == 0 && to_state == NULL) //the fail is S0, we need to throw the current letter
			continue;
		if (to_state->output == NULL) //go to the next state without append the output list
		{
			state = to_state;
			continue;
		}
		temp = slist_head(to_state->output);
		while (temp) //pass on the output list of the state, each node is match
		{
			new_match = malloc(sizeof(pm_match_t));
			if (new_match == NULL)
			{
                perror("failed malloc match pattern\n");
                return NULL;
			}
			new_match->pattern = (char *)(temp->data);
			new_match->start_pos = i - (int)strlen(new_match->pattern) + 1;
			new_match->end_pos = i;
			new_match->fstate = to_state;
			slist_append(match_list, new_match); //add new match to the matches list
			printf("Pattern: %s, start at: %d, ends at: %d, last state: %d\n", new_match->pattern, new_match->start_pos,
					new_match->end_pos, new_match->fstate->id);
			temp = slist_next(temp);
		}
		state = to_state;
	}
	return match_list;
}
//-------------------------------------------------------------------------
//free all dynamic allocations - destroy the automata
void pm_destroy(pm_t *pm)
{
	recursive_destroy(pm->zerostate);
	free(pm);
}
//-------------------------------------------------------------------------
//a recursive destroing for the states
void recursive_destroy(pm_state_t* state) 
{
	slist_node_t *temp_trans;
	pm_state_t *temp_state;
	if (!state)
		return;
	if (!(state->_transitions)) //free state without transitions
	{
		slist_destroy(state->output, SLIST_LEAVE_DATA);
		free(state);
		return;
	}
	temp_trans = slist_head(state->_transitions);
	while (temp_trans) //pass on transitions
	{
		temp_state = ((pm_labeled_edge_t*)(temp_trans->data))->state;
		recursive_destroy(temp_state);
		temp_trans = slist_next(temp_trans);
	}
	slist_destroy(state->output, SLIST_LEAVE_DATA);
	slist_destroy(state->_transitions, SLIST_FREE_DATA); //destroy the transitions list
	free(state);
}
//---------------------------------------------------------------------------------------
//end of the algorithm