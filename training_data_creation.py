#!/bin/python
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
from PIL import Image

#img = mpimg.imread('../_static/stinkbug.png')
#imgplot = plt.imshow(img)


#   1.  Read all file names.
#   2.  Convert all to grayscale.
#   3.  Create Pixel map and store in text files.


############################# function for extraction of file name
png_file_list = []
actual_op_list = []
def extract_filenames(name):
    filenames = []
    filen = file(name,"r")
    lines = filen.readlines()
    #print lines
    for i in lines:
        if '#' not in i:
            y = i.split()
            filenames.append(y[0])
            actual_op_list.append(y[1])
    filen.close()
    return filenames
    
#filenames = extract_filenames("trainingdata6.txt")
#print filenames

############################## functions for converting to gray

##def rgb2gray(rgb):
##    return np.dot(rgb[...,:3], [0.299, 0.587, 0.144])    

def convert2gray(filenames):
    #print filenames
    for name in filenames:
        img = Image.open(name).convert('LA')
        img.save(name)
        #print name
    print("Converting images to gray")


############################### functions for creation of pixel map and storing in files
def putpixelinfile(name):
    img = mpimg.imread(name)
    newname = name.partition(".")[0]+".txt"
    png_file_list.append(newname)
    newfile = file(newname,"w")
    for row in range(len(img)):
##      newfile.writelines(str(img[row])+"\n")
        for col in range(len(img[row])):
            newfile.writelines(str(img[row][col][:1]))
        newfile.writelines("\n")
    newfile.close()
    #print("FILE "+name+" done")

##
##putpixelinfile("new01.png")
##
##filex = file("new01.txt","r")
##filerows = filex.readlinesh()
##print len(filerows)
##


#######################################################################

def create_input_data():
    filenames = extract_filenames("trainingdata3.txt")
    convert2gray(filenames)
    #print("-----------converted to grayscale")
    #print("CREATING INPUT DATA ######################")
    for name in filenames:
        putpixelinfile(name)
        print("FOR "+name+" DONE")
        
    #print("------FILE With Pixels Created.")


#create_input_data()







        

