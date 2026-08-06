import math
import random

NUM_BODIES = 3000
OUTPUT_FILENAME = "uniform_planets_3000.txt"

# Physical constants matching your unit system
G = 4 * (math.pi ** 2)
M_SUN = 1.0

with open(OUTPUT_FILENAME, "w") as f:
    f.write("// Perfectly Uniform Isotropic 3D Planetary Field for Octree Benchmarking\n")
    f.write("// position(x y z) velocity(vx vy vz) acceleration(ax ay az) mass radius colours(r g b)\n")
    
    # Central Sun to keep things stable
    f.write(f"0.0 0.0 0.0   0.0 0.0 0.0   0.0 0.0 0.0   {M_SUN}   12   255 244 214\n")
    
    # Linearly space the distances to guarantee uniform radial density (no clumping)
    min_dist = 2.0
    max_dist = 60.0
    
    for i in range(NUM_BODIES - 1):
        # Evenly space the orbital distances across the entire volume
        distance = min_dist + (i / (NUM_BODIES - 2)) * (max_dist - min_dist)
        
        # Add a tiny bit of random jitter to the radius so they aren't on perfect tracks
        distance += random.uniform(-0.01, 0.01)
        
        # Completely randomize the 3D angles to distribute them spherically
        phi = random.uniform(0, 2 * math.pi)
        costheta = random.uniform(-1.0, 1.0)
        theta = math.acos(costheta)
        
        # 3D Position calculations
        x = distance * math.sin(theta) * math.cos(phi)
        y = distance * math.sin(theta) * math.sin(phi)
        z = distance * math.cos(theta)
        
        # Calculate ideal stable Keplerian velocity magnitude: v = sqrt(G*M / r)
        v_mag = math.sqrt((G * M_SUN) / distance)
        
        # Create a velocity vector tangent to the sphere to establish stable orbits
        # We find two vectors perpendicular to the position vector
        v_raw_x = -math.sin(phi)
        v_raw_y = math.cos(phi)
        v_raw_z = 0.0
        
        # Apply the velocity magnitude
        vx = v_raw_x * v_mag
        vy = v_raw_y * v_mag
        vz = v_raw_z * v_mag
        
        # Randomize planet properties (Clean mass range, no hyper-massive clumping triggers)
        mass = random.uniform(1e-7, 1e-5) # Earth to Neptune-ish masses
        radius = random.randint(2, 5)
        
        # Highly diverse aesthetic variations
        r_col = random.randint(50, 255)
        g_col = random.randint(50, 255)
        b_col = random.randint(50, 255)
        
        f.write(f"{x:.6f} {y:.6f} {z:.6f}   {vx:.6f} {vy:.6f} {vz:.6f}   0.000000 0.000000 0.000000   {mass:.2e}   {radius}   {r_col} {g_col} {b_col}\n")

print(f"Success! Generated {NUM_BODIES} perfectly un-clustered planets in '{OUTPUT_FILENAME}'")
