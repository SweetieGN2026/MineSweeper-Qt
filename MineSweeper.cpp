#include "MineSweeper.h"
#include <queue>
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace std; // 全工程只允许这一处 using namespace std;

// 8 个相邻方向的偏移量：上、下、左、右、左上、右上、左下、右下
// static 限定为本编译单元可见，避免与其它源文件中的同名数组冲突
static const int DR[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
static const int DC[8] = {-1,  0,  1, -1, 1, -1, 0, 1};

// BFS 队列中保存的坐标点，写法与教材一致
struct Pos
{
    int r; // 行
    int c; // 列
};

/* 随机种子只在程序生命周期内播种一次。
 * 若每次 init 都 srand(time(NULL))，而 time() 精度为"秒"，
 * 同一秒内连点两次"新游戏"会得到完全相同的雷图。 */
static void seedRandOnce()
{
    static bool seeded = false; // 静态局部变量，只在第一次进入函数时初始化
    if (!seeded)
    {
        srand(static_cast<unsigned>(time(NULL)));
        seeded = true;
    }
}

/* ===== 构造函数：所有数据成员统一赋初值 ===== */
MineSweeper::MineSweeper()
{
    rows = 0;
    cols = 0;
    mineCount = 0;
    gameOver = false;
    firstClick = true;
}

/* 边界检查：私有工具函数，声明为 const，不修改任何成员 */
bool MineSweeper::inBounds(int r, int c) const
{
    return r >= 0 && r < rows && c >= 0 && c < cols;
}

/* 初始化棋盘：机试风格不抛异常，参数非法直接返回 false，调用方负责提示 */
bool MineSweeper::init(int r0, int c0, int m0)
{
    if (r0 <= 0 || c0 <= 0)
    {
        return false;
    }
    if (m0 <= 0 || m0 >= r0 * c0) // 雷数至少 1 个，且不能铺满整个棋盘
    {
        return false;
    }

    rows = r0;
    cols = c0;
    mineCount = m0;
    gameOver = false;
    firstClick = true;

    // 四个二维数组统一分配并清零：前三个为 false，adj 为 0
    mine.assign(rows, vector<bool>(cols, false));
    revealed.assign(rows, vector<bool>(cols, false));
    flagged.assign(rows, vector<bool>(cols, false));
    adj.assign(rows, vector<int>(cols, 0));

    placeMines();      // 随机布雷
    computeAdjacent(); // 统计周围雷数
    return true;
}

/* 随机布雷：反复随机取坐标，位置为空才落雷，直到放满 mineCount 个 */
void MineSweeper::placeMines()
{
    int placed;
    int r;
    int c;

    seedRandOnce(); // 只播种一次
    placed = 0;
    while (placed < mineCount)
    {
        r = rand() % rows;
        c = rand() % cols;
        if (!mine[r][c])
        {
            mine[r][c] = true;
            placed = placed + 1;
        }
    }
}

/* 计算每个格子周围 8 格的地雷数量，结果存入 adj */
void MineSweeper::computeAdjacent()
{
    int r;
    int c;
    int k;
    int cnt;
    int nr;
    int nc;
    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            cnt = 0;
            for (k = 0; k < 8; k++)
            {
                nr = r + DR[k];
                nc = c + DC[k];
                if (inBounds(nr, nc) && mine[nr][nc])
                {
                    cnt = cnt + 1;
                }
            }
            adj[r][c] = cnt;
        }
    }
}

/* 左键翻开格子：
 *   · 越界 / 本局已结束 / 已翻开 / 已插旗 —— 无效操作，返回 true（不改状态）
 *   · 踩到雷：若是本局第一次点击，先把雷挪走（首点必不踩雷）；
 *             否则置 gameOver 并翻开该雷，返回 false
 *   · 安全：BFS 洪水填充展开空白区域，并顺带完成通关判定 */
bool MineSweeper::reveal(int r, int c)
{
    if (!inBounds(r, c))
    {
        return true;
    }
    if (gameOver || revealed[r][c] || flagged[r][c])
    {
        return true;
    }

    if (mine[r][c])
    {
        if (firstClick)
        {
            moveMine(r, c); // 首点击踩雷：先把雷挪到别的空位
        }
        else
        {
            gameOver = true; // 踩雷，本局结束
            revealed[r][c] = true;
            return false;
        }
    }

    firstClick = false; // 无论首点是否踩雷，之后都不再享有保护
    bfsReveal(r, c);

    // 胜负状态由逻辑层统一维护：非雷格全部翻开即通关（界面层不再改写状态）
    if (isWin())
    {
        gameOver = true;
    }
    return true;
}

