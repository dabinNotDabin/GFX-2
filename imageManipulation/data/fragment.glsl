#version 410

in vec3		toFragShaderColour;
out vec4 	FragmentColour;

void main() {
  FragmentColour = vec4(toFragShaderColour,1);
}
