# SCPurityTool

This tool can be used to extract the electron lifetime from a data-taking run at the SingleCube test for DUNE ND-LAr using simple reconstruction of anode-cathode-crossing cosmic muon tracks.  The code package makes use of a python script, two compiled C++/ROOT applications, additional program code implementing the DBSCAN clustering algorithm (for clustering charge associated with LArPix data packets into reconstructed tracks), and a map file used to convert LArPix channel/chip ID to spatial coordinates.

The first step is building the code to produce the two C++/ROOT applications:

```$ make```

Next, procure a raw data file in the "h5" file format associated with the data-taking run of interest.  You will need to convert this raw data file to ROOT file format:

```$ python3 raw_to_root.py --in_file datalog.h5 --out_file datalog.root```

Then run following command to produce a ROOT file containing clustered tracks:

```$ ./TrackMaker datalog.root```

This step produces a "datalog_analysis.root" file containing information associated with clustered tracks.  To view the tracks contained in this file in an event display, you have a few options:

-View a single track:
```$ python3 eventDisplay.py --in_file datalog_analysis.root --track_num 3```

-View N tracks:
```$ python3 eventDisplay.py --in_file datalog_analysis.root --num_tracks 8```

-View N tracks with a minimum temporal extent requirement (in us):
```$ python3 eventDisplay.py --in_file datalog_analysis.root --num_tracks 8 --min_temporal_extent 800```

Finally, to calculate the electron lifetime, run this command:

```$ ./PurityStudy datalog_analysis.root <efield>```

Here "&lt;efield&gt;" should be the electric field value associated with the run of interest in units of V/cm.  For instance, if the value of the electric field for the run was 1 kV/cm, run this explicit command:

```$ ./PurityStudy datalog_analysis.root 1000```

Note that 500 V/cm is assumed by default if no value is entered for the electric field.  The electron lifetime and associated uncertainty is printed to the terminal, and several useful plots are created for reference.  Additionally, a "results.root" file is produced containing information needed to reproduce the plots.

Please note that for the electron lifetime measurement to be reliable, at least 100 anode-cathode-crossing cosmic muon tracks are needed.  At the time of writing these instructions, in the first run of SingleCube at the University of Bern, this corresponds to roughly ten minutes of data-taking.  Ideally one should have 300 or more tracks, corresponding to a 30-minute data-taking run.  For a 30-minute data-taking run, the entire chain of processing should take less than ten minutes to finish (the TrackMaker application takes the longest to run by far).  However, it may take much longer to finish if there is a problem, such as many pixel channels missing in the data stream.  This should be checked first before processing the data-taking run using this tool, as the electron lifetime results would not be reliable in this case.

For any questions about this code, please contact Mike Mooney:  mrmooney@colostate.edu
