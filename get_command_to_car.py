import neural_network_basic as NNB
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
from PIL import Image

import training_data_creation as TDC


def read_weights(name):
    filen = file(name , 'r')
    lines = filen.readlines()
    
    weights_layer1 = []
    weights_layer2 = []
    
    for i in range(len(lines)):
        x = lines[i].split()
        temp_weights = []
        for all_values in range(len(x)):
            temp = ''
            for z in x[all_values]:
                if z not in [',','[',']']: temp += z
            if i != 64: weights_layer1.append(float(temp))
            else: weights_layer2.append(float(temp))
        
        
        
    return [weights_layer1 , weights_layer2]
    
    
def init_nn():
    [weights_layer1 , weights_layer2] = read_weights('final_weights_file.txt')
    #print weights_layer2
    global layer_ip_hidden 
    layer_ip_hidden = NNB.layer1(64,weights_layer1)
    global layer_hidden_op 
    layer_hidden_op = NNB.layer2(1,weights_layer2)
    
init_nn()


def get_command(name):
    ### GET INPUT IN MATRIX FORMAT.
    # converting to gray
    
    
    ############################### UNCOMMENT LATTER !!!!!!!!!!!!!!!!!!!!! >>>>>>>>>>>>>>>>>>>
    #img = Image.open(name).convert('LA')
    #img.save(name)
    
    # reading pixel data
    img = mpimg.imread(name)
    image_pixels = []
    for row in range(len(img)):
        #print img[row]
        for col in range(len(img[row])):
            #print img[row][col][:1]
            image_pixels.append(float(img[row][col][:1][0]))
    #print image_pixels
    
    ##### FEED FORWARD.
    ip_from_hdn_to_op = []
    for neuron in range(layer_ip_hidden.no_of_neurons):
        layer_ip_hidden.all_neurons[neuron].fire(image_pixels)
        ip_from_hdn_to_op.append(float(layer_ip_hidden.all_neurons[neuron].output))
    
    layer_hidden_op.all_neurons[0].fire(ip_from_hdn_to_op)
    
    return layer_hidden_op.all_neurons[0].output
    
 

def extract_directions():
    filenames = []
    filen = file('trainingdata3.txt',"r")
    lines = filen.readlines()
    #print lines
    for i in lines:
        if '#' not in i:
            y = i.split()
            filenames.append(y[1])
            #actual_op_list.append(y[1])
    filen.close()
    return filenames   
  
  
    
def shell_test():
    #names_list = ['new0'+str(i)+'.png' for i in range(1,10)]
    names_list = []
    #for i in range(10 , 101):
    #    if i != 135:
    #        names_list.append('new'+str(i)+'.png')
    for i in range(136 , 149):
        if i != 135:
            names_list.append('new'+str(i)+'.png')
    
    
    
    
    i = 0
    error = 0.0
    avg_error = 0.0
    for name in names_list: 
        print  name
        actual_op_list = extract_directions()
        direc = get_command(name)
        ac_direc = float(actual_op_list[i])
        i += 1
        
        error = abs(direc - ac_direc)
        print error
        avg_error += error
    
    print 
    print
    print "AVERAGE ERROR : "+str(float(avg_error/(148-136)))

shell_test()
        

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
        
        
    
    
    
    
#read_weights('final_weights_file.txt')
