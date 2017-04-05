#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const Vec3f light_dir = Vec3f(0,0,1);
Model *model = NULL;
const int width  = 800;
const int height = 800;

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

    Vec3f normal = (vreels[2]-vreels[0])^(vreels[1]-vreels[0]);
    normal.normalize();

    float intens = normal*light_dir;
    if (intens > 0){
      triangleFull(vvisu[0], vvisu[1], vvisu[2], image, TGAColor(255*intens,255*intens,255*intens,255));
    }
  }
}

int main(int argc, char** argv){
  model = new Model(argv[1]);
  TGAImage i = TGAImage(width, height, TGAImage::RGB);
  model3dBasicIntensity(model, i);

  i.flip_vertically();
  i.write_tga_file("output.tga");

  delete model;
  return 0; 
}



/*
 - Améliorer dessin triangles (chap. 2)
*/
