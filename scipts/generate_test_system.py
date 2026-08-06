import math
import random

NUM_BODIES = 3000
OUTPUT_FILENAME = "planetary_system_3000.txt"

# Physical constants matching your unit system
G = 4 * (math.pi ** 2)
M_SUN = 1.0

# Setup the output file
with open(OUTPUT_FILENAME, "w") as f:
    f.write("// Massive Clustered 3D Planetary System for Octree Benchmarking\n")
    f.write("// position(x y z) velocity(vx vy vz) acceleration(ax ay az) mass radius colours(r g b)\n")
    
    # 1. Add the central supermassive star/core
    f.write(f"0.0 0.0 0.0   0.0 0.0 0.0   0.0 0.0 0.0   {M_SUN}   15   255 230 180\n")
    
    # Define a few major cluster centers (acting as massive "Gas Giants")
    # scattered deep across 3D space to force heavy Octree splitting
    num_clusters = 8
    cluster_centers = []
    
    for _ in range(num_clusters):
        # Broad 3D sphere distribution
        distance = random.uniform(5.0, 45.0)
        phi = random.uniform(0, 2 * math.pi)
        theta = random.uniform(0, math.pi) # Full 3D vertical distribution
        
        cx = distance * math.sin(theta) * math.cos(phi)
        cy = distance * math.sin(theta) * math.sin(phi)
        cz = distance * math.cos(theta)
        
        # Orbital speed around the sun: v = sqrt(G*M / r)
        v_mag = math.sqrt((G * M_SUN) / distance)
        # Simple perpendicular velocity vector in 3D
        cvx = -v_mag * math.sin(phi)
        cvy = v_mag * math.cos(phi)
        cvz = random.uniform(-v_mag * 0.2, v_mag * 0.2)
        
        c_mass = random.uniform(1e-4, 5e-3) # Up to 5 Jupiter masses
        c_radius = random.randint(5, 9)
        
        cluster_centers.append({
            'pos': (cx, cy, cz), 'vel': (cvx, cvy, cvz), 'mass': c_mass
        })
        
        # Write the cluster parent planet
        r_col, g_col, b_col = random.randint(150, 255), random.randint(100, 200), random.randint(50, 150)
        f.write(f"{cx:.6f} {cy:.6f} {cz:.6f}   {cvx:.6f} {cvy:.6f} {cvz:.6f}   0.0 0.0 0.0   {c_mass:.2e}   {c_radius}   {r_col} {g_col} {b_col}\n")

    # 2. Distribute the remaining bodies across space
    # 60% are captured "moons" clustered around the gas giants, 40% are rogue scattered planets
    bodies_left = NUM_BODIES - 1 - num_clusters
    
    for i in range(bodies_left):
        if random.random() < 0.60:
            # Clustered Body (Moon orbiting a giant)
            parent = random.choice(cluster_centers)
            px, py, pz = parent['pos']
            pvx, pvy, pvz = parent['vel']
            
            # Distance from parent planet (local neighborhood)
            r_local = random.uniform(0.1, 1.5)
            l_phi = random.uniform(0, 2 * math.pi)
            l_theta = random.uniform(0, math.pi)
            
            x = px + r_local * math.sin(l_theta) * math.cos(l_phi)
            y = py + r_local * math.sin(l_theta) * math.sin(l_phi)
            z = pz + r_local * math.cos(l_theta)
            
            # Local high velocity orbiting the parent
            v_local = math.sqrt((G * parent['mass']) / r_local)
            vx = pvx - v_local * math.sin(l_phi)
            vy = pvy + v_local * math.cos(l_phi)
            vz = pvz + random.uniform(-v_local * 0.3, v_local * 0.3)
            
            mass = random.uniform(1e-8, 1e-6) # Earth to Moon class masses
            radius = random.randint(1, 3)
        else:
            # Rogue scattered planet (broad 3D distribution around the Sun)
            distance = random.uniform(1.0, 50.0)
            phi = random.uniform(0, 2 * math.pi)
            theta = random.uniform(0.1, math.pi - 0.1)
            
            x = distance * math.sin(theta) * math.cos(phi)
            y = distance * math.sin(theta) * math.sin(phi)
            z = distance * math.cos(theta)
            
            v_mag = math.sqrt((G * M_SUN) / distance)
            vx = -v_mag * math.sin(phi)
            vy = v_mag * math.cos(phi)
            vz = random.uniform(-v_mag * 0.5, v_mag * 0.5)
            
            mass = random.uniform(1e-7, 5e-5)
            radius = random.randint(2, 5)
            
        # Aesthetic variations
        r_col = random.randint(50, 240)
        g_col = random.randint(80, 240)
        b_col = random.randint(100, 255)
        
        f.write(f"{x:.6f} {y:.6f} {z:.6f}   {vx:.6f} {vy:.6f} {vz:.6f}   0.000000 0.000000 0.000000   {mass:.2e}   {radius}   {r_col} {g_col} {b_col}\n")

print(f"Done! Saved {NUM_BODIES} properly distributed 3D bodies to '{OUTPUT_FILENAME}'")
