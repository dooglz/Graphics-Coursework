#version 430

// Time passed since last frame
uniform float delta_time;

// Incoming geometry
layout(points) in;
// Outgoing geometry
layout(points, max_vertices = 1) out;

// Incoming position data
layout(location = 0) in vec3 position[];
// Incoming velocity data
layout(location = 1) in vec3 velocity[];

layout(location = 2) in vec2 lifetime[];

// Outgoing position after update
layout(location = 0) out vec3 position_out;
// Outgoing velocity
layout(location = 1) out vec3 velocity_out;
// Outgoing lifetime
layout(location = 2) out vec2 lifetime_out;

void main() {

  float life = lifetime[0].x + delta_time;
  // Update the position using standard velocity step
  vec3 newposition = position[0] + (delta_time * velocity[0]);

  // Ensure particle does not go out of bounds
  // if(newposition.y>30.0){newposition.y = 0;}
  if (life > lifetime[0].y) {
    life = 0.0f;
    newposition = vec3(0.0f);
  }

  // Output data
  position_out = newposition;
  velocity_out = velocity[0];
  lifetime_out = vec2(life,lifetime[0].y);

  // Emit vertex and end primitive
  EmitVertex();
  EndPrimitive();
}