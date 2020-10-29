#!/usr/bin/python

import sys, getopt
import ROOT
from ROOT import TFile, TTree, gROOT, AddressOf
from root_numpy import array2tree, root2array
import h5py
import numpy as np

def main(argv):
    inputfile = ''
    outputfile = 'output.root'
    opts, args = getopt.getopt(argv,"hi:o:",["ifile=","ofile="])
    for opt, arg in opts:
        if opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg

    #select raw data file
    f = h5py.File(inputfile, 'r')
	
    #check for "corrupt" packets
    data_mask = f['packets'][:]['packet_type'] == 0
    valid_parity_mask = f['packets'][data_mask]['valid_parity'] == 1
    good_data = (f['packets'][data_mask])[valid_parity_mask]
	
    #relevant packet info
    chip_id = good_data['chip_id'].astype(np.uint64)
    channel_id = good_data['channel_id'].astype(np.uint64)
    timestamps = good_data['timestamp']
    adcs = good_data['dataword']
    
    #placing relevant data in np arrays
    adcs_array = np.array(adcs)
    adcs_array.dtype = [('adc_counts', 'uint8')]

    timestamps_array = np.array(timestamps)
    timestamps_array.dtype = [('timestamp', 'uint64')]
	
    chipids_array = np.array(chip_id)
    chipids_array.dtype = [('chip_id', 'uint64')]
	
    channel_ids_array = np.array(channel_id)
    channel_ids_array.dtype = [('channel_id', 'uint64')]
	
    #ROOT
    f = TFile(outputfile, 'RECREATE')
	
    #fill TTree
    tree = array2tree(adcs_array)
    array2tree(timestamps_array, tree=tree)
    array2tree(chipids_array, tree=tree)
    array2tree(channel_ids_array, tree=tree)
	
    tree.Write()
    
if __name__ == "__main__":
    main(sys.argv[1:])
