#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const Vec3f light_dir = Vec3f(0,0,-1);
Model *model = NULL;
const int width  = 800;
const int height = 800;


//Piqué dans le code / à changer
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

//Draw an empty triangle
void triangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, TGAColor color){
  line(v0, v1, image, color);
  line(v1, v2, image, color);
  line(v2, v0, image, color);
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

void triangleFullZBuffer(Vec3f v0, Vec3f v1, Vec3f v2, float* zBuffer, TGAImage& image, TGAColor color){

  //v0 higher than v1 higher than v2
  if (v2.y>v1.y) std::swap(v2, v1);
  if (v2.y>v0.y) std::swap(v2, v0);
  if (v1.y>v0.y) std::swap(v1, v0);

  float totalHeight = v0.y-v2.y;
  float bottomHeight = v1.y-v2.y;

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
      if (bary.x<0 || bary.y<0 || bary.z<0) continue;
      float z = 0;
      z += v0.y * bary.x;
      z += v1.y * bary.y;
      z += v2.y * bary.z;

      int idx = j+i*width;
      if (zBuffer[idx]<z) {
	zBuffer[idx] = z;
	image.set(j, i, color);
      }
    }
  }
  
  float topHeight = v0.y-v1.y;
    
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
      //rasterizing
      if (bary.x<0 || bary.y<0 || bary.z<0) continue;
      float z = 0;
      z += v0.y * bary.x;
      z += v1.y * bary.y;
      z += v2.y * bary.z;

      int idx = j+i*width;
      if (zBuffer[idx]<z) {
	zBuffer[idx] = z;
	image.set(j, i, color);
      }
    }
  }
}

void model3dEmptyTriangles(Model* model, TGAImage& image){

  //Pour toutes les faces
  for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i);
    //Pour les trois vecteurs du triangle
    for (int j=0; j<3; j++) {
      //On récupère les coordonnées du début et de la fin
      Vec3f v0 = model->vert(face[j]);
      Vec3f v1 = model->vert(face[(j+1)%3]);
      //On définit les coordonnées du triangle dans l'espacex 
      Vec2i ve0 = Vec2i((v0.x+1.)*width/2., (v0.y+1.)*height/2.) ;
      Vec2i ve1 = Vec2i((v1.x+1.)*width/2., (v1.y+1.)*height/2.);
      line(ve0, ve1, image, red);
    } 
  }
}

void model3dRandomTriangles(Model* model, TGAImage& image){

  //Pour toutes les faces
  for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i);
    Vec2i vvisu[3];
    //Pour les trois vecteurs du triangle
    for (int j=0; j<3; j++) {
      Vec3f vreel = model->vert(face[j]);
      //On définit les coordonnées du triangle dans l'espacex
      vvisu[j] = Vec2i((vreel.x+1.)*width/2., (vreel.y+1.)*height/2.);
    }
    triangleFull(vvisu[0], vvisu[1], vvisu[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
  }
}

void model3dBasicIntensity(Model* model, TGAImage& image){

  //Pour toutes les faces
  for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i);
    Vec2i vvisu[3];
    Vec3f vreels[3];
    //Pour les trois vecteurs du triangle
    for (int j=0; j<3; j++) {
      Vec3f vreel = model->vert(face[j]);
      //On définit les coordonnées du triangle dans l'espacex
      vvisu[j] = Vec2i((vreel.x+1.)*width/2., (vreel.y+1.)*height/2.);
      vreels[j] = vreel;
    }

    Vec3f normal = cross((vreels[2]-vreels[0]),(vreels[1]-vreels[0]));
    normal.normalize();

    float intens = normal*light_dir;
    if (intens > 0){
      triangleFull(vvisu[0], vvisu[1], vvisu[2], image, TGAColor(255*intens,255*intens,255*intens,255));
    }
  }
}

void model3dZbuffer(Model* model, TGAImage& image){

  float *zbuffer = new float[width*height];
  for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());
  
  //Pour toutes les faces
  for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i);
    Vec3f vvisu[3];
    Vec3f vreels[3];
    //Pour les trois vecteurs du triangle
    for (int j=0; j<3; j++) {
      Vec3f vreel = model->vert(face[j]);
      //On définit les coordonnées du triangle dans l'espacex
      vvisu[j] = Vec3f(int((vreel.x+1.)*width/2.+.5), int((vreel.y+1.)*height/2.+.5), vreel.z);
      vreels[j] = vreel;
    }

    Vec3f normal = cross((vreels[2]-vreels[0]),(vreels[1]-vreels[0]));
    normal.normalize();

    float intens = normal*light_dir;
    if (intens > 0){
      triangleFullZBuffer(vvisu[0], vvisu[1], vvisu[2], zbuffer, image, TGAColor(255*intens,255*intens,255*intens,255));
    }
  }
}


int main(int argc, char** argv){
  model = new Model(argv[1]);
  TGAImage i = TGAImage(width, height, TGAImage::RGB);
  model3dZbuffer(model, i);
  
  i.flip_vertically();
  i.write_tga_file("output.tga");

  delete model;
  return 0; 
}
