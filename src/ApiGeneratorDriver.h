#pragma once
#include <iostream>
#include <map>
#include <string>
#include <regex>
#include <sstream>
#include <fstream>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "ApiGeneratorVisitor.h"

using namespace llvm;
using namespace antlr4;

class ApiGeneratorDriver : public ApiGeneratorVisitor {
private:
  LLVMContext context;
  std::unique_ptr<Module> module;
  std::unique_ptr<IRBuilder<>> irBuilder;

  std::ostringstream handlerBuffer;
  std::ofstream out;

public:
  ApiGeneratorDriver() :
    module(std::make_unique<Module>("ApiGenerator", context)),
    irBuilder(std::make_unique<IRBuilder<>>(context)) {
    out.open("output/main.cpp");
  }

  ~ApiGeneratorDriver() {
    writeMain();
    out.close();
  }

  virtual std::any visitProgram(ApiGeneratorParser::ProgramContext *ctx) {
    auto result = visitChildren(ctx);

    std::error_code ec;
    raw_fd_ostream irFile("output/generated.ll", ec, sys::fs::OF_Text);
    module->print(irFile, nullptr);

    return result;
  }

  virtual std::any visitGetEndpoint(ApiGeneratorParser::GetEndpointContext *ctx) {
    std::string path = ctx->ENDPOINT()->getText();
    path = path.substr(1, path.length() - 2);  // elimina comillas

    std::string fnName = "handle_" + path;
    std::replace(fnName.begin(), fnName.end(), '/', '_');

    handlerBuffer << "    if (mg_strcmp(hm->uri, mg_str(\"" << path
                  << "\")) == 0 && mg_strcmp(hm->method, mg_str(\"GET\")) == 0) {\n";

    std::vector<std::string> filters;

    auto* searchCtx = dynamic_cast<ApiGeneratorParser::SearchContentContext*>(ctx->search());
    if (searchCtx && searchCtx->ID().size() > 0) {
      for (auto id : searchCtx->ID()) {
        std::string key = id->getText();
        filters.push_back(key);
        handlerBuffer << "      char " << key << "[100];\n";
        handlerBuffer << "      mg_http_get_var(&hm->query, \"" << key
                      << "\", " << key << ", sizeof(" << key << "));\n";
      }
    }

    handlerBuffer << "      std::map<std::string, std::string> filtros;\n";
    for (const auto& key : filters) {
      handlerBuffer << "      filtros[\"" << key << "\"] = std::string(" << key << ");\n";
    }

    std::string coleccionRaw = searchCtx->COLLECTIONDB()->getText();  // e.g., %usuario
    std::string coleccion = coleccionRaw.substr(1);  // quita el %

    handlerBuffer << "      std::string resultado = buscarEntidad(\""
                  << coleccion << "\", filtros);\n";
    handlerBuffer << "      mg_http_reply(c, 200, \"Content-Type: application/json\\r\\n"
                  << "Access-Control-Allow-Origin: http://127.0.0.1:5500\\r\\n\", "
                  << "\"%s\\n\", resultado.c_str());\n";
    handlerBuffer << "      return;\n";
    handlerBuffer << "    }\n";

    return nullptr;
  }

