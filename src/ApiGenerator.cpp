#include <iostream>

#include "ApiGeneratorDriver.h"
#include "ApiGeneratorLexer.h"
#include "ApiGeneratorParser.h"

using namespace antlr4;
using namespace std;

int main(int argc, const char *argv[]) {
  ifstream ifile;
  if (argc > 1) {
    ifile.open(argv[1]);
    if (!ifile.is_open()) {
      exit(-1);
    }
  }
  istream &stream = argc > 1 ? ifile : cin;
  ANTLRInputStream input(stream);
  ApiGeneratorLexer lexer(&input);
  CommonTokenStream tokens(&lexer);
  ApiGeneratorParser parser(&tokens);

  tree::ParseTree *tree = parser.program();
  auto driver = new ApiGeneratorDriver();
  driver->visit(tree);

  delete driver;

  return 0;
}
