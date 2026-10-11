#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <vector>

/* ======================================================================
 * 类名：MineSweeper —— 扫雷业务逻辑层
 *   · 本类只负责"扫雷规则"，不含任何 Qt 代码，可脱离界面单独编译测试；
 *   · 封装：数据成员一律 private，外部只能通过 public 成员函数访问；
 *   · 只读查询一律声明为 const，保证界面层拿到的状态是"只读"的。
 * ==================================================================== */
class MineSweeper
{
public:
    /* ------------------- 构造与初始化 ------------------- */
    // 构造函数：把全部数据成员初始化为"空棋盘"状态
    MineSweeper();

    // 初始化 rows × cols 棋盘，随机布置 mineCount 个雷
    // 参数非法（行列为负 / 雷数不合理）时返回 false，对象保持空棋盘状态
    bool init(int r0, int c0, int m0);

    /* ------------------- 玩家操作（改变棋盘状态） ------------------- */
    // 左键翻开格子：踩雷返回 false，安全翻开（含洪水填充）返回 true
    bool reveal(int r, int c);

    // 右键插旗 / 取消插旗：旗子总数不超过地雷数
    void toggleFlag(int r, int c);

    // 游戏失败后翻开全部地雷，供界面层展示
    void revealAllMines();

    /* ------------- 只读查询接口（全部 const，只读不写） ------------- */
    bool isWin() const;                     // 胜利判定：非雷格全部翻开
    bool isGameOver() const;                // 本局是否已结束（踩雷或通关）
    int  remainingMines() const;            // 剩余雷数 = 总雷数 - 已插旗数
    int  getRows() const;                   // 棋盘行数
    int  getCols() const;                   // 棋盘列数
    int  getMineCount() const;              // 地雷总数
    bool isMine(int r, int c) const;        // 该格是否埋雷
    bool isRevealed(int r, int c) const;    // 该格是否已翻开
    bool isFlagged(int r, int c) const;     // 该格是否已插旗
    int  adjacentMines(int r, int c) const; // 该格周围 8 格的雷数

private:
    /* ---------- 私有工具函数：只服务于类内部逻辑，外部不可见 ---------- */
    bool inBounds(int r, int c) const; // 边界检查（const：不改成员）
    void placeMines();                 // 随机布雷
    void computeAdjacent();            // 统计每格周围 8 格的雷数
    void bfsReveal(int r, int c);      // BFS 队列洪水填充，展开空白区域
    void moveMine(int r, int c);       // 首次点击踩雷时把该雷挪走
    int  countFlags() const;           // 统计当前已插旗数量

    /* ---------- 私有数据成员：外部不可直接读写，体现封装 ---------- */
    int  rows;       // 行数
    int  cols;       // 列数
    int  mineCount;  // 地雷总数
    bool gameOver;   // 本局是否已结束
    bool firstClick; // 是否处于"第一次点击"（首点必不踩雷）

    // 注意 "> >" 中间留空格：老式编译器不接受连写的 ">>"
    std::vector<std::vector<bool> > mine;     // 雷分布
    std::vector<std::vector<bool> > revealed; // 是否已翻开
    std::vector<std::vector<bool> > flagged;  // 是否已插旗
    std::vector<std::vector<int> >  adj;      // 每格周围 8 格的雷数
};

#endif // MINESWEEPER_H