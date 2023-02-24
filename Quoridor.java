import java.awt.Color;
import java.awt.Component;
import java.awt.BorderLayout;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import java.util.*;
import static java.util.Arrays.*;
import java.io.*;

public class Quoridor extends JFrame implements MouseListener, ActionListener
{
  static final int ROWS = 9, COLS = 9;
  static final int MDIM = 50, mDIM = 10;
  static final String[] NAMES = {"W", "B"};
  static final String[] LONG_NAMES = {"White", "Black"};
  boolean frozen = false;

  int turn = 0;
  int[] walls_left = { 10, 10 };

  JButton[] pawns = new JButton[2];

  JButton[][] squares = new JButton[ROWS][COLS];
  JButton[][] cwalls = new JButton[ROWS-1][COLS-1];
  JButton[][][] vwalls = new JButton[ROWS][COLS-1][2];
  JButton[][][] hwalls = new JButton[ROWS-1][COLS][2];
  JTextField tfturn = new JTextField("Turn: White");
  JTextField[] tfwalls = { new JTextField("White walls: 10"), new JTextField("Black walls: 10") };

  private void setPawn(int turn, JButton square) {
    if(pawns[turn] != null) {
      pawns[turn].setText("");
    }
    pawns[turn] = square;
    pawns[turn].setText(NAMES[turn]);
  }

  private void setVerticalWall(int r, int c, int h) {
    vwalls[r + 2 *h - 1][c][h].setBackground(Color.GRAY);
    vwalls[r + 2 *h - 1][c][1 - h].setBackground(Color.GRAY);
    vwalls[r][c][h].setBackground(Color.GRAY);
    vwalls[r][c][1-h].setBackground(Color.GRAY);
    cwalls[r + h - 1][c].setBackground(Color.GRAY);
  }

  private void setHorizontalWall(int r, int c, int h) {
    hwalls[r][c + 2 * h - 1][h].setBackground(Color.GRAY);
    hwalls[r][c + 2 * h - 1][1 - h].setBackground(Color.GRAY);
    hwalls[r][c][h].setBackground(Color.GRAY);
    hwalls[r][c][1-h].setBackground(Color.GRAY);
    cwalls[r][c + h - 1].setBackground(Color.GRAY);
  }



  private List<Integer> getSquare(Object source) {
    for (int i = 0; i < squares.length; ++i) {
      for (int j = 0; j < squares[i].length; ++j) {
        if (squares[i][j] == source) {
          return asList(i, j);
        }
      }
    }
    return null;
  }

  private List<Integer> getVerticalWall(Object source) {
    for (int i = 0; i < vwalls.length; ++i) {
      for (int j = 0; j < vwalls[i].length; ++j) {
        for (int k = 0; k < vwalls[i][j].length; ++k) {
          if (vwalls[i][j][k] == source) {
            return asList(i, j, k);
          }
        }
      }
    }
    return null;
  }

  private List<Integer> getHorizontalWall(Object source) {
    for (int i = 0; i < hwalls.length; ++i) {
      for (int j = 0; j < hwalls[i].length; ++j) {
        for (int k = 0; k < hwalls[i][j].length; ++k) {
          if (hwalls[i][j][k] == source) {
            return asList(i, j, k);
          }
        }
      }
    }
    return null;
  }

  private JButton square(Color bg)
  {
    JButton square = new JButton();
    square.setBorder(new EmptyBorder(0,0,0,0));
    square.setBackground(bg);
    square.setForeground(Color.WHITE);
    square.addActionListener(this);
    return square;
  }

  private void initArrays()
  {
    for(int r = 0; r < ROWS  ; ++r) for(int c = 0; c < COLS  ; ++c) squares[r][c] = square(Color.BLACK);
    for(int r = 0; r < ROWS  ; ++r) for(int c = 0; c < COLS-1; ++c) for(int i = 0; i < 2; ++i) vwalls[r][c][i] = square(Color.WHITE);
    for(int r = 0; r < ROWS-1; ++r) for(int c = 0; c < COLS  ; ++c) for(int i = 0; i < 2; ++i) hwalls[r][c][i] = square(Color.WHITE);
    for(int r = 0; r < ROWS-1; ++r) for(int c = 0; c < COLS-1; ++c) cwalls[r][c] = square(Color.WHITE);
  }

