#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const float MAGNITUDE = 0.1f;

out vec3 vcolor;

void GenerateVertexNormal(int index)
{
    vec3 P = gl_in[index].gl_Position.xyz;
    vec3 N = gs_in[index].normal;
    
    gl_Position = projection * view * model * vec4(P, 1.0);
	vcolor = vec3(1.0,1.0,0.0);
    EmitVertex();
    
    gl_Position = projection * view * model * vec4(P + N * MAGNITUDE, 1.0);
	vcolor = vec3(1.0,0.0,0.0);
    EmitVertex();
    
    EndPrimitive();
}

void GenerateFaceNormal()
{
  int i;
  
  //------ 3 lines for the 3 vertex normals
  //
  for(i=0; i&lt;gl_in.length(); i++)
  {
    vec3 P = gl_in[i].gl_Position.xyz;
    vec3 N = gs_in[i].normal.xyz;
    
    gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P, 1.0);
    vertex_color = vertex[i].color;
    EmitVertex();
    
    gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P + N * normal_length, 1.0);
    vertex_color = vertex[i].color;
    EmitVertex();
    
    EndPrimitive();
  }
  
 
  //------ One line for the face normal
  //
  vec3 P0 = gl_in[0].gl_Position.xyz;
  vec3 P1 = gl_in[1].gl_Position.xyz;
  vec3 P2 = gl_in[2].gl_Position.xyz;
  
  vec3 V0 = P0 - P1;
  vec3 V1 = P2 - P1;
  
  vec3 N = cross(V1, V0);
  N = normalize(N);
  
  // Center of the triangle
  vec3 P = (P0+P1+P2) / 3.0;
  
  gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P, 1.0);
  vertex_color = vec4(1, 0, 0, 1);
  EmitVertex();
  
  gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P + N * normal_length, 1.0);
  vertex_color = vec4(1, 0, 0, 1);
  EmitVertex();
  EndPrimitive();

}

void main()
{
    GenerateVertexNormal(0); // First vertex normal
    GenerateVertexNormal(1); // Second vertex normal
    GenerateVertexNormal(2); // Third vertex normal
}