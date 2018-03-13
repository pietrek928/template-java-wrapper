/* File : example.h */

class Square {
  Square() {
    //nshapes++;
  }
  virtual ~Square() {
    //nshapes--;
  };

private:
  double  x, y;   
  double width;

public:
  Square(double w) : width(w) { };
  void    move(double dx, double dy);
  double area();
  double perimeter();
};

