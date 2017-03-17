#include "tgaimage.h"
#include <cstdlib>
#include <cmath>

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
  bool steep = false; 
  if (std::abs(x0-x1)<std::abs(y0-y1)) { // if the line is steep, we transpose the image 
    std::swap(x0, y0); 
    std::swap(x1, y1); 
    steep = true; 
  }
    
  if (x0>x1) { // make it left−to−right 
    std::swap(x0, x1); 
    std::swap(y0, y1); 
  }
    
  for (int x=x0; x<=x1; x++) { 
    float t = (x-x0)/(float)(x1-x0); 
    int y = (int)round(y0*(1.-t) + y1*t); 
    if (steep) { 
      image.set(y, x, color); // if transposed, de−transpose 
    } else { 
      image.set(x, y, color); 
    } 
  } 
}

void line2(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
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
  float derror = std::abs(dy/float(dx)); 
  float error = 0; 
  int y = y0; 
  for (int x=x0; x<=x1; x++) { 
    if (steep) { 
      image.set(y, x, color); 
    } else { 
      image.set(x, y, color); 
    } 
    error += derror; 
    if (error>.5) { 
      y += (y1>y0?1:-1); 
      error -= 1.; 
    } 
  } 
} 

int main(int argc, char** argv){
  TGAColor red = TGAColor(255, 0, 0, 255);
  TGAColor blue = TGAColor(0, 0, 255, 255);
  TGAColor green = TGAColor(0, 255, 0, 255);
  TGAColor white = TGAColor(255, 255, 255, 255);
  TGAImage i = TGAImage(100, 100, TGAImage::RGB);
  line2(5, 5, 50, 50, i, red);
  line2(50, 50, 5, 5, i, blue);
  line2(5, 5, 5, 75, i, green);
  line2(13, 20, 80, 40, i, white); 
  line2(20, 13, 40, 80, i, red); 
  line2(80, 40, 13, 20, i, red);
  i.flip_vertically();
  i.write_tga_file("output.tga");
  return 0;
}
