import org.antlr.v4.runtime.tree.TerminalNode;

public class ApiPrinterVisitor extends ApiGeneratorBaseVisitor<Void> {
        @Override
    public Void visitApiDefinition(ApiGeneratorParser.ApiDefinitionContext ctx) {
        System.out.println("\n API Name: " + ctx.API_N().getText().replace("\"", ""));
        return super.visitApiDefinition(ctx); // Continue visiting child nodes 
    }

    @Override
    public Void visitGetEndpoint(ApiGeneratorParser.GetEndpointContext ctx) {
        System.out.println("GET " + ctx.STRING().getText());
        System.out.println("  -> " + ctx.response().getText());
        return null;
    }

    @Override
    public Void visitPostEndpoint(ApiGeneratorParser.PostEndpointContext ctx) {
        System.out.println("POST " + ctx.STRING().getText());
        System.out.print("  Params: ");
        for (TerminalNode id : ctx.params().idList().ID()) {
            System.out.print(id.getText() + " ");
        }
        System.out.println("\n  -> " + ctx.response().getText());
        return null;
    }

    @Override
    public Void visitPutEndpoint(ApiGeneratorParser.PutEndpointContext ctx) {
        System.out.println("PUT " + ctx.STRING().getText());
        System.out.print("  Params: ");
        for (TerminalNode id : ctx.params().idList().ID()) {
            System.out.print(id.getText() + " ");
        }
        System.out.println("\n  -> " + ctx.response().getText());
        return null;
    }
    
    @Override
     public Void visitResponseContent(ApiGeneratorParser.ResponseContentContext ctx)
     { 
        
        return super.visitResponseContent(ctx);
     }

}
