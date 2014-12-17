import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import java.util.*;
import static java.util.Arrays.*;
import static java.util.Collections.*;
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
  static final String[] DEFAULT_ARGS = "SETBOARD O 5 77 9 9".split(" ");

  // (O(R,C), X(R,C)). -1 = Don't care
  static final int[][] VICTORY = {{ROWS-1, -1}, {0, -1}};
  static final int[][] DIRS = {{1,0},{-1,0},{0,-1},{0,1}};

  static final Color WALL_COLOR = Color.GRAY;

  int turn = 0;
  int[] walls_left = {INITIAL_WALLS, INITIAL_WALLS};

  JButton[] pawns = new JButton[2];

  JButton[][] squares = new JButton[ROWS][COLS];
  JButton[][] cwalls = new JButton[ROWS-1][COLS-1];
  JButton[][][] vwalls = new JButton[ROWS][COLS-1][2];
  JButton[][][] hwalls = new JButton[ROWS-1][COLS][2];
  JButton[][][][] walls = {vwalls, hwalls};


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

  private int[] getSquare(Object source) {
    for (int i = 0; i < squares.length; ++i) {
      for (int j = 0; j < squares[i].length; ++j) {
        if (squares[i][j] == source) {
          return new int[]{i, j};
        }
      }
    }
    return null;
  }

  private int[] getWall(Object source) {
    for (int i = 0; i < walls.length; ++i) {
      for (int j = 0; j < walls[i].length; ++j) {
        for (int k = 0; k < walls[i][j].length; ++k) {
          for (int l = 0; l < walls[i][j][k].length; ++l) {
            if (walls[i][j][k][l] == source) {
              return new int[]{i, j, k, l};
            }
          }
        }
      }
    }
    return null;
  }

  private boolean isBlocked(int row, int col, int rdir, int cdir) {
    return isBlocked(row, col, rdir, cdir, emptySet(), emptySet());
  }

  private boolean isBlocked(int row, int col, int rdir, int cdir, Set<Point> pvwall, Set<Point> phwall) {
    if (0 <= row + rdir && 0 <= col + cdir && row + rdir < squares.length && col + cdir < squares[row + rdir].length) {
      JButton[][][] walls = rdir == 0 ? vwalls : hwalls;
      Set<Point> pwall = rdir == 0 ? pvwall : phwall;
      // rdir == 1,0 -> 0, -1 -> -1
      int rwall = row + (rdir - 1) / 2;
      int cwall = col + (cdir - 1) / 2;

      if (walls[rwall][cwall][0].getBackground().equals(WALL_COLOR)) {
        DEBUGF("Move (%d, %d) + (%d, %d) is blocked by an existing wall (%d, %d)", row, col, rdir, cdir, rwall, cwall);
        return true;
      } else if (pwall.contains(new Point(rwall, cwall))) {
        DEBUGF("Move (%d, %d) + (%d, %d) is blocked by a possible wall (%d, %d)", row, col, rdir, cdir, rwall, cwall);
        return true;
      }
    } else {
      DEBUGF("Location (%d, %d) is out of bounds", row + rdir, col + cdir);
      return true;
    }
    return false;
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
      result = !isBlocked(row, col, rdir, cdir);
    }
    DEBUGF("MOVE (%d,%d) + (%d,%d) is %sblocked", row, col, rdir, cdir, result ? "un" : "");
    return result;
  }

  private boolean checkPawn(int... args) {
    int row = args[0];
    int col = args[1];
    JButton square = squares[row][col];
    if (!square.getText().equals("")) {
      return false;
    }
    if (pawns[turn] != null) {
      int[] old = getSquare(pawns[turn]);
      int orow = old[0];
      int ocol = old[1];
      if (!square.getText().equals("")) {
        return false;
      }
      if (!checkWalls(orow, ocol, row - orow, col - ocol)) {
        return false;
      }
    }
    return true;
  }

  private boolean checkPath(int row, int col, int vrow, int vcol, Set<Point> seen, Set<Point> pvwall, Set<Point> phwall) {
    DEBUGF("Reached (%d, %d)", row, col);
    if ((vrow == -1 || vrow == row) && (vcol == -1 || vcol == col)) {
      return true;
    } else if (seen.contains(new Point(row, col))) {
      return false;
    }
    seen.add(new Point(row, col));
    for (int[] dir : DIRS) {
      int rdir = dir[0];
      int cdir = dir[1];
      if (!isBlocked(row, col, rdir, cdir, pvwall, phwall) && checkPath(row + rdir, col + cdir, vrow, vcol, seen, pvwall, phwall)) {
        return true;
      }
    }
    return false;
  }

  private boolean checkWall(int... args) {
    boolean vertical = args[0] == 0;
    int r = args[1];
    int c = args[2];
    int h = args[3];

    JButton[][][] walls = vertical ? vwalls : hwalls;
    int vp = vertical ? 1 : 0;
    int hp = 1 - vp;

    int rp = r + vp * (2 * h - 1);
    int cp = c + hp * (2 * h - 1);
    int[][] locations = { { r, c, h }, { r, c, 1 - h }, { rp, cp, h }, { rp, cp, 1 - h} };

    int cr = r + vp * (h - 1);
    int cc = c + hp * (h - 1);

    if (getWallsLeft() == 0) {
      return false;
    }

    for (int[] l : locations) {
      if (walls[l[0]][l[1]][l[2]].getBackground().equals(WALL_COLOR)) {
        DEBUGF("Blocked by wall in same direction");
        return false;
      }
    }

    if (cwalls[cr][cc].getBackground().equals(WALL_COLOR)) {
      DEBUGF("Blocked by perpendicular wall");
      return false;
    }

    Set<Point> pwall = new HashSet<Point>(asList(new Point(r,c), new Point(rp, cp)));
    Set<Point> pvwall =  vertical ? pwall : emptySet();
    Set<Point> phwall = !vertical ? pwall : emptySet();

    for (int i = 0; i < VICTORY.length; ++i) {
      int[] loc = getSquare(pawns[i]);
      if (!checkPath(loc[0], loc[1], VICTORY[i][0], VICTORY[i][1], new HashSet<Point>(), pvwall, phwall)) {
        DEBUGF("No valid path for player %d to victory", i);
        return false;
      }
    }

    return true;
  }

  private boolean setPawn(int... args) {
    int row = args[0];
    int col = args[1];
    JButton square = squares[row][col];

    if (!checkPawn(args)) {
      return false;
    }
    if (pawns[turn] != null) {
      pawns[turn].setText("");
    } else {
      /* This is hopefully during setup... */
    }
    pawns[turn] = square;
    pawns[turn].setText(NAMES[turn]);
    turn = 1 - turn;
    return true;
  }

  private boolean setWall(int... args) {
    boolean vertical = args[0] == 0;
    int r = args[1];
    int c = args[2];
    int h = args[3];

    JButton[][][] walls = vertical ? vwalls : hwalls;
    int vp = vertical ? 1 : 0;
    int hp = 1 - vp;

    int rp = r + vp * (2 * h - 1);
    int cp = c + hp * (2 * h - 1);
    int[][] locations = { { r, c, h }, { r, c, 1 - h }, { rp, cp, h }, { rp, cp, 1 - h} };

    int cr = r + vp * (h - 1);
    int cc = c + hp * (h - 1);

    if (!checkWall(args)) {
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

  private void init(String[] args)
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

    setPawn(parseSquare(args[2]));
    setPawn(parseSquare(args[3]));

    for (int i = 6; i < args.length; ++i) {
      setWall(parseWall(args[i]));
    }

    walls_left[0] = Integer.parseInt(args[4]);
    walls_left[1] = Integer.parseInt(args[5]);

    turn = args[1].equals("O") ? 0 : 1;

    setContentPane(pane);
  }

  private Quoridor() {
    this(DEFAULT_ARGS);
  }

  private Quoridor(String[] args)
  {
    setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
    init(args);
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

  private static String squareToString(int... square) {
    return 9 * square[0] + square[1] + 1 + "";
  }

  private static int[] parseSquare(String s) {
    int value = Integer.parseInt(s) - 1;
    return new int[] { value / 9, value % 9 };
  }

  static final char[] DIR = { '|', '-' };
  private static String wallToString(int... wall) {
    return String.format("%c%d%c", DIR[wall[0]], wall[1] + 1 + (1 - wall[0]) * (wall[3] - 1), wall[2] + wall[0] * (wall[3] - 1) + 'A');
  }

  private static int[] parseWall(String s) {
    return new int[] { s.charAt(0) == '|' ? 0 : 1, s.charAt(1) - '1', s.charAt(2) - 'A', 1 };
  }

  public void actionPerformed(ActionEvent event) {
    try {
      int[] square = getSquare(event.getSource());
      int[] wall = getWall(event.getSource());
      int old_turn = turn;
      if (square != null) {
        DEBUGF("Attempting to set pawn (%d, %d)", square[0], square[1]);
        if (setPawn(square)) {
          System.out.println(String.format("MOVE %s %s %d", PROTOVER_NAMES[old_turn], squareToString(square), walls_left[old_turn]));
        }
      } else if (wall != null) {
        DEBUGF("Attempting to place wall (%d, %d, %d, %d)", wall[0], wall[1], wall[2], wall[3]);
        if (setWall(wall)) {
          System.out.println(String.format("MOVE %s %s %d", PROTOVER_NAMES[old_turn], wallToString(wall), walls_left[old_turn]));
        }
      }  else {
        System.err.println("Unidentified action");
      }
    } catch (ArrayIndexOutOfBoundsException aex) {
      throw aex;
    }
  }

  private void getBoard() {
    System.out.print(String.format("GAMESTATE %s %s %s %d %d", PROTOVER_NAMES[turn], squareToString(getSquare(pawns[0])), squareToString(getSquare(pawns[1])), walls_left[0], walls_left[1]));
    for (int r = 0; r < hwalls.length; ++r) {
      for (int c = 0; c < hwalls[r].length; ++c) {
        if (hwalls[r][c][1].getBackground().equals(WALL_COLOR)) {
          System.out.print(" " + wallToString(1, r, c, 1));
          ++c;
        }
      }
    }
    for (int c = 0; c < vwalls[0].length; ++c) {
      for (int r = 0; r < vwalls.length; ++r) {
        if (vwalls[r][c][1].getBackground().equals(WALL_COLOR)) {
          System.out.print(" " + wallToString(0, r, c, 1));
          ++r;
        }
     
      }
    }
    System.out.println();
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
            if(walls_left != q.getWallsLeft() - 1 || !q.setWall(parseWall(cmd[2]))) {
              mverror();
            }
          } else {
            if(walls_left != q.getWallsLeft() || !q.setPawn(parseSquare(cmd[2]))) {
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
        } else if (cmd[0].equals("GAMESTATE")) {
          // Ignoring for now
        } else if (cmd[0].equals("WHITE")) {
          // Ignoring for now
        } else if (cmd[0].equals("BLACK")) {
          // Ignorning for now
	} else if (cmd[0].equals("PROTOVER")) {
          // We sent 0.1, nothing lower
        } else if (cmd[0].equals("FEATURE")) {
          System.out.println("REJECTED");
        } else if (cmd[0].equals("GETBOARD")) {
          q.getBoard();
        } else if (cmd[0].equals("SETBOARD")) {
          Quoridor temp = new Quoridor(cmd);
          q.setVisible(false);
          q.dispose();
          q = temp;
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
