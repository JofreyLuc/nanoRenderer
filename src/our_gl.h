#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

#ifndef OURGL
#define OURGL

const int width  = 1000;
const int height = 1000;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

struct IShader {
  virtual ~IShader(){};
  virtual Vec4f vertex(int iface, int nthvert) = 0;
  virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

//Piqué dans le code
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, float Px, float Py) {
  Vec3f s[2];
  Vec3f P = Vec3f(Px, Py, 0);
  for (int i=2; i--; ) {
    s[i][0] = C[i]-A[i];
    s[i][1] = B[i]-A[i];
    s[i][2] = A[i]-P[i];
  }
  Vec3f u = cross(s[0], s[1]);
  if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
  return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

Vec3f barycentric(Vec4f A, Vec4f B, Vec4f C, float Px, float Py) {
  Vec3f s[2];
  Vec3f P = Vec3f(Px, Py, 0);
  for (int i=2; i--; ) {
    s[i][0] = C[i]-A[i];
    s[i][1] = B[i]-A[i];
    s[i][2] = A[i]-P[i];
  }
  Vec3f u = cross(s[0], s[1]);
  if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
  return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}


//Piqué dans le code : regarde un point
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
  Vec3f z = (eye-center).normalize();
  Vec3f x = cross(up,z).normalize();
  Vec3f y = cross(z,x).normalize();
  Matrix Minv = Matrix::identity();
  Matrix Tr   = Matrix::identity();
  for (int i=0; i<3; i++) {
    Minv[0][i] = x[i];
    Minv[1][i] = y[i];
    Minv[2][i] = z[i];
    Tr[i][3] = -center[i];
  }
  Matrix m = (Minv*Tr);
  return m;
}

//Piqué dans le code : projette sur un cube (coordonnées réeelles -> coordonnées écran)
Matrix viewport(int x, int y, int w, int h) {
  Matrix m = Matrix::identity();
  m[0][3] = x+w/2.f;
  m[1][3] = y+h/2.f;

  m[0][0] = w/2.f;
  m[1][1] = h/2.f;
  return m;
}

//Dessine une ligne entre deux points
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
  bool steep = false; 
  if (std::abs(x0-x1)<std::abs(y0-y1)) { 
    std::swap(x0, y0); 
    std::swap(x1, y1); 
    steep = true; 
  } 
  if (x0>x1) { 
    std::swap(x0, x1); 
    std::swap(y0, y1); 
  } 
  int dx = x1-x0; 
  int dy = y1-y0; 
  int derror2 = std::abs(dy)*2; 
  int error2 = 0; 
  int y = y0; 
  for (int x=x0; x<=x1; x++) { 
    if (steep) { 
      image.set(y, x, color); 
    } else { 
      image.set(x, y, color); 
    } 
    error2 += derror2; 
    if (error2 > dx) { 
      y += (y1>y0?1:-1); 
      error2 -= dx*2; 
    } 
  } 
}

//Dessine une ligne entre deux vecteurs
void line(Vec3f v0, Vec3f v1, TGAImage &image, TGAColor color) {
  int x0 = v0.x, y0 = v0.y;
  int x1 = v1.x, y1 = v1.y;
  bool steep = false; 
  if (std::abs(x0-x1)<std::abs(y0-y1)) { 
    std::swap(x0, y0); 
    std::swap(x1, y1); 
    steep = true; 
  } 
  if (x0>x1) { 
    std::swap(x0, x1); 
    std::swap(y0, y1); 
  } 
  int dx = x1-x0; 
  int dy = y1-y0; 
  int derror2 = std::abs(dy)*2; 
  int error2 = 0; 
  int y = y0; 
  for (int x=x0; x<=x1; x++) { 
    if (steep) { 
      image.set(y, x, color); 
    } else { 
      image.set(x, y, color); 
    } 
    error2 += derror2; 
    if (error2 > dx) { 
      y += (y1>y0?1:-1); 
      error2 -= dx*2; 
    } 
  } 
} 

void line(Vec2i v0, Vec2i v1, TGAImage &image, TGAColor color) {
  int x0 = v0.x, y0 = v0.y;
  int x1 = v1.x, y1 = v1.y;
  bool steep = false; 
  if (std::abs(x0-x1)<std::abs(y0-y1)) { 
    std::swap(x0, y0); 
    std::swap(x1, y1); 
    steep = true; 
  } 
  if (x0>x1) { 
    std::swap(x0, x1); 
    std::swap(y0, y1); 
  } 
  int dx = x1-x0; 
  int dy = y1-y0; 
  int derror2 = std::abs(dy)*2; 
  int error2 = 0; 
  int y = y0; 
  for (int x=x0; x<=x1; x++) { 
    if (steep) { 
      image.set(y, x, color); 
    } else { 
      image.set(x, y, color); 
    } 
    error2 += derror2; 
    if (error2 > dx) { 
      y += (y1>y0?1:-1); 
      error2 -= dx*2; 
    } 
  } 
} 


