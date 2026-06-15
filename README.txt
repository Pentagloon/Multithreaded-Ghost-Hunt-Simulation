Name: Eduard Patlea
Student Number: 101268119

Complation Instructions:
    - run: make
Execution Instructions:
    - For the base program:
        - ./p5
    - For the fork + exec:
        - ./runner     runs the program 30 times (by default)
        - ./runner X   to run the program X amount of times 

Completed Bonuses:
    (5%) Arguments and Multiprocessing

Purpose of each file:
    - defs.h: Provide the definitions for the constants as well as the various 
    structures used throughout the program, like Room, Hunter, Ghost, etc.
    
    - helpers.c: contains many useful helper functioms that are used throughout the program, many of which 
    were already provided. I've defined new ones like printing the end game result, as well as the hunter/ghost win ratio
    that i use in the multi-process version

    - house.c: allows the program to initialize the house, which internally populates the rooms of the house 
    and initializes the hunter array (empty at first)

    - room.c: Allows for the initialization of rooms, as well as connecting them. Also provides functions for the 
    breadcrumb stack that the hunters use (like push, pop, etc)

    - ghost.c: contains all the functinality that a ghost needs, like initialization, picking it's type randomly, 
    choosing an action to do and then taking it, etc.

    - hunter.c: similar to ghost.c, provides all the functionality for the hunter structure. In addition to initalization, 
    taking action, and similar functions, the file contains all the necessary functions to perform operations on the hunterArray 
    structure (like adding, cleaning up, etc) that is stored in the house.

    - main.c: specifies the main control flow of the program. Calls the initialization funtions, allows user to add
    hunters to the hunterArray structure, runs the game, and prints out the outcome at the end.

    - runner.c: allows for the simulation to be run many times (as many as specified by the user), then sends a 
    signal (if the --report flag is used) and prints out the results of all the simulations that have been run

    - Makefile: makes for an easy time when compiling/linking. Just run make to get the 2 executables.








