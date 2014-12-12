import java.awt.*;

public class Example5 extends java.applet.Applet
{
   TextArea ta = null;

   public void init()
   {
      Panel p;

      setLayout(new BorderLayout());

      p = new Panel();

      ta = new TextArea();

      p.add(ta);

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

   public boolean action(Event e, Object o)
   {
      String str = (String)o;

      ta.appendText(str + "\n");

      return false;
   }

   public static void main(String [] args)
   {
      Frame f = new Frame("Example 5");

      Example5 ex = new Example5();

      ex.init();

      f.add("Center", ex);

      f.pack();
      f.show();
   }
}

