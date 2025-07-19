import numpy as np
import csv

# Corners' coordinates for the grid
NW = (6.267458,-75.569145) #(6.267422,-75.569137) #(6.267413, -75.569133) # (0,0)     ---> 16
NE = (6.267263,-75.569178) #(6.267243,-75.569166) #(6.267266, -75.569151) # (19,0)    ---> biblio
SW = (6.267447,-75.569366) #(6.267447,-75.569380) #(6.267529, -75.569414) # (0,26)    ---> MUUA
SE = (6.267273,-75.569422) #(6.267265,-75.569426) #(6.267280, -75.569425) # (19,26)   ---> cafe

# Grid size
rows = 15 #27
cols = 10 #20

# Function to interpolate latitudes and longitudes
def interpolate_grid(NW, NE, SW, SE, rows, cols):
    grid = []
    for i in range(rows):
        row = []
        t = i / (rows - 1)
        
        lat_left = NW[0] * (1 - t) + SW[0] * t
        lon_left = NW[1] * (1 - t) + SW[1] * t
        
        lat_right = NE[0] * (1 - t) + SE[0] * t
        lon_right = NE[1] * (1 - t) + SE[1] * t

        for j in range(cols):
            s = j / (cols - 1)
            lat = lat_left * (1 - s) + lat_right * s
            lon = lon_left * (1 - s) + lon_right * s
            row.append((lat, lon))
        grid.append(row)
    return grid

# Create the grid
grid = interpolate_grid(NW, NE, SW, SE, rows, cols)

# Save to CSV
with open("./interface/grid_coordinates.csv", "w", newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["row", "col", "latitude", "longitude"])
    for i in range(rows):
        for j in range(cols):
            lat, lon = grid[i][j]
            writer.writerow([i, j, lat, lon])
