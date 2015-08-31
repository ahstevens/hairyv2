//!!ARBvp1.0    
#version 330 core

//# ******* attributes *********                         
//                                                       
//ATTRIB ipos  = vertex.position;                        
//ATTRIB iprev = vertex.texcoord[0];                     
//ATTRIB inext = vertex.texcoord[1];       
layout (location = 0) in vec3 ipos;
layout (location = 1) in vec3 iprev;
layout (location = 2) in vec3 inext;
layout (location = 3) in vec4 vertex_color;

//# ******* parameters *********                         
//                                                       
//PARAM mv_mat[]   = { state.matrix.modelview };         
//PARAM mvp_mat[]  = { state.matrix.mvp };               
//PARAM light_pos  = state.light[0].position;            
//PARAM const      = { 0, 0.5, 1, 2 };                   
uniform mat4 mv_mat;
uniform mat4 mvp_mat;
uniform vec3 light_pos;

//# ******* outputs *********                            
//                                                       
//OUTPUT opos     = result.position;                     
// GLSL: ^becomes gl_Position in main()
//OUTPUT otangent = result.texcoord[0];                  
//OUTPUT olight   = result.texcoord[1];                  
//OUTPUT oview    = result.texcoord[2];                  
out vec4 otangent;
out vec4 olight;
out vec4 oview;
out vec4 result_color;

//# ******** program *********                           
//                                                       
void main()
{
	//# ******** temporaries *********                       
	//                                                       
	//TEMP tangent;                                          
	//TEMP eye_pos;                                          
	vec4 tangent;
	vec4 eye_pos;

	//# transform vertex position to camera coordinates      
	//DP4 eye_pos.x, mv_mat[0], ipos;                        
	//DP4 eye_pos.y, mv_mat[1], ipos;                        
	//DP4 eye_pos.z, mv_mat[2], ipos;                        
	//DP4 eye_pos.w, mv_mat[3], ipos;                        
	eye_pos = mv_mat * vec4( ipos, 1.0 );

	//# compute tangent                                      
	//SUB tangent, inext, iprev;                             
	tangent = vec4( inext - iprev, 0.0 );

	//DP3 tangent.w, tangent, tangent;                       
	tangent.w = dot( tangent, tangent );

	//RSQ tangent.w, tangent.w;                              
	tangent.w = inversesqrt( tangent.w );

	//MUL tangent.xyz, tangent, tangent.w;                   
	tangent.xyz = tangent * tangent.w;

	//DP3 otangent.x, mv_mat[0], tangent;                    
	//DP3 otangent.y, mv_mat[1], tangent;                    
	//DP3 otangent.z, mv_mat[2], tangent;                    
	//MOV otangent.w, const.z;                               
	otangent = mat3( mv_mat ) * tangent.xyz;
	otangent.w = 1.0;

	//# compute light direction                              
	//SUB olight.xyz, light_pos, eye_pos;                    
	//MOV olight.w, const.z;                                 
	olight = vec4( light_pos - eye_pos.xyz, 1.0 );

	//# compute viewing direction                            
	//MOV oview, -eye_pos;                                   
	oview = -eye_pos;

	//# transform vertex position                            
	//DP4 opos.x, mvp_mat[0], ipos;                          
	//DP4 opos.y, mvp_mat[1], ipos;                          
	//DP4 opos.z, mvp_mat[2], ipos;                          
	//DP4 opos.w, mvp_mat[3], ipos;             
	gl_Position = mvp_mat * ipos;

	//# pass through color                                   
	//MOV result.color, vertex.color;                        
	result_color = vertex_color;

//END                                                    
}