
#include <cstdlib>
#include "App.hpp"


int main(int argc, char *argv[])
{
  App app;
  
  app.init( argc, argv);
  app.run();
  
  return EXIT_SUCCESS;
}
