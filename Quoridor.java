import java.awt.Color;
import java.awt.Component;
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
  static final boolean DEBUG = false;
  static final int QUADRANT = 1;
  static final int INITIAL_WALLS = 9;
  static final int ROWS = 9, COLS = 9;
  static final int MDIM = 50, mDIM = 10;
  static final String[] NAMES = {"W", "B"};
  static final String[] PROTOVER_NAMES = {"O", "X"};

  static final Color WALL_COLOR = Color.GRAY;

  int turn = 0;
  int[] walls_left = {INITIAL_WALLS, INITIAL_WALLS};

  JButton[] pawns = new JButton[2];

  JButton[][] squares = new JButton[ROWS][COLS];
  JButton[][] cwalls = new JButton[ROWS-1][COLS-1];
  JButton[][][] vwalls = new JButton[ROWS][COLS-1][2];
  JButton[][][] hwalls = new JButton[ROWS-1][COLS][2];


  private void DEBUGF(String fmt, Object... args) {
    if (DEBUG) {
      System.err.println(String.format(fmt, args));
    }
  }

  private void DEBUGE(Exception ex) {
    if (DEBUG) {
      System.err.println(ex);
    }
  }

  private int getWallsLeft() {
    return walls_left[turn];
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

  private boolean checkWalls(int row, int col, int rdir, int cdir) {
    int dist = Math.abs(rdir) + Math.abs(cdir);
    boolean result = false;
    if (dist == 2) {
      if (rdir == 0 || cdir == 0) {
        /* Jump straight ahead */
        rdir /= 2;
        cdir /= 2;
        result = checkWalls(row, col, rdir, cdir) && checkWalls(row + rdir, col + cdir, rdir, cdir) && !squares[row + rdir][col + cdir].getText().equals("");
      } else {
       result = 
            !squares[row + rdir][col].getText().equals("") ? checkWalls(row, col, rdir, 0) && checkWalls(row + rdir, col, 0, cdir) && !checkWalls(row + rdir, col, rdir, 0) :
            !squares[row][col + cdir].getText().equals("") ? checkWalls(row, col, 0, cdir) && checkWalls(row, col + cdir, rdir, 0) && !checkWalls(row, col + cdir, 0, cdir) :
            false;
      }
    } else if (dist == 1) {
      JButton[][][] walls = rdir == 0 ? vwalls : hwalls;
      result = !walls[row + Math.min(0, rdir)][col + Math.min(0, cdir)][0].getBackground().equals(WALL_COLOR);
    }
    DEBUGF("MOVE (%d,%d) + (%d,%d) is %sblocked", row, col, rdir, cdir, result ? "un" : "");
    return result;
  }

  private boolean setPawn(int row, int col) {
    JButton square = squares[row][col];
    if (!square.getText().equals("")) {
      return false;
    }
    if (pawns[turn] != null) {
      List<Integer> old = getSquare(pawns[turn]);
      int orow = old.get(0);
      int ocol = old.get(1);
      if (!square.getText().equals("")) {
        return false;
      }
      if (!checkWalls(orow, ocol, row - orow, col - ocol)) {
        return false;
      }

      pawns[turn].setText("");
    } else {
      /* This is hopefully during setup... */
    }
    pawns[turn] = square;
    pawns[turn].setText(NAMES[turn]);
    turn = 1 - turn;
    return true;
  }

  private boolean setWall(boolean vertical, int r, int c, int h) {
    JButton[][][] walls = vertical ? vwalls : hwalls;
    int vp = vertical ? 1 : 0;
    int hp = 1 - vp;

    int rp = r + vp * (2 * h - 1);
    int cp = c + hp * (2 * h - 1);
    int[][] locations = { { r, c, h }, { r, c, 1 - h }, { rp, cp, h }, { rp, cp, 1 - h} };

    int cr = r + vp * (h - 1);
    int cc = c + hp * (h - 1);

    for (int[] l : locations) {
      if (walls[l[0]][l[1]][l[2]].getBackground().equals(WALL_COLOR)) {
        return false;
      }
    }
    if (cwalls[cr][cc].getBackground().equals(WALL_COLOR)) {
      return false;
    }
    for (int[] l : locations) {
      walls[l[0]][l[1]][l[2]].setBackground(WALL_COLOR);
    }
    cwalls[cr][cc].setBackground(WALL_COLOR);
    --walls_left[turn];
    turn = 1 - turn;
    return true;
  }

  private JButton square(Color bg, boolean actionable)
  {
    JButton square = new JButton();
    square.setBorder(new EmptyBorder(0,0,0,0));
    square.setBackground(bg);
    square.setForeground(Color.WHITE);
    if (actionable) {
      square.addActionListener(this);
    }
    return square;
  }

  private void initArrays()
  {
    // Iterate over rows backwards to do 1st Quadrant (as opposed to 4th)
    for(int r = 0  ; r < ROWS; ++r) {
      for(int c = 0; c < COLS  ; ++c) {
        squares[r][c] = square(Color.BLACK, true);
        for(int i = 0; i < 2; ++i) {
          if (c < COLS - 1) vwalls[r][c][i] = square(Color.WHITE, (r + i) * (ROWS - r - i) != 0);
          if (r < ROWS - 1) hwalls[r][c][i] = square(Color.WHITE, (c + i) * (COLS - c - i) != 0);
        }
        if (r < ROWS - 1 && c < COLS - 1) cwalls[r][c] = square(Color.WHITE, false);
      }
    }
  }

  private void init()
  {
    setSize(530,550);
    initArrays();
    Component[][] grid = new Component[2 * ROWS - 1][2 * COLS - 1];
    int rp, rp2, ri, cp, cp2, ci;

    for(int r = 0; r < ROWS; ++r) for(int c = 0; c < COLS  ; ++c) {
      if (QUADRANT < 3) {
        rp2 = (rp = ROWS - r - 1) - 1;
        ri = 1;
      } else {
        rp2 = rp = r;
        ri = 0;
      }

      if ((QUADRANT - 1) / 2 == QUADRANT % 2) {
        cp2 = (cp = COLS - c - 1) - 1;
        ci = 1;
      } else {
        cp2 = cp = c;
        ci = 0;
      }

      grid[2*rp][2*cp] = squares[r][c];
      if (c < COLS - 1) {
        JPanel panel = new JPanel(new GridLayout(2,1));
        panel.add(vwalls[r][c][ri]);
        panel.add(vwalls[r][c][1-ri]);
        grid[2*rp][2*cp2+1] = panel;
      }
      if (r < ROWS - 1) {
        JPanel panel = new JPanel(new GridLayout(1,2));
        panel.add(hwalls[r][c][ci]);
        panel.add(hwalls[r][c][1-ci]);
        grid[2*rp2+1][2*cp] = panel;
      }
      if (c < COLS -1 && r < ROWS - 1) {
        grid[2*rp2+1][2*cp2+1] = cwalls[r][c];
      }
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
    setPawn(0, COLS / 2);
    setPawn(ROWS-1, COLS / 2);
    setContentPane(pane);
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
    try {
      List<Integer> square = getSquare(event.getSource());
      List<Integer> vwall = getVerticalWall(event.getSource());
      List<Integer> hwall = getHorizontalWall(event.getSource());
      int old_turn = turn;
      if (square != null) {
        DEBUGF("Attempting to set pawn (%d, %d)", square.get(0), square.get(1));
        if (setPawn(square.get(0), square.get(1))) {
          System.out.println(String.format("MOVE %s %d %d", PROTOVER_NAMES[old_turn], 9 *  square.get(0) + square.get(1) + 1, walls_left[old_turn]));
        }
      } else if (vwall != null) {
        DEBUGF("Attempting to place vwall (%d, %d, %d)", vwall.get(0), vwall.get(1), vwall.get(2));
        if (setWall(true, vwall.get(0), vwall.get(1), vwall.get(2))) {
          System.out.println(String.format("MOVE %s |%d%c %d", PROTOVER_NAMES[old_turn], vwall.get(0) + vwall.get(2), vwall.get(1) + 'A', walls_left[old_turn]));
        }
      } else if (hwall != null) {
        DEBUGF("Attempting to set hwall (%d, %d, %d)", hwall.get(0), hwall.get(1), hwall.get(2));
        if (setWall(false, hwall.get(0), hwall.get(1), hwall.get(2))) {
          System.out.println(String.format("MOVE %s |%d%c %d", PROTOVER_NAMES[old_turn], hwall.get(0) + 1, hwall.get(1) + hwall.get(2) - 1 + 'A', walls_left[old_turn]));
        }
      } else {
        System.err.println("Unidentified action");
      }
    } catch (ArrayIndexOutOfBoundsException aex) {
      throw aex;
    }
  }

  private static void mverror() {
    System.out.println("MVERROR");
  }

  public static void main(String[] args) throws Throwable
  {
    UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
    Quoridor q = new Quoridor();
    BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
    int protover = 0;

    System.out.println("HELLO java-ui");
    System.out.println("PROTOVER 0.1");


    for (String line = br.readLine(); line != null; line = br.readLine()) {
      String[] cmd = line.split(" ");
      try {
        if (cmd[0].equals("MOVE")) {
          if (!PROTOVER_NAMES[q.turn].equals(cmd[1])) {
            mverror();
            continue;
          }
          int walls_left = Integer.parseInt(cmd[3]);
          if (cmd[2].length() == 3) {
            boolean vertical = cmd[2].charAt(0) == '|';
            int row = cmd[2].charAt(1) - '1';
            int col = cmd[2].charAt(2) - 'A';
            
            if(walls_left != q.getWallsLeft() - 1 || !q.setWall(vertical, row, col, 1)) {
              mverror();
            }
          } else {
            int value = Integer.parseInt(cmd[2]) - 1;
            int row = value / 9;
            int col = value % 9;
            if(walls_left != q.getWallsLeft() || !q.setPawn(row, col)) {
              mverror();
            }
          }
        } else if (cmd[0].equals("NEW")) {
          q.setVisible(false);
          q.dispose();
          q = new Quoridor();
          if (cmd.length == 1) {
            System.out.println("NEW CONFIRMED");
          } else {
            // We initiated the NEW
          }
        } else if (cmd[0].equals("HELLO")) {
          // NOTHING TO DO
        } else if (cmd[0].equals("WHITE")) {
          // Ignoring for now
        } else if (cmd[0].equals("BLACK")) {
          // Ignorning for now
	} else if (cmd[0].equals("PROTOVER")) {
          // We sent 0.1, nothing lower
        } else if (cmd[0].equals("FEATURE")) {
          System.out.println("REJECTED");
        } else if (cmd[0].equals("SETBOARD")) {
          System.out.println("ERROR");
        } else if (cmd[0].equals("QUIT")) {
          System.exit(0);
        } else {
          System.out.println("ERROR");
//          System.err.println("Unrecognized line: " + line);
        }
      } catch (Exception ex) {
        if (cmd.length > 0 && cmd[0].equals("MOVE")) {
          System.out.println("MVERROR");
        } else {
          System.out.println("ERROR");
        }
        throw ex;
      }
    }
  }
}
