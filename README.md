types:
	s = star
	p = planet
	m = moon
	a = asteroid
	
units: 
	position = AU
	velocity = AU/year
	acceleration = AU/?
	mass = solar masses
	radius = solar radius?
	time (dt) = years

Inputfile format 

planets and moons:
	type position velocity acceleration mass radius colours(r g b)

stars:
	type star_type position velocity acceleration mass radius colours(r g b)
	
asteroid:
	type amount central_body_idx min_distance_from_central_body max_distance_from_central_body colours(r g b)
	
example for the Sun:
s sun_type 0 0 0   0 0 0   0 0 0   1   10   255 244 214