//Draw a triangle filled with color
void triangleFull(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, TGAColor color){

  //v0 higher than v1 higher than v2
  if (v2.y>v1.y) std::swap(v2, v1); 
  if (v2.y>v0.y) std::swap(v2, v0); 
  if (v1.y>v0.y) std::swap(v1, v0); 

  int totalHeight = v0.y-v2.y;
  int bottomHeight = v1.y-v2.y;

  for (int i=v2.y; i<=v1.y; i++){
    float alpha = 0;
    if (totalHeight!=0) alpha = (float)(i-v2.y)/totalHeight; 
    float beta = 0;
    if (bottomHeight!=0) beta  = (float)(i-v2.y)/bottomHeight;
    Vec2i A = Vec2i((v2.x + (v0.x-v2.x)*alpha), i); 
    Vec2i B = Vec2i((v2.x + (v1.x-v2.x)*beta), i);
    line(A, B, image, color);
  }

  int topHeight = v0.y-v1.y;
  
  for (int i=v1.y; i<=v0.y; i++){
    float alpha = 0;
    if (totalHeight!=0) alpha = (float)(i-v2.y)/totalHeight; 
    float beta = 0;
    if (topHeight!=0) beta  = (float)(i-v1.y)/topHeight;
    Vec2i A = Vec2i((v2.x + (v0.x-v2.x)*alpha),i); 
    Vec2i B = Vec2i((v1.x + (v0.x-v1.x)*beta),i);  
    line(A, B, image, color);
  }
}

//Dessine un triangle plein en utilisant un zbuffer pour savoir ce qui est au premier plan + textures
void triangleFullZBuffer(Vec3f v0, Vec3f v1, Vec3f v2, float* zBuffer, Vec2f* UVs, float ecl, TGAImage& image, IShader &shader, Model* model){

  TGAColor color;
  
  //v0 higher than v1 higher than v2
  if (v2.y>v1.y) { std::swap(v2, v1); std::swap(UVs[2], UVs[1]); }
  if (v2.y>v0.y) { std::swap(v2, v0); std::swap(UVs[2], UVs[0]); }
  if (v1.y>v0.y) { std::swap(v1, v0); std::swap(UVs[1], UVs[0]); }

  float totalHeight = v0.y-v2.y;
  float bottomHeight = v1.y-v2.y;

  //Partie inférieure
  for (float i=v2.y; i<=v1.y; i++){
    float alpha = 0;
    if (totalHeight!=0) alpha = (float)(i-v2.y)/totalHeight; 
    float beta = 0;
    if (bottomHeight!=0) beta  = (float)(i-v2.y)/bottomHeight;
    Vec2f A = Vec2f((v2.x + (v0.x-v2.x)*alpha), i); 
    Vec2f B = Vec2f((v2.x + (v1.x-v2.x)*beta), i);
    if (A.x > B.x) std::swap(A, B);

    for (float j=A.x; j < B.x; j++){
      Vec3f bary = barycentric(v0, v1, v2, j, i);
      //rasterizing
      float z = 0;
      z += v0.z * bary.x;
      z += v1.z * bary.y;
      z += v2.z * bary.z;

      float u = 0;
      u += UVs[0].x * bary.x;
      u += UVs[1].x * bary.y;
      u += UVs[2].x * bary.z;

      float v = 0;
      v += UVs[0].y * bary.x;
      v += UVs[1].y * bary.y;
      v += UVs[2].y * bary.z;
      
      color = model->diffuse(Vec2f(u,v));

      if (j < 0 || j >= width || i < 0 || i >= height) continue;
      int idx = int(j)+int(i)*width;      
      if (zBuffer[idx]<z) {

	if (!shader.fragment(bary, color)){ //Shaderisation
	  zBuffer[idx] = z;
	  image.set(j, i, color*ecl);
	}
      }
    }
  }
  float topHeight = v0.y-v1.y;

  //Partie supérieure
  for (float i=v1.y; i<=v0.y; i++){
    float alpha = 0;
    if (totalHeight!=0) alpha = (float)(i-v2.y)/totalHeight; 
    float beta = 0;
    if (topHeight!=0) beta  = (float)(i-v1.y)/topHeight;
    Vec2f A = Vec2f((v2.x + (v0.x-v2.x)*alpha),i); 
    Vec2f B = Vec2f((v1.x + (v0.x-v1.x)*beta),i);  
    if (A.x > B.x) std::swap(A, B);
    for (float j=A.x; j < B.x; j++){

      Vec3f bary = barycentric(v0, v1, v2, j, i);
      //interpolation
      float z = 0;
      z += v0.z * bary.x;
      z += v1.z * bary.y;
      z += v2.z * bary.z;

      float u = 0;
      u += UVs[0].x * bary.x;
      u += UVs[1].x * bary.y;
      u += UVs[2].x * bary.z;

      float v = 0;
      v += UVs[0].y * bary.x;
      v += UVs[1].y * bary.y;
      v += UVs[2].y * bary.z;
      
      color = model->diffuse(Vec2f(u,v));

      if (j < 0 || j >= width || i < 0 || i >= height) continue;
      int idx = (int)j+(int)i*width;

      if (zBuffer[idx]<z) {

	if (!shader.fragment(bary, color)){ //Shaderisation
	  zBuffer[idx] = z;
	  image.set(j, i, color*ecl);
	}

      }
    }
  }
}

