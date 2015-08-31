//!!ARBfp1.0   
#version 330 core

//# tex0 : 2D lookup texture (diffuse term)              
//# tex1 : 2D lookup texture (specular term)             
//# tc0  : tangent vector                                
//# tc1  : light vector                                  
//# tc2  : view vector          
uniform sampler2D tex0;
uniform sampler2D tex1;

//# ******* attributes *********                         
//                                                       
//# texture coordinate in [0, 1]^3                       
//ATTRIB tangent = fragment.texcoord[0];                 
//ATTRIB light_v = fragment.texcoord[1];                 
//ATTRIB view    = fragment.texcoord[2];       
in vec3 vcolor;

//# ******* parameters *********                         
//                                                       
//PARAM spec_exp = state.light[0].attenuation;           
//PARAM const    = { 0, 0.5, 1, 2 };            
uniform float spec_exp;

//# ******** program *********                           
//                               
void main()
{
	//# ******** temporaries *********                       
	//                                                       
	//# tangent, normal and binormal basis                   
	//TEMP tan;                                              
	//TEMP nrm;                                              
	//TEMP bin;                                              
	//                                                       
	//# halfway vector                                       
	//TEMP half;                                             


	//# ******** aliases *********                           
	//                                                       
	//ALIAS lu_tc = bin;                                     
	//ALIAS diff  = nrm;                                     
	//ALIAS spec  = tan;                                     
	//ALIAS light = half;                                    
	//ALIAS sqr_f = half;                                    


	//# compute local basis                                  
	//                                                       
	//# tan = tangent / |tangent|                            
	//DP3 tan.w, tangent, tangent;                           
	//RSQ tan.w, tan.w;                                      
	//MUL tan.xyz, tangent, tan.w;                           


	//# bin = tangent * view, bin = bin / |bin|              
	//XPD bin, tangent, view;                                
	//DP3 bin.w, bin, bin;                                   
	//RSQ bin.w, bin.w;                                      
	//MUL bin.xyz, bin, bin.w;                               


	//# nrm = bin * tan                                      
	//XPD nrm, bin, tan;                                     


	//# compute normalized light direction                   
	//DP3 light.w, light_v, light_v;                         
	//RSQ light.w, light.w;                                  
	//MUL light, light_v, light.w;                           


	//# lu_tc.xy = light . { nrm, tan }                      
	//DP3 lu_tc.x, light, nrm;                               
	//DP3 lu_tc.y, light, tan;                               


	//# compute normalized half vector                       
	//DP3 half.w, view, view;                                
	//RSQ half.w, half.w;                                    
	//MAD half.xyz, view, half.w, light;                     
	//DP3 half.w, half, half;                                
	//RSQ half.w, half.w;                                    
	//MUL half.xyz, half, half.w;                            


	//# lu_tc.zw = half . { nrm, tan }                       
	//DP3 lu_tc.z, half, nrm;                                
	//DP3 lu_tc.w, half, tan;                                


	//# 1/sqrt(1- {light, half} . tan ^ 2)                   
	//MAD sqr_f.zw, lu_tc.xzyw, -lu_tc.xzyw, const.z;        
	//RSQ sqr_f.z, sqr_f.z;                                  
	//RSQ sqr_f.w, sqr_f.w;                                  
	//MUL lu_tc.zw, lu_tc.yzxz, sqr_f;                       


	//# make tex coords lie in [0,1]                         
	//MAD lu_tc, lu_tc, const.y, const.y;                    


	//# read diffuse factor from tex                         
	//TEX diff.x, lu_tc.zyxw, texture[0], 2D;                


	//# read spec factor from tex                            
	//TEX spec.x, lu_tc.zwxy, texture[1], 2D;                


	//# (1 - half . tan^2)^(exp/2)                           
	//POW lu_tc.w, sqr_f.w, -spec_exp.w;                     


	//MUL_SAT spec.w, spec.x, lu_tc;                         


	//# combine                                              
	//MAD result.color.xyz, diff.x, fragment.color, spec.w;  
	//# keep the alpha value of the fragment                 
	//MOV result.color.w, fragment.color;                    


//END                                                    
}                    