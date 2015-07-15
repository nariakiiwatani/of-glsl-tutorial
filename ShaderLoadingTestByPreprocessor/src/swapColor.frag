#ifndef GLSL120
#define GLSL120(...) \
"#version 120\n"\
#__VA_ARGS__
#endif
GLSL120(
void main() {
	gl_FragColor = gl_Color.bgra;
}
)