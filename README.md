# Elementary-Cellular-Automata-Simulator

This repository contains a tool implemented in C to simulate and analyze elementary cellular automata, a computational model with applications in nature, traffic flow, and problem-solving. Developed for the COMP10002 Foundations of Algorithms course, this project explores dynamic memory allocation, linked data structures, and practical problem-solving using automata.

## Overview
The project focuses on simulating the behavior of one-dimensional cellular automata with user-defined configurations and rules. It demonstrates the following features:

1. **Automaton Initialization**
   - Reads the size, update rule, and initial state of the automaton from user input.
   - Uses dynamic memory allocation to construct flexible data structures.
   - Visualizes the automaton's configuration and update rules.

2. **Automaton Execution**
   - Simulates the evolution of the automaton over a user-defined number of time steps.
   - Tracks and reports state transitions for specific cells over a defined time period.
   - Outputs a detailed history of the automaton's states in chronological order.

3. **Solving the Density Classification Problem**
   - Implements a procedure using two cellular automaton rules (184 and 232) to classify the density of a binary array.
   - Identifies whether an array has more on or off states, or an equal distribution, through automaton evolution.
   - Outputs intermediate states, results, and classification summaries.

4. **Extensibility**
   - Encourages further exploration of practical problems solvable with cellular automata.
   - Promotes modular design for implementing additional automaton-based solutions.

## Documentation
For detailed description about the tasks, please refer to `ass2 (1).pdf`. Furthermore, there are some sample testcases (like `test0.txt`) and outputs (like `test0-out.txt`) for testing functions. 

## Applications
This project serves as a foundation for understanding and leveraging cellular automata in:
- Modeling complex behaviors in systems (e.g., natural patterns, traffic dynamics).
- Solving computational problems like density classification.
- Exploring algorithmic properties of automaton rules.

## How to Use
1. Compile the program using a C compiler like `gcc`.
2. Provide the configuration and instructions through a formatted input file.
3. Run the program to simulate, analyze, and generate reports based on the automaton.

### Example:
```bash
gcc -o automata_simulator main.c
./automata_simulator < input.txt > output.txt
```
