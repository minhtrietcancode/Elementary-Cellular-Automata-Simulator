/* NOTE ON IMPLEMENTATION CHOICES:
   1. Fixed-size array for cell states:
      - The total number of steps (including all stages) is deterministic
        based on input data.
      - Using a fixed-size array allocated upfront is more efficient than
        reallocating for each stage in this context.

   2. Input data handling:
      - As input is guaranteed to be valid, simple scanf() is used for
        efficiency in reading data.

   These choices prioritize efficiency given the assignment's constraints.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************/
/* DEFINE'S */
#define SDELIM "==STAGE %d============================\n"
#define MDELIM "-------------------------------------\n"
#define THEEND "==THE END============================\n"
#define NBRHDS 8
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

/* constants for printing header */
#define STAGE_0 0
#define STAGE_1 1
#define STAGE_2 2

/* constants for generating rules and evolving cell state */
#define ON_STATE '*'
#define OFF_STATE '.'
#define ON_STATE_VALUE 1
#define SHIFT_TWO_BITS 2
#define SHIFT_ONE_BIT 1
#define BIT_MASK_LSB 1 // least significant bit mask

/* constant for calculating evolving time steps for each rules */
#define RULE_184 184
#define RULE_232 232
#define RULE_184_SUBSTRACTION 2
#define RULE_232_SUBSTRACTION 1
#define RULES_DIVISOR 2

/*******************************************************************/
/* Structure definitions */

/* Cellular automaton (CA) configuration */
typedef struct {
    int size;              // Size of the cellular automaton (number of cells)
    int rule;              // Rule number for cellular automaton evolution
    char *cell_states;     // Initial states of the cells
    int time_steps;        // Number of time steps to evolve in stage 1
    int stage1_cell_position;     // Stage 1 cell position for observation
    int stage1_start_time;        // Stage 1 starting time step for observation
    int stage2_cell_position;     // Stage 2 cell position for observation
    int stage2_start_time;        // Stage 2 starting time step for observation
} CA_t;

/* A structure to represent the state of three neighboring cells and the result 
  of applying the rule */
typedef struct {
    int left_neighbour;    // Left cell state (0 or 1)
    int current_character; // Current cell state (0 or 1)
    int right_neighbour;   // Right cell state (0 or 1)
    int new;               // New state resulting from the rule
} cell_rule_t;

/* Structure to represent a rule, consisting of all possible 
   3-cell neighborhoods and its new state */
typedef struct {
    /* Array of 8 possible (left, current, right) triplets and its new state */
    cell_rule_t pairs[NBRHDS];  
} rule_t;

/*******************************************************************/

/* Function prototypes */

/* helper function for main function to allocated memory for all stages */
char **allocate_and_copy_states(int total_steps, int ca_size, 
                                char *initial_state);
    
/* helper function for main function to free all memory */
void free_memory(char **states, CA_t *input_data, int total_steps);

/* helper functions for stage 0 to read_configuration from input into variables 
*/
void read_configuration(CA_t *input_data);

/* helper functions for stage 0 and 2 to initializes the rule for cellular 
   automaton based on a given integers*/
void initialize_rule(int rule, rule_t *output_rule);

/* helper and main functions for evolve_cellular_automation_range, which is 
   helper function for stage 1 and 2 */
char get_neighbourhood(char left, char current, char right);
void evolve_automaton(char *current_state, char *next_state, 
                      rule_t *rule, int size);
void evolve_cellular_automation_range(char **states, int start_time, 
                     int end_time, rule_t *rule, int size);

/* helper function for stage 1 and 2 to print cell state in a defined range 
   of time */
void print_ca_states(char **states, int start_time, int end_time);

/* helper functions for stage 1 and 2 to counts ON/OFF states for a cell over 
   time and print out the counting report*/
void print_on_off_report(char **states, int start_time, 
                         int end_time, int cell_position);

/* STAGE 0 */
void print_stage_0(CA_t *input_data, rule_t *output_rule);

/* STAGE 1 */
void print_stage_1(CA_t *input_data, rule_t *output_rule, char **states);

/* STAGE 2 */
void print_classify_density(char **states, int size, 
                            int time_step, int total_steps);
