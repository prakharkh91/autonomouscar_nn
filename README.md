autonomouscar_nn
================

this script sends a command to a Robot Controller which directs the car to right or left by certain degrees. 


the robot car was programmed on a software called Webots . 


GET_TRAINING_DATA.c

    Data is collected from here , which contains:
    1. Photos of road ahead as seen by car(1 per 5 or 10 timesteps(this is predefined by software) depending on test).
    2. Degree of turn.
    3. Speed at that point.
    4. name of the photo.


training_data_creation.py

    Data is converted in format which can be used by the neural network to train itself.

neural_network_basic.py

    Data is given to neural network to train itself .

get_command_to_car.py

    The photo taken by car when it runs is fed to the trained neural network which gives the degree by which it has to turn.
