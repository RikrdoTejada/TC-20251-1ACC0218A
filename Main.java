import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;
import org.antlr.v4.gui.TreeViewer;

import javax.swing.*;
import java.util.Arrays;

public class Main {
    public static void main(String[] args) throws Exception {
        CharStream input = CharStreams.fromFileName("TP_test.expr"); 

        HorariosLexer lexer = new HorariosLexer(input);
        CommonTokenStream tokens = new CommonTokenStream(lexer);
        HorariosParser parser = new HorariosParser(tokens);

        ParseTree tree = parser.program(); 

        // Crear el visor del árbol
        TreeViewer viewer = new TreeViewer(Arrays.asList(parser.getRuleNames()), tree);
        viewer.setScale(1.5); // zoom opcional

        // ScrollPane para evitar que se corte el árbol
        JScrollPane scrollPane = new JScrollPane(viewer);
        scrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);

        // Crear y mostrar ventana
        JFrame frame = new JFrame("Árbol de Parseo");
        frame.add(scrollPane);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(1000, 800);
        frame.setVisible(true);

        //Listener
        HorariosParserBaseListenerImpl listener = new HorariosParserBaseListenerImpl();
        ParseTreeWalker walker = new ParseTreeWalker();
        walker.walk(listener, tree);
    }
}
