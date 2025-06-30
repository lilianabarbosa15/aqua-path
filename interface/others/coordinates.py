import numpy as np
import csv

# Corners' coordinates for the grid
NW = (6.267413, -75.569133)
NE = (6.267266, -75.569151)
SW = (6.267529, -75.569414)
SE = (6.267280, -75.569425)

# Grid size
rows = 27
cols = 20

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
