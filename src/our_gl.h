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
Matrix modelView;
Matrix viewPort;
Matrix projection;

// Interface des shaders
struct IShader {
  virtual ~IShader(){};
  virtual Vec4f vertex(int iface, int nthvert) = 0;
  virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

// Calcul des matrices de projection
void viewport(int x, int y, int w, int h) {
    viewPort = Matrix::identity();
    viewPort[0][3] = x+w/2.f;
    viewPort[1][3] = y+h/2.f;
    viewPort[2][3] = 255.f/2.f;
    viewPort[0][0] = w/2.f;
    viewPort[1][1] = h/2.f;
    viewPort[2][2] = 255.f/2.f;
}
void project(float coeff) {
    projection = Matrix::identity();
    projection[3][2] = coeff;
}
void lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    modelView = Matrix::identity();
    for (int i=0; i<3; i++) {
        modelView[0][i] = x[i];
        modelView[1][i] = y[i];
        modelView[2][i] = z[i];
        modelView[i][3] = -center[i];
    }
}

// Calcul du barycentre d'un triangle
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, float Px, float Py) {
  Vec3f s[2];
  Vec3f P = Vec3f(Px, Py, 0);
  for (int i=2; i--; ) {
    s[i][0] = C[i]-A[i];
    s[i][1] = B[i]-A[i];
    s[i][2] = A[i]-P[i];
  }
  Vec3f u = cross(s[0], s[1]);
  if (std::abs(u[2])>1e-2) // Cas dégénéré
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
  return Vec3f(-1,1,1);
}

// Pareil avec des Vec4f
Vec3f barycentric(Vec4f A, Vec4f B, Vec4f C, float Px, float Py) {
  Vec3f s[2];
  Vec3f P = Vec3f(Px, Py, 0);
  for (int i=2; i--; ) {
    s[i][0] = C[i]-A[i];
    s[i][1] = B[i]-A[i];
    s[i][2] = A[i]-P[i];
  }
  Vec3f u = cross(s[0], s[1]);
  if (std::abs(u[2])>1e-2)
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
  return Vec3f(-1,1,1); 
}


//Dessine un triangle plein en utilisant un zbuffer pour savoir ce qui est au premier plan + textures
void triangleFullZBuffer(Vec3f v0, Vec3f v1, Vec3f v2, float* zBuffer, Vec2f* UVs, float ecl, TGAImage& image, Model* model){

  TGAColor color;
  
  //v0 higher than v1 higher than v2
  if (v2[1]>v1[1]) { std::swap(v2, v1); std::swap(UVs[2], UVs[1]); }
  if (v2[1]>v0[1]) { std::swap(v2, v0); std::swap(UVs[2], UVs[0]); }
  if (v1[1]>v0[1]) { std::swap(v1, v0); std::swap(UVs[1], UVs[0]); }

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
      z += v0[2] * bary[0];
      z += v1[2] * bary[1];
      z += v2[2] * bary[2];

      float u = 0;
      u += UVs[0][0] * bary[0];
      u += UVs[1][0] * bary[1];
      u += UVs[2][0] * bary[2];

      float v = 0;
      v += UVs[0][1] * bary[0];
      v += UVs[1][1] * bary[1];
      v += UVs[2][1] * bary[2];
      
      color = model->diffuse(Vec2f(u,v));
      
      if (j < 0 || j >= width || i < 0 || i >= height) continue;
      int idx = int(j)+int(i)*width;
      // Zbuffering
      if (zBuffer[idx]<z) {

	  zBuffer[idx] = z;
	  image.set(j, i, color*ecl);

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
      z += v0[2] * bary[0];
      z += v1[2] * bary[1];
      z += v2[2] * bary[2];

      float u = 0;
      u += UVs[0][0] * bary[0];
      u += UVs[1][0] * bary[1];
      u += UVs[2][0] * bary[2];

      float v = 0;
      v += UVs[0][1] * bary[0];
      v += UVs[1][1] * bary[1];
      v += UVs[2][1] * bary[2];
      
      color = model->diffuse(Vec2f(u,v));


      if (j < 0 || j >= width || i < 0 || i >= height) continue;
      int idx = (int)j+(int)i*width;

      // Zbuffering
      if (zBuffer[idx]<z) {

	  zBuffer[idx] = z;
	  image.set(j, i, color*ecl);

      }
    }
  }
}

// Dessine un triangle en utilisant un shader pour récupérer les couleurs
void triangleShader(Vec4f v0, Vec4f v1, Vec4f v2, float* zBuffer, TGAImage& image, IShader &shader, Model* model){

  TGAColor color;
  
  //v0 higher than v1 higher than v2
  if (v2[1]>v1[1]) { std::swap(v2, v1);}
  if (v2[1]>v0[1]) { std::swap(v2, v0);}
  if (v1[1]>v0[1]) { std::swap(v1, v0);}

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
      z += v0[2] * bary[0];
      z += v1[2] * bary[1];
      z += v2[2] * bary[2];


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
      z += v0[2] * bary[0];
      z += v1[2] * bary[1];
      z += v2[2] * bary[2];
  
      if (j < 0 || j >= width || i < 0 || i >= height) continue;
      int idx = (int)j+(int)i*width;

      if (zBuffer[idx]<z) {
	color = white;

	if (!shader.fragment(bary, color)){ //Shaderisation
	  zBuffer[idx] = z;
	  image.set(j, i, color);
	}
      }
    }
  }
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

//Dessine une ligne entre deux vecteurs (Vec4f)
void line(Vec4f v0, Vec4f v1, TGAImage &image, TGAColor color) {
  int x0 = v0[0], y0 = v0[1];
  int x1 = v1[0], y1 = v1[1];
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

// Dessine une ligne entre deux vecteurs (Vec2i)
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


//Dessine un triangle plein
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

//Dessine un triangle vide
void triangle(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage& image, TGAColor color){
  line(v0, v1, image, color);
  line(v1, v2, image, color);
  line(v2, v0, image, color);
}

void triangle(Vec4f v0, Vec4f v1, Vec4f v2, TGAImage& image, TGAColor color){
  line(v0, v1, image, color);
  line(v1, v2, image, color);
  line(v2, v0, image, color);
}

#endif
