import random
import math
from training_data_creation import create_input_data
import training_data_creation as TDC
#   1. Function for initiation of neural network weights.
#   2. Function to read neural network weights into lists.
#   3. Neural Network Layer creation, NN class creation.


############################################### initialization of weights
def weights_init():
    newfile = file("init_weights_file.txt","w")
    
    " " " Input layer and hidden layer  " " "
    
    lower_range = float(-1/math.sqrt(8192))
    upper_range = float(1/math.sqrt(8192))

    weights_layer1 = []
    
    for i in range(64):
        for j in range(8192):
             weights_layer1.append(random.uniform(lower_range,upper_range))
        #print("LAYER 1 init weights "+ str(i))
        #newfile.writelines(str(weights_layer1))
        #newfile.writelines("\n")
    newfile.writelines(str(weights_layer1))
    newfile.writelines("\n")
    
    " " " Hidden layer and output layer " " "
    " " " 1 output neuron taking inputs from 64 neurons " " "
    lower_range = float(-1/math.sqrt(64))
    upper_range = float(1/math.sqrt(64))

    weights_layer2 = []
    
    for i in range(1):
        for j in range(64):
            weights_layer2.append(random.triangular(lower_range,upper_range))
        newfile.writelines(str(weights_layer2))
        newfile.writelines("\n")

    #newfile.close()
    return [weights_layer1 , weights_layer2]
#weights_init()        
#x = weights_init()
#print(x[0])


# Get input from files . in this format.
# x[8192]

real_numbers = ["0","1",'2','3','4','5','6','7','8','9','.']
def get_input_data(name):
    filen = file(name,"r")
    lines = filen.readlines()
    ip_data = []
    for i in range(len(lines)):
            temp = ""
            for x in range(len(lines[i])):
                if lines[i][x] in real_numbers :
                    temp += lines[i][x]
                if lines[i][x] == "]":
                    #print temp
                    ip_data.append(float(temp))
                    temp = ""
            #ip_data.append(float(t.partition("]")[1]))
            #ip_data.append(float(lines[i][j]))
    return ip_data


#x = get_input_data("new01.txt")
#print x




################################################ Neural Network Code
############ 
#   1. Perceptron ( weights , activation_function )
#       1.1 Fire - Summation of weights and input , then their activation. 
#       1.2 Update - Error Function to correct weights
#       
#   2. Layer ( no_of_neurons , perceptron(no_of_neuron) )
#       2.1 Fire - will invoke fire for each perceptron
#       2.2 
#
#   PUT WEIGHTS in a class to turn them into matrix format?????
#
from math import tanh , exp

######## ACTIVATION FUNCTIONS

def treshold(x):
    if x >= 0:
        return 1
    else:
        return 0


tansig = tanh

########--ENDS





######## PERCEPTRON CODE[]
class perceptron(object):
    def __init__(self, weights , bias , activation):
        self.weights = weights
        self.bias = bias
        self.activation = activation
        #######  Needed for Back-Propagation
        self.output = float(0)
        self.delta_bp = float(0)
        
    
    def fire(self, input_vector):
        summed = sum([i*w for (i,w) in zip(input_vector, self.weights)])
        self.output = self.activation(summed + self.bias)
    
    
    " " " the factor element is learning rate which we will assume to be 0.7 as in most cases it is good. " " "
    def update(self, input_vector, factor):
        self.weights = [(w + self.delta_bp*factor*x) for (w, x) in
                        zip(self.weights, input_vector)]
        #self.bias = self.bias + factor # x = +1

###############--ENDS





############### LAYER CLASS
class layer1(object):        
    def __init__(self , no_of_neurons , weight):
        self.no_of_neurons  = no_of_neurons
        self.weights = weight
        self.all_neurons = []
        for i in range(self.no_of_neurons):
            neuron = perceptron(self.weights[8192*i : 8192*(i+1)] , 1 , tansig)
            #print("**********************!!!!!!!!@@@@@@@@@@@@@#########################*********************")
            #print(self.weights[8192*i : 8192*(i+1)])
            self.all_neurons.append(neuron)
    
    def init_neurons(self):
        all_n = []
        for i in range(self.no_of_neurons):
            temp = perceptron(self.weights[8192*i : len(self.weights)%(i+1)] , 1 , tansig)
            all_n.append(temp)
        return all_n

    

