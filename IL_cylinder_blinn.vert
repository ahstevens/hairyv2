//!!ARBvp1.0
#version 330 core

//# ******* attributes *********
//
//ATTRIB ipos     = vertex.position;                     
//ATTRIB itangent = vertex.texcoord[0];     
layout(location = 0) in vec3 ipos;
layout(location = 1) in vec3 itangent;
layout(location = 2) in vec4 vertex_color;

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
	//TEMP eye_pos;                                          
	//        
	vec4 eye_pos;

	//# transform vertex position to camera coordinates      
	//DP4 eye_pos.x, mv_mat[0], ipos;                        
	//DP4 eye_pos.y, mv_mat[1], ipos;                        
	//DP4 eye_pos.z, mv_mat[2], ipos;                        
	//DP4 eye_pos.w, mv_mat[3], ipos;     
	eye_pos = mv_mat * vec4( ipos, 1.0 );

	//# compute tangent                                      
	//DP3 otangent.x, mv_mat[0], itangent;                   
	//DP3 otangent.y, mv_mat[1], itangent;                   
	//DP3 otangent.z, mv_mat[2], itangent;                   
	//MOV otangent.w, const.z;     
	otangent = vec3( mv_mat ) * itangent;
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
	gl_Position = mvp_mat * vec4( ipos, 1.0 );

	//# pass through color
	//MOV result.color, vertex.color;
	result_color = vertex_color;

//END
}