void print_stage_2(CA_t *input_data, rule_t *rule_original, 
                   rule_t *rule_184, rule_t *rule_232, char **states,
                   int rule_184_step, int rule_232_step, int total_steps);

/*******************************************************************/
/* traffic control for the whole code structure
*/
int main(int argc, char *argv[]) {
    /* setting up variables to store data */
    CA_t input_data;
    rule_t rule_original, rule_184, rule_232;
    
    /* store the data from input into the variables */
    read_configuration(&input_data);

    /* initialize the rule based on the given rule, rule 184 and 232 */
    initialize_rule(input_data.rule, &rule_original);
    initialize_rule(RULE_184, &rule_184);
    initialize_rule(RULE_232, &rule_232);
    
    /* Calculate the total number of steps needed for all stages */
    int ca_size = input_data.size;
    int rule_184_step = (ca_size - RULE_184_SUBSTRACTION) / RULES_DIVISOR;
    int rule_232_step = (ca_size - RULE_232_SUBSTRACTION) / RULES_DIVISOR;
    int total_steps = input_data.time_steps + rule_184_step + rule_232_step;
    
    /* Allocate memory and copy the initial state into the first time step */
    char **states = allocate_and_copy_states(total_steps, input_data.size, 
                                             input_data.cell_states);
    
    /* Print the different stages */
    print_stage_0(&input_data, &rule_original);
    print_stage_1(&input_data, &rule_original, states);
    print_stage_2(&input_data, &rule_original, &rule_184, &rule_232, states,
                  rule_184_step, rule_232_step, total_steps);
    
    /* Print the final delimiter */
    printf(THEEND);
    
    /* Free allocated memory */
    free_memory(states, &input_data, total_steps);
    
    /* algorithms are fun */
    return EXIT_SUCCESS;
}

/*******************************************************************
                       FREE_MOMERY FUNCTION
*******************************************************************/
/* helper function for main function to free all allocated memory
*/
void free_memory(char **states, CA_t *input_data, int total_steps) {
    /* Free allocated memory */
    free(input_data->cell_states);
    for (int i = 0; i <= total_steps; i++) {
        free(states[i]);
    }
    free(states);
}

/*******************************************************************
                ALLOCATE_AND_COPY_STATES FUNCTION
*******************************************************************/
/* helper function for main function to allocate memory for all stages and copy
   the initial state into the first time step
*/
char **allocate_and_copy_states(int total_steps, int ca_size, 
                                char *initial_state) {
    /* Allocate memory to store the states of the automaton at each time step */
    char **states = (char **)malloc((total_steps + 1) * sizeof(char *));
    if (!states) {
        fprintf(stderr, "Memory allocation failed for states array\n");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i <= total_steps; i++) {
        states[i] = (char *)malloc((ca_size + 1) * sizeof(char));
        /* check if memory allocation successful or not */
        if (!states[i]) {
            fprintf(stderr, "Memory allocation failed for state %d\n", i);
            // Free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(states[j]);
            }
            free(states);
            exit(EXIT_FAILURE);
        }
    }
    
    /* Copy the initial state into the first time step */
    strncpy(states[0], initial_state, ca_size);
    states[0][ca_size] = '\0';
    
    return states;
}

/*******************************************************************
                    READ_CONFIGURATION FUNCTION 
*******************************************************************/
/* helper function for stage 0 to read configuration input for the cellular 
   automaton 
*/
void read_configuration(CA_t *input_data) {
    // Read size
    scanf("%d", &input_data->size);
    
    // Read rule
    scanf("%d", &input_data->rule);
    
    // Allocate memory for cell_states
    int cell_states_size = input_data->size + 1;
    input_data->cell_states = (char *)malloc(cell_states_size * sizeof(char));
    if (!input_data->cell_states) {
        fprintf(stderr, "Memory allocation failed for cell states\n");
        exit(EXIT_FAILURE);
    }
    
    // Read cell states
    scanf("%s", input_data->cell_states);
    
    // Read time_steps
    scanf("%d", &input_data->time_steps);
    
    // Read stage1_cell_position and stage1_start_time 
    scanf("%d,%d", &input_data->stage1_cell_position, 
                   &input_data->stage1_start_time);
    
    // Read stage2_cell_position and stage2_start_time
    scanf("%d,%d", &input_data->stage2_cell_position, 
                   &input_data->stage2_start_time);
}

