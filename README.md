# Voxel-Cone-Tracing

Real Time Global Illumination With Voxel Cone Tracing

#Introduction

Voxel Cone Tracing is one of few algorithms that can create real time global ilummination and have a reputation to be notoriously difficult to implement correctly. Voxel Cone Tracing algorithm can be summarize as follow:
1. voxelize the scene and store all lighting information into a 3D Texture. In Crassin paper, they use a Sparse Voxel Octree(SVO) to save storage space, but that alone is very difficult to implement correctly so I only use 3D Texture, which have different level of Mipmap and can provide what SVO can offer.
2. Render the scene to a Shadow Buffer.
3. We now have a 3D Texture and a Shadow Buffer, Voxel Cone Tracing now work quite similar to Ray Tracing. Rays are shot throught each pixel and when they intersect with a triangle, we create reflectance Ray and trace them a long on all 6 "Precompute" reflectance directions. As we trace, we add lighting value in 3D Texture and shadow value in Depth Buffer. PCF shadow mapping is used to create soft shadow.
4. unlike ray marching, voxel cone tracing increase the traveling distance at each step by a constant of factor 2 and these voxel form a cone shape and give it the name "Voxel cone Tracing", and at each step, we also increase the level of voxel texture (3D Texture).

![Illustration-of-the-sampling-scheme-employed-by-voxel-cone-tracing-the-cone-radius-at](https://user-images.githubusercontent.com/93391908/154071894-e8865967-62da-4857-b2c4-3a2b7d2221de.png)

5. Algorithm stop when we reach a certain distance or occlusion threshold. 

I really want to use the old source code form my OpenGL object loading, but Assimp make the code much more cleaner and easier to read so I simp for Assimp. The Assimp code is derived from LearnOpenGL.com.

Voxel Cone Tracing is so hard, Damn it.

Voxel Cone Tracing is the second Herculean task I have ever accomplished, and second in term of scale (Physical based rendering with Lux is the biggest), wink wink.


![vct1](https://user-images.githubusercontent.com/93391908/151729015-72fd47c6-2511-4daa-bc71-6314f122057e.png)
![vct2](https://user-images.githubusercontent.com/93391908/151729020-1ddbf8c4-a46e-4094-93a7-88aff041c7b6.png)
