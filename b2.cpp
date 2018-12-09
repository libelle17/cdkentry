#include <iostream>
using namespace std;

struct Weapon
{
      Weapon() { cout << "Loading weapon features.\n"; }
      
      virtual void features()
         { cout << "Loading weapon features.\n"; }
};

struct Bomb : public Weapon
{
       void features()
         { 
            this->Weapon::features(); 
            cout << "Loading bomb features.\n"; 
         }
};

struct Gun : public Weapon
{
       void features()
         {
            this->Weapon::features(); 
            cout << "Loading gun features.\n"; 
         }
};

struct Loader
{
   public:
     void loadFeatures(Weapon *weapon)
     {  
        weapon->features();
     }     
};

int main()
{
    Loader *l = new Loader;
    Weapon *w;
    Bomb b;
    Gun g;

    w = &b;
    l->loadFeatures(w);

    w = &g;
    l->loadFeatures(w);

    return 0;
}
