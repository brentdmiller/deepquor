   //Filename: HelloWorldApplet.java
   import java.applet.Applet;
   import java.awt.Graphics;   

   public class HelloWorld extends Applet
   {
     public void paint(Graphics g)
     {
       g.drawString("Hello World!", 50, 25);
     }//paint()
   }//class HelloWorld
