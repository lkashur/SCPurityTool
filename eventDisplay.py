# imports
import numpy as np
import pandas as pd
import uproot
import plotly.express as px
import plotly.graph_objects as go
import argparse

def main(args):
    
    # Load data
    f = uproot.open(args.in_file)
    tree = f['trackTree']
    branches = tree.arrays()
    
    #df = tree.arrays(["trackHitX", "trackHitY", "trackHitT", "minT_T"], library="pd")

    traces = []

    # Plot anode
    surf_t = np.zeros(100)
    surf_xy = np.linspace(-150, 150, 100)
    cmap = [[0, '#9b7600'],
                [1, '#9b7600']]
    plane = go.Surface(x=surf_xy, y=surf_t, z=np.array([surf_xy] * len(surf_t)).T, colorscale=cmap, opacity=0.40, showscale=False, hoverinfo='skip') 
    traces.append(plane)
    
    # Plot N tracks
    if args.num_tracks:
        for i in range(args.num_tracks):
        
            trackHitRelT = branches['trackHitT'][i] - branches['minT_T'][i]
            if args.min_temporal_extent is not None:
                if max(trackHitRelT) < args.min_temporal_extent:
                    continue
                
            track_trace = go.Scatter3d(x=branches['trackHitX'][i],
                                       y=trackHitRelT,
                                       z=branches['trackHitY'][i],
                                       mode='markers',
                                       marker=dict(size=2),
                                       customdata=np.stack((branches['trackHitX'][i], branches['trackHitY'][i], trackHitRelT), axis=-1),
                                       hovertemplate='<b>x</b>: %{customdata[0]:,.2f}<br>' +
                                                     '<b>y</b>: %{customdata[1]:,.2f}<br>' +
                                                     '<b>t</b>: %{customdata[2]:,.2f}',
                                       name='Track ' + str(i))
            traces.append(track_trace)

    # Plot a single track
    if args.track_num:
        j = args.track_num
        trackHitRelT = branches['trackHitT'][j] - branches['minT_T'][j]

        track_trace = go.Scatter3d(x=branches['trackHitX'][j],
                                   y=trackHitRelT,
                                   z=branches['trackHitY'][j],
                                   mode='markers',
                                   marker=dict(size=2),
                                   customdata=np.stack((branches['trackHitX'][j], branches['trackHitY'][j], trackHitRelT), axis=-1),
                                   hovertemplate='<b>x</b>: %{customdata[0]:,.2f}<br>' +
                                                 '<b>y</b>: %{customdata[1]:,.2f}<br>' +
                                                 '<b>t</b>: %{customdata[2]:,.2f}')
        traces.append(track_trace)
        
    fig = go.Figure(traces)
    fig.update_layout(
        scene = dict(xaxis = dict(range=[-150,150]),
                     xaxis_title='x [mm]',
                     yaxis = dict(range=[-1,2000]),
                     yaxis_title='t [0.1μs]',
                     zaxis = dict(range=[-150,150]),
                     zaxis_title = 'y [mm]',
                     aspectmode = 'cube'))
    fig.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--in_file', required=True)
    parser.add_argument('--num_tracks', type=int)
    parser.add_argument('--min_temporal_extent', type=float)
    parser.add_argument('--track_num', type=int)
    args = parser.parse_args()
    main(args)