class layer2(object):
    def __init__(self , no_of_neurons , weight):
        self.no_of_neurons  = no_of_neurons
        self.weights = weight
        self.all_neurons = layer2.init_neurons(self)
    def init_neurons(self):
        all_n = []
        for i in range(self.no_of_neurons):
            temp = perceptron(self.weights[0:64] , 1 , tansig)
            all_n.append(temp)
        return all_n
##########--ENDS
 
 
 
################ NEURAL NETWORK LEARNING ALGORITHM.
def neural_network_learning():
    [weights_layer1 , weights_layer2] = weights_init()
    
    
    #print("       type of weight :")
    #print(type(weights_layer2[0]))
    #print(weights_layer2)
    
    
    layer_ip_hidden = layer1(64,weights_layer1)
    layer_hidden_op = layer2(1,weights_layer2)
    
    print "ENTER LEARING FACTOR"
    lr = float(raw_input())
    #print("ip hidden layer weight!!")
    #print(layer_ip_hidden.all_neurons[0].weights)
    
    
    """ making input data """
    create_input_data()
    
    
    print "FOR LEARNING RATE 0.2"
    #for i in range(10):
    iteration = 0
    for name in TDC.png_file_list:
        temp_ip_data = get_input_data(name)
        #print("TEMP IP DATA ------")
        #print(temp_ip_data)
        print("FOR FILE "+name+" :::::::::::::::::::::::::::::::::::")

        ip_from_hdn_to_op = []
        for neuron in range(layer_ip_hidden.no_of_neurons):
            layer_ip_hidden.all_neurons[neuron].fire(temp_ip_data)
            ip_from_hdn_to_op.append(float(layer_ip_hidden.all_neurons[neuron].output))

        #print("               IP TO HIDDEN -- ")
        #print(type(ip_from_hdn_to_op))
        #print(type(ip_from_hdn_to_op[0]))


        layer_hidden_op.all_neurons[0].fire(ip_from_hdn_to_op)


        #print("--------------------FEED FORWARD DONE")
        #print("--------------------BACK-PROPAGATION STARTS")

        """ BP STARTS """
        layer_hidden_op.all_neurons[0].delta_bp = float(TDC.actual_op_list[iteration]) - layer_hidden_op.all_neurons[0].output

        for neuron in range(layer_ip_hidden.no_of_neurons):
            #print("           NEURON VALUE "+ str(neuron) )
            #print("           WEIGHT VALUE "+ str(layer_hidden_op.all_neurons[0].weights[neuron]))
            layer_ip_hidden.all_neurons[neuron].delta_bp = layer_hidden_op.all_neurons[0].weights[neuron] * layer_hidden_op.all_neurons[0].delta_bp
            layer_ip_hidden.all_neurons[neuron].update(temp_ip_data , lr)
            
        layer_hidden_op.all_neurons[0].update(ip_from_hdn_to_op , lr)
    
            #print("--------------------"+name+" done")
        
            #print layer_hidden_op.all_neurons[0].weights[1]
            #print "bias:: "+str(layer_hidden_op.all_neurons[0].bias)
        
        #print(" *************************************************************************************** ")
        #print(" *************************************************************************************** ")
        #print(" *************************************************************************************** ")
        #print("IN ITERATION "+str(i))
        #print layer_hidden_op.all_neurons[0].weights[0]
        
        
        
        
    filen = file("final_weights_file.txt" , 'w')
    
    for i in range(layer_ip_hidden.no_of_neurons):
        filen.writelines(str(layer_ip_hidden.all_neurons[i].weights))
        #print("********************************** FOR NEURON "+str(i))
        #print(layer_ip_hidden.all_neurons[i].weights)
        filen.writelines("\n")
    filen.writelines(str(layer_hidden_op.all_neurons[0].weights))
    filen.writelines("\n")
    
    filen.close()
    
    
    
if __name__ == '__main__' :
    neural_network_learning()
    print("#################### LEARNING DONE ######################")
        
        










    
    
################################################ LAUNCH TRAINING OF NEURAL NETWORK
##
##
##if __name__ == '__main__' :
##    create_input_data()
##    print("Input Data Files Successfully Created!!!")
##    weights_init()
##    print("Weights for Neural Network Initialized Successfully")
##    print("Layer 1 : Initialized Using random.uniform()")
##    print("Layer 2 : Initialized Using Triangular Distribut ion")
##    print("#############################################################\n#############################################################")
##    
##
##