  private void init()
  {
    setSize(530,590);
    initArrays();
    Component[][] grid = new Component[2 * ROWS - 1][2 * COLS - 1];

    for(int r = 0; r < ROWS; ++r) for(int c = 0; c < COLS  ; ++c)
      grid[2*r][2*c] = squares[r][c];

    for(int r = 0; r < ROWS; ++r) for(int c = 0; c < COLS-1; ++c)
    {
      JPanel panel = new JPanel(new GridLayout(2,1));
      for(int i = 0; i < 2; ++i)
        panel.add(vwalls[r][c][i]);
      grid[2*r][2*c+1] = panel;
    }

    for(int r = 0; r < ROWS-1; ++r) for(int c = 0; c < COLS; ++c)
    {
      JPanel panel = new JPanel(new GridLayout(1,2));
      for(int i = 0; i < 2; ++i)
        panel.add(hwalls[r][c][i]);
      grid[2*r+1][2*c] = panel;
    }

    for(int r = 0; r < ROWS-1; ++r) for(int c = 0; c < COLS-1; ++c)
    {
      grid[2*r+1][2*c+1] = cwalls[r][c];
    }
    JPanel outer = new JPanel(new BorderLayout());
       
    JPanel pane = new JPanel();
    GroupLayout layout = new GroupLayout(pane);
    pane.setLayout(layout);

    GroupLayout.SequentialGroup vsGroup = layout.createSequentialGroup();
    GroupLayout.SequentialGroup hsGroup = layout.createSequentialGroup();
    for(int i = 0; i < grid.length; ++i)
    {
      GroupLayout.ParallelGroup hpGroup = layout.createParallelGroup(GroupLayout.Alignment.BASELINE, false);
      GroupLayout.ParallelGroup vpGroup = layout.createParallelGroup();
      for(int j = 0; j < grid.length; ++j)
      {
        hpGroup.addComponent(grid[i][j], GroupLayout.PREFERRED_SIZE, (i % 2 == 0) ? MDIM : mDIM , GroupLayout.PREFERRED_SIZE);
        vpGroup.addComponent(grid[j][i], GroupLayout.PREFERRED_SIZE, (i % 2 == 0) ? MDIM : mDIM, GroupLayout.PREFERRED_SIZE);
      }
      vsGroup.addGroup(hpGroup);
      hsGroup.addGroup(vpGroup);
    }
    layout.setHorizontalGroup(hsGroup);
    layout.setVerticalGroup(vsGroup);
    setPawn(0, squares[ROWS-1][COLS / 2]);
    setPawn(1, squares[0][COLS / 2]);
    outer.add(pane, BorderLayout.CENTER);

    JPanel info = new JPanel();
    info.add(tfturn);
    info.add(tfwalls[0]);
    info.add(tfwalls[1]);
    outer.add(info, BorderLayout.SOUTH);

    setContentPane(outer);
  }

  private Quoridor()
  {
    setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
    init();
    setVisible(true);
  }

  public void paint(Graphics g)
  {
    super.paint(g);
  }

  public void mouseClicked(MouseEvent event) {}
  public void mousePressed(MouseEvent event) {}
  public void mouseReleased(MouseEvent event) {}
  public void mouseEntered(MouseEvent event) {}
  public void mouseExited(MouseEvent event) {}

  public void actionPerformed(ActionEvent event) {
    if (frozen) {
      return;
    }
    try {
      List<Integer> square = getSquare(event.getSource());
      List<Integer> vwall = getVerticalWall(event.getSource());
      List<Integer> hwall = getHorizontalWall(event.getSource());
      if (square != null) {
        System.out.println("MOVE " + (9 * (8 - square.get(0)) + square.get(1) + 1));
      } else if (vwall != null) {
        System.out.println("MOVE |" + (9 - vwall.get(0) - vwall.get(2)) + "" + (char)(vwall.get(1) + 'A'));
      } else if (hwall != null) {
        System.out.println("MOVE -" + (8 - hwall.get(0)) + "" + (char)(hwall.get(1) + hwall.get(2) + 'A'));
      } else {
        System.err.println("Unidentified action");
        return;
      }
      frozen = true;
    } catch (ArrayIndexOutOfBoundsException aex) {
//      throw aex;
    }
  }

  boolean handleMove(String arg) {
    try {
      if (arg.charAt(0) == '-') {
        int row = 7 - (arg.charAt(1) - '1');
        int col = arg.charAt(2) - 'A';
        setHorizontalWall(row, col, 1);
        walls_left[turn]--;
      } else if (arg.charAt(0) == '|') {
        int row = 7 - (arg.charAt(1) - '1');
        int col = arg.charAt(2) - 'A';
        setVerticalWall(row, col, 1);
        walls_left[turn]--;
      } else {
        int value = Integer.parseInt(arg) - 1;
        int row = 8 - value / 9;
        int col = value % 9;
        setPawn(turn, squares[row][col]);
      }
      turn = 1 - turn;
      tfturn.setText("Turn: " + LONG_NAMES[turn]);
      tfwalls[0].setText("White walls: " + walls_left[0]);
      tfwalls[1].setText("Black walls: " + walls_left[1]);
      return true;
    } catch (Exception ex) {
      System.err.println(ex);
    }
    return false;
  }

  public static void main(String[] args) throws Throwable
  {
    UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
    System.out.println("HELLO GUI");
    System.out.println("PROTOVER 0.0");
    Quoridor q = new Quoridor();
    BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
    for (String line = br.readLine(); line != null; line = br.readLine()) {
      args = line.split(" ");
      if (args.length == 0) {
      } else if (args[0].equals("MOVE")) {
        if (args.length == 1) continue;
        if (q.handleMove(args[1])) {
          System.out.println("ACKMOVE " + args[1]);
        } else {
          System.out.println("MVERROR " + args[1]);
        }
      } else if (args[0].equals("ACKMOVE")) {
        q.frozen = false;
        if (args.length == 1) continue;
        q.handleMove(args[1]);
      } else if (args[0].equals("MVERROR")) {
        q.frozen = false;
      } else if (args[0].equals("HELLO")) {
      } else if (args[0].equals("PROTOVER")) {
      } else {
        System.err.println("Invalid command: " + args[0]);
      }
    }
  }
}