/*******************************************************************
                INITIALIZE_RULE FUNCTION 
*******************************************************************/
/* helper functions for all stage 0 and 2 to initialize a rule based on its 
   given integer rule number
*/
void initialize_rule(int rule, rule_t *output_rule) {
    for (int i = 0; i < NBRHDS; i++) {
        /* Extract new state from the rule number */
        output_rule->pairs[i].new = (rule >> i) & BIT_MASK_LSB;

        /* Extract left neighbor, current, and right neighbor state */
        output_rule->pairs[i].left_neighbour = i >> SHIFT_TWO_BITS;
        output_rule->pairs[i].current_character = (i >> SHIFT_ONE_BIT) & 
                                                   BIT_MASK_LSB;
        output_rule->pairs[i].right_neighbour = i & BIT_MASK_LSB;
    }
}

/*******************************************************************
            EVOLVE_CELLULAR_AUTOMATION_RANGE FUNCTION
*******************************************************************/
/* helper function for evolve_automation, given three neighboring 
   cells, determine their neighborhood code 
*/
char get_neighbourhood(char left, char current, char right) {
    /* Generate a 3-bit code based on the states */
    return ((left == ON_STATE) << SHIFT_TWO_BITS) | 
           ((current == ON_STATE) << SHIFT_ONE_BIT) | 
           (right == ON_STATE);
}

/*******************************************************************/
/* helper function for evolve_cellular_automation_range to evolve the next 
   state based on the previous state and a certain rule 
*/
void evolve_automaton(char *current_state, char *next_state, 
                      rule_t *rule, int size) {
    for (int i = 0; i < size; i++) {
        /* Handle wrap-around for left and right neighbors */
        char left = current_state[(i - 1 + size) % size];
        char current = current_state[i];
        char right = current_state[(i + 1) % size];
        char neighbourhood = get_neighbourhood(left, current, right);
        
        next_state[i] = (rule->pairs[(unsigned int)neighbourhood].new 
                         == ON_STATE_VALUE) ? ON_STATE : OFF_STATE;
    }
    /* Null-terminate the string */
    next_state[size] = '\0';
}

/*******************************************************************/
/* helper function for stage 1 and 2 to evolve cellular automaton over a range 
   of time steps using the specified rule 
*/
void evolve_cellular_automation_range(char **states, int start_time, 
                     int end_time, rule_t *rule, int size) {
    for (int t = start_time; t < end_time; t++) {
        evolve_automaton(states[t], states[t + 1], rule, size);
    }
}

/*******************************************************************
                    PRINT_CA_STATES FUNCTION
*******************************************************************/
/* helper function for stage 1 and 2 to print cell state in a defined range of 
   time 
*/
void print_ca_states(char **states, int start_time, int end_time) {
    for (int t = start_time; t <= end_time; t++) {
        printf("%4d: %s\n", t, states[t]);
    }
}

/*******************************************************************
                   PRINT_ON_OFF_REPORT FUNCTION
*******************************************************************/
/* helper function for stage 1 and 2 to print the number of ON and OFF states 
   for a specific cell position over a range of time steps 
*/
void print_on_off_report(char **states, int start_time, int end_time, 
                         int cell_position) {
    int on = 0, off = 0;
    
    // Count ON and OFF states
    for (int t = start_time; t <= end_time; t++) {
        if (states[t][cell_position] == ON_STATE) {
            on++;
        } else {
            off++;
        }
    }
    
    // Print the report
    printf("#ON=%d #OFF=%d CELL#%d START@%d\n", 
           on, off, cell_position, start_time);
}

/*******************************************************************
                TRAFFIC CONTROL FOR STAGE 0
*******************************************************************/
/* Print the initial configuration and rule (Stage 0) 
*/
void print_stage_0(CA_t *input_data, rule_t *output_rule) {
    /* print header for stage 0*/
    printf(SDELIM, STAGE_0);

    /* print the given size and rule */
    printf("SIZE: %d\n", input_data->size);
    printf("RULE: %d\n", input_data->rule);

    /* print dashlines */
    printf(MDELIM);
    
    /* print the 8 combinations */
    for (int i = 0; i < NBRHDS; i++) {
        printf(" %d%d%d", output_rule->pairs[i].left_neighbour,
                          output_rule->pairs[i].current_character, 
                          output_rule->pairs[i].right_neighbour);
    }
    printf("\n");
    
    /* print the result for each combinations based on new rule */
    for (int i = 0; i < NBRHDS; i++) {
        printf("  %d ", output_rule->pairs[i].new);
    }
    printf("\n");
    
    /* print dashlines */
    printf(MDELIM);

    /* print the cell state at time step 0 */
    printf("%4d: %s\n", 0, input_data->cell_states);
}