void triangleShader(Vec4f v0, Vec4f v1, Vec4f v2, float* zBuffer, TGAImage& image, IShader &shader, Model* model){

  TGAColor color;
  
  //v0 higher than v1 higher than v2
  if (v2[1]>v1[1]) { std::swap(v2, v1); /*std::swap(UVs[2], UVs[1]);*/ }
  if (v2[1]>v0[1]) { std::swap(v2, v0); /*std::swap(UVs[2], UVs[0]);*/ }
  if (v1[1]>v0[1]) { std::swap(v1, v0); /*std::swap(UVs[1], UVs[0]);*/ }

  float totalHeight = v0[1]-v2[1];
  float bottomHeight = v1[1]-v2[1];

  //Partie inférieure
  for (float i=v2[1]; i<=v1[1]; i++){
    float alpha = 0;
    if (totalHeight!=0) alpha = (float)(i-v2[1])/totalHeight; 
    float beta = 0;
    if (bottomHeight!=0) beta  = (float)(i-v2[1])/bottomHeight;
    Vec2f A = Vec2f((v2[0] + (v0[0]-v2[0])*alpha), i); 
    Vec2f B = Vec2f((v2[0] + (v1[0]-v2[0])*beta), i);
    if (A[0] > B[0]) std::swap(A, B);

    for (float j=A[0]; j < B[0]; j++){
      Vec3f bary = barycentric(v0, v1, v2, j, i);
      //rasterizing
      float z = 0;
      z += v0[2] * bary.x;
      z += v1[2] * bary.y;
      z += v2[2] * bary.z;

      /*float u = 0;
      u += UVs[0].x * bary.x;
      u += UVs[1].x * bary.y;
      u += UVs[2].x * bary.z;

      float v = 0;
      v += UVs[0].y * bary.x;
      v += UVs[1].y * bary.y;
      v += UVs[2].y * bary.z;
      
      color = model->diffuse(Vec2f(u,v));*/

      if (j < 0 || j >= width || i < 0 || i >= height) continue;
      int idx = int(j)+int(i)*width;      
      if (zBuffer[idx]<z) {

	if (!shader.fragment(bary, color)){ //Shaderisation
	  zBuffer[idx] = z;
	  image.set(j, i, color);
	}
      }
    }
  }
  float topHeight = v0[1]-v1[1];

  //Partie supérieure
  for (float i=v1[1]; i<=v0[1]; i++){
    float alpha = 0;
    if (totalHeight!=0) alpha = (float)(i-v2[1])/totalHeight; 
    float beta = 0;
    if (topHeight!=0) beta  = (float)(i-v1[1])/topHeight;
    Vec2f A = Vec2f((v2[0] + (v0[0]-v2[0])*alpha),i); 
    Vec2f B = Vec2f((v1[0] + (v0[0]-v1[0])*beta),i);  
    if (A[0] > B[0]) std::swap(A, B);
    for (float j=A[0]; j < B[0]; j++){

      Vec3f bary = barycentric(v0, v1, v2, j, i);
      //interpolation
      float z = 0;
      z += v0[2] * bary.x;
      z += v1[2] * bary.y;
      z += v2[2] * bary.z;

      /*float u = 0;
      u += UVs[0].x * bary.x;
      u += UVs[1].x * bary.y;
      u += UVs[2].x * bary.z;

      float v = 0;
      v += UVs[0].y * bary.x;
      v += UVs[1].y * bary.y;
      v += UVs[2].y * bary.z;
      
      color = model->diffuse(Vec2f(u,v));*/
      
      if (j < 0 || j >= width || i < 0 || i >= height) continue;
      int idx = (int)j+(int)i*width;

      if (zBuffer[idx]<z) {

	if (!shader.fragment(bary, color)){ //Shaderisation
	  zBuffer[idx] = z;
	  image.set(j, i, color);
	}

      }
    }
  }
}


//Draw an empty triangle
void triangle(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage& image, TGAColor color){
  line(v0, v1, image, color);
  line(v1, v2, image, color);
  line(v2, v0, image, color);
}




#endif