  virtual std::any visitPostEndpoint(ApiGeneratorParser::PostEndpointContext *ctx) {
    std::string path = ctx->ENDPOINT()->getText();
    path = path.substr(1, path.length() - 2);

    std::string fnName = "handle_" + path;
    std::replace(fnName.begin(), fnName.end(), '/', '_');

    handlerBuffer << "    if (mg_strcmp(hm->uri, mg_str(\"" << path << "\")) == 0 && mg_strcmp(hm->method, mg_str(\"POST\")) == 0) {\n";
    for (auto id : ctx->params()->idList()->ID()) {
      std::string name = id->getText();
      handlerBuffer << "      char " << name << "[100];\n";
      handlerBuffer << "      mg_http_get_var(&hm->body, \"" << name << "\", " << name << ", sizeof(" << name << "));\n";
    }

    handlerBuffer << "      static auto handler = getHandler(\"" << fnName << "\");\n";
    handlerBuffer << "      const char* response = handler(";
    bool first = true;
    for (auto id : ctx->params()->idList()->ID()) {
      if (!first) handlerBuffer << ", ";
      handlerBuffer << id->getText();
      first = false;
    }
    handlerBuffer << ");\n";
    handlerBuffer << "      guardarEntidad(response);\n";
    handlerBuffer << "      mg_http_reply(c, 200, \"Content-Type: application/json\\r\\nAccess-Control-Allow-Origin: http://127.0.0.1:5500\\r\\n\", \"%s\\n\", response);\n";
    handlerBuffer << "      return;\n";
    handlerBuffer << "    }\n";

    std::vector<Type*> argTypes(ctx->params()->idList()->ID().size(),
      PointerType::getUnqual(Type::getInt8Ty(context)));
    FunctionType *fnType = FunctionType::get(PointerType::getUnqual(Type::getInt8Ty(context)), argTypes, false);
    Function *fn = Function::Create(fnType, Function::ExternalLinkage, fnName, module.get());
    BasicBlock *entry = BasicBlock::Create(context, "entry", fn);
    irBuilder->SetInsertPoint(entry);

    std::ostringstream jsonStream;
    jsonStream << "{\"" << path.substr(1) << "\": {";

    first = true;
    for (auto id : ctx->params()->idList()->ID()) {
        if (!first) jsonStream << ", ";
        std::string key = id->getText();
        jsonStream << "\"" << key << "\": \"%s\"";
        first = false;
    }
    jsonStream << "}}";

    std::string response = jsonStream.str();
    Value *fmt = irBuilder->CreateGlobalStringPtr(response.c_str());

    FunctionCallee mallocFn = module->getOrInsertFunction("malloc",
      FunctionType::get(PointerType::getUnqual(Type::getInt8Ty(context)), {Type::getInt64Ty(context)}, false));
    Value *bufferPtr = irBuilder->CreateCall(mallocFn, ConstantInt::get(Type::getInt64Ty(context), 512));

    FunctionCallee sprintfFn = module->getOrInsertFunction("sprintf",
      FunctionType::get(Type::getInt32Ty(context), {PointerType::getUnqual(Type::getInt8Ty(context)), PointerType::getUnqual(Type::getInt8Ty(context))}, true));

    std::vector<Value*> args;
    args.push_back(bufferPtr);
    args.push_back(fmt);
    for (auto &arg : fn->args()) {
      args.push_back(&arg);
    }
    irBuilder->CreateCall(sprintfFn, args);

    irBuilder->CreateRet(bufferPtr);

    return nullptr;
  }

