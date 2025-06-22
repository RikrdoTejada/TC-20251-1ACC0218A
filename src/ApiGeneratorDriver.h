#pragma once
#include <iostream>
#include <map>
#include <string>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "ApiGeneratorVisitor.h"



using namespace llvm;
using namespace antlr4;

class ApiGeneratorDriver : public ApiGeneratorVisitor {

private:
  //std::map<std::string, float> symbols;
  
  LLVMContext context;
  std::unique_ptr<Module> module;
  std::unique_ptr<IRBuilder<>> irBuilder;

  std::ostringstream handlerBuffer;
  std::ofstream out;
public:
  ApiGeneratorDriver(): module(std::make_unique<Module>("ApiGenerator", context)),
  irBuilder(std::make_unique<IRBuilder<>>(context)) {
    out.open("output/main.cpp");
  }

  ~ApiGeneratorDriver() {
    writeMain();
    out.close();
  }


  virtual std::any visitProgram(ApiGeneratorParser::ProgramContext *ctx){
    return visitChildren(ctx);
  }

  virtual std::any visitApiDefinition(ApiGeneratorParser::ApiDefinitionContext *ctx){
    return visitChildren(ctx);
  }

  virtual std::any visitGetEndpoint(ApiGeneratorParser::GetEndpointContext *ctx){
    return visitChildren(ctx);
  }

  virtual std::any visitPostEndpoint(ApiGeneratorParser::PostEndpointContext *ctx){
    std::string path = ctx->STRING()->getText();
    path = path.substr(1, path.length() - 2); // quita comillas

    handlerBuffer << "    if (mg_strcmp(hm->uri, mg_str(\"" << path << "\")) == 0) {\n";
    for (auto id : ctx->params()->idList()->ID()) {
        std::string name = id->getText();
        handlerBuffer << "      char " << name << "[100];\n";
        handlerBuffer << "      mg_http_get_var(&hm->body, \"" << name << "\", " << name << ", sizeof(" << name << "));\n";
    }
    auto *respCtx = dynamic_cast<ApiGeneratorParser::ResponseContentContext *>(ctx->response());
    std::string response = respCtx->STRING()->getText();
    response = response.substr(1, response.length() - 2); // sin comillas

    // interpolación de ${nombre} → << nombre <<
    for (auto id : ctx->params()->idList()->ID()) {
        std::string name = id->getText();
        std::string pattern = "${" + name + "}";
        size_t pos = response.find(pattern);
        if (pos != std::string::npos) {
            response.replace(pos, pattern.length(), "\" << " + name + " << \"");
        }
    }

    handlerBuffer << "      std::ostringstream respuesta;\n";
    handlerBuffer << "      respuesta << \"" << response << "\";\n";
    handlerBuffer << "      mg_http_reply(c, 200, \"Content-Type: text/plain\\r\\n\", \"%s\\n\", respuesta.str().c_str());\n";
    handlerBuffer << "      return;\n";
    handlerBuffer << "    }\n";

    return nullptr;
  }

  virtual std::any visitPutEndpoint(ApiGeneratorParser::PutEndpointContext *ctx){
    return visitChildren(ctx);
  }

  virtual std::any visitParams(ApiGeneratorParser::ParamsContext *ctx){
    return visitChildren(ctx);
  }

  virtual std::any visitIdList(ApiGeneratorParser::IdListContext *ctx){
    return visitChildren(ctx);
  }

  virtual std::any visitResponseContent(ApiGeneratorParser::ResponseContentContext *ctx){
    return visitChildren(ctx);
  }
  void writeMain() {
    out << "#include \"mongoose.h\"\n"
        << "#include <string>\n"
        << "#include <sstream>\n\n"
        << "static void handle_request(struct mg_connection *c, int ev, void *ev_data) {\n"
        << "  if (ev == MG_EV_HTTP_MSG) {\n"
        << "    struct mg_http_message *hm = (struct mg_http_message *) ev_data;\n";

    out << handlerBuffer.str();

    out << "    mg_http_reply(c, 404, \"\", \"Ruta no encontrada\\n\");\n"
        << "  }\n"
        << "}\n\n"
        << "int main() {\n"
        << "  struct mg_mgr mgr;\n"
        << "  mg_mgr_init(&mgr);\n"
        << "  mg_http_listen(&mgr, \"http://0.0.0.0:8080\", handle_request, nullptr);\n"
        << "  printf(\"Servidor corriendo en http://localhost:8080\\n\");\n"
        << "  for (;;) mg_mgr_poll(&mgr, 1000);\n"
        << "  mg_mgr_free(&mgr);\n"
        << "  return 0;\n"
        << "}\n";
  } 

};
