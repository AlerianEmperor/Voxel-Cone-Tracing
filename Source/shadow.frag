#version 430 core

void main() {
	//gl_FragDepth = gl_FragCoord.z;
	//上面这行代码无论有没有都会执行相应的操作，即把gl_FragCoord.z写进深度
	//理论上来说不写的话效率高一点，但是差别很小，基本感觉不到
}