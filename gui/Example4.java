import java.awt.*;

public class Example4 extends java.applet.Applet
{
   public void init()
   {
      Panel p;

      setLayout(new BorderLayout());

      p = new Panel();

      p.add(new TextArea());

      add("Center", p);

      p = new Panel();

      p.add(new Button("One"));
      p.add(new Button("Two"));

      Choice c = new Choice();

      c.addItem("one");
      c.addItem("two");
      c.addItem("three");

      p.add(c);

      add("South", p);
   }

   public static void main(String [] args)
   {
      Frame f = new Frame("Example 4");

      Example4 ex = new Example4();

      ex.init();

      f.add("Center", ex);

      f.pack();
      f.show();
   }
}
