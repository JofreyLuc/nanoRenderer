#include "our_gl.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

Model *model = NULL;
const Vec3f light_dir = Vec3f(1,1,1);
const Vec3f camera = Vec3f(1, 1, 6);
const Vec3f center = Vec3f(0, 0, 0);
const Vec3f up = Vec3f(0,1,0);


Matrix modelView;
Matrix viewPort;
Matrix projection;

struct Gouraud_Shader : public IShader {
  Vec3f varying_intensity;
  mat<2,3,float> varying_uv; 
  
  // Renvoie le sommet du fichier obj (met varying_intensity du sommet à 0 si non visible)
  virtual Vec4f vertex(int iface, int nthvert) {
    varying_uv.set_col(nthvert, model->uv(iface, nthvert));
    varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir);
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); 
    return viewPort * projection * modelView * gl_Vertex;
  }

  // "Intensifie" la couleur en interpolant les coords bary
  virtual bool fragment(Vec3f bar, TGAColor &color) {
    float intensity = varying_intensity*bar; // Interpol
    Vec2f uv = varying_uv*bar;               // Interpol uv
    color = model->diffuse(uv)*intensity;
    return false; // On accepte tous les pixels pour le moment
    }
};


void model3dZbuffer(Model* model, TGAImage& image){
  float *zbuffer = new float[width*height];
  for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

  modelView = lookat(camera, center, up);
  viewPort = viewport(width/8, height/8, width*3/4, height*3/4);
  projection = Matrix::identity();
  projection[3][2] = -1. / camera.z;

  Gouraud_Shader shader;
  //Pour toutes les faces
  for (int i=0; i<model->nfaces(); i++) {
    Vec4f screen_coords[3];
    for (int j=0; j<3; j++) {
      screen_coords[j] = shader.vertex(i, j);
    }
    triangleShader(screen_coords[0],screen_coords[1],screen_coords[2], zbuffer, image, shader,  model);
  }
  
  { // dump z-buffer (debugging purposes only)
    TGAImage zbimage(width, height, TGAImage::RGBA);
    float zbmin = 1e10, zbmax = -1e10;
    for (int i=0; i<width*height; i++) {
      if (zbuffer[i]<-1e10) continue;
      zbmin = std::min(zbmin, zbuffer[i]);
      zbmax = std::max(zbmax, zbuffer[i]);
    }
	
    for (int i=0; i<width; i++) {
      for (int j=0; j<height; j++) {
	float z = zbuffer[i+j*width];
	if (z<-1e10) continue;
	z = 255*(z-zbmin)/(zbmax-zbmin);
	zbimage.set(i, j, TGAColor(z,z,z,255 ));
      }
    }
    zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    zbimage.write_tga_file("zbuffer.tga");
  }
}




/*void model3dZbuffer(Model* model, TGAImage& image){
  float *zbuffer = new float[width*height];
  for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

  modelView = lookat(camera, center, up);
  viewPort = viewport(width/8, height/8, width*3/4, height*3/4);

  //Pour toutes les faces
  for (int i=0; i<model->nfaces(); i++) { 
  std::vector<int> face = model->face(i);
  Vec3f vvisu[3];
  Vec3f vreels[3];
  Vec2f UVs[3]; //Coordonnées texture
  float transfo_proj; //Valeur de transformation pour la projection
  //Pour les trois vecteurs du triangle
  for (int j=0; j<3; j++) {
  Vec3f vreel = model->vert(face[j]);
  //On définit les coordonnées du triangle dans l'espacex
  Matrix matVreel;
  matVreel[0][0] = vreel.x;
  matVreel[1][0] = vreel.y;
  matVreel[2][0] = vreel.z;
  matVreel[3][0] = 1;

  projection = Matrix::identity();
  projection[3][2] = -1. / camera.z;
      
  Matrix matVvisu = projection * modelView * matVreel;

  for (int ii=0; ii<4; ii++)
  matVvisu[ii][0] = matVvisu[ii][0]/matVvisu[3][0];

  matVvisu = viewPort * matVvisu;
      
  vvisu[j] = Vec3f(matVvisu[0][0],matVvisu[1][0],matVvisu[2][0]);

  //Pour les textures
  vreels[j] = vreel;
  UVs[j] = model->uv(i, j);
  }

  //Calcul vecteur normal au triangle pour intensité
  Vec3f normal = cross((vreels[2]-vreels[0]),(vreels[1]-vreels[0]));
  normal.normalize();
  float intens = 1;//normal*light_dir;
  if (intens > 0){
  Gouraud_Shader shader;
  triangleFullZBuffer(vvisu[0], vvisu[1], vvisu[2], zbuffer, UVs, intens, image, shader, model);
  }
  }
  
  { // dump z-buffer (debugging purposes only)
  TGAImage zbimage(width, height, TGAImage::RGBA);
  float zbmin = 1e10, zbmax = -1e10;
  for (int i=0; i<width*height; i++) {
  if (zbuffer[i]<-1e10) continue;
  zbmin = std::min(zbmin, zbuffer[i]);
  zbmax = std::max(zbmax, zbuffer[i]);
  }
	
  for (int i=0; i<width; i++) {
  for (int j=0; j<height; j++) {
  float z = zbuffer[i+j*width];
  if (z<-1e10) continue;
  z = 255*(z-zbmin)/(zbmax-zbmin);
  zbimage.set(i, j, TGAColor(z,z,z,255 ));
  }
  }
  zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
  zbimage.write_tga_file("zbuffer.tga");
  }
  }*/


  int main(int argc, char** argv){
  model = new Model(argv[1]);
  TGAImage i = TGAImage(width, height, TGAImage::RGB);
  model3dZbuffer(model, i);
  
  i.flip_vertically();
  i.write_tga_file("output.tga");

  delete model;
  return 0; 
  }




  //Stuff


  /*void model3dEmptyTriangles(Model* model, TGAImage& image){

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
*/
