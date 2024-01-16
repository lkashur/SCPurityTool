# imports
import numpy as np
import pandas as pd
import h5py
import ROOT
import argparse

def main(args):
    
    # Load data and store (temporarily) in a pandas dataframe
    f = h5py.File(args.in_file, 'r')
    data_mask = f['packets'][:]['packet_type'] == 0
    valid_parity_mask = f['packets'][data_mask]['valid_parity'] == 1
    data = (f['packets'][data_mask])[valid_parity_mask]
    df = pd.DataFrame(np.array(data))

    # Save to ROOT TTree                                                                                        
    chip_id = df['chip_id'].to_numpy().astype('int32')                                                                                                 
    channel_id = df['channel_id'].to_numpy().astype('int32')            
    timestamp = df['timestamp'].to_numpy().astype('int32')
    dataword = df['dataword'].to_numpy().astype('int32')
    data_dict = {'chip_id': chip_id, 'channel_id': channel_id, 'timestamp': timestamp, 'dataword': dataword}                                 
    rdf = ROOT.RDF.FromNumpy(data_dict)                                                                                                                
    rdf.Snapshot('tree', args.out_file)                                                                                                    
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--in_file', required=True)
    parser.add_argument('--out_file', required=True)
    args = parser.parse_args()
    main(args)
