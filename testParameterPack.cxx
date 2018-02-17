//
// Created by Gabriele Gaetano Fronzé on 16/02/2018.
//


class Foo{
  public:
    int fIsOk;
    bool fIsNotOk;
    bool fTheThird;
    Foo() : fIsOk(5),fIsNotOk(false),fTheThird(true){}
    inline bool isOk() const {return fIsOk; }
    inline bool isNotOk() const {return fIsNotOk; }
    inline bool theThird() const {return fTheThird; }
    inline int is1(int yours1) const {return fIsOk+yours1; }
    inline bool is2(bool yours1,bool yours2) const {return fTheThird && yours1 && yours2; }
    inline bool getTrue() const {return true;}
};

class User{
  public:
    User() : pippo(){};
    template<typename T,class ...Args> T doIt(int i=2, T (Foo::*condition)(Args...)const = &Foo::getTrue,Args... args){
      return (pippo.*condition)(args...)+i;
    }
  private:
    Foo pippo;
};

int testParameterPack(){
  User pluto;
  std::cout<<pluto.doIt(1,&Foo::isOk)<<std::endl;
  std::cout<<pluto.doIt(2,&Foo::is1,1)<<std::endl;
  std::cout<<pluto.doIt(3,&Foo::is2,true,false)<<std::endl;
  return 0;
}

