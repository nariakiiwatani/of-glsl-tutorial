#version 120

void main() {
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix*(gl_Vertex+vec4(100,0,0,0));
}