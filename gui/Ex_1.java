    //program to demonstrate the construction of a Container and a Button
    import java.awt.*;

    class Gui extends Frame
    {
        // constructor
        public Gui(String s)
        {   super(s);    //construct Frame part of Gui
            setBackground(Color.yellow);
            setLayout(new FlowLayout());
            Button pushButton = new Button("press me");
            add(pushButton);
        }
    } //class Gui

    class Ex_1
    {
        public static void main(String[] args)
        {   Gui screen = new Gui("Example 1");
            screen.setSize(500,100);
            screen.setVisible(true);
        }
    } //class Ex_1 