  virtual std::any visitApiDefinition(ApiGeneratorParser::ApiDefinitionContext *ctx) {
    return visitChildren(ctx);
  }
  virtual std::any visitPutEndpoint(ApiGeneratorParser::PutEndpointContext *ctx) {
    return visitChildren(ctx);
  }
  virtual std::any visitParams(ApiGeneratorParser::ParamsContext *ctx) {
    return visitChildren(ctx);
  }
  virtual std::any visitIdList(ApiGeneratorParser::IdListContext *ctx) {
    return visitChildren(ctx);
  }
  virtual std::any visitResponseContent(ApiGeneratorParser::ResponseContentContext *ctx) {
    return visitChildren(ctx);
  }
  virtual std::any visitSearchContent(ApiGeneratorParser::SearchContentContext *ctx) {
    return visitChildren(ctx);
  }
  void writeMain() {
  out << "#define MG_ENABLE_HTTP 1\n";
  out << "#include \"mongoose.h\"\n"
      << "#include <string>\n"
      << "#include <sstream>\n"
      << "#include <iostream>\n"
      << "#include <fstream>\n"
      << "#include <map>\n"
      << "#include \"llvm/IR/LLVMContext.h\"\n"
      << "#include \"llvm/IR/Module.h\"\n"
      << "#include \"llvm/IRReader/IRReader.h\"\n"
      << "#include \"llvm/Support/SourceMgr.h\"\n"
      << "#include \"llvm/ExecutionEngine/Orc/LLJIT.h\"\n"
      << "#include \"llvm/Support/TargetSelect.h\"\n"
      << "#include \"json.hpp\"\n"
      << "using namespace llvm;\n"
      << "using namespace llvm::orc;\n"
      << "using json = nlohmann::json;\n"
      << "LLJIT* jit = nullptr;\n"
      << "using HandlerFunc = const char*(*)(...);\n\n";

  // Función para cargar el handler
  out << "HandlerFunc getHandler(const std::string& name) {\n"
      << "  auto sym = jit->lookup(name);\n"
      << "  if (!sym) { std::cerr << \"No se encontró \" << name << std::endl; exit(1); }\n"
      << "  return reinterpret_cast<HandlerFunc>(sym->toPtr<void*>());\n"
      << "}\n\n";

  // Función para guardar json en archivo
  out << "void guardarEntidad(const std::string& responseJsonStr) {\n"
      << "  std::ifstream inFile(\"/compilers/TF/data/api_data.txt\");\n"
      << "  json root;\n"
      << "  if (inFile.peek() != std::ifstream::traits_type::eof()) {\n"
      << "    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());\n"
      << "    root = json::parse(content, nullptr, false);\n"
      << "    if (root.is_discarded()) root = json::object();\n"
      << "  }\n"
      << "  inFile.close();\n"
      << "  json nuevo = json::parse(responseJsonStr, nullptr, false);\n"
      << "  if (!nuevo.is_object() || nuevo.empty()) return;\n"
      << "  std::string nombreColeccion = nuevo.begin().key();\n"
      << "  json objeto = nuevo.begin().value();\n"
      << "  if (!root.contains(nombreColeccion) || !root[nombreColeccion].is_array()) {\n"
      << "    root[nombreColeccion] = json::array();\n"
      << "  }\n"
      << "  root[nombreColeccion].push_back(objeto);\n"
      << "  std::ofstream outFile(\"/compilers/TF/data/api_data.txt\");\n"
      << "  outFile << root.dump(2);\n"
      << "}\n\n";

  // Función para buscar entidades por filtros
  out << "std::string buscarEntidad(const std::string& coleccion, const std::map<std::string, std::string>& filtros) {\n"
      << "  std::ifstream inFile(\"/compilers/TF/data/api_data.txt\");\n"
      << "  json db;\n"
      << "  if (inFile.peek() != std::ifstream::traits_type::eof()) {\n"
      << "    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());\n"
      << "    db = json::parse(content, nullptr, false);\n"
      << "    if (db.is_discarded()) db = json::object();\n"
      << "  }\n"
      << "  inFile.close();\n"
      << "  if (!db.contains(coleccion)) return \"{}\";\n"
      << "  for (const auto& item : db[coleccion]) {\n"
      << "    bool match = true;\n"
      << "    for (const auto& f : filtros) {\n"
      << "      if (!item.contains(f.first) || item[f.first].get<std::string>() != f.second) {\n"
      << "        match = false;\n"
      << "        break;\n"
      << "      }\n"
      << "    }\n"
      << "    if (match) return item.dump();\n"
      << "  }\n"
      << "  return \"{}\";\n"
      << "}\n\n";

  // handler principal
  out << "static void handle_request(struct mg_connection *c, int ev, void *ev_data) {\n"
      << "  if (ev == MG_EV_HTTP_MSG) {\n"
      << "    struct mg_http_message *hm = (struct mg_http_message *) ev_data;\n";

  // Aquí se insertan dinámicamente los handlers de cada endpoint
  out << handlerBuffer.str();

  out << "    mg_http_reply(c, 404, \"\", \"Ruta no encontrada\\n\");\n"
      << "  }\n"
      << "}\n\n";

  // main()
  out << "int main() {\n"
      << "  InitializeNativeTarget();\n"
      << "  InitializeNativeTargetAsmPrinter();\n"
      << "  auto jitResult = LLJITBuilder().create();\n"
      << "  if (!jitResult) { std::cerr << \"Error creando JIT\\n\"; return 1; }\n"
      << "  jit = jitResult->release();\n"
      << "  auto ctx = std::make_unique<LLVMContext>();\n"
      << "  SMDiagnostic err;\n"
      << "  auto mod = parseIRFile(\"output/generated.ll\", err, *ctx);\n"
      << "  if (!mod) { std::cerr << \"Error leyendo IR\\n\"; err.print(\"\", errs()); return 1; }\n"
      << "  auto errModule = jit->addIRModule(ThreadSafeModule(std::move(mod), std::move(ctx)));\n"
      << "  if (errModule) {\n"
      << "    std::cerr << \"Error al añadir IR al JIT\\n\";\n"
      << "    logAllUnhandledErrors(std::move(errModule), errs(), \"\");\n"
      << "    return 1;\n"
      << "  }\n"
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
