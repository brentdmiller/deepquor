import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

public class Quoridor extends JFrame implements MouseListener, ActionListener
{
  static final int ROWS = 9, COLS = 9;
  static final int MDIM = 50, mDIM = 10;


  JButton[][] squares = new JButton[ROWS][COLS];
  JButton[][] cwalls = new JButton[ROWS-1][COLS-1];
  JButton[][][] vwalls = new JButton[ROWS][COLS-1][2];
  JButton[][][] hwalls = new JButton[ROWS-1][COLS][2];

  private Point getSquare(Object source) {
    for (int i = 0; i < ROWS; ++i) {
      for (int j = 0; j < COLS; ++j) {
        if (squares[i][j] == source) {
          return new Point(i, j);
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
    setSize(530,550);
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

    squares[0][COLS / 2].setText("B");
    squares[ROWS-1][COLS / 2].setText("W");

    setContentPane(pane);
  }

  private Quoridor()
  {
    init();
    setVisible(true);
  }

  public void paint(Graphics g)
  {
  }

  public void mouseClicked(MouseEvent event) {}
  public void mousePressed(MouseEvent event) {}
  public void mouseReleased(MouseEvent event) {}
  public void mouseEntered(MouseEvent event) {}
  public void mouseExited(MouseEvent event) {}

  public void actionPerformed(ActionEvent event) {
    Point square = getSquare(event.getSource());
    if (square != null) {
      System.out.println(square);
    } else {
      System.out.println("Action performed");
    }
  }

  public static void main(String[] args) throws Throwable
  {
    UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
    new Quoridor();
  }
}