/*******************************************************************
                TRAFFIC CONTROL FOR STAGE 1
*******************************************************************/
/* Print the evolution of the cellular automaton and count statistics (Stage 1) 
*/
void print_stage_1(CA_t *input_data, rule_t *output_rule, char **states) {
    /* print the header for stage 1 */
    printf(SDELIM, STAGE_1);
    
    /* Evolve the cellular automaton */
    evolve_cellular_automation_range(states, 0, input_data->time_steps, 
                    output_rule, input_data->size);
    
    /* Print the evolution */
    print_ca_states(states, 0, input_data->time_steps);
    
    /* print dashlines */
    printf(MDELIM);

    /* Print the ON/OFF report */
    print_on_off_report(states, input_data->stage1_start_time, 
                                input_data->time_steps, 
                                input_data->stage1_cell_position);
}

/*******************************************************************
                        STAGE 2  
*******************************************************************/
/* + Classify and print the density of ON ('*') cells in a given state based on 
   the result of applying rules 184 and 232 
   + We just need to compare the first and second element of the final state as 
   there are just 3 possible cases after applying rules 184 and 232 according 
   to the provided result in the description of stage 2
*/
void print_classify_density(char **states, int size, 
                            int time_step, int total_steps) {
    /* the last state after completing stage 1 that is being classified */
    char *classify_state = states[time_step];
    /* last state after applying rules 184 and 232 */
    char *final_state = states[total_steps - 1];  
    
    /* print the state that is being classified */
    printf("%4d: %s\n", time_step, classify_state);
    
    /* print the result of classification */
    printf("AT T=%d: ", time_step);
    if (final_state[0] == ON_STATE && final_state[1] == ON_STATE) {
        // All cells are ON
        printf("#ON/#CELLS > 1/2\n");
    } else if (final_state[0] == OFF_STATE && final_state[1] == OFF_STATE) {
        // All cells are OFF
        printf("#ON/#CELLS < 1/2\n");
    } else {
        // Mixed sequence (alternating ON and OFF)
        printf("#ON/#CELLS = 1/2\n");
    }
}

/*******************************************************************/
/* traffic control for stage 2
   Compare the behavior of different rules (Stage 2)
*/
void print_stage_2(CA_t *input_data, rule_t *rule_original, 
                   rule_t *rule_184, rule_t *rule_232, char **states,
                   int steps_184, int steps_232, int total_steps) {
    /* print the header for stage 2 */
    printf(SDELIM, STAGE_2);
    
    /* Rule 184 evolution */
    printf("RULE: 184; STEPS: %d.\n", steps_184);
    printf(MDELIM);
    evolve_cellular_automation_range(states, input_data->time_steps, 
                                     input_data->time_steps + steps_184, 
                                     rule_184, input_data->size);
    print_ca_states(states, input_data->time_steps, 
                    input_data->time_steps + steps_184);
    
    /* print dashlines */ 
    printf(MDELIM);
    
    /* Rule 232 evolution */
    printf("RULE: 232; STEPS: %d.\n", steps_232);
    printf(MDELIM);
    evolve_cellular_automation_range(states, input_data->time_steps + steps_184, 
                                     total_steps, rule_232, input_data->size);
    print_ca_states(states, input_data->time_steps + steps_184, total_steps);
    
    /* print dashlines */
    printf(MDELIM);
    
    /* Print the ON/OFF report */
    print_on_off_report(states, input_data->stage2_start_time, 
                        total_steps, input_data->stage2_cell_position);
    
    /* print dashlines */
    printf(MDELIM);
    
    /* Print the result for density classification */
    print_classify_density(states, input_data->size, 
                           input_data->time_steps, total_steps);
}

/*******************************************************************
                            THE END
*******************************************************************/
