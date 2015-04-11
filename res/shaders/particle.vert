#version 430

// Incoming position
layout(location = 0) in vec3 position;
// Incoming velocity
layout(location = 1) in vec3 velocity;
// Incoming liftime
layout(location = 2) in vec2 lifetime;

// Outgoing position
layout(location = 0) out vec3 position_out;
// Outgoing velocity
layout(location = 1) out vec3 velocity_out;
// Outgoing lifetime
layout(location = 2) out vec2 lifetime_out;

void main() {
  // Pass through the values
  position_out = position;
  velocity_out = velocity;
  lifetime_out = lifetime;
}