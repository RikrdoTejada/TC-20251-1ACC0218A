import java.io.IOException;

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

public class ApiGenMain {
    public static void main(String[] args) throws Exception {
        CharStream input = CharStreams.fromFileName("pruebita.expr");
        ApiGeneratorLexer lexer = new ApiGeneratorLexer(input);
        CommonTokenStream tokens = new CommonTokenStream(lexer);
        ApiGeneratorParser parser = new ApiGeneratorParser(tokens);

        ApiGeneratorParser.ProgramContext tree = parser.program();

        ApiPrinterVisitor visitor = new ApiPrinterVisitor();
        visitor.visit(tree);
    }
}