/* 首次点击踩到雷时调用：把 (r,c) 处的雷挪到棋盘上第一个空位。
 * 说明：扫描从 (0,0) 开始，雷通常被挪到左上角附近；
 *       由于 (r,c) 本身是雷，"非雷"条件已排除它，无需再判 (i==r && j==c)。 */
void MineSweeper::moveMine(int r, int c)
{
    int i;
    int j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            if (!mine[i][j])
            {
                mine[r][c] = false;
                mine[i][j] = true;
                computeAdjacent(); // 雷的位置变了，周围雷数必须重算
                return;
            }
        }
    }
}

/**
 * @brief BFS 洪水填充：把"周围没有雷"的格子依次入队展开，
 *        用队列 + 循环替代 DFS 递归，避免大面积棋盘递归栈溢出。
 * @param r 起始行
 * @param c 起始列
 */
void MineSweeper::bfsReveal(int r, int c)
{
    queue<Pos> q;
    Pos cur;
    Pos newPos;
    int k;
    int cr;
    int cc;
    int nr;
    int nc;

    if (!inBounds(r, c) || revealed[r][c] || flagged[r][c] || mine[r][c])
    {
        return;
    }

    revealed[r][c] = true;
    newPos.r = r;
    newPos.c = c;
    q.push(newPos);

    while (!q.empty())
    {
        cur = q.front();
        q.pop();
        cr = cur.r;
        cc = cur.c;

        // 当前格周围有雷，属于边界格：只翻开它本身，不再向外扩散
        if (adj[cr][cc] > 0)
        {
            continue;
        }

        for (k = 0; k < 8; k++)
        {
            nr = cr + DR[k];
            nc = cc + DC[k];
            // 关键：flagged 的格子必须跳过，否则会把玩家插的旗子自动翻开，
            // 造成"旗子消失但剩余雷数仍扣减、且再也无法取消"的不一致
            if (inBounds(nr, nc) && !revealed[nr][nc]
                && !flagged[nr][nc] && !mine[nr][nc])
            {
                revealed[nr][nc] = true;
                newPos.r = nr;
                newPos.c = nc;
                q.push(newPos);
            }
        }
    }
}

/* 右键插旗 / 取消插旗：旗子总数不超过地雷数，已翻开或已结束的格子不能插旗 */
void MineSweeper::toggleFlag(int r, int c)
{
    if (!inBounds(r, c))
    {
        return;
    }
    if (gameOver || revealed[r][c])
    {
        return;
    }

    // 情况一：该格已插旗 —— 取消插旗
    if (flagged[r][c])
    {
        flagged[r][c] = false;
        return;
    }

    // 情况二：该格未插旗 —— 旗子达到上限时禁止新增
    if (countFlags() >= mineCount)
    {
        return;
    }
    flagged[r][c] = true;
}

/* 统计当前已插旗数量：private + const，供 toggleFlag / remainingMines 复用 */
int MineSweeper::countFlags() const
{
    int i;
    int j;
    int flagCnt;
    flagCnt = 0;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            if (flagged[i][j])
            {
                flagCnt = flagCnt + 1;
            }
        }
    }
    return flagCnt;
}

/* 游戏失败后翻开全部地雷，供界面层展示 */
void MineSweeper::revealAllMines()
{
    int r;
    int c;
    if (!gameOver) // 保护：只有本局已结束才允许展示雷图
    {
        return;
    }
    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            if (mine[r][c])
            {
                revealed[r][c] = true;
            }
        }
    }
}

/* 胜利判定：所有非雷格子是否均已翻开（const：不修改任何成员） */
bool MineSweeper::isWin() const
{
    int r;
    int c;
    if (rows <= 0 || cols <= 0) // 空棋盘不算通关
    {
        return false;
    }
    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            if (!mine[r][c] && !revealed[r][c])
            {
                return false;
            }
        }
    }
    return true;
}

/* 剩余雷数 = 总雷数 - 已插旗数（插旗已限制上限，结果不会为负） */
int MineSweeper::remainingMines() const
{
    return mineCount - countFlags();
}

/* ===== 以下为 const 只读查询接口：界面层只读，不能借此改动棋盘 ===== */

bool MineSweeper::isGameOver() const
{
    return gameOver;
}

int MineSweeper::getRows() const
{
    return rows;
}

int MineSweeper::getCols() const
{
    return cols;
}

int MineSweeper::getMineCount() const
{
    return mineCount;
}

bool MineSweeper::isMine(int r, int c) const
{
    return mine[r][c];
}

bool MineSweeper::isRevealed(int r, int c) const
{
    return revealed[r][c];
}

bool MineSweeper::isFlagged(int r, int c) const
{
    return flagged[r][c];
}

int MineSweeper::adjacentMines(int r, int c) const
{
    return adj[r][c];